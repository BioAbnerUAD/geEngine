#include <gePrerequisitesUtil.h>

#if GE_PLATFORM == GE_PLATFORM_WIN32
# include <Win32/geWin32Windows.h>
#endif

#include <geRTTIPlainField.h>
#include <geException.h>
#include <geMath.h>

#include <geCrashHandler.h>
#include <geDynLibManager.h>
#include <geFileSystem.h>
#include <geTime.h>
#include <geUnicode.h>

#include <SFML/Graphics.hpp>

#include <commdlg.h>
#include <imgui.h>
#include <imgui-sfml.h>

#include "RTSConfig.h"
#include "RTSApplication.h"
#include "RTSTiledMap.h"
#include "RTSMapGridWalker.h"
#include "RTSUnit.h"

void
loadMapFromFile(RTSApplication* pApp);

void
mainMenu(RTSApplication* pApp);

RTSApplication::RTSApplication()
  : m_window(nullptr),
    m_fpsTimer(0.0f),
    m_fpsCounter(0.0f),
    m_framesPerSecond(0.0f)
{}

RTSApplication::~RTSApplication() {}

int32
RTSApplication::run() {
  CrashHandler::startUp();
  DynLibManager::startUp();
  Time::startUp();
  GameOptions::startUp();

  __try {
    initSystems();
    gameLoop();
    destroySystems();
  }
  __except (g_crashHandler().reportCrash(GetExceptionInformation())) {
    PlatformUtility::terminate(true);
  }

  GameOptions::shutDown();
  Time::shutDown();
  DynLibManager::shutDown();
  CrashHandler::shutDown();

  return 0;
}

void
RTSApplication::initSystems() {
  if (nullptr != m_window) {  //Window already initialized
    return; //Shouldn't do anything
  }

  //Create the application window
  m_window = ge_new<sf::RenderWindow>(sf::VideoMode(GameOptions::s_Resolution.x,
                                                    GameOptions::s_Resolution.y),
                                      "RTS Game",
                                      sf::Style::Fullscreen);
  if (nullptr == m_window) {
    GE_EXCEPT(InvalidStateException, "Couldn't create Application Window");
  }

  m_arialFont = ge_new<sf::Font>();
  if (nullptr == m_arialFont) {
    GE_EXCEPT(InvalidStateException, "Couldn't create a Font");
  }
  
  
  if (!m_arialFont->loadFromFile("Fonts/arial.ttf")) {
    GE_EXCEPT(FileNotFoundException, "Arial font not found");
  }

  //m_window->setVerticalSyncEnabled(true);

  initGUI();
}

void
RTSApplication::initGUI() {
  ImGui::SFML::Init(*m_window);
}

void
RTSApplication::destroySystems() {
  ImGui::SFML::Shutdown();

  if (nullptr != m_window) {
    m_window->close();
    ge_delete(m_window);
  }

  if (nullptr != m_arialFont) {
    ge_delete(m_arialFont);
  }
}

void
RTSApplication::gameLoop() {
  if (nullptr == m_window) {  //Windows not yet initialized
    return; //Shouldn't do anything
  }

  postInit();

  while (m_window->isOpen()) {
    sf::Event event;
    while (m_window->pollEvent(event)) {
      ImGui::SFML::ProcessEvent(event);
      
      if (event.type == sf::Event::Closed) {
        m_window->close();
      }
    }

    g_time()._update();
    ge_frame_mark();
    updateFrame();
    renderFrame();
    ge_frame_clear();
  }

  postDestroy();
}

