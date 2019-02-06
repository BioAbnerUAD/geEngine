#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "RTSBreadthFirstSearchMapGridWalker.h"
#include "RTSTiledMap.h"

const Vector2I RTSBreadthFirstSearchMapGridWalker::s_neighborOffsets[] = {
  Vector2I(0, 1),
  Vector2I(0,-1),
  Vector2I(1, 0),
  Vector2I(-1, 0),
};

RTSBreadthFirstSearchMapGridWalker::
RTSBreadthFirstSearchMapGridWalker(RTSTiledMap * m_pTiledMap) :
  RTSMapGridWalker(m_pTiledMap), m_pTargetShape(nullptr) {

}

RTSBreadthFirstSearchMapGridWalker::~RTSBreadthFirstSearchMapGridWalker() {
  if (nullptr != m_pTargetShape)
  {
    ge_delete(m_pTargetShape);
  }
}

bool 
RTSBreadthFirstSearchMapGridWalker::init()
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
RTSBreadthFirstSearchMapGridWalker::render(sf::RenderTarget * target)
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

  if (IsSearching()) {
    for each (auto node in m_path) {
      if (nullptr != node) {
        node->render(target, *GetTiledMap());
      }
    }

  }
}

void
RTSBreadthFirstSearchMapGridWalker::StartSeach(bool stepMode) {
  Vector2I s = GetPosition();
  Vector2I mapSize = GetTiledMap()->getMapSize();

  searching = true;
  foundPath = false;

  m_queue.clear();
  m_queue.push_back(s); // enqueue source

  m_path.clear();
  m_path.resize(mapSize.x * mapSize.y, nullptr);
  m_path[(s.y*mapSize.x) + s.x] = new RTSPathNode(GetPosition(),
                                                  Vector2I::ZERO); // mark source as visited.
  
  if (!stepMode) { //when not in stepMode run entire search all at once
    while (!m_queue.empty() && !foundPath) {
      StepSearch();
    }
  }
}

void
RTSBreadthFirstSearchMapGridWalker::StepSearch() {
  GE_ASSERT(searching || !foundPath);

  //TODO: this condition actually happens when no path is possible so mark it as such
  if (m_queue.empty()) {
    foundPath = true;
    return;
  }

  Vector2I mapSize = GetTiledMap()->getMapSize();
  
  //Removing that vertex from queue, whose neighbors will be visited now
  Vector2I v = m_queue.front();
  m_queue.pop_front();

  //processing all the neighbors of v
  for (SIZE_T i = 0; i < ge_size(s_neighborOffsets); ++i) {
    Vector2I w = v + s_neighborOffsets[i];
    //if neighbor is target then a path has been found
    if (GetTargetPos() == w) {
      foundPath = true;
      break;
    }

    //make sure it's a valid neighbor
    if (w.x >= 0 && w.x <= mapSize.x && w.y >= 0 && w.y <= mapSize.y) {
      //make sure it's not an obstacle and it isn't marked as visited
      if (TERRAIN_TYPE::kObstacle != GetTiledMap()->getType(w.x, w.y) &&
        nullptr == m_path[(w.y*mapSize.x) + w.x]) {
        
        m_queue.push_back(w); //enqueue w

        m_path[(w.y*mapSize.x) + w.x] 
          = new RTSPathNode(w, s_neighborOffsets[i]); //mark w as visited.
                                                         
      }
    }
  }
}