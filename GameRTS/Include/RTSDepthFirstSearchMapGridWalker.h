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
  GetPath(const Vector2I& pos, 
          const Vector2I& target, 
          Vector<Vector2I>* path, 
          bool stepMode = false) override;

  void
  StepSearch() override;
};
