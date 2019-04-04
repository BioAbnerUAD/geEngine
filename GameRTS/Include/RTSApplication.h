#pragma once

#include "RTSTexture.h"
#include "RTSWorld.h"

using namespace geEngineSDK;

namespace sf{
  class RenderWindow;
  class Font;
}

class RTSApplication
{
 public:
  RTSApplication();
  virtual ~RTSApplication();

  int32
  run();

  sf::RenderWindow*
  getRenderWindow() {
    return m_window;
  }

  RTSWorld*
  getWorld() {
    return &RTSWorld::instance();
  }

  float getFPS() const {
    return m_framesPerSecond;
  }

 protected:

 private:
   void
   initSystems();

   void
   initGUI();

   void
   destroySystems();

   void
   gameLoop();

   void
   postInit();

   void
   postDestroy();

   void
   updateFrame();
   
   void
   renderFrame();

 private:
   sf::RenderWindow* m_window;
   sf::Font* m_arialFont;

   float m_fpsTimer;
   float m_fpsCounter;
   float m_framesPerSecond;
};
