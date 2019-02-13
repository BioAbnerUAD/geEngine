#pragma once

#include <gePrerequisitesUtil.h>

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
  sf::Shape* m_pTargetShape;
};
