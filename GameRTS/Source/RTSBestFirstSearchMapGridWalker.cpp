#include "RTSBestFirstSearchMapGridWalker.h"

#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "RTSPathNode.h"
#include "RTSTiledMap.h"

RTSBestFirstSearchMapGridWalker::
RTSBestFirstSearchMapGridWalker(RTSTiledMap * pTiledMap) :
  RTSMapGridWalker(pTiledMap) {

}

RTSBestFirstSearchMapGridWalker::~RTSBestFirstSearchMapGridWalker() {}

void
RTSBestFirstSearchMapGridWalker::StartSeach(bool stepMode) {
  Vector2I mapSize = GetTiledMap()->getMapSize();
  Vector2I s = GetPosition();

  Reset();

  m_CurrentState = GRID_WALKER_STATE::kSearching;

  m_openList.clear();
  m_openList.push_back(s); // enqueue source

  // mark source as visited.
  AddToClosedList(GetPosition(), Vector2I::ZERO);

  if (!stepMode) { // when not in stepMode run entire search all at once
    while (!m_openList.empty() && 
           m_CurrentState == GRID_WALKER_STATE::kSearching) {
      StepSearch();
    }
    while (m_CurrentState == GRID_WALKER_STATE::kBacktracking) {
      StepBacktrack();
    }
  }
}

void
RTSBestFirstSearchMapGridWalker::StepSearch() {
  GE_ASSERT(m_CurrentState == GRID_WALKER_STATE::kSearching);

  if (m_openList.empty()) {
    m_CurrentState = GRID_WALKER_STATE::kDisplaying;
    return;
  }

  Vector2I mapSize = GetTiledMap()->getMapSize();

  //Removing that vertex from queue, whose neighbors will be visited now
  Vector2I v = m_openList.front().v;
  m_openList.pop_front();

  //processing all the neighbors of v
  Vector2I w;
  int32 wIndex;

  for (SIZE_T i = 0; i < s_nextDirection.size(); ++i) {
    w = v + s_nextDirection[i];
    wIndex = (w.y*mapSize.x) + w.x;
    //if neighbor is target then a path has been found
    if (GetTargetPos() == w) {
      
      //mark w as visited.
      AddToClosedList(w, s_nextDirection[i]);
      
      m_CurrentState = GRID_WALKER_STATE::kBacktracking;
      return;
    }

    //make sure it's a valid neighbor
    if (w.x >= 0 && w.x < mapSize.x && w.y >= 0 && w.y < mapSize.y) {
      //make sure it's not an obstacle and it isn't marked as visited
      if (TERRAIN_TYPE::kObstacle != GetTiledMap()->getType(w.x, w.y) &&
        nullptr == GetClosedListNode(wIndex)) {

        //if diagonal make sure it has a valid diagonal connection
        if (TERRAIN_TYPE::kObstacle != GetTiledMap()->getType(v.x, w.y)
          || TERRAIN_TYPE::kObstacle != GetTiledMap()->getType(w.x, v.y)) {
          PriorityPushBack(w); //enqueue w

          //mark w as visited.
          AddToClosedList(w, s_nextDirection[i]);
        }
      }
    }
  }
}

// TODO: move this somewhere else
void 
RTSBestFirstSearchMapGridWalker::PriorityPushBack(Vector2I& w)
{
  Vector2I target = GetTargetPos();
  uint32 d = target.manhattanDist(w);

  for (auto it = m_openList.begin(); it != m_openList.end(); ++it) {
    if (it->v.manhattanDist(target) > d) {
      m_openList.insert(it, w);
      return;
    }
  }

  m_openList.push_back(w);
}
