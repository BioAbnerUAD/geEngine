#pragma once

#include <gePrerequisitesUtil.h>

#include "RTSMapGridWalker.h"

using namespace geEngineSDK;

class RTSDepthFirstSearchMapGridWalker :
  public RTSMapGridWalker
{
public:
  RTSDepthFirstSearchMapGridWalker(RTSTiledMap* pTiledMap);
  ~RTSDepthFirstSearchMapGridWalker();

  void 
  StartSeach(bool stepMode);

  void
  StepSearch();
};
