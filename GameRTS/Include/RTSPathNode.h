#pragma once

#include <gePrerequisitesUtil.h>
#include <geVector2I.h>

using namespace geEngineSDK;

namespace sf
{
  class Shape;
  class RenderTarget;
  class Font;
  class Text;
}

class RTSTiledMap;

class RTSPathNode
{
public:
  RTSPathNode(const Vector2I& position,
              const Vector2I& direction);

  RTSPathNode(const Vector2I& position,
              const Vector2I& direction, 
              float cost);

  ~RTSPathNode();

  void 
  render(sf::RenderTarget * target, const RTSTiledMap & tileMap);

  void
  SetNewDirAndCost(Vector2I newDir, float newCost);

  FORCEINLINE Vector2I 
  GetPosition() const {
    return m_position;
  }

  FORCEINLINE Vector2I 
  GetDirection() const {
    return m_direction;
  }

  FORCEINLINE float 
  GetCost() const {
    return m_cost;
  }

private:
  static UPtr<sf::Font>  s_pArialFont;

  sf::Shape* m_pShape = nullptr;
  sf::Shape* m_pDirShape = nullptr;
  sf::Text*  m_pCostText = nullptr;

  Vector2I   m_position;
  Vector2I   m_direction;
  float      m_cost;
};

