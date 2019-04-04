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
  GetPath(const Vector2I& pos, 
          const Vector2I& target, 
          Vector<Vector2I>* path, 
          bool stepMode = false) override;

  void
  StepSearch() override;

private:
  void
  PriorityPushBack(const Vector2I& v, float vCost);

  bool m_foundPath = false;
};
