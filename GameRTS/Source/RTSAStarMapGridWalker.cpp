#include "RTSAStarMapGridWalker.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "RTSPathNode.h"
#include "RTSTiledMap.h"

RTSAStarMapGridWalker::
RTSAStarMapGridWalker(RTSTiledMap * m_pTiledMap) :
  RTSMapGridWalker(m_pTiledMap), m_pTargetShape(nullptr) {

}

RTSAStarMapGridWalker::~RTSAStarMapGridWalker() {
  if (nullptr != m_pTargetShape) {
    ge_delete(m_pTargetShape);
  }
}

bool
RTSAStarMapGridWalker::init() {
  auto shape = ge_new<sf::RectangleShape>(sf::Vector2f(10.f, 10.f));
  shape->setFillColor(sf::Color::Red);
  shape->setOrigin(5, 5);

  m_pShape = shape;

  auto shapeT = ge_new <sf::RectangleShape>(sf::Vector2f(10.f, 10.f));
  shapeT->setFillColor(sf::Color::Green);
  shapeT->setOrigin(5, 5);

  m_pShape = shape;
  m_pTargetShape = shapeT;

  return false;
}

void
RTSAStarMapGridWalker::render(sf::RenderTarget * target) {
  Vector2I screenPos;
  GetTiledMap()->getMapToScreenCoords(GetPosition().x, GetPosition().y,
                                      screenPos.x, screenPos.y);

  m_pShape->setPosition(static_cast<float>(screenPos.x + HALFTILESIZE_X),
                        static_cast<float> (screenPos.y + HALFTILESIZE_Y));

  GetTiledMap()->getMapToScreenCoords(GetTargetPos().x, GetTargetPos().y,
                                      screenPos.x, screenPos.y);

  m_pTargetShape->setPosition(static_cast<float>(screenPos.x + HALFTILESIZE_X),
                              static_cast<float> (screenPos.y + HALFTILESIZE_Y));

  target->draw(*m_pShape);
  target->draw(*m_pTargetShape);

  if (m_CurrentState != GRID_WALKER_STATE::kIdle) {
    //draw nodes in closed list
    for (auto it = m_fastClosedList.begin(); it != m_fastClosedList.end(); ++it) {
      (*it)->render(target, *GetTiledMap());
    }

    if (nullptr != m_pPathShape) {
      for (SIZE_T i = 0; i < m_path.size(); ++i) {
        GetTiledMap()->getMapToScreenCoords(m_path[i].x, m_path[i].y, 
                                            screenPos.x, screenPos.y);

        (*m_pPathShape)[i].position = sf::Vector2f(
          static_cast<float>(screenPos.x + HALFTILESIZE_X),
          static_cast<float>(screenPos.y + HALFTILESIZE_Y)
        );

        (*m_pPathShape)[i].color = sf::Color::White;
      }

      target->draw(*m_pPathShape);
    }
  }
}

void
RTSAStarMapGridWalker::StartSeach(bool stepMode) {
  Vector2I mapSize = GetTiledMap()->getMapSize();
  Vector2I s = GetPosition();
  int32 sIndex = (s.y * mapSize.x) + s.x;

  Reset();

  m_CurrentState = GRID_WALKER_STATE::kSearching;

  m_openListAstar.clear();

  // enqueue source
  m_openListAstar.push_back({ s, 0 }); //zero cost cause I'm already here

  // mark source as visited.
  m_closedList[sIndex] = ge_new<RTSPathNode>(GetPosition(), Vector2I::ZERO);
  m_fastClosedList.push_front(m_closedList[sIndex]);

  if (!stepMode) { //when not in stepMode run entire search all at once
    while (m_CurrentState == GRID_WALKER_STATE::kSearching) {
      StepSearch();
    }
    while (m_CurrentState == GRID_WALKER_STATE::kBacktracking) {
      StepBacktrack();
    }
  }
}

/*
TODO: AStar should stop if it finds first path, if it goes a certain amount of steps,
or even searching all the map
*/

void
RTSAStarMapGridWalker::StepSearch() {
  GE_ASSERT(m_CurrentState == GRID_WALKER_STATE::kSearching);
  if (m_openListAstar.empty()) {
    m_CurrentState = GRID_WALKER_STATE::kBacktracking;
  }

  Vector2I mapSize = GetTiledMap()->getMapSize();
  Vector2I target = GetTargetPos();

  //Removing that vertex from queue, whose neighbors will be visited now
  Vector2I v = m_openListAstar.front().v;
  int8 vCost = m_openListAstar.front().cost;

  m_openListAstar.pop_front();

  //required variables
  Vector2I w;
  int8 wCost;
  int32 wIndex;
  uint32 distance;

  //processing all the neighbors of v
  for (SIZE_T i = 0; i < s_nextDirection.size(); ++i) {
    w = v + s_nextDirection[i];
    wIndex = (w.y*mapSize.x) + w.x;

    //if neighbor is target then a path has been found
    if (GetTargetPos() == w) {
      wCost = vCost + GetTiledMap()->getCost(w.x, w.y);

      //mark w as visited.
      m_closedList[wIndex] = ge_new<RTSPathNode>(w, s_nextDirection[i], wCost);
      m_fastClosedList.push_front(m_closedList[wIndex]);

      m_CurrentState = GRID_WALKER_STATE::kBacktracking;
      
      return;
    }

    //make sure it's a valid neighbor
    if (w.x >= 0 && w.x < mapSize.x && w.y >= 0 && w.y < mapSize.y) {
      //make sure it's not an obstacle
      if (TERRAIN_TYPE::kObstacle != GetTiledMap()->getType(w.x, w.y)) {
        // if it isn't marked as visited just push it
        wCost = vCost + GetTiledMap()->getCost(w.x, w.y);
        distance = v.manhattanDist(target);

        if (nullptr == m_closedList[wIndex]) {
          PriorityPushBack(w, wCost); // enqueue w

          //mark w as visited.
          m_closedList[wIndex] =
                ge_new<RTSPathNode>(w, s_nextDirection[i], wCost);

          m_fastClosedList.push_front(m_closedList[wIndex]);
        }
        else if (wCost < m_closedList[wIndex]->GetCost()) {
          PriorityPushBack(w, wCost); // enqueue w again

          //update lesser cost for node
          m_closedList[wIndex]->SetNewDirAndCost(s_nextDirection[i], wCost);
        }
      }
    }
  }
}

// TODO: Move this to somewhere else and also bestFirstSearch's Priority Push Back
void 
RTSAStarMapGridWalker::PriorityPushBack(Vector2I v, int8 vCost) {
  Vector2I target = GetTargetPos();
  uint32 distance = v.manhattanDist(target);

  for (auto it = m_openListAstar.begin(); it != m_openListAstar.end(); ++it) {
    if (it->cost + it->distance > vCost + distance) {
      m_openListAstar.insert(it, { v, vCost, distance });
      return;
    }
  }

  m_openListAstar.push_back({v, vCost, distance });
}
