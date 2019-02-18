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
    for each (auto node in m_closedList) {
      if (nullptr != node) {
        node->render(target, *GetTiledMap());
      }
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
  Vector2I s = GetPosition();
  Vector2I mapSize = GetTiledMap()->getMapSize();

  Reset();
  m_foundPath = false;

  m_CurrentState = GRID_WALKER_STATE::kSearching;

  m_openListWithCosts.clear();

  // enqueue source
  m_openListWithCosts.push_back({ s, 0 }); //zero cost cause I'm already here

  // mark source as visited.
  m_closedList[(s.y * mapSize.x) + s.x] = ge_new<RTSPathNode>(GetPosition(),
                                                              Vector2I::ZERO);

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
  Vector2I mapSize = GetTiledMap()->getMapSize();

  if (m_foundPath) {
    //m_openListWithCosts.front().cost > m_closedList[GetTargetPos()]
    Vector2I t = GetTargetPos();
    int8 targetCost = m_closedList[(t.y*mapSize.x) + t.x]->GetCost();
    if (targetCost <= m_openListWithCosts.front().cost) {
      m_CurrentState = GRID_WALKER_STATE::kBacktracking;
    }
  }
  else if (m_openListWithCosts.empty()) {
    m_CurrentState = GRID_WALKER_STATE::kDisplaying;
    return;
  }

  //Removing that vertex from queue, whose neighbors will be visited now
  Vector2I v = m_openListWithCosts.front().v;
  int8 vCost = m_openListWithCosts.front().cost;

  m_openListWithCosts.pop_front();

  //processing all the neighbors of v
  Vector2I w;
  int8 wCost;

  for (SIZE_T i = 0; i < ge_size(s_nextDirection4); ++i) {
    w = v + s_nextDirection4[i];
    int32 wIndex = (w.y*mapSize.x) + w.x;

    //if neighbor is target then a path has been found
    if (GetTargetPos() == w) {
      if (!m_foundPath) {
        m_foundPath = true;

        wCost = vCost + GetTiledMap()->getCost(w.x, w.y);

        //mark w as visited.
        m_closedList[(w.y*mapSize.x) + w.x] 
          = ge_new<RTSPathNode>(w, s_nextDirection4[i], wCost);
      }
      else if (wCost < m_closedList[wIndex]->GetCost()) {
        m_closedList[wIndex]->SetNewDirAndCost(s_nextDirection4[i], wCost);
      }
      
      return;
    }

    //make sure it's a valid neighbor
    if (w.x >= 0 && w.x < mapSize.x && w.y >= 0 && w.y < mapSize.y) {
      //make sure it's not an obstacle
      if (TERRAIN_TYPE::kObstacle != GetTiledMap()->getType(w.x, w.y)) {
        // if it isn't marked as visited just push it
        wCost = vCost + GetTiledMap()->getCost(w.x, w.y);
        if (nullptr == m_closedList[wIndex]) {
          PriorityPushBack(w, wCost); // enqueue w

          //mark w as visited.
          m_closedList[wIndex] =
            ge_new<RTSPathNode>(w, s_nextDirection4[i], wCost);
        }
        else if (wCost < m_closedList[wIndex]->GetCost()) {
          PriorityPushBack(w, wCost); // enqueue w again

          //update lesser cost for node
          m_closedList[wIndex]->SetNewDirAndCost(s_nextDirection4[i], wCost);
        }
      }
    }
  }
}

void
RTSAStarMapGridWalker::StepBacktrack() {
  GE_ASSERT(m_CurrentState == GRID_WALKER_STATE::kBacktracking);

  Vector2I v = GetTargetPos();
  Vector2I mapSize = GetTiledMap()->getMapSize();

  m_path.push_back(GetTargetPos());

  while (v != GetPosition()) {
    v -= m_closedList[(v.y*mapSize.x) + v.x]->GetDirection();
    m_path.push_back(v);
  }

  m_pPathShape = ge_new<sf::VertexArray>(sf::LineStrip, m_path.size());

  m_CurrentState = GRID_WALKER_STATE::kDisplaying;
}

// TODO: Move this to somewhere else and also bestFirstSearch's Priority Push Back
void 
RTSAStarMapGridWalker::PriorityPushBack(Vector2I v, int8 vCost) {
  Vector2I target = GetTargetPos();

  for (auto it = m_openListWithCosts.begin(); it != m_openListWithCosts.end(); ++it) {
    if (it->cost > vCost) {
      m_openListWithCosts.insert(it, { v, vCost });
      return;
    }
  }

  m_openListWithCosts.push_back({v, vCost});
}
