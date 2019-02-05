#pragma once

#include <gePrerequisitesUtil.h>
#include <geVector2I.h>

#include <SFML/Graphics.hpp>

using namespace geEngineSDK;

class RTSTiledMap;
class RTSMapGridWalker;

namespace RTSGame {
  class RTSUnitType;
}

class RTSWorld
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

  void
  StartSearch();

  void 
  StepSearch();

 private:

  void 
  queryLeftClickEvent();

  void
  queryRightClickEvent();


  RTSTiledMap* m_pTiledMap;
  List<RTSGame::RTSUnitType*> m_lstUnitTypes;
  //List<RTSUnit*> m_lstUnits;
  
  Vector<RTSMapGridWalker*> m_walkersList;
  RTSMapGridWalker* m_activeWalker = nullptr;
  int8 m_activeWalkerIndex = -1;

  sf::RenderTarget* m_pTarget;
};
