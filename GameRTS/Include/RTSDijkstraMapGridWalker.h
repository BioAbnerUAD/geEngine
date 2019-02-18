#pragma once

#include <gePrerequisitesUtil.h>
#include <geVector2I.h>

#include "RTSMapGridWalker.h"

using namespace geEngineSDK;

class RTSDijkstraMapGridWalker :
  public RTSMapGridWalker
{
public:
  RTSDijkstraMapGridWalker(RTSTiledMap* m_pTiledMap);
  ~RTSDijkstraMapGridWalker();

  bool
  init();

  void
  render(sf::RenderTarget* target);

  void 
  StartSeach(bool stepMode);

  void
  StepSearch();

  void
  StepBacktrack();

private:
  struct nodeAndCost
  {
    Vector2I v;
    int8 cost;
  };

  void
  PriorityPushBack(Vector2I v, int8 vCost);

  bool m_foundPath = false;

  sf::Shape* m_pTargetShape;
  List<nodeAndCost> m_openListWithCosts;
};
