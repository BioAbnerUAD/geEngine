#include "RTSAStarMapGridWalker.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "RTSPathNode.h"
#include "RTSTiledMap.h"

RTSAStarMapGridWalker::
RTSAStarMapGridWalker(RTSTiledMap * m_pTiledMap) :
  RTSMapGridWalker(m_pTiledMap) {}

RTSAStarMapGridWalker::~RTSAStarMapGridWalker() {}

void
RTSAStarMapGridWalker::GetPath(const Vector2I& pos,
                               const Vector2I& target,
                               Vector<Vector2I>* path,
                               bool stepMode /*= false*/) {
  m_position = pos;
  m_targetPos = target;

  Reset();

  m_CurrentState = GRID_WALKER_STATE::kSearching;

  // enqueue source
  m_openList.push_back({ m_position, 0 }); //zero cost cause I'm already here

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
RTSAStarMapGridWalker::StepSearch() {
  GE_ASSERT(m_CurrentState == GRID_WALKER_STATE::kSearching);
  if (m_openList.empty()) {
    m_CurrentState = GRID_WALKER_STATE::kDisplaying;
    return;
  }

  Vector2I mapSize = GetTiledMap()->getMapSize();

  //Removing that vertex from queue, whose neighbors will be visited now
  Vector2I v = m_openList.front().v;
  float vCost = m_openList.front().cost;

  m_openList.pop_front();

  //required variables
  Vector2I w;
  float wCost;
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
      wCost = vCost + GetTiledMap()->getCost(w.x, w.y);

      //mark w as visited.
      AddToClosedList(w, s_nextDirection[i], wCost);

      *m_pPathOutput = Backtrack();
      
      return;
    }

    //make sure it's not an obstacle
    if (TERRAIN_TYPE::kObstacle != GetTiledMap()->getType(w.x, w.y)) {

      // if it isn't marked as visited just push it
      float costMult = (s_nextDirection[i].sizeSquared() > 1) ? 1.5f : 1.f;
      float tileCost = static_cast<float>(GetTiledMap()->getCost(w.x, w.y));

      wCost = vCost + tileCost * costMult;

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

// TODO: Move this to somewhere else and also bestFirstSearch's Priority Push Back
void 
RTSAStarMapGridWalker::PriorityPushBack(const Vector2I& v, float vCost) {

  uint32 distance = v.manhattanDist(m_targetPos);
  uint32 tempDistance;

  for (auto it = m_openList.begin(); it != m_openList.end(); ++it) {
    tempDistance = it->v.manhattanDist(m_targetPos);
    if (it->cost + tempDistance > vCost + distance) {
      m_openList.insert(it, { v, vCost });
      return;
    }
  }

  m_openList.push_back({v, vCost });
}
