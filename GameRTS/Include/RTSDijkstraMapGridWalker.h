#pragma once

#include <gePrerequisitesUtil.h>
#include <geVector2I.h>

#include "RTSMapGridWalker.h"

using namespace geEngineSDK;

class RTSDijkstraMapGridWalker :
  public RTSMapGridWalker
{
public:
  RTSDijkstraMapGridWalker(RTSTiledMap* m_pTiledMap);
  ~RTSDijkstraMapGridWalker();

  void 
  StartSeach(bool stepMode);

  void
  StepSearch();

private:
  void
  PriorityPushBack(const Vector2I& v, float vCost);

  bool m_foundPath = false;
};
