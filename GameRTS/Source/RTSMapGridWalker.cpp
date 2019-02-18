#include "RTSMapGridWalker.h"

#include "RTSTiledMap.h"
#include "RTSPathNode.h"

RTSMapGridWalker::RTSMapGridWalker(RTSTiledMap * pTiledMap) :
  m_pTiledMap(pTiledMap), m_pShape(nullptr), m_pPathShape(nullptr),
  m_position(Vector2I::ZERO), m_targetPos(Vector2I::ZERO){

  Vector2I mapSize = pTiledMap->getMapSize();
  m_closedList.resize(mapSize.x * mapSize.y);
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
RTSMapGridWalker::ResetClosedList(){
  for (int32 i = 0; i < m_closedList.size(); ++i) {
    if (nullptr != m_closedList[i]) {
      ge_delete(m_closedList[i]);
      m_closedList[i] = nullptr;
    }
  }
}

const Vector2I RTSMapGridWalker::s_nextDirection4[] = {
  Vector2I( 0,  1),
  Vector2I( 0, -1),
  Vector2I( 1,  0),
  Vector2I(-1,  0),
  Vector2I( 1,  1),
  Vector2I( 1, -1),
  Vector2I(-1,  1),
  Vector2I(-1, -1),
};

const Vector2I RTSMapGridWalker::s_nextDirection8[] = {
  Vector2I( 0,  1),
  Vector2I( 0, -1),
  Vector2I( 1,  0),
  Vector2I( 1,  1),
  Vector2I( 1, -1),
  Vector2I(-1,  0),
  Vector2I(-1,  1),
  Vector2I(-1, -1),
};
