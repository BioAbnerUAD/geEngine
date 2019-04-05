#pragma once

//Define only if the map should be isometric
//#define MAP_IS_ISOMETRIC

#ifdef MAP_IS_ISOMETRIC
//Tiles pixel size
# define TILESIZE_X 64
# define TILESIZE_Y 32
#else
//Tiles pixel size
# define TILESIZE_X 64
# define TILESIZE_Y 64
#endif

#define HALFTILESIZE_X GameOptions::TILEHALFSIZE.x
#define HALFTILESIZE_Y GameOptions::TILEHALFSIZE.y

#include <geModule.h>
#include <geVector2I.h>
#include <geVector2.h>
#include <geColor.h>

using namespace geEngineSDK;

namespace RTSTools
{
  enum E
  {
    kTerrain = 0,
    kPlaceUnit,
    kMoveUnit,
    kNumTools
  };
}

class GameOptions : public Module<GameOptions>
{
 public:
  //Application values
  static Vector2I s_Resolution;
  static Vector2 s_MapMovementSpeed;

  //World Values

  //Map Values
  static bool s_MapShowGrid;
  static Color s_MapGridColor;

  //Global Constants
  static const Vector2I TILEHALFSIZE;
  static const Vector2I BITSFHT_TILEHALFSIZE;
  static const Vector2I BITSHFT_TILESIZE;

  static bool s_GUIBlockingMouse;

  static int8 s_CurrentWalkerIndex;

  static bool s_gridWalkerStepMode;

  static const vector< ANSICHAR*> s_pathfinderNames;

  static int32 s_selectedTerrainIndex;

  static uint8 s_currentPlayerID;
  static int32 s_unitTypeIndex;

  static int32 s_brushSize;
  static int32 s_activeTool;
  static bool s_drawGridWalkerGizmos;
};

GameOptions&
g_gameOptions();