void
RTSApplication::updateFrame() {
  float deltaTime = g_time().getFrameDelta();
  
  m_fpsTimer += deltaTime;
  if (1.0f < m_fpsTimer) {
    m_framesPerSecond = m_fpsCounter;
    m_fpsCounter = 0.0f;
    m_fpsTimer = 0.0f;
  }
  m_fpsCounter += 1.0f;

  //Update the interface
  ImGui::SFML::Update(*m_window, sf::seconds(deltaTime));

  //Begin the menu 
  mainMenu(this);

  //Check for camera movement
  Vector2 axisMovement(FORCE_INIT::kForceInitToZero);
  Vector2I mousePosition;
  mousePosition.x = sf::Mouse::getPosition(*m_window).x;
  mousePosition.y = sf::Mouse::getPosition(*m_window).y;

  if (0 == mousePosition.x ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
#ifdef MAP_IS_ISOMETRIC
    axisMovement += Vector2(-1.f, 1.f);
#else
    axisMovement += Vector2(-1.f, 0.f);
#endif
  }
  if (GameOptions::s_Resolution.x - 1 == mousePosition.x ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
#ifdef MAP_IS_ISOMETRIC
    axisMovement += Vector2(1.f, -1.f);
#else
    axisMovement += Vector2(1.f, 0.f);
#endif
  }
  if (0 == mousePosition.y ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
#ifdef MAP_IS_ISOMETRIC
    axisMovement += Vector2(-1.f, -1.f);
#else
    axisMovement += Vector2(0.f, -1.f);
#endif
  }
  if (GameOptions::s_Resolution.y - 1 == mousePosition.y ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::S) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
#ifdef MAP_IS_ISOMETRIC
    axisMovement += Vector2(1.f, 1.f);
#else
    axisMovement += Vector2(0.f, 1.f);
#endif
  }

  axisMovement *= GameOptions::s_MapMovementSpeed * deltaTime;

  getWorld()->getTiledMap()->moveCamera(axisMovement.x, axisMovement.y);

  //Update the world
  getWorld()->update(deltaTime);
}

void
RTSApplication::renderFrame() {
  m_window->clear(sf::Color::Blue);

  getWorld()->render();

  ImGui::SFML::Render(*m_window);

  /*
  sf::Text text;
  text.setPosition(0.f, 30.f);
  text.setFont(*m_arialFont);
  text.setCharacterSize(24);
  text.setFillColor(sf::Color::Red);
  text.setString( toString(1.0f/g_time().getFrameDelta()).c_str() );
  m_window->draw(text);
  */
  m_window->display();
}

void
RTSApplication::postInit() {
  RTSWorld::startUp();
  getWorld()->init(m_window);
  getWorld()->updateResolutionData();
}

void
RTSApplication::postDestroy() {
  getWorld()->destroy();
  RTSWorld::shutDown();
}

void
loadMapFromFile(RTSApplication* pApp) {
  OPENFILENAMEW ofn = { 0 };

  WString fileName;
  fileName.resize(MAX_PATH);
  bool bMustLoad = false;

  Path currentDirectory = FileSystem::getWorkingDirectoryPath();

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = nullptr;
  ofn.lpstrDefExt = L".bmp";
  ofn.lpstrFilter = L"Bitmap File\0*.BMP\0All\0*.*\0";
  ofn.lpstrInitialDir = L"Maps\\";
  ofn.lpstrFile = &fileName[0];
  ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile = MAX_PATH;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  if (GetOpenFileNameW(&ofn)) {
    if (fileName.size() > 0) {
      bMustLoad = true;
    }
  }

  SetCurrentDirectoryW(UTF8::toWide(currentDirectory.toString()).c_str());

  if (bMustLoad) {
    pApp->getWorld()->getTiledMap()->loadFromImageFile(pApp->getRenderWindow(),
                                                       toString(fileName));
  }
}

