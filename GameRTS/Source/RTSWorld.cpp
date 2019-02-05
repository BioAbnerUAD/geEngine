#include "RTSWorld.h"
#include "RTSTiledMap.h"

#include "RTSUnitType.h"

#include "RTSBreadthFirstSearchMapGridWalker.h"

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

  //Init the walker objects

  for (SIZE_T it = 0; it < m_walkersList.size(); ++it) {
    m_walkersList[it]->init();
  }

  //Set the first walker as the active walker
  setCurrentWalker(m_walkersList.size() > 0 ? 0 : -1);

  m_activeWalker->SetPosition(Vector2I(0, 0));
  m_activeWalker->SetTargetPos(Vector2I(0, 1));

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

  queryLeftClickEvent();
  queryRightClickEvent();

  if (m_activeWalker)
  {
    if (m_activeWalker->IsSearching() && !m_activeWalker->HasFoundPath()) {
      StepSearch();
    }
  }
}

void
RTSWorld::queryLeftClickEvent() {
  //TODO:(Abner) find some place decent to put these so they're not static
  static const Vector2I nullMapPos = Vector2I(-1, -1);
  static Vector2I lastClickedMapPos = nullMapPos;

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

    //Make sure it's not the same tile that was already processed
    if (lastClickedMapPos != mapPos) {
      //Cycle the terrain types of the clicked tile
      clickedTileType = m_pTiledMap->getType(mapPos.x, mapPos.y);

      if (TERRAIN_TYPE::kObstacle == clickedTileType) {
        clickedTileType = TERRAIN_TYPE::kGrass;
      }
      else
      {
        clickedTileType = TERRAIN_TYPE::kObstacle;
      }

      m_pTiledMap->setType(mapPos.x, mapPos.y, clickedTileType);

      //mark this tile as already processed
      lastClickedMapPos = mapPos;
    }
  }
  else if (nullMapPos != lastClickedMapPos) {
    //reset processed tile
    lastClickedMapPos = nullMapPos;
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

    //Make sure it's not the same tile where the 'Active Walker' is already at
    if (m_activeWalker->GetPosition() != mapPos) {
      //move the 'Active Walker'
      m_activeWalker->SetTargetPos(mapPos);
    }
  }
}

void
RTSWorld::StartSearch() {
  //Start Search in step mode
  m_activeWalker->StartSeach(true);
}

void
RTSWorld::StepSearch() {
  m_activeWalker->StepSearch();
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
  //Revisamos que el walker exista (en modo de debug)
  GE_ASSERT(m_walkersList.size() > static_cast<SIZE_T>(index));

  m_activeWalker = m_walkersList[index];
  m_activeWalkerIndex = index;
}
