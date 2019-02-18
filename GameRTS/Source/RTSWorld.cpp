#include "RTSWorld.h"
#include "RTSTiledMap.h"

#include "RTSUnitType.h"

#include "RTSBreadthFirstSearchMapGridWalker.h"
#include "RTSDepthFirstSearchMapGridWalker.h"
#include "RTSBestFirstSearchMapGridWalker.h"
#include "RTSDijkstraMapGridWalker.h"

using namespace RTSGame;

RTSWorld::RTSWorld() {
  m_pTiledMap = nullptr;
  m_activeWalkerIndex = -1;	//-1 = Invalid index
}

RTSWorld::~RTSWorld() {
  destroy();
}

bool
RTSWorld::init(sf::RenderTarget* pTarget) {
  GE_ASSERT(nullptr == m_pTiledMap && "World was already initialized");
  destroy();

  m_pTarget = pTarget;

  //Initialize the map (right now it's an empty map)
  m_pTiledMap = ge_new<RTSTiledMap>();
  GE_ASSERT(m_pTiledMap);
  m_pTiledMap->init(m_pTarget, Vector2I(256, 256));

  //Create the path finding classes and push them to the walker list
  m_walkersList.push_back(ge_new<RTSBreadthFirstSearchMapGridWalker>(m_pTiledMap));
  m_walkersList.push_back(ge_new<RTSDepthFirstSearchMapGridWalker>(m_pTiledMap));
  m_walkersList.push_back(ge_new<RTSBestFirstSearchMapGridWalker>(m_pTiledMap));
  m_walkersList.push_back(ge_new<RTSDijkstraMapGridWalker>(m_pTiledMap));

  //Init the walker objects

  for (SIZE_T it = 0; it < m_walkersList.size(); ++it) {
    m_walkersList[it]->init();
  }

  //Set the first walker as the active walker
  setCurrentWalker(m_walkersList.size() > 0 ? 0 : -1);

  RTSGame::RTSUnitType unitTypes;
  unitTypes.loadAnimationData(m_pTarget, 1);

  return true;
}

void
RTSWorld::destroy() {
 //Destroy all the walkers
  while (m_walkersList.size() > 0) {
    ge_delete(m_walkersList.back());
    m_walkersList.pop_back();
  }

  //As the last step, destroy the full map
  if (nullptr != m_pTiledMap) {
    ge_delete(m_pTiledMap);
    m_pTiledMap = nullptr;
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

  if (m_activeWalker)
  {
    if (m_activeWalker->GetState() == GRID_WALKER_STATE::kSearching) {
      m_activeWalker->StepSearch();
    }
    else if (m_activeWalker->GetState() == GRID_WALKER_STATE::kBacktracking) {
      m_activeWalker->StepBacktrack();
    }
  }
}

void
RTSWorld::queryLeftClickEvent() {

  sf::Vector2i mousePos;
  Vector2I mapPos;
  int8 clickedTileType;

  //This cycles the type of terrain on the tile that is being clicked on
  if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
    //Check which tile was clicked on
    mousePos = sf::Mouse::getPosition();

    m_pTiledMap->getScreenToMapCoords(static_cast<int32>(mousePos.x),
                                      static_cast<int32>(mousePos.y),
                                      mapPos.x, mapPos.y);

    clickedTileType = static_cast<int8>(GameOptions::s_selectedTerrainIndex);
    m_pTiledMap->setType(mapPos.x, mapPos.y, clickedTileType);
    m_pTiledMap->setCost(mapPos.x, mapPos.y, TERRAIN_TYPE::ECost[clickedTileType]);
  }
}

void 
RTSWorld::queryRightClickEvent() {
  sf::Vector2i mousePos = sf::Mouse::getPosition();
  Vector2I mapPos;

  //This moves the 'Active Walker' to the tile that is being clicked on
  if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
    //Check which tile was clicked on
    mousePos = sf::Mouse::getPosition();

    m_pTiledMap->getScreenToMapCoords(static_cast<int32>(mousePos.x),
                                      static_cast<int32>(mousePos.y),
                                      mapPos.x, mapPos.y);

    //Make sure that there is no obstacle in this place
    if (TERRAIN_TYPE::kObstacle != m_pTiledMap->getType(mapPos.x, mapPos.y)) {

      if (GameOptions::s_MoveWalkerOrTarget) {
        // Make sure it's not the same tile where the Target is already at
        if (m_activeWalker->GetTargetPos() != mapPos) {
          //move the 'Active Walker'
          m_activeWalker->SetPosition(mapPos);
        }
      }
      else {
        // Make sure it's not the same tile where the 'Active Walker' is already at
        if (m_activeWalker->GetTargetPos() != mapPos) {
          //move the Target
          m_activeWalker->SetTargetPos(mapPos);
        }
      }

    }
  }
}

void
RTSWorld::StartSearch() {
  //Start Search in step mode
  m_activeWalker->StartSeach(true);
}

void
RTSWorld::render() {
  m_pTiledMap->render();
  if (m_activeWalker) {
    m_activeWalker->render(m_pTarget);
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
  Vector2I posTarget = Vector2I::ZERO;

  if (m_activeWalker) {
    m_activeWalker->Reset();

    // Save positions for replacement walker to be at the same place
    posWalker = m_activeWalker->GetPosition();
    posTarget = m_activeWalker->GetTargetPos();
  }
  m_activeWalker = m_walkersList[index];
  m_activeWalkerIndex = index;

  // Put new walker at the same place as old one
  m_activeWalker->SetPosition(posWalker);
  m_activeWalker->SetTargetPos(posTarget);
}
