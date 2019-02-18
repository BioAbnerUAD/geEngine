#include "RTSBreadthFirstSearchMapGridWalker.h"

#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "RTSPathNode.h"
#include "RTSTiledMap.h"

RTSBreadthFirstSearchMapGridWalker::
RTSBreadthFirstSearchMapGridWalker(RTSTiledMap * pTiledMap) :
  RTSMapGridWalker(pTiledMap), m_pTargetShape(nullptr) {

}

RTSBreadthFirstSearchMapGridWalker::~RTSBreadthFirstSearchMapGridWalker() {
  if (nullptr != m_pTargetShape) {
    ge_delete(m_pTargetShape);
  }
}

bool
RTSBreadthFirstSearchMapGridWalker::init()
{
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
RTSBreadthFirstSearchMapGridWalker::render(sf::RenderTarget * target)
{
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
RTSBreadthFirstSearchMapGridWalker::StartSeach(bool stepMode) {
  Vector2I s = GetPosition();
  Vector2I mapSize = GetTiledMap()->getMapSize();

  Reset();

  m_CurrentState = GRID_WALKER_STATE::kSearching;

  m_openList.clear();
  m_openList.push_back(s); // enqueue source

  // mark source as visited.
  m_closedList[(s.y * mapSize.x) + s.x] = ge_new<RTSPathNode>(GetPosition(),
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
RTSBreadthFirstSearchMapGridWalker::StepSearch() {
  GE_ASSERT(m_CurrentState == GRID_WALKER_STATE::kSearching);

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
      
      //mark w as visited.
      m_closedList[(w.y*mapSize.x) + w.x] = ge_new<RTSPathNode>(w, s_nextDirection4[i]);
      
      m_CurrentState = GRID_WALKER_STATE::kBacktracking;
      return;
    }

    //make sure it's a valid neighbor
    if (w.x >= 0 && w.x < mapSize.x && w.y >= 0 && w.y < mapSize.y) {
      //make sure it's not an obstacle and it isn't marked as visited
      if (TERRAIN_TYPE::kObstacle != GetTiledMap()->getType(w.x, w.y) &&
        nullptr == m_closedList[(w.y*mapSize.x) + w.x]) {

        m_openList.push_back(w); // enqueue w

        //mark w as visited.
        m_closedList[(w.y*mapSize.x) + w.x] = 
          ge_new<RTSPathNode>(w, s_nextDirection4[i]);
      }
    }
  }
}

void
RTSBreadthFirstSearchMapGridWalker::StepBacktrack() {
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
