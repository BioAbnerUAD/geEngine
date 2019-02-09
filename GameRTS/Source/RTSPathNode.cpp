#include "RTSPathNode.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include "RTSTiledMap.h"

RTSPathNode::RTSPathNode(const Vector2I & position,
                         const Vector2I & direction) :
  m_position(position), m_direction(direction) {

  m_pShape = new sf::RectangleShape(sf::Vector2f(5.f, 5.f));
  m_pShape->setFillColor(sf::Color::Red);
  m_pShape->setOrigin(2.5f, 2.5f);

  if (Vector2I::ZERO != direction) {
    m_pDirShape = new sf::RectangleShape(sf::Vector2f(10.f, 1.f));
    m_pDirShape->setFillColor(sf::Color::Blue);

# ifdef MAP_IS_ISOMETRIC
    // project into isometric space
    float sum = static_cast<float>(direction.y + direction.x);
    float delta = static_cast<float>(direction.y - direction.x);

    Vector2 isoDir = { -0.5f * delta, 0.2887f * sum };

    Radian dirAngle = Math::atan2(isoDir.y, isoDir.x);

    m_pDirShape->setRotation(dirAngle.valueDegrees());
# else
    Radian dirAngle = Math::atan2(static_cast<float>(direction.y),
                                  static_cast<float>(direction.x));

    m_pDirShape->setRotation(dirAngle.valueDegrees());
# endif

  }
  else {
    m_pDirShape = nullptr;
  }
}

RTSPathNode::~RTSPathNode() {
  if (nullptr != m_pShape) {
    ge_delete(m_pShape);
  }
  if (nullptr != m_pDirShape) {
    ge_delete(m_pDirShape);
  }
}

void RTSPathNode::render(sf::RenderTarget* target,
                         const RTSTiledMap& tileMap) {
  Vector2I screenPos;
  tileMap.getMapToScreenCoords(m_position.x, m_position.y,
                               screenPos.x, screenPos.y);

  m_pShape->setPosition(static_cast<float>(screenPos.x + HALFTILESIZE_X),
                        static_cast<float> (screenPos.y + HALFTILESIZE_Y));

  target->draw(*m_pShape);
  
  if (nullptr != m_pDirShape) {
    m_pDirShape->setPosition(m_pShape->getPosition());

    target->draw(*m_pDirShape);
  }
}