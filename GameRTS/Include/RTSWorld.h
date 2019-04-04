#pragma once

#include <gePrerequisitesUtil.h>
#include <geVector2I.h>
#include <geModule.h>

#include <SFML/Graphics.hpp>

#include "RTSHealthBar.h"

using namespace geEngineSDK;

class RTSTiledMap;
class RTSMapGridWalker;
class RTSUnit;

namespace RTSGame {
  class RTSUnitType;
}

class RTSWorld : public Module<RTSWorld>
{
 public:
  RTSWorld();
  ~RTSWorld();
public:
  bool
  init(sf::RenderTarget* pTarget);

  void
  destroy();

  void
  update(float deltaTime);

  void
  render();

  RTSTiledMap*
  getTiledMap() {
    return m_pTiledMap;
  }

  void
  updateResolutionData();

  void
  setCurrentWalker(const int8 index);

  int8
  getCurrentWalkerState();

  FORCEINLINE const List<RTSUnit*>
  GetActiveUnits() const { return m_lsActiveUnits; }

  void 
  DestoryUnit(RTSUnit* unit);

 private:

  void 
  queryLeftClickEvent();

  void
  queryRightClickEvent();

  void 
  paintTiles();

  void 
  putUnit();

  void 
  selectUnit();

  void 
  doubleClickUnit(RTSUnit* unit);

  void 
  moveUnit();

  RTSTiledMap* m_pTiledMap;
  Vector<RTSGame::RTSUnitType*> m_lstUnitTypes;

  List<RTSUnit*> m_lstUnits;
  List<RTSUnit*> m_lsActiveUnits;
  
  Vector<RTSMapGridWalker*> m_walkersList;
  RTSMapGridWalker* m_activeWalker = nullptr;
  int8 m_activeWalkerIndex = -1;

  RTSHealthBar* m_pHealthBar;

  sf::RenderTarget* m_pTarget;
};
