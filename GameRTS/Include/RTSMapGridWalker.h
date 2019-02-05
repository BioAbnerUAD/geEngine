#pragma once

#include <gePrerequisitesUtil.h>
#include <geVector2I.h>

using namespace geEngineSDK;

namespace sf{
  class Shape;
  class RenderTarget;
}

class RTSTiledMap;

class RTSMapGridWalker
{
public:
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

  RTSMapGridWalker(RTSTiledMap* pTiledMap);
  ~RTSMapGridWalker();

  virtual bool
  init() = 0;

  virtual void
  render(sf::RenderTarget* target) = 0;

  virtual void
  StartSeach(bool stepMode) = 0;

  virtual void
  StepSearch() = 0;
  
  FORCEINLINE Vector2I
  GetPosition() const { 
    return m_position; 
  }

  FORCEINLINE void
  SetPosition(Vector2I position) {
    m_position = position; 
  }

  FORCEINLINE Vector2I
  GetTargetPos() const { 
    return m_targetPos;
  }

  FORCEINLINE void
  SetTargetPos(Vector2I position) {
    m_targetPos = position;
  }

  FORCEINLINE const sf::Shape*
  GetShape() const { 
    return m_pShape; 
  }

  FORCEINLINE bool
  IsSearching() const { 
    return searching; 
  }

  FORCEINLINE bool 
  HasFoundPath() const {
    return foundPath; 
  }
protected:
  sf::Shape* m_pShape;
  List<Vector2I>m_queue;
  Vector<RTSPathNode*> m_path;

  bool searching = false;
  bool foundPath = false;
  
  FORCEINLINE const RTSTiledMap*
    GetTiledMap() const {
    return m_pTiledMap;
  }

private:

  RTSTiledMap* m_pTiledMap;
  Vector2I m_position;
  Vector2I m_targetPos;
};
