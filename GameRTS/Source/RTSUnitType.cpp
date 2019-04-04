#include <gePrerequisitesUtil.h>
#include <geFileSystem.h>
#include <geDataStream.h>
#include <Externals/json.hpp>
#include "RTSUnitType.h"

#include "RTSWorld.h"
#include "RTSTiledMap.h"

using namespace geEngineSDK;
using nlohmann::json;

namespace RTSGame {
  RTSUnitType::RTSUnitType() {}

  RTSUnitType::~RTSUnitType() {}

  void
  RTSUnitType::loadAnimationData(sf::RenderTarget* pTarget, uint32 idUnitType) {
    struct tmpStruct
    {
      uint32 id;
      Map<String, uint32> animation;
    };

    Path filePath = "RTS/assets/game_objects/units/";
    Path jsonPath = filePath;
    jsonPath += toString(idUnitType) + ".json";
    DataStreamPtr fData = FileSystem::openFile(jsonPath);
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
    m_id = idUnitType;

    for (auto& unit : unitsMap) {
      if (1 == unit.second.id) {
        m_name = unit.first;
        m_animationFrames.resize(unit.second.animation.size());

        uint32 count = 0;
        for (auto& animation : unit.second.animation) {
          Animation& locAnim = m_animationFrames[count];
          locAnim.name = animation.first;
          locAnim.numFrames = animation.second;
          locAnim.duration = 1.0f; //Seconds

          for (uint32 i = 0; i < DIRECTIONS::kNUM_DIRECTIONS; ++i) {
            locAnim.frames[i].resize(locAnim.numFrames);
            
            for (uint32 j = 0; j < locAnim.numFrames; ++j) {
              StringStream frameName;
              String fullKey;
              frameName << m_name << "/" << locAnim.name << "_";
              frameName << dirSubIndex[i] << "/";
              frameName << locAnim.name << dirSubIndex[i];
              frameName << std::setfill('0') << std::setw(4) << (j+1);
              frameName << ".png";
              frameName >> fullKey;

              auto thisFrame = frames[fullKey.c_str()]["frame"];
              locAnim.frames[i][j].x = thisFrame["x"].get<int32>();
              locAnim.frames[i][j].y = thisFrame["y"].get<int32>();
              locAnim.frames[i][j].w = thisFrame["w"].get<int32>();
              locAnim.frames[i][j].h = thisFrame["h"].get<int32>();
              locAnim.frames[i][j].bSwap = invertedSubIndex[i];
            }
          }
          ++count;
        }
        break;
      }
    }

    //Load the texture for this unit type
    m_pTarget = pTarget;
    m_texture.loadFromFile(pTarget, filePath.toString() + "units.png");
  }

  int32
  RTSUnitType::GetAnimIndex(ANIMATIONS::E activeAnim) const {
    for (int32 i = 0; i < m_animationFrames.size(); ++i) {
    	if (m_animationFrames[i].name == ANIMATIONS::Name[activeAnim]) {
        return i;
    	}
    }
    return -1;
  }

  float
  RTSUnitType::GetAnimDuration(ANIMATIONS::E activeAnim) const {
    return m_animationFrames[GetAnimIndex(activeAnim)].duration;
  }

  void 
  RTSUnitType::Render(ANIMATIONS::E activeAnim, 
                      DIRECTIONS::E direction, 
                      const Vector2 & position, 
                      float m_animTime) {
    Vector2 screenPos;
    Vector2I iScreenPos, iPosition(Math::round(position.x), Math::round(position.y));
    Vector2I mapSize = RTSWorld::instance().getTiledMap()->getMapSize();

    iPosition.x = Math::clamp(iPosition.x, 0, mapSize.x - 1);
    iPosition.y = Math::clamp(iPosition.y, 0, mapSize.y - 1);

    RTSWorld::instance().getTiledMap()->getMapToScreenCoords(iPosition.x,
                                                             iPosition.y,
                                                             iScreenPos.x,
                                                             iScreenPos.y);

    screenPos.x = iScreenPos.x + (position.x - iPosition.x) * TILESIZE_X;
    screenPos.y = iScreenPos.y + (position.y - iPosition.y) * TILESIZE_Y;

    int32 animIndex = GetAnimIndex(activeAnim);

    int32 numFrame = Math::floor(m_animTime / m_animationFrames[animIndex].duration
                                 * m_animationFrames[animIndex].numFrames);

    if (numFrame < int32(m_animationFrames[animIndex].numFrames)) {
      auto frame = m_animationFrames[animIndex].frames[direction][numFrame];

      m_texture.setPosition(screenPos.x + HALFTILESIZE_X, screenPos.y + HALFTILESIZE_Y);
      m_texture.setSrcRect(frame.x, frame.y, frame.w, frame.h);
      m_texture.setOrigin(frame.w / 2, frame.h);
      m_texture.setScale(frame.bSwap ? -1.f : 1.f, 1.f);

      m_texture.draw();
    }
  }
}
