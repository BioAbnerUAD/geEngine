#pragma once

#include <gePrerequisitesUtil.h>
#include <geVector2I.h>

using namespace geEngineSDK;

namespace sf{
  class Shape;
  class RenderTarget;
  class VertexArray;
}

class RTSTiledMap;
class RTSPathNode;

namespace GRID_WALKER_STATE {
  enum E {
    kIdle = 0,
    kSearching,
    kBacktracking,
    kDisplaying
  };
}

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
  
  virtual void
  StepBacktrack() = 0;

  FORCEINLINE Vector2I
  GetPosition() const { 
    return m_position; 
  }

  FORCEINLINE void
  SetPosition(Vector2I position) {
    m_position = position;
    Reset();
  }

  FORCEINLINE Vector2I
  GetTargetPos() const { 
    return m_targetPos;
  }

  FORCEINLINE void
  SetTargetPos(Vector2I position) {
    m_targetPos = position;
    Reset();
  }

  FORCEINLINE const sf::Shape*
  GetShape() const { 
    return m_pShape; 
  }

  FORCEINLINE GRID_WALKER_STATE::E
  GetState() const { 
    return m_CurrentState; 
  }

  void
  ResetClosedList();

  FORCEINLINE void
  Reset();

protected:
  FORCEINLINE const RTSTiledMap*
  GetTiledMap() const {
    return m_pTiledMap;
  }

  
  sf::Shape*           m_pShape;
  sf::VertexArray*     m_pPathShape;

  List<Vector2I>       m_openList;
  Vector<RTSPathNode*> m_closedList;
  Vector<Vector2I>     m_path;

  GRID_WALKER_STATE::E m_CurrentState;

  /* TODO: enable choosing if diagonal movement is allowed
  and whether it can cross diagonals that are blocked non diagonaly*/
  static const Vector2I s_nextDirection4[8];
  static const Vector2I s_nextDirection8[8];

private:

  RTSTiledMap* m_pTiledMap;
  Vector2I m_position;
  Vector2I m_targetPos;
};

FORCEINLINE void
RTSMapGridWalker::Reset()
{
  m_CurrentState = GRID_WALKER_STATE::kIdle;
  ResetClosedList();
  m_path.clear();
  m_openList.clear();

  if (m_pPathShape) {
    ge_delete(m_pPathShape);
    m_pPathShape = nullptr;
  }
}
