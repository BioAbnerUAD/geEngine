#pragma once

#include <gePrerequisitesUtil.h>
#include <geVector2I.h>

#include "RTSMapGridWalker.h"

using namespace geEngineSDK;

class RTSAStarMapGridWalker :
  public RTSMapGridWalker
{
public:
  RTSAStarMapGridWalker(RTSTiledMap* m_pTiledMap);
  ~RTSAStarMapGridWalker();

  bool
  init();

  void
  render(sf::RenderTarget* target);

  void
  StartSeach(bool stepMode);

  void
  StepSearch();

private:
  struct AStarNode
  {
    Vector2I v;
    int8 cost;
    uint32 distance;
  };

  void
  PriorityPushBack(Vector2I v, int8 vCost);

  sf::Shape* m_pTargetShape;
  List<AStarNode> m_openListAstar;
};
