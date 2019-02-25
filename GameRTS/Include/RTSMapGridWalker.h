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

struct NodeWithCost
{
  Vector2I v;
  float cost;

  FORCEINLINE NodeWithCost(Vector2I v, float cost)
    : v(v), cost(cost) {}

  FORCEINLINE NodeWithCost(Vector2I v)
    : v(v), cost(0.f) {}

  bool operator ==(NodeWithCost other) {
    return (v == other.v) && (cost == other.cost);
  }
};

class RTSMapGridWalker
{
public:
  RTSMapGridWalker(RTSTiledMap* pTiledMap);
  ~RTSMapGridWalker();

  bool
  init();

  void
  render(sf::RenderTarget* target);

  virtual void
  StartSeach(bool stepMode) = 0;

  virtual void
  StepSearch() = 0;
  
  void
  StepBacktrack();

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

  FORCEINLINE void 
  SetClosedListRef(SPtr<Vector<RTSPathNode*>> closedList) {
    m_closedList = closedList; 
  }

  void
  ResetClosedList();

  void
  RenderClosedList(sf::RenderTarget* target);

  void
  AddToClosedList(const Vector2I & position,
                  const Vector2I & direction);

  void
  AddToClosedList(const Vector2I & position,
                  const Vector2I & direction,
                  float cost);

  FORCEINLINE RTSPathNode*
  GetClosedListNode(int32 index){
    return (*m_closedList)[index];
  }

  FORCEINLINE void
  Reset();

protected:
  FORCEINLINE const RTSTiledMap*
  GetTiledMap() const {
    return m_pTiledMap;
  }

  sf::Shape*        m_pShape;
  sf::Shape*        m_pTargetShape;
  sf::VertexArray*  m_pPathShape;

  List<NodeWithCost>         m_openList;
  Vector<Vector2I>           m_path;
  
  GRID_WALKER_STATE::E m_CurrentState;

  /*
  Offsets to add to the current node in order to get neighbor nodes
  TODO: enable choosing if diagonal movement is allowed
  and whether it can cross diagonals that are blocked non diagonally
  */
  static const Vector<Vector2I> s_nextDirection;

private:
  SPtr<Vector<RTSPathNode*>> m_closedList;
  ForwardList<RTSPathNode*>  m_fastClosedList;

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
