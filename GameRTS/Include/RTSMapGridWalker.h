#pragma once

#include <gePrerequisitesUtil.h>
#include <geVector2I.h>

using namespace geEngineSDK;

namespace sf{
  class RenderTarget;
  class VertexArray;
}

class RTSTiledMap;
class RTSPathNode;

namespace GRID_WALKER_STATE {
  enum E {
    kIdle = 0,
    kSearching,
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
  GetPath(const Vector2I& pos, 
          const Vector2I& target, 
          Vector<Vector2I>* path, 
          bool stepMode = false) = 0;

  virtual void
  StepSearch() = 0;

  FORCEINLINE GRID_WALKER_STATE::E
  GetState() const { 
    return m_CurrentState; 
  }

  FORCEINLINE void 
  SetClosedListRef(SPtr<Vector<RTSPathNode*>> closedList) {
    m_closedList = closedList; 
  }

  FORCEINLINE void
  Reset();

protected:
  FORCEINLINE RTSPathNode*
  GetClosedListNode(int32 index) {
    return (*m_closedList)[index];
  }

  Vector<Vector2I>
  Backtrack();

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

  FORCEINLINE const RTSTiledMap*
  GetTiledMap() const {
    return m_pTiledMap;
  }

  sf::VertexArray*  m_pPathShape;

  List<NodeWithCost>   m_openList;
  Vector<Vector2I>*    m_pPathOutput;
  
  GRID_WALKER_STATE::E m_CurrentState;

  /*
  Offsets to add to the current node in order to get neighbor nodes
  TODO: enable choosing if diagonal movement is allowed
  and whether it can cross diagonals that are blocked non diagonally
  */
  static const Vector<Vector2I> s_nextDirection;

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
  m_openList.clear();
}
