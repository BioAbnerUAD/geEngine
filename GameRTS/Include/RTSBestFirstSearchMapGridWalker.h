#pragma once

#include <gePrerequisitesUtil.h>
#include "RTSMapGridWalker.h"

using namespace geEngineSDK;

class RTSBestFirstSearchMapGridWalker :
  public RTSMapGridWalker
{
public:
  RTSBestFirstSearchMapGridWalker(RTSTiledMap* tileMap);
  ~RTSBestFirstSearchMapGridWalker();

  void 
  StartSeach(bool stepMode);

  void
  StepSearch();

private:
  void
  PriorityPushBack(Vector2I& w);
};

