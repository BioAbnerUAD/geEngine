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

  void 
  StartSeach(bool stepMode);

  void
  StepSearch();
};
