#include "RTSMapGridWalker.h"

#include "RTSTiledMap.h"
#include "RTSPathNode.h"

RTSMapGridWalker::RTSMapGridWalker(RTSTiledMap * pTiledMap) :
  m_pTiledMap(pTiledMap), 
  m_pShape(nullptr), 
  m_pPathShape(nullptr),
  m_position(Vector2I::ZERO), 
  m_targetPos(Vector2I::ZERO) {
  
  Vector2I mapSize = pTiledMap->getMapSize();
  m_closedList.resize(mapSize.x * mapSize.y, nullptr);
}

RTSMapGridWalker::~RTSMapGridWalker() {
  if (nullptr != m_pShape) {
    ge_delete(m_pShape);
  }
  if (nullptr != m_pPathShape) {
    ge_delete(m_pPathShape);
  }
}

void 
RTSMapGridWalker::StepBacktrack() {
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

void 
RTSMapGridWalker::ResetClosedList(){
  Vector2I mapSize = GetTiledMap()->getMapSize();

  for (auto it = m_fastClosedList.begin(); it != m_fastClosedList.end(); ++it) {
    Vector2I pos = (*it)->GetPosition();
    int32 index = (pos.y*mapSize.x) + pos.x;
    ge_delete(*it);
    m_closedList[index] = nullptr;
  }

  m_fastClosedList.clear();
}

// TODO: split diagonals from straight paths
const Vector<Vector2I> RTSMapGridWalker::s_nextDirection = {
  Vector2I( 0,  1),
  Vector2I( 0, -1),
  Vector2I( 1,  0),
  Vector2I(-1,  0),
  Vector2I( 1,  1),
  Vector2I( 1, -1),
  Vector2I(-1,  1),
  Vector2I(-1, -1),
};
