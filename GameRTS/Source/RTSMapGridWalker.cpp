#include "RTSMapGridWalker.h"

#include "RTSTiledMap.h"
#include "RTSPathNode.h"

RTSMapGridWalker::RTSMapGridWalker(RTSTiledMap * pTiledMap) :
  m_pTiledMap(pTiledMap), 
  m_pShape(nullptr),
  m_pTargetShape(nullptr),
  m_pPathShape(nullptr),
  m_position(Vector2I::ZERO), 
  m_targetPos(Vector2I::ZERO) {
  
  Vector2I mapSize = pTiledMap->getMapSize();
}

RTSMapGridWalker::~RTSMapGridWalker() {
  if (nullptr != m_pShape) {
    ge_delete(m_pShape);
  }
  if (nullptr != m_pTargetShape) {
    ge_delete(m_pTargetShape);
  }
  if (nullptr != m_pPathShape) {
    ge_delete(m_pPathShape);
  }
  if (m_closedList) {
    for (auto it = m_fastClosedList.begin(); it != m_fastClosedList.end(); ++it) {
      Vector2I pos = (*it)->GetPosition();
      int32 index = (pos.y * GetTiledMap()->getMapSize().x) + pos.x;
      ge_delete((*m_closedList)[index]);
    }
    m_closedList.reset();
  }
}



bool
RTSMapGridWalker::init() {

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
RTSMapGridWalker::render(sf::RenderTarget * target) {

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
    RenderClosedList(target);

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
RTSMapGridWalker::StepBacktrack() {
  GE_ASSERT(m_CurrentState == GRID_WALKER_STATE::kBacktracking);

  Vector2I v = GetTargetPos();
  Vector2I mapSize = GetTiledMap()->getMapSize();

  m_path.push_back(GetTargetPos());

  while (v != GetPosition()) {
    v -= (*m_closedList)[(v.y*mapSize.x) + v.x]->GetDirection();
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
    int32 index = (pos.y * mapSize.x) + pos.x;
    ge_delete((*m_closedList)[index]);
    (*m_closedList)[index] = nullptr;
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
