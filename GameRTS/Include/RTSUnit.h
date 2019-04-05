#pragma once

#include "RTSMapGridWalker.h"
#include "RTSUnitType.h"

using namespace geEngineSDK;

class RTSUnit
{
public:
  RTSUnit(SIZE_T playerID, 
          RTSGame::RTSUnitType* pUnitType, 
          Vector2 initPos, 
          RTSMapGridWalker** lpGridWalker);

  ~RTSUnit();

  void
  GoToPosition(const Vector2I& position);

  void 
  AttackUnit(RTSUnit* other);

  void 
  Update(float deltaTime);

  void
  Render();

  RTSUnit*
  GetTarget() {
    return m_target;
  }

  void
  ClearTarget() {
    m_target = nullptr;
  }

  Vector2I 
  GetPosition() const;

  Vector2 
  GetRawPosition() const { 
    return m_position;
  }

  const RTSGame::RTSUnitType* 
  GetUnitType() const { return m_pUnitType; }

  uint8
  GetPlayerID() const { return m_pUnitType->GetPlayerID(); }

  uint32 
  GetMaxHP() const { return m_maxHP; }

  int32 
  GetCurrentHP() const { return m_currentHP; }

  uint32 
  GetDPS() const { return m_damagePerSecond; }

  float 
  GetAttackRange() const { return m_range; }

private:
  void 
  FollowPath(float deltaTime);

  SIZE_T m_playerID;
  Vector2 m_position;

  uint32 m_maxHP;
  int32 m_currentHP;

  uint32 m_damagePerSecond;
  float m_range;
  RTSUnit* m_target = nullptr;

  Vector<Vector2I> m_pathToFollow;

  RTSGame::RTSUnitType* m_pUnitType;
  RTSGame::DIRECTIONS::E m_lookDirection;
  RTSGame::ANIMATIONS::E m_currentAnim;

  RTSMapGridWalker** m_pGridWalker;
  float m_animTime;
};