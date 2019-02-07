#pragma once

#include <gePrerequisitesUtil.h>

#include "RTSMapGridWalker.h"

using namespace geEngineSDK;

class RTSDepthFirstSearchMapGridWalker :
  public RTSMapGridWalker
{
public:
  RTSDepthFirstSearchMapGridWalker(RTSTiledMap* m_pTiledMap);
  ~RTSDepthFirstSearchMapGridWalker();

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