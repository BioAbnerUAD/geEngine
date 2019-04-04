#pragma once

#include <geBox2D.h>

#include "RTSConfig.h"
#include "RTSTexture.h"

namespace RTSGame {
  using geEngineSDK::Box2D;

  namespace UNIT_TYPES
  {
    enum E
    {
      kARCHER = 0,
      kKNIGHT = 1,
      kPIKEMAN = 2,
      kNUM_UNIT_TYPES
    };

    static float range[] = {
      3.0f,
      0.6f,
      1.0f
    };

    static uint32 health[] = {
      60,
      140,
      100
    };

    static int32 damagePerSecond[] = {
      5,
      10,
      15
    };
  }

  namespace ANIMATIONS {
    enum E {
      kIDLE = 0,
      kRUN,
      kATTACK,
      kDIE,
      kNUM_ANIMATIONS
    };

    static String Name[] = {
      "idle",
      "run",
      "attack",
      "die"
    };
  }

  namespace DIRECTIONS {
    enum E {
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

    static DIRECTIONS::E 
    vectorToDirection(Vector2 vector) {
      Radian angle = Math::atan2(-vector.x, -vector.y);

      #ifdef MAP_IS_ISOMETRIC
      angle -= Radian(Math::HALF_PI / 2.f);
      #endif

      angle.unwindRadians();
      int32 octant = Math::round(8 * angle.valueRadians() / Math::TWO_PI + 8) % 8;

      return static_cast<DIRECTIONS::E>(octant);
    }
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

  class RTSUnitType
  {
  public:
     RTSUnitType();
     ~RTSUnitType();

     /*static RTSUnitType*
     loadFromFile(uint32 idUnitType);*/

     void
     loadAnimationData(sf::RenderTarget* pTarget, uint32 idUnitType);

     int32 
     GetAnimIndex(ANIMATIONS::E activeAnim) const;

     float 
     GetAnimDuration(ANIMATIONS::E activeAnim) const;

     void
     Render(ANIMATIONS::E activeAnim,
            DIRECTIONS::E direction,
            const Vector2& position,
            float m_animTime);

     uint32
     GetID() const {
       return m_id;
     }

     String
     GetName() const {
       return m_name;
     }

   private:
     uint32 m_id;
     String m_name;

     Vector<Animation> m_animationFrames;
     RTSTexture m_texture;
     sf::RenderTarget* m_pTarget;
  };
}
