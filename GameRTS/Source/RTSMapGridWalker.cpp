#include "RTSMapGridWalker.h"

#include "RTSTiledMap.h"
#include "RTSPathNode.h"

RTSMapGridWalker::RTSMapGridWalker(RTSTiledMap * pTiledMap) :
  m_pTiledMap(pTiledMap),
  m_pPathShape(nullptr),
  m_position(Vector2I::ZERO), 
  m_targetPos(Vector2I::ZERO) {
}

RTSMapGridWalker::~RTSMapGridWalker() {
  if (m_pPathShape) {
    ge_delete(m_pPathShape);
  }
  if (m_closedList) {
    for (auto it = m_fastClosedList.begin(); it != m_fastClosedList.end(); ++it) {
      Vector2I pos = (*it)->GetPosition();
      int32 index = (pos.y * GetTiledMap()->getMapSize().x) + pos.x;
      if ((*m_closedList)[index]) {
        ge_delete((*m_closedList)[index]);
      }
    }
    m_closedList.reset();
  }
}

bool
RTSMapGridWalker::init() {
  Reset();
  m_pPathShape = ge_new<sf::VertexArray>(sf::LineStrip);
  return true; // TODO: This might need some error checking here
}

void
RTSMapGridWalker::render(sf::RenderTarget * target) {

  if (GameOptions::s_drawGridWalkerGizmos && 
      m_CurrentState != GRID_WALKER_STATE::kIdle) {

    Vector2I screenPos;

    RenderClosedList(target);

    if (m_pPathOutput) {

      m_pPathShape->resize(m_pPathOutput->size());

      for (SIZE_T i = 0; i < m_pPathOutput->size(); ++i) {
        GetTiledMap()->getMapToScreenCoords((*m_pPathOutput)[i].x, 
                                            (*m_pPathOutput)[i].y,
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

Vector<Vector2I>
RTSMapGridWalker::Backtrack() {
  Vector<Vector2I> path(0);

  Vector2I v = m_targetPos;
  Vector2I mapSize = GetTiledMap()->getMapSize();

  path.push_back(m_targetPos);

  while (v != m_position) {
    v -= (*m_closedList)[(v.y*mapSize.x) + v.x]->GetDirection();
    path.push_back(v);
  }

  m_CurrentState = GRID_WALKER_STATE::kDisplaying;

  return path;
}

void 
RTSMapGridWalker::ResetClosedList(){
  Vector2I mapSize = GetTiledMap()->getMapSize();

  for (auto it = m_fastClosedList.begin(); it != m_fastClosedList.end(); ++it) {
    Vector2I pos = (*it)->GetPosition();
    int32 index = (pos.y * mapSize.x) + pos.x;
    if ((*m_closedList)[index]) {
      ge_delete((*m_closedList)[index]);
      (*m_closedList)[index] = nullptr;
    }
  }

  m_fastClosedList.clear();
}

void 
RTSMapGridWalker::RenderClosedList(sf::RenderTarget* target) {
  Vector2I mapSize = GetTiledMap()->getMapSize();

  int32 tileIniX = 0, tileIniY = 0;
  int32 tileFinX = 0, tileFinY = 0;

  uint32 scrStartx, scrStarty;
  uint32 scrEndx, scrEndy;

  GetTiledMap()->getStart(scrStartx, scrStarty);
  GetTiledMap()->getEnd(scrEndx, scrEndy);

#ifdef MAP_IS_ISOMETRIC
  int32 trashCoord = 0;
  GetTiledMap()->getScreenToMapCoords(scrStartx, scrStarty, tileIniX, trashCoord);
  GetTiledMap()->getScreenToMapCoords(scrEndx, scrEndy, tileFinX, trashCoord);

  GetTiledMap()->getScreenToMapCoords(scrEndx, scrStarty, trashCoord, tileIniY);
  GetTiledMap()->getScreenToMapCoords(scrStartx, scrEndy, trashCoord, tileFinY);
#else
  GetTiledMap()->getScreenToMapCoords(scrStartx, scrStarty, tileIniX, tileIniY);
  GetTiledMap()->getScreenToMapCoords(scrEndx, scrEndy, tileFinX, tileFinY);
#endif

  int32 i;
  for (int32 iterX = tileIniX; iterX <= tileFinX; ++iterX) {
    for (int32 iterY = tileIniY; iterY <= tileFinY; ++iterY) {
      i = (iterY * mapSize.x) + iterX;
      if (nullptr != (*m_closedList)[i]) {
        (*m_closedList)[i]->render(target, *GetTiledMap());
      }
    }
  }
}

void 
RTSMapGridWalker::AddToClosedList(const Vector2I & position, 
                                  const Vector2I & direction) {

  Vector2I mapSize = GetTiledMap()->getMapSize();
  int32 wIndex = (position.y * mapSize.x) + position.x;

  (*m_closedList)[wIndex] = ge_new<RTSPathNode>(position, direction);
  m_fastClosedList.push_front((*m_closedList)[wIndex]);
}

void 
RTSMapGridWalker::AddToClosedList(const Vector2I & position, 
                                  const Vector2I & direction, 
                                  float cost) {

  Vector2I mapSize = GetTiledMap()->getMapSize();
  int32 wIndex = (position.y * mapSize.x) + position.x;

  (*m_closedList)[wIndex] = ge_new<RTSPathNode>(position, direction, cost);
  m_fastClosedList.push_front((*m_closedList)[wIndex]);
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
