#include "RTSWorld.h"

#include <geTime.h>

#include "RTSTiledMap.h"

#include "RTSUnitType.h"
#include "RTSUnit.h"

#include "RTSBreadthFirstSearchMapGridWalker.h"
#include "RTSDepthFirstSearchMapGridWalker.h"
#include "RTSBestFirstSearchMapGridWalker.h"
#include "RTSDijkstraMapGridWalker.h"
#include "RTSAStarMapGridWalker.h"

using namespace RTSGame;

RTSWorld::RTSWorld() {
  m_pTiledMap = nullptr;
  m_pHealthBar = nullptr;
  m_pSelectionRect = nullptr;
  m_pActiveUnitCircle = nullptr;
  m_activeWalkerIndex = -1;	//-1 = Invalid index
}

RTSWorld::~RTSWorld() {
  destroy();
}

bool
RTSWorld::init(sf::RenderTarget* pTarget) {
  GE_ASSERT(nullptr == m_pTiledMap && "World was already initialized");
  destroy();

  Vector2I mapSize = Vector2I(4096, 4096);
  
  m_pTarget = pTarget;

  //Initialize the map (right now it's an empty map)
  m_pTiledMap = ge_new<RTSTiledMap>();
  GE_ASSERT(m_pTiledMap);
  m_pTiledMap->init(m_pTarget, mapSize);

  //Create the path finding classes and push them to the walker list
  m_walkersList.push_back(ge_new<RTSBreadthFirstSearchMapGridWalker>(m_pTiledMap));
  m_walkersList.push_back(ge_new<RTSDepthFirstSearchMapGridWalker>(m_pTiledMap));
  m_walkersList.push_back(ge_new<RTSBestFirstSearchMapGridWalker>(m_pTiledMap));
  m_walkersList.push_back(ge_new<RTSDijkstraMapGridWalker>(m_pTiledMap));
  m_walkersList.push_back(ge_new<RTSAStarMapGridWalker>(m_pTiledMap));

  auto closedList = ge_shared_ptr_new<Vector<RTSPathNode*>>();
  closedList->resize(mapSize.x * mapSize.y, nullptr);

  //Init the walker objects

  for (SIZE_T it = 0; it < m_walkersList.size(); ++it) {
    m_walkersList[it]->init();
    m_walkersList[it]->SetClosedListRef(closedList);
  }

  //Set the first walker as the active walker
  setCurrentWalker(m_walkersList.size() > 0 ? 0 : -1);

  m_lstUnitTypes.reserve(UNIT_TYPES::kNUM_UNIT_TYPES);

  for (uint32 i = 0; i < UNIT_TYPES::kNUM_UNIT_TYPES; ++i) {

    auto unitType = ge_new<RTSGame::RTSUnitType>();
    unitType->loadAnimationData(m_pTarget, i + 1);

    m_lstUnitTypes.emplace_back(unitType);
  }

  m_pHealthBar = ge_new<RTSHealthBar>(*m_pTarget);

  m_pSelectionRect = ge_new<sf::RectangleShape>();
  m_pSelectionRect->setFillColor(sf::Color::Transparent);
  m_pSelectionRect->setOutlineColor(sf::Color::White);
  m_pSelectionRect->setOutlineThickness(1);

  m_pActiveUnitCircle = ge_new<sf::CircleShape>();
  m_pActiveUnitCircle->setFillColor(sf::Color::Transparent);
  m_pActiveUnitCircle->setOutlineColor(sf::Color::White);
  m_pActiveUnitCircle->setOutlineThickness(1);
  m_pActiveUnitCircle->setRadius(HALFTILESIZE_X * .5f);
  m_pActiveUnitCircle->setOrigin(HALFTILESIZE_X * .5f, HALFTILESIZE_X * .5f);

  #ifdef MAP_IS_ISOMETRIC
  m_pActiveUnitCircle->setScale(sf::Vector2f(1.f, 0.5f));
  #endif

  return true;
}

void
RTSWorld::destroy() {
 //Destroy all the walkers
  while (m_walkersList.size() > 0) {
    ge_delete(m_walkersList.back());
    m_walkersList.pop_back();
  }

  while (m_lstUnitTypes.size() > 0) {
    ge_delete(m_lstUnitTypes.back());
    m_lstUnitTypes.pop_back();
  }

  while (m_lstUnits.size() > 0) {
    ge_delete(m_lstUnits.back());
    m_lstUnits.pop_back();
  }

  //As the last step, destroy the full map
  if (nullptr != m_pTiledMap) {
    ge_delete(m_pTiledMap);
    m_pTiledMap = nullptr;
  }

  if (m_pHealthBar) {
    ge_delete(m_pHealthBar);
    m_pHealthBar = nullptr;
  }

  if (m_pSelectionRect) {
    ge_delete(m_pSelectionRect);
    m_pSelectionRect = nullptr;
  }

  if (m_pActiveUnitCircle) {
    ge_delete(m_pActiveUnitCircle);
    m_pActiveUnitCircle = nullptr;
  }
}