void
mainMenu(RTSApplication* pApp) {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("Map")) {
      if (ImGui::MenuItem("Load...", "CTRL+O")) {
        loadMapFromFile(pApp);
      }
      if (ImGui::MenuItem("Save...", "CTRL+S")) {

      }
      ImGui::Separator();

      if (ImGui::MenuItem("Quit", "CTRL+Q")) {
        pApp->getRenderWindow()->close();
      }

      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  ImGui::Begin("Game Options");
  {
    ImGui::Text("Framerate: %f", pApp->getFPS());

    ImGui::SliderFloat("Map movement speed X",
      &GameOptions::s_MapMovementSpeed.x,
      0.0f,
      10240.0f);
    ImGui::SliderFloat("Map movement speed Y",
      &GameOptions::s_MapMovementSpeed.y,
      0.0f,
      10240.0f);

    ImGui::Checkbox("Show grid", &GameOptions::s_MapShowGrid);
  }
  ImGui::End();

  ImGui::Begin("Tool");
  {
    ImGui::RadioButton("Edit Terrain", &GameOptions::s_activeTool, RTSTools::kTerrain);
    ImGui::SameLine();
    ImGui::RadioButton("Place Unit", &GameOptions::s_activeTool, RTSTools::kPlaceUnit);
    ImGui::SameLine();
    ImGui::RadioButton("Move Unit", &GameOptions::s_activeTool, RTSTools::kMoveUnit);

    String player = toString(GameOptions::s_currentPlayerID + 1);
    if (ImGui::BeginCombo("Current Player", player.c_str())) {
      for (int8 n = 0; n < 2; n++) {
        String nString = toString(n + 1);
        bool is_selected = (player == nString);
        if (ImGui::Selectable(nString.c_str(), is_selected)) {
          player = nString;
          GameOptions::s_currentPlayerID = n;
        }
        if (is_selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }

    ImGui::Spacing(); ImGui::Spacing();

    if (GameOptions::s_activeTool == RTSTools::kTerrain) {
      ImGui::Text("Terrain Type");

      for (int8 i = 0; i < TERRAIN_TYPE::kNumObjects; i++) {
        ImGui::RadioButton(TERRAIN_TYPE::ES[i].c_str(),
                           &GameOptions::s_selectedTerrainIndex,
                           static_cast<TERRAIN_TYPE::E> (i));
      }

      ImGui::SliderInt("Brush Size", &GameOptions::s_brushSize, 1, 10);
    }
    else if (GameOptions::s_activeTool == RTSTools::kPlaceUnit){
      ImGui::Text("Unit Type");

      ImGui::RadioButton("Archer", &GameOptions::s_unitTypeIndex, 0);
      ImGui::RadioButton("Knight", &GameOptions::s_unitTypeIndex, 1);
      ImGui::RadioButton("Pikeman", &GameOptions::s_unitTypeIndex, 2);
    }
    else if (GameOptions::s_activeTool == RTSTools::kMoveUnit) {

      ImGui::Checkbox("Draw Gizmos", &GameOptions::s_drawGridWalkerGizmos);
      ImGui::SameLine();
      ImGui::Checkbox("Step Mode", &GameOptions::s_gridWalkerStepMode);

      ImGui::Spacing();

      auto pPathfinderName =
        GameOptions::s_pathfinderNames[GameOptions::s_CurrentWalkerIndex];

      if (ImGui::BeginCombo("Pathfinding Algorithm", pPathfinderName)) {
        for (int8 n = 0; n < GameOptions::s_pathfinderNames.size(); n++) {
          bool is_selected =
            (pPathfinderName == GameOptions::s_pathfinderNames[n]);
          if (ImGui::Selectable(GameOptions::s_pathfinderNames[n], is_selected)) {
            pPathfinderName = GameOptions::s_pathfinderNames[n];
            GameOptions::s_CurrentWalkerIndex = n;
          }
          if (is_selected) {
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }

      ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
      const List<RTSUnit*> activeUnits = pApp->getWorld()->GetActiveUnits();
      if (activeUnits.size() == 1) {
        auto activeUnit = activeUnits.front();

        ImGui::Text(("\tUnit: " + activeUnit->GetUnitType()->GetName()).c_str());

        ImGui::Text(("\tHP: " + toString(activeUnit->GetCurrentHP()) + "/" + 
                                toString(activeUnit->GetMaxHP())).c_str());

        ImGui::Text(("\tPosition: (" + toString(activeUnit->GetRawPosition().x) + ", " +
                                       toString(activeUnit->GetRawPosition().y) + ")").c_str());

        ImGui::Text(("\tAttack: " + toString(activeUnit->GetDPS())).c_str());

        ImGui::Text(("\tRange: " + toString(activeUnit->GetAttackRange())).c_str());
      }
      else if (activeUnits.size() > 1) {
        for (auto it = activeUnits.begin(); it != activeUnits.end(); ++it) {
          auto activeUnit = *it;
          ImGui::Text(("\tUnit: " + activeUnit->GetUnitType()->GetName()).c_str());

          ImGui::SameLine();

          ImGui::Text(("\tHP: " + toString(activeUnit->GetCurrentHP()) + "/" +
                       toString(activeUnit->GetMaxHP())).c_str());
        }
      }
      else {
        ImGui::Text("\tUnit: NONE");
      }
    }
  }
  ImGui::End();

  GameOptions::s_GUIBlockingMouse = ImGui::IsRootWindowOrAnyChildFocused() ||
                                    ImGui::IsAnyWindowHovered() ||
                                    ImGui::IsAnyItemActive();
}
