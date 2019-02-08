#include "RTSMapGridWalker.h"

#include "RTSTiledMap.h"
#include "RTSPathNode.h"

RTSMapGridWalker::RTSMapGridWalker(RTSTiledMap * pTiledMap) :
  m_pTiledMap(pTiledMap), m_pShape(nullptr) {
  Vector2I mapSize = pTiledMap->getMapSize();
  m_path.resize(mapSize.x * mapSize.y);
}

RTSMapGridWalker::~RTSMapGridWalker() {
  if (nullptr != m_pShape) {
    ge_delete(m_pShape);
  }
}

void 
RTSMapGridWalker::ResetPath(){
  for (int32 i = 0; i < m_path.size(); ++i) {
    if (nullptr != m_path[i]) {
      ge_delete(m_path[i]);
      m_path[i] = nullptr;
    }
  }
}

const Vector2I RTSMapGridWalker::s_neighborOffsets[] = {
  Vector2I(0, 1),
  Vector2I(0,-1),
  Vector2I(1, 0),
  Vector2I(-1, 0),
};