void
RTSWorld::update(float deltaTime) {
  m_pTiledMap->update(deltaTime);

  if (!GameOptions::s_GUIBlockingMouse) {
    queryLeftClickEvent();
    queryRightClickEvent();
  }
  
  // If the selected walker index is different than the one registered than update it
  if (m_activeWalkerIndex != GameOptions::s_CurrentWalkerIndex &&
    m_walkersList.size() > GameOptions::s_CurrentWalkerIndex) {
    setCurrentWalker(GameOptions::s_CurrentWalkerIndex);
  }

  if (nullptr != m_activeWalker && 
      m_activeWalker->GetState() == GRID_WALKER_STATE::kSearching) {

      m_activeWalker->StepSearch();
  }

  auto it = m_lsActiveUnits.begin();
  while (it != m_lsActiveUnits.end()) {
    if ((*it)->GetCurrentHP() <= 0) {
      it = m_lsActiveUnits.erase(it);
    }
    else {
      ++it;
    }
  }

  it = m_lstUnits.begin();
  while (it != m_lstUnits.end()) {
    if (nullptr == (*it)) {
      it = m_lstUnits.erase(it);
    }
    else {
      (*it)->Update(deltaTime);
      ++it;
    }
  }
}

void
RTSWorld::queryLeftClickEvent() {
  static bool leftClickWasPressed = false;
  static Vector2I clickStartPos;

  if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
    selectionRectEnabled = false;

    if (GameOptions::activeTool == RTSTools::kTerrain) {
      paintTiles();
    }
    else if (!leftClickWasPressed) {
      if (GameOptions::activeTool == RTSTools::kPlaceUnit) {
        putUnit();
      }
      else if (GameOptions::activeTool == RTSTools::kMoveUnit) {
        auto sfPos = sf::Mouse::getPosition();
        clickStartPos = { sfPos.x, sfPos.y };
      }
    }
    else if (GameOptions::activeTool == RTSTools::kMoveUnit)
    {
      auto sfPos = sf::Mouse::getPosition();
      Vector2I currClickPos = { sfPos.x, sfPos.y };

      if (clickStartPos.Vector2I::manhattanDist(currClickPos) > 20) {
        selectionRectEnabled = true;
        m_pSelectionRect->setPosition(float(clickStartPos.x), 
                                      float(clickStartPos.y));

        Vector2I rectSize = currClickPos - clickStartPos;

        m_pSelectionRect->setSize(sf::Vector2f(float(rectSize.x), 
                                               float(rectSize.y)));
      }
    }

    leftClickWasPressed = true;
  }
  else {
    if (GameOptions::activeTool == RTSTools::kMoveUnit 
        && leftClickWasPressed) {
      if (selectionRectEnabled) {
        auto sfPos = sf::Mouse::getPosition();
        Vector2I currClickPos = { sfPos.x, sfPos.y };

        selectAllUnitsIn(clickStartPos, currClickPos);
      }
      else {
        selectUnit();
      }
    }
    leftClickWasPressed = false;
    selectionRectEnabled = false;
  }
}

void 
RTSWorld::queryRightClickEvent() {
  static bool rightClickWasPressed = false;

  if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
    if (GameOptions::activeTool == RTSTools::kMoveUnit &&
        !rightClickWasPressed) {
      moveUnit();
    }

    rightClickWasPressed = true;
  }
  else {
    rightClickWasPressed = false;
  }
}

void
RTSWorld::paintTiles()   {
  sf::Vector2i mousePos;
  Vector2I mapPos;
  int8 clickedTileType;

  if (m_activeWalker->GetState() == GRID_WALKER_STATE::kDisplaying) {
    m_activeWalker->Reset();
  }

  //This sets the terrain on the tiles clicked on depending on the brush and the selected terrain type
  if (m_activeWalker->GetState() == GRID_WALKER_STATE::kIdle) {

    //Check which tile was clicked on
    mousePos = sf::Mouse::getPosition();

    m_pTiledMap->getScreenToMapCoords(mousePos.x, mousePos.y,
                                      mapPos.x, mapPos.y);

    clickedTileType = static_cast<int8>(GameOptions::s_selectedTerrainIndex);

    if (GameOptions::s_brushSize <= 1) {
      m_pTiledMap->setType(mapPos.x, mapPos.y, clickedTileType);
      m_pTiledMap->setCost(mapPos.x, mapPos.y, TERRAIN_TYPE::ECost[clickedTileType]);
    }
    else {
      Vector2I mapSize = m_pTiledMap->getMapSize();
      int32 halfSizeL = GameOptions::s_brushSize / 2;
      int32 halfSizeU = (GameOptions::s_brushSize + 1) / 2;

      int32 lowerX = Math::max(0, mapPos.x - halfSizeL);
      int32 lowerY = Math::max(0, mapPos.y - halfSizeL);
      int32 upperX = Math::min(mapSize.x - 1, mapPos.x + halfSizeU);
      int32 upperY = Math::min(mapSize.y - 1, mapPos.y + halfSizeU);

      for (int32 i = lowerX; i < upperX; i++) {
        for (int32 j = lowerY; j < upperY; j++) {
          m_pTiledMap->setType(i, j, clickedTileType);
          m_pTiledMap->setCost(i, j, TERRAIN_TYPE::ECost[clickedTileType]);
        }
      }

    }
  }
}

