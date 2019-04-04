#include "RTSBreadthFirstSearchMapGridWalker.h"

#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "RTSPathNode.h"
#include "RTSTiledMap.h"

RTSBreadthFirstSearchMapGridWalker::
RTSBreadthFirstSearchMapGridWalker(RTSTiledMap * pTiledMap) :
  RTSMapGridWalker(pTiledMap) {}

RTSBreadthFirstSearchMapGridWalker::~RTSBreadthFirstSearchMapGridWalker() {}

void
RTSBreadthFirstSearchMapGridWalker::GetPath(const Vector2I& pos,
                                            const Vector2I& target,
                                            Vector<Vector2I>* path,
                                            bool stepMode /*= false*/) {
  m_position = pos;
  m_targetPos = target;

  Reset();

  m_CurrentState = GRID_WALKER_STATE::kSearching;

  m_openList.clear();
  m_openList.push_back(m_position); // enqueue source

  // mark source as visited.
  AddToClosedList(m_position, Vector2I::ZERO);

  m_pPathOutput = path;

  if (!stepMode) { //when not in stepMode run entire search all at once

    while (m_CurrentState == GRID_WALKER_STATE::kSearching) {
      StepSearch();
    }
  }
}

void
RTSBreadthFirstSearchMapGridWalker::StepSearch() {
  GE_ASSERT(m_CurrentState == GRID_WALKER_STATE::kSearching);

  if (m_openList.empty()) {
    m_CurrentState = GRID_WALKER_STATE::kDisplaying;
    return;
  }

  Vector2I mapSize = GetTiledMap()->getMapSize();

  //Removing that vertex from queue, whose neighbors will be visited now
  Vector2I v = m_openList.front().v;
  m_openList.pop_front();

  //required variables
  Vector2I w;
  int32 wIndex;

  //processing all the neighbors of v
  for (SIZE_T i = 0; i < s_nextDirection.size(); ++i) {
    w = v + s_nextDirection[i];
    wIndex = (w.y*mapSize.x) + w.x;

    //make sure it's a valid neighbor
    if (w.x < 0 || w.x >= mapSize.x || w.y < 0 || w.y >= mapSize.y ||
        //if diagonal make sure it has a valid diagonal connection
        TERRAIN_TYPE::kObstacle == GetTiledMap()->getType(v.x, w.y) ||
        TERRAIN_TYPE::kObstacle == GetTiledMap()->getType(w.x, v.y)) {
      continue;
    }

    //if neighbor is target then a path has been found
    if (m_targetPos == w) {
      
      //mark w as visited.
      AddToClosedList(w, s_nextDirection[i]);
      
      *m_pPathOutput = Backtrack();
      return;
    }

    //make sure it's not an obstacle and it isn't marked as visited
    if (TERRAIN_TYPE::kObstacle != GetTiledMap()->getType(w.x, w.y) &&
        nullptr == GetClosedListNode(wIndex)) {

      m_openList.push_back(w); // enqueue w

      //mark w as visited.
      AddToClosedList(w, s_nextDirection[i]);
    }
  }
}
