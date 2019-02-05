#pragma once

#include <gePrerequisitesUtil.h>

#include "RTSMapGridWalker.h"

using namespace geEngineSDK;

class RTSBreadthFirstSearchMapGridWalker :
  public RTSMapGridWalker
{
public:
  RTSBreadthFirstSearchMapGridWalker(RTSTiledMap* m_pTiledMap);
  ~RTSBreadthFirstSearchMapGridWalker();

  bool
  init();

  void
  render(sf::RenderTarget* target);

  void 
  StartSeach(bool stepMode);

  void
  StepSearch();

private:
  sf::Shape* m_pTargetShape;
};