void 
RTSWorld::putUnit() {
  sf::Vector2i mousePos = sf::Mouse::getPosition();

  if (!m_pTiledMap->isScreenCoordInMap(mousePos.x, mousePos.y)) {
    return;
  }

  if (m_activeWalker->GetState() == GRID_WALKER_STATE::kDisplaying) {
    m_activeWalker->Reset();
  }

  if (m_activeWalker->GetState() == GRID_WALKER_STATE::kIdle) {

    Vector2 mapPos;
    m_pTiledMap->getRawScreenToMapCoords(mousePos.x, mousePos.y,
                                         mapPos.x, mapPos.y);

    m_lstUnits.push_back(new RTSUnit(0, 
                                     m_lstUnitTypes[GameOptions::s_unitTypeIndex], 
                                     mapPos, 
                                     &m_activeWalker));
  }
}

void
RTSWorld::selectUnit() {
  static float lastClickTime = 0;
  float clickTime;
  sf::Vector2i mousePos;
  Vector2 mapPos;

  clickTime = Time::instance().getTime();

  mousePos = sf::Mouse::getPosition();

  m_pTiledMap->getRawScreenToMapCoords(mousePos.x, mousePos.y,
                                       mapPos.x, mapPos.y);

  List<RTSUnit*> prevActiveUnits = m_lsActiveUnits;
  RTSUnit* lowPriority = nullptr;
  m_lsActiveUnits.clear();

  for (auto it = m_lstUnits.begin(); it != m_lstUnits.end(); ++it) {
    if (nullptr != (*it) && 
        (*it)->GetCurrentHP() > 0 &&
        Vector2::distSquared((*it)->GetRawPosition(), mapPos) <=  0.4f) {
      auto search = std::find(prevActiveUnits.begin(), prevActiveUnits.end(), (*it));
      if (prevActiveUnits.end() != search) {
        if (clickTime - lastClickTime <= 0.25f) {
          doubleClickUnit(*it);
          break;
        }
        else {
          lowPriority = *it;
        }
      }
      else {
        m_lsActiveUnits.push_back(*it);
        break;
      }
    }
  }

  if (m_lsActiveUnits.size() == 0 && lowPriority) {
    m_lsActiveUnits.push_back(lowPriority);
  }

  lastClickTime = clickTime;
  m_activeWalker->Reset();
}

void 
RTSWorld::doubleClickUnit(RTSUnit* unit) {
  for each (RTSUnit* otherUnit in m_lstUnits) {
    if (unit->GetUnitType() == otherUnit->GetUnitType() &&
        otherUnit->GetCurrentHP() > 0 && 
        100.f >= Vector2::distSquared(otherUnit->GetRawPosition(), 
                                      unit->GetRawPosition())) {
      m_lsActiveUnits.push_back(otherUnit);
    }
  }
}

void
RTSWorld::moveUnit() {
  if (m_lsActiveUnits.size() == 0) {
    return;
  }

  //This moves the 'Active Walker' to the tile that is being clicked on
  sf::Vector2i mousePos = sf::Mouse::getPosition();
  Vector2I mapPos; Vector2 mapPosRaw;

  //Check which tile was clicked on
  mousePos = sf::Mouse::getPosition();

  m_pTiledMap->getRawScreenToMapCoords(mousePos.x, mousePos.y,
                                       mapPosRaw.x, mapPosRaw.y);

  m_pTiledMap->getScreenToMapCoords(mousePos.x, mousePos.y,
                                    mapPos.x, mapPos.y);

  //Make sure that there is no obstacle in this place
  if (TERRAIN_TYPE::kObstacle != m_pTiledMap->getType(mapPos.x, mapPos.y)) {

    RTSUnit* attackedUnit = nullptr;

    for (auto it = m_lstUnits.begin(); it != m_lstUnits.end(); ++it) {
      auto search = std::find(m_lsActiveUnits.begin(), m_lsActiveUnits.end(), (*it));
      if (search == m_lsActiveUnits.end() && (*it)->GetCurrentHP() > 0  &&
          Vector2::distSquared((*it)->GetRawPosition(), mapPosRaw) <= 0.4f) {

        attackedUnit = (*it);
        break;
      }
    }

    if (attackedUnit) {
      for each(RTSUnit* unit in m_lsActiveUnits) {
        unit->AttackUnit(attackedUnit);
      }
    }
    else {
      for each(RTSUnit* unit in m_lsActiveUnits) {
        // Make sure it's not the same tile where the 'Active Unit' is already at
        if (unit->GetPosition() != mapPos) {
          unit->ClearTarget();
          unit->GoToPosition(mapPos);
        }
      }
    }
  }
}

