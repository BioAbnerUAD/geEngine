#include "RTSUnit.h"

#include "geTime.h"
#include "geRandom.h"

#include "RTSConfig.h"
#include "RTSWorld.h"

#include "RTSTiledMap.h"

using namespace RTSGame;

RTSUnit::RTSUnit(SIZE_T playerID,
                 RTSGame::RTSUnitType* pUnitType,
                 Vector2I initPos,
                 RTSMapGridWalker** lpGridWalker)
  : m_playerID(playerID), 
    m_pUnitType(pUnitType), 
    m_position(initPos),
    m_pGridWalker(lpGridWalker),
    m_animTime(0.f) {

  static Random rand = Random();
  rand.setRandomSeed();

  m_lookDirection = static_cast<RTSGame::DIRECTIONS::E>(
    rand.getRange(0, RTSGame::DIRECTIONS::kNUM_DIRECTIONS - 1)
  );

  m_maxHP = m_currentHP = RTSGame::UNIT_TYPES::health[pUnitType->GetID() - 1];
  m_damagePerSecond = RTSGame::UNIT_TYPES::damagePerSecond[pUnitType->GetID() - 1];
  m_range = RTSGame::UNIT_TYPES::range[pUnitType->GetID() - 1];

  m_currentAnim = ANIMATIONS::kIDLE;
}


RTSUnit::~RTSUnit() {}

void
RTSUnit::GoToPosition(const Vector2I& position) {
  Vector2I iPosition = Vector2I(Math::round(m_position.x),
                                Math::round(m_position.y));

  (*m_pGridWalker)->GetPath(iPosition,
                            position,
                            &m_pathToFollow,
                            GameOptions::s_gridWalkerStepMode);
}

void
RTSUnit::AttackUnit(RTSUnit* target) {

  m_target = target;
  GoToPosition(Vector2I(Math::round(m_target->m_position.x),
                        Math::round(m_target->m_position.y)));
}

void
RTSUnit::Update(float deltaTime) {
  if (m_currentHP <= 0) {
    if (m_currentAnim != ANIMATIONS::kDIE) {
      m_currentAnim = ANIMATIONS::kDIE;
      m_animTime = 0;
    }
    if (m_animTime >= 1.f) {
      RTSWorld::instance().DestoryUnit(this);
    }
  }
  else if (m_target) {
    if (m_target->GetCurrentHP() <= 0) {
      m_target = nullptr;
      m_pathToFollow.clear();
    }
    else if (Vector2::distance(m_target->m_position, m_position) <= m_range) {
      m_currentAnim = ANIMATIONS::kATTACK;
      m_lookDirection = DIRECTIONS::vectorToDirection(m_target->m_position - m_position);
    }
    else if (!m_pathToFollow.empty()) {
      FollowPath(deltaTime);
    }
    else {
      GoToPosition(Vector2I(Math::round(m_target->m_position.x),
                            Math::round(m_target->m_position.y)));
    }
  }
  else if (!m_pathToFollow.empty()) {
    FollowPath(deltaTime);
  }
  else {
    m_currentAnim = ANIMATIONS::kIDLE;
  }
}

void
RTSUnit::FollowPath(float deltaTime) {
  Vector2 node(static_cast<float>(m_pathToFollow.back().x),
                 static_cast<float>(m_pathToFollow.back().y));

  if (m_pathToFollow.size() == 1) {
    if (Vector2::distance(m_position, node) <= 0.1f) {
      m_pathToFollow.pop_back();
    }
  }
  else if (Vector2::distance(m_position, node) <= 0.6f) {
    m_pathToFollow.pop_back();
  }

  Vector2 dir = (node - m_position).getSafeNormal();
  m_lookDirection = DIRECTIONS::vectorToDirection(dir);
  m_currentAnim = ANIMATIONS::kRUN;
  m_position += dir * deltaTime;
}

void
RTSUnit::Render() {
  m_animTime += Time::instance().getFrameDelta();

  float duration = m_pUnitType->GetAnimDuration(m_currentAnim);

  if (m_currentAnim == ANIMATIONS::kDIE) {
    if (m_animTime >= duration) {
      m_animTime = duration;
    }
  }
  else {
    while (m_animTime >= duration) {
      if (m_currentAnim == ANIMATIONS::kATTACK &&  m_target) {
        if (m_target->m_currentHP > 0) {
          m_target->m_currentHP -= m_damagePerSecond;
        }
      }
      m_animTime -= duration;
    }
  }

  m_pUnitType->Render(m_currentAnim,
                      m_lookDirection,
                      m_position,
                      m_animTime);
}

Vector2I 
RTSUnit::GetPosition() const {
  Vector2I mapSize = RTSWorld::instance().getTiledMap()->getMapSize();
  return Vector2I(Math::clamp(Math::round(m_position.x), 0, mapSize.x - 1),
                  Math::clamp(Math::round(m_position.y), 0, mapSize.y - 1));
}
