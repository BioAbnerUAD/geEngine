#pragma once

#include <gePrerequisitesUtil.h>
#include <geVector2I.h>

#include "RTSMapGridWalker.h"

using namespace geEngineSDK;

class RTSAStarMapGridWalker :
  public RTSMapGridWalker
{
public:
  RTSAStarMapGridWalker(RTSTiledMap* m_pTiledMap);
  ~RTSAStarMapGridWalker();

  void
  StartSeach(bool stepMode);

  void
  StepSearch();

private:
  void
  PriorityPushBack(const Vector2I& v, float vCost);
};
