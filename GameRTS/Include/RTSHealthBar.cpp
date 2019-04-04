#include "RTSHealthBar.h"

#include <SFML/Graphics/RenderTarget.hpp>

RTSHealthBar::RTSHealthBar(sf::RenderTarget& renderTarget) 
  : m_renderTarget(renderTarget) {

  //Set health bar colors
  m_HealthBackground.setFillColor(sf::Color::Red);
  m_CurrentHealth.setFillColor(sf::Color::Green);

  //Set initial sizes
  m_HealthBackground.setSize(sf::Vector2f(40, 3));
  m_CurrentHealth.setSize(sf::Vector2f(40, 3));

  //Set middle points
  m_HealthBackground.setOrigin(sf::Vector2f(20, 1.5f));
  m_CurrentHealth.setOrigin(sf::Vector2f(20, 1.5f));
}


RTSHealthBar::~RTSHealthBar() {}

void
RTSHealthBar::Draw(Vector2 position, uint32 currentHealth, uint32 maxHealth) {
  m_CurrentHealth.setSize(sf::Vector2f(40.f * float(currentHealth) / maxHealth, 3));

  m_HealthBackground.setPosition(position.x, position.y);
  m_CurrentHealth.setPosition(position.x, position.y);

  m_renderTarget.draw(m_HealthBackground);
  m_renderTarget.draw(m_CurrentHealth);
}
