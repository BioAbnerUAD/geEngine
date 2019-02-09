#include "RTSDijkstraMapGridWalker.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "RTSPathNode.h"
#include "RTSTiledMap.h"

RTSDijkstraMapGridWalker::
RTSDijkstraMapGridWalker(RTSTiledMap * m_pTiledMap) :
  RTSMapGridWalker(m_pTiledMap), m_pTargetShape(nullptr) {

}

RTSDijkstraMapGridWalker::~RTSDijkstraMapGridWalker() {
  if (nullptr != m_pTargetShape) {
    ge_delete(m_pTargetShape);
  }
}

bool
RTSDijkstraMapGridWalker::init()
{
  auto shape = new sf::RectangleShape(sf::Vector2f(10.f, 10.f));
  shape->setFillColor(sf::Color::Red);
  shape->setOrigin(5, 5);

  m_pShape = shape;

  auto shapeT = new sf::RectangleShape(sf::Vector2f(10.f, 10.f));
  shapeT->setFillColor(sf::Color::Green);
  shapeT->setOrigin(5, 5);

  m_pShape = shape;
  m_pTargetShape = shapeT;

  return false;
}

void
RTSDijkstraMapGridWalker::render(sf::RenderTarget * target)
{
  Vector2I screenPos;
  GetTiledMap()->getMapToScreenCoords(GetPosition().x, GetPosition().y,
    screenPos.x, screenPos.y);

  m_pShape->setPosition(static_cast<float>(screenPos.x + TILESIZE_X / 2),
    static_cast<float> (screenPos.y + TILESIZE_Y / 2));

  GetTiledMap()->getMapToScreenCoords(GetTargetPos().x, GetTargetPos().y,
    screenPos.x, screenPos.y);

  m_pTargetShape->setPosition(static_cast<float>(screenPos.x + TILESIZE_X / 2),
    static_cast<float> (screenPos.y + TILESIZE_Y / 2));

  target->draw(*m_pShape);
  target->draw(*m_pTargetShape);

  if (m_CurrentState != GRID_WALKER_STATE::kIdle) {
    for each (auto node in m_closedList) {
      if (nullptr != node) {
        node->render(target, *GetTiledMap());
      }
    }

  }
}

void
RTSDijkstraMapGridWalker::StartSeach(bool stepMode) {
  Vector2I s = GetPosition();
  Vector2I mapSize = GetTiledMap()->getMapSize();

  Reset();

  m_CurrentState = GRID_WALKER_STATE::kSearching;

  m_openList.clear();
  m_openList.push_back(s); // enqueue source

  // mark source as visited.
  m_closedList[(s.y*mapSize.x) + s.x] = new RTSPathNode(GetPosition(),
                                                        Vector2I::ZERO); 

  if (!stepMode) { //when not in stepMode run entire search all at once
    while (!m_openList.empty() &&
      m_CurrentState == GRID_WALKER_STATE::kSearching) {
      StepSearch();
    }
    while (m_CurrentState == GRID_WALKER_STATE::kBacktracking) {
      StepBacktrack();
    }
  }
}

void
RTSDijkstraMapGridWalker::StepSearch() {
  GE_ASSERT(m_CurrentState == GRID_WALKER_STATE::kSearching);

  //TODO: this condition actually happens when no path is possible so mark it as such
  if (m_openList.empty()) {
    m_CurrentState = GRID_WALKER_STATE::kDisplaying;
    return;
  }

  Vector2I mapSize = GetTiledMap()->getMapSize();

  //Removing that vertex from queue, whose neighbors will be visited now
  Vector2I v = m_openList.front();
  m_openList.pop_front();

  //processing all the neighbors of v
  Vector2I w;

  for (SIZE_T i = 0; i < ge_size(s_nextDirection4); ++i) {
    w = v + s_nextDirection4[i];
    //if neighbor is target then a path has been found
    if (GetTargetPos() == w) {
      //TODO: m_foundPath = true;
      m_CurrentState = GRID_WALKER_STATE::kBacktracking;
      break;
    }

    //make sure it's a valid neighbor
    if (w.x >= 0 && w.x < mapSize.x && w.y >= 0 && w.y < mapSize.y) {
      //make sure it's not an obstacle and it isn't marked as visited
      if (TERRAIN_TYPE::kObstacle != GetTiledMap()->getType(w.x, w.y) &&
        nullptr == m_closedList[(w.y*mapSize.x) + w.x]) {

        m_openList.push_back(w); //enqueue w

        m_closedList[(w.y*mapSize.x) + w.x]
          = new RTSPathNode(w, s_nextDirection4[i]); //mark w as visited.

      }
    }
  }
}