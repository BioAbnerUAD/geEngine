#pragma once
#include <gePrerequisitesUtil.h>

#include <geVector2.h>

#include <SFML/Graphics/RectangleShape.hpp>

using namespace geEngineSDK;

class RTSHealthBar
{
public:
  RTSHealthBar(sf::RenderTarget& renderTarget);
  ~RTSHealthBar();

  void 
  Draw(Vector2I position, uint32 currentHealth, uint32 maxHealth);

  sf::RectangleShape m_CurrentHealth;
  sf::RectangleShape m_HealthBackground;

  sf::RenderTarget& m_renderTarget;
};