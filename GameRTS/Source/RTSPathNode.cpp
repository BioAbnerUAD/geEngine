#include "RTSPathNode.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

#include "RTSTiledMap.h"

#ifdef MAP_IS_ISOMETRIC
# define COSTTEXT_OFFSET sf::Vector2f(-4.f, -0.f)
#else
# define COSTTEXT_OFFSET sf::Vector2f(-4.f, -4.f)
#endif // MAP_IS_ISOMETRIC


UPtr<sf::Font> RTSPathNode::s_pArialFont = ge_unique_ptr<sf::Font>(nullptr);

RTSPathNode::RTSPathNode(const Vector2I & position,
                         const Vector2I & direction) :
  m_position(position), 
  m_direction(direction), 
  m_cost(-1) {

  m_pShape = ge_new<sf::RectangleShape>(sf::Vector2f(5.f, 5.f));
  m_pShape->setFillColor(sf::Color::Red);
  m_pShape->setOrigin(2.5f, 2.5f);

  if (Vector2I::ZERO != direction) {
    m_pDirShape = ge_new<sf::RectangleShape>(sf::Vector2f(10.f, 1.f));
    m_pDirShape->setFillColor(sf::Color::Blue);

    //TODO: put this formula on a separate function
# ifdef MAP_IS_ISOMETRIC
    // project into isometric space
    float sum = static_cast<float>(direction.y + direction.x);
    float delta = static_cast<float>(direction.y - direction.x);

    Vector2 isoDir = { -0.5f * delta, 0.2887f * sum };

    Radian dirAngle = Math::atan2(isoDir.y, isoDir.x);
# else
    Radian dirAngle = Math::atan2(static_cast<float>(direction.y),
                                  static_cast<float>(direction.x));
# endif

    m_pDirShape->setRotation(dirAngle.valueDegrees());
  }
  else {
    m_pDirShape = nullptr;
  }
}

RTSPathNode::RTSPathNode(const Vector2I & position, 
                         const Vector2I & direction, 
                         float cost) :
  RTSPathNode(position, direction) {
  m_cost = cost;

  if (!s_pArialFont) {
    s_pArialFont = ge_unique_ptr_new<sf::Font>();
    if (!s_pArialFont) {
      GE_EXCEPT(InvalidStateException, "Couldn't create a Font");
    }

    if (!s_pArialFont->loadFromFile("Fonts/arial.ttf")) {
      GE_EXCEPT(FileNotFoundException, "Arial font not found");
    }
  }

  m_pCostText = ge_new<sf::Text>(toString(m_cost).c_str(), *s_pArialFont, 10);
}

RTSPathNode::~RTSPathNode() {
  if (nullptr != m_pShape) {
    ge_delete(m_pShape);
  }
  if (nullptr != m_pDirShape) {
    ge_delete(m_pDirShape);
  }
  if (nullptr != m_pCostText) {
    ge_delete(m_pCostText);
  }
}

void 
RTSPathNode::render(sf::RenderTarget* target,
                    const RTSTiledMap& tileMap) {
  Vector2I screenPos;
  tileMap.getMapToScreenCoords(m_position.x, m_position.y,
                               screenPos.x, screenPos.y);

  m_pShape->setPosition(static_cast<float>(screenPos.x + HALFTILESIZE_X),
                        static_cast<float> (screenPos.y + HALFTILESIZE_Y));

  target->draw(*m_pShape);
  
  if (nullptr != m_pCostText) {
    m_pCostText->setPosition(m_pShape->getPosition() + COSTTEXT_OFFSET);

    target->draw(*m_pCostText);
  }

  if (nullptr != m_pDirShape) {
    m_pDirShape->setPosition(m_pShape->getPosition());

    target->draw(*m_pDirShape);
  }
}

void
RTSPathNode::SetNewDirAndCost(Vector2I newDir, float newCost) {
  m_direction = newDir;
  m_cost = newCost;

  if (nullptr != m_pDirShape) {
#  ifdef MAP_IS_ISOMETRIC
    // project into isometric space
    float sum = static_cast<float>(newDir.y + newDir.x);
    float delta = static_cast<float>(newDir.y - newDir.x);

    Vector2 isoDir = { -0.5f * delta, 0.2887f * sum };

    Radian dirAngle = Math::atan2(isoDir.y, isoDir.x);
#  else
    Radian dirAngle = Math::atan2(static_cast<float>(newDir.y),
                                  static_cast<float>(newDir.x));
#  endif

    m_pDirShape->setRotation(dirAngle.valueDegrees());
  }

  if (nullptr != m_pCostText) {
    m_pCostText->setString(toString(m_cost).c_str());
  }
}
