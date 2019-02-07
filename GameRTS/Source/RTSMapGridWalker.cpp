#include "RTSMapGridWalker.h"
#include "RTSTiledMap.h"

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

RTSMapGridWalker::RTSPathNode::RTSPathNode(const Vector2I & position,
                                           const Vector2I & direction) :
  m_position(position), m_direction(direction) {

  m_pShape = new sf::RectangleShape(sf::Vector2f(5.f, 5.f));
  m_pShape->setFillColor(sf::Color::Red);
  m_pShape->setOrigin(2.5f, 2.5f);

  if (Vector2I::ZERO != direction) {
    m_pDirShape = new sf::RectangleShape(sf::Vector2f(10.f, 1.f));
    m_pDirShape->setFillColor(sf::Color::White);

    Radian dirAngle = Math::atan2(static_cast<float>(direction.y),
                                  static_cast<float>(direction.x));
    m_pDirShape->setRotation(dirAngle.valueDegrees());
  }
  else {
    m_pDirShape = nullptr;
  }
}

RTSMapGridWalker::RTSPathNode::~RTSPathNode() {
  if (nullptr != m_pShape) {
    ge_delete(m_pShape);
  }
  if (nullptr != m_pDirShape) {
    ge_delete(m_pDirShape);
  }
}

void RTSMapGridWalker::RTSPathNode::render(sf::RenderTarget* target,
                                           const RTSTiledMap& tileMap) {
  Vector2I screenPos;
  tileMap.getMapToScreenCoords(m_position.x, m_position.y,
                               screenPos.x, screenPos.y);

  m_pShape->setPosition(static_cast<float>(screenPos.x + TILESIZE_X / 2),
    static_cast<float> (screenPos.y + TILESIZE_Y / 2));

  target->draw(*m_pShape);
  
  if (nullptr != m_pDirShape) {
    m_pDirShape->setPosition(m_pShape->getPosition());

    target->draw(*m_pDirShape);
  }
}

const Vector2I RTSMapGridWalker::s_neighborOffsets[] = {
  Vector2I(0, 1),
  Vector2I(0,-1),
  Vector2I(1, 0),
  Vector2I(-1, 0),
};
