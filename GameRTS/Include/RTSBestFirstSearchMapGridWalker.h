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
  GetPath(const Vector2I& pos, 
          const Vector2I& target, 
          Vector<Vector2I>* path, 
          bool stepMode = false) override;

  void
  StepSearch() override;

private:
  void
  PriorityPushBack(Vector2I& w);
};

