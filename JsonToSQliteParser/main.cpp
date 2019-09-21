#include <gePrerequisitesUtil.h>
#include <geFileSystem.h>
#include <geDataStream.h>
#include <Externals/json.hpp>

#include <iostream>

using nlohmann::json;
using namespace geEngineSDK;

namespace DIRECTIONS
{
  enum E
  {
    kN = 0,
    kNW,
    kW,
    kSW,
    kS,
    kSE,
    kE,
    kNE,
    kNUM_DIRECTIONS
  };
}

struct AnimationFrame
{
  int32 x;
  int32 y;
  int32 w;
  int32 h;
  bool bSwap;
};

struct Animation
{
  uint32 numFrames;
  float duration;
  String name;
  Vector<AnimationFrame> frames[DIRECTIONS::kNUM_DIRECTIONS];
};

int main() {
  ////////////HARDCODE_FTW/////////////////
  uint32 idUnitType = 1;

  ////////////////////////////////////////

  struct tmpStruct
  {
    uint32 id;
    Map<String, uint32> animation;
  };

  Path filePath = "RTS/assets/game_objects/units/1.json";
  DataStreamPtr fData = FileSystem::openFile(filePath);
  auto myJSON = json::parse(fData->getAsString());

  auto& frames = myJSON["frames"];
  Map<String, tmpStruct> unitsMap;
  Map<String, uint32> animsMap;

  for (auto iter = frames.begin(); iter != frames.end(); ++iter) {
    Vector<String> parsedKey = StringUtil::split(iter.key().c_str(), "/");
    Vector<String> parsedAnim = StringUtil::split(parsedKey[1], "_");

    //Categorize all the animations in the file
    tmpStruct& tmpObj = unitsMap[parsedKey[0]];
    tmpObj.id = static_cast<uint32>(unitsMap.size());
    if (tmpObj.animation.end() == tmpObj.animation.find(parsedAnim[0])) {
      tmpObj.animation[parsedAnim[0]] = 0;
    }

    //Add a frame only on one direction to avoid repetition
    if (0 == StringUtil::compare(parsedAnim[1], String("N"))) {
      tmpObj.animation[parsedAnim[0]]++;
    }
  }

  String dirSubIndex[DIRECTIONS::kNUM_DIRECTIONS] = {
    "N", "NW", "W", "SW", "S", "SW", "W", "NW"
  };

  bool invertedSubIndex[DIRECTIONS::kNUM_DIRECTIONS] = {
    false, false, false, false, false, true, true, true
  };

  //Read the information pertinent from the main json and copy to the class
  std::cout << "ID: " << idUnitType << std::endl; //m_id = idUnitType;

  for (auto& unit : unitsMap) {
    if (1 == unit.second.id) {
      std::cout << "\tName: " << unit.first << std::endl; auto m_name = unit.first;
      //m_animationFrames.resize(unit.second.animation.size());

      uint32 count = 0;
      for (auto& animation : unit.second.animation) {
        Animation locAnim;
        std::cout << "\t\tAnimation: " << animation.first << std::endl; locAnim.name = animation.first;
        std::cout << "\t\tNum Frames: " << animation.second << std::endl; locAnim.numFrames = animation.second;

        for (uint32 i = 0; i < DIRECTIONS::kNUM_DIRECTIONS; ++i) {
          locAnim.frames[i].resize(locAnim.numFrames);

          for (uint32 j = 0; j < locAnim.numFrames; ++j) {
            StringStream frameName;
            String fullKey;
            frameName << m_name << "/" << locAnim.name << "_";
            frameName << dirSubIndex[i] << "/";
            frameName << locAnim.name << dirSubIndex[i];
            frameName << std::setfill('0') << std::setw(4) << (j + 1);
            frameName << ".png";
            frameName >> fullKey;
            std::cout << "\t\t\t" << fullKey <<":";

            auto thisFrame = frames[fullKey.c_str()]["frame"];
            std::cout << "x:" << thisFrame["x"].get<int32>() << "\t";
            std::cout << "y:" << thisFrame["y"].get<int32>() << "\t";
            std::cout << "w:" << thisFrame["w"].get<int32>() << "\t";
            std::cout << "h:" << thisFrame["h"].get<int32>() << "\t";
            std::cout << "bSwap:" << invertedSubIndex[i] << "\t";
            std::cout << std::endl;
          }
        }
        ++count;
      }
      break;
    }
  }

  //Load the texture for this unit type
  //m_pTarget = pTarget;
  //m_playerID = playerID;
  //m_texture.loadFromFile(pTarget, filePath.toString() + "units-" +
  //                       toString(playerID + 1) + ".png");

  return 0;
}