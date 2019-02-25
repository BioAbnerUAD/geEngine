#include "RTSAStarMapGridWalker.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "RTSPathNode.h"
#include "RTSTiledMap.h"

RTSAStarMapGridWalker::
RTSAStarMapGridWalker(RTSTiledMap * m_pTiledMap) :
  RTSMapGridWalker(m_pTiledMap) {

}

RTSAStarMapGridWalker::~RTSAStarMapGridWalker() {}

void
RTSAStarMapGridWalker::StartSeach(bool stepMode) {
  Vector2I mapSize = GetTiledMap()->getMapSize();
  Vector2I s = GetPosition();

  Reset();

  m_CurrentState = GRID_WALKER_STATE::kSearching;

  // enqueue source
  m_openList.push_back({ s, 0 }); //zero cost cause I'm already here

  // mark source as visited.
  AddToClosedList(GetPosition(), Vector2I::ZERO);

  if (!stepMode) { //when not in stepMode run entire search all at once
    while (m_CurrentState == GRID_WALKER_STATE::kSearching) {
      StepSearch();
    }
    while (m_CurrentState == GRID_WALKER_STATE::kBacktracking) {
      StepBacktrack();
    }
  }
}

/*
TODO: AStar should stop if it finds first path, if it goes a certain amount of steps,
or even searching all the map
*/

void
RTSAStarMapGridWalker::StepSearch() {
  GE_ASSERT(m_CurrentState == GRID_WALKER_STATE::kSearching);
  if (m_openList.empty()) {
    m_CurrentState = GRID_WALKER_STATE::kBacktracking;
  }

  Vector2I mapSize = GetTiledMap()->getMapSize();
  Vector2I target = GetTargetPos();

  //Removing that vertex from queue, whose neighbors will be visited now
  Vector2I v = m_openList.front().v;
  float vCost = m_openList.front().cost;

  m_openList.pop_front();

  //required variables
  Vector2I w;
  float wCost;
  int32 wIndex;
  uint32 distance;

  //processing all the neighbors of v
  for (SIZE_T i = 0; i < s_nextDirection.size(); ++i) {
    w = v + s_nextDirection[i];
    wIndex = (w.y*mapSize.x) + w.x;

    //if neighbor is target then a path has been found
    if (GetTargetPos() == w) {
      wCost = vCost + GetTiledMap()->getCost(w.x, w.y);

      //mark w as visited.
      AddToClosedList(w, s_nextDirection[i], wCost);

      m_CurrentState = GRID_WALKER_STATE::kBacktracking;
      
      return;
    }

    //make sure it's a valid neighbor
    if (w.x >= 0 && w.x < mapSize.x && w.y >= 0 && w.y < mapSize.y) {
      //make sure it's not an obstacle
      if (TERRAIN_TYPE::kObstacle != GetTiledMap()->getType(w.x, w.y)) {
        // if it isn't marked as visited just push it
        float costMult = (s_nextDirection[i].sizeSquared() > 1) ? 1.5f : 1.f;
        float tileCost = static_cast<float>(GetTiledMap()->getCost(w.x, w.y));

        wCost = vCost + tileCost * costMult;

        distance = v.manhattanDist(target);

        if (nullptr == GetClosedListNode(wIndex)) {
          PriorityPushBack(w, wCost); // enqueue w

          //mark w as visited.
          AddToClosedList(w, s_nextDirection[i], wCost);
        }
        // Doesn't take distance into account because it's the same node
        else if (wCost < GetClosedListNode(wIndex)->GetCost()) {
          PriorityPushBack(w, wCost); // enqueue w again

          //update lesser cost for node
          GetClosedListNode(wIndex)->SetNewDirAndCost(s_nextDirection[i], wCost);
        }
      }
    }
  }
}

// TODO: Move this to somewhere else and also bestFirstSearch's Priority Push Back
void 
RTSAStarMapGridWalker::PriorityPushBack(const Vector2I& v, float vCost) {
  Vector2I target = GetTargetPos();
  uint32 distance = v.manhattanDist(target);
  uint32 tempDistance;

  for (auto it = m_openList.begin(); it != m_openList.end(); ++it) {
    tempDistance = it->v.manhattanDist(target);
    if (it->cost + tempDistance > vCost + distance) {
      m_openList.insert(it, { v, vCost });
      return;
    }
  }

  m_openList.push_back({v, vCost });
}
