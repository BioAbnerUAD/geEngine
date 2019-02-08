#pragma once

#include <gePrerequisitesUtil.h>
#include <geVector2I.h>

using namespace geEngineSDK;

namespace sf{
  class Shape;
  class RenderTarget;
}

class RTSTiledMap;
class RTSPathNode;

class RTSMapGridWalker
{
public:
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
    return m_searching; 
  }

  FORCEINLINE bool 
  HasFoundPath() const {
    return m_foundPath; 
  }

  void
  ResetPath();

protected:
  FORCEINLINE const RTSTiledMap*
  GetTiledMap() const {
    return m_pTiledMap;
  }

  
  sf::Shape* m_pShape;
  List<Vector2I> m_openList;
  Vector<RTSPathNode*> m_path;

  bool m_searching = false;
  bool m_foundPath = false;

  static const Vector2I s_neighborOffsets[4];

private:

  RTSTiledMap* m_pTiledMap;
  Vector2I m_position;
  Vector2I m_targetPos;
};
