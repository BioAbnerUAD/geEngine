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
  GetPath(const Vector2I& pos, 
          const Vector2I& target, 
          Vector<Vector2I>* path, 
          bool stepMode = false) override;

  void
  StepSearch() override;
};
