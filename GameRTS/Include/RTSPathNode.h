#pragma once

#include <gePrerequisitesUtil.h>
#include <geVector2I.h>

using namespace geEngineSDK;

namespace sf 
{
  class Shape;
  class RenderTarget;
}

class RTSTiledMap;

class RTSPathNode
{
public:
  RTSPathNode(const Vector2I& position,
              const Vector2I& direction);
  ~RTSPathNode();

  void
  render(sf::RenderTarget * target,
         const RTSTiledMap& tileMap);
private:
  Vector2I m_position;
  Vector2I m_direction;
  sf::Shape* m_pShape;
  sf::Shape* m_pDirShape;
};

