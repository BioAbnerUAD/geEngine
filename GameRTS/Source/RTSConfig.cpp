#include "RTSConfig.h"

Vector2I GameOptions::s_Resolution = Vector2I(1920, 1080);
Vector2 GameOptions::s_MapMovementSpeed = Vector2(1024.0f, 1024.0f);

bool GameOptions::s_MapShowGrid = true;
Color GameOptions::s_MapGridColor = Color(255, 0, 0, 255);

const Vector2I
GameOptions::TILEHALFSIZE = Vector2I(TILESIZE_X >> 1, TILESIZE_Y >> 1);

const Vector2I
GameOptions::BITSHFT_TILESIZE = Vector2I(
  Math::countTrailingZeros(TILESIZE_X),
  Math::countTrailingZeros(TILESIZE_Y)
);

const Vector2I
GameOptions::BITSFHT_TILEHALFSIZE = Vector2I(GameOptions::BITSHFT_TILESIZE.x - 1,
                                             GameOptions::BITSHFT_TILESIZE.y - 1);

bool GameOptions::s_GUIBlockingMouse = false;

const vector< ANSICHAR*> GameOptions::s_pathfinderNames = {
      "Breath First Search",
      "Depth First Search",
      "Best First Search",
      "Dijkstra",
      "AStar"
};

int8 GameOptions::s_CurrentWalkerIndex = static_cast<int8>(s_pathfinderNames.size() - 1);

int32 GameOptions::s_selectedTerrainIndex = 0;

int32 GameOptions::s_unitTypeIndex = 0;

int32 GameOptions::s_brushSize = 1;

int32 GameOptions::s_activeTool = 0;

uint8 GameOptions::s_currentPlayerID = 0;

bool GameOptions::s_drawGridWalkerGizmos = false;

bool GameOptions::s_gridWalkerStepMode = false;

GameOptions&
g_gameOptions() {
  return GameOptions::instance();
}