void 
RTSWorld::selectAllUnitsIn(Vector2I clickStartPos, Vector2I currClickPos) {
  m_lsActiveUnits.clear();

  for each (RTSUnit* unit in m_lstUnits) {
    if (nullptr != unit && unit->GetCurrentHP() > 0) {

      Vector2I screenPos;
      Vector2 unitPos = unit->GetRawPosition();

      m_pTiledMap->getRawMapToScreenCoords(unitPos.x, unitPos.y, screenPos.x, screenPos.y);

      if (screenPos.x <= Math::max(clickStartPos.x, currClickPos.x) &&
          screenPos.x >= Math::min(clickStartPos.x, currClickPos.x) &&
          screenPos.y <= Math::max(clickStartPos.y, currClickPos.y) &&
          screenPos.y >= Math::min(clickStartPos.y, currClickPos.y)) {
        m_lsActiveUnits.push_back(unit);
      }
    }
  }

  m_activeWalker->Reset();
}

void 
RTSWorld::DestoryUnit(RTSUnit* unit) {
  for (auto it = m_lsActiveUnits.begin(); it != m_lsActiveUnits.end();) {
    if ((*it) == unit) {
      it = m_lsActiveUnits.erase(it);
    }
    else {
      ++it;
    }
  }

  decltype(m_lstUnits.begin()) UnitInList;
  for (auto it = m_lstUnits.begin(); it != m_lstUnits.end(); ++it) {
    if ((*it) == unit) {
      UnitInList = it;
    }
    if ((*it)->GetTarget() == unit) {
      (*it)->ClearTarget();
    }
  }

  ge_delete(unit);
  *UnitInList = nullptr;
}

void
RTSWorld::render() {
  m_pTiledMap->render();
  if (m_activeWalker && m_lsActiveUnits.size() > 0) {
    m_activeWalker->render(m_pTarget);
  }
  for each (auto unit in m_lsActiveUnits) {
    if (nullptr == unit || unit->GetCurrentHP() <= 0) { continue; }

    Vector2 position = unit->GetRawPosition();
    Vector2I screenPos;

    m_pTiledMap->getRawMapToScreenCoords(position.x, position.y,
                                         screenPos.x, screenPos.y);

    m_pActiveUnitCircle->setPosition(float(screenPos.x), float(screenPos.y));
    m_pTarget->draw(*m_pActiveUnitCircle);
  }

  for each (auto unit in m_lstUnits) {
    if (unit) {
      unit->Render();
    }
  }

  for each (auto unit in m_lsActiveUnits) {
    if (nullptr == unit || unit->GetCurrentHP() <= 0) { continue; }

    Vector2 position = unit->GetRawPosition();
    Vector2I screenPos;

    m_pTiledMap->getRawMapToScreenCoords(position.x, position.y,
                                         screenPos.x, screenPos.y);

    m_pHealthBar->Draw(screenPos,
                       unit->GetCurrentHP(),
                       unit->GetMaxHP());
  }
  if (selectionRectEnabled) {
    m_pTarget->draw(*m_pSelectionRect);
  }
}

void
RTSWorld::updateResolutionData() {
  if (nullptr != m_pTiledMap) {
    Vector2I appResolution = g_gameOptions().s_Resolution;
    
    m_pTiledMap->setStart(0, 0);
    m_pTiledMap->setEnd(appResolution.x, appResolution.y - 175);
    
    //This ensures a clamp if necessary
    m_pTiledMap->moveCamera(0, 0);
  }
}

void
RTSWorld::setCurrentWalker(const int8 index) {
  //We check that the Walker exists (in debug mode)
  GE_ASSERT(m_walkersList.size() > static_cast<SIZE_T>(index));
  Vector2I posWalker = Vector2I::ZERO;

  if (m_activeWalker) {
    m_activeWalker->Reset();
  }
  m_activeWalker = m_walkersList[index];
  m_activeWalkerIndex = index;
}

int8 
RTSWorld::getCurrentWalkerState() {
  return static_cast<int8>(m_activeWalker->GetState());
}
