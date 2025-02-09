#include "BWAPI/GameImpl.h"

#include <Util/StringUtil.h>

#include "BWAPI/PlayerImpl.h"

#include "Config.h"

#include <BW/Constants.h>
#include <BW/OrderTypes.h>

#include "../../svnrev.h"
#include "../../Debug.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

using namespace BWAPI;

std::string GameImpl::getTournamentString()
{
  static std::string sMemo;
  if ( sMemo.empty() )
  {
    std::stringstream ss;
    ss << "BWAPI " BWAPI_VER "." << SVN_REV << " Tournament Mode Engaged!";
    sMemo = ss.str();
  }
  return sMemo;
}

//------------------------------------------------- UPDATE -------------------------------------------------
void GameImpl::update()
{
  //this function is called every frame from a hook attached in DllMain.cpp
  this->inGame = true;

  // Calculate APM, FPS, etc.
  updateStatistics();

  //the first time update() is called, we also call onGameStart to initialize some things
  if ( !onStartCalled )
    this->onGameStart();

  if ( !this->calledMatchEnd && frameCount > 1 )
  {
    bool win     = false;
    bool allDone = false;
    if ( this->BWAPIPlayer )
    {
      if ( this->BWAPIPlayer->isVictorious() )
      {
        win     = true;
        allDone = true;
      }
      if ( this->BWAPIPlayer->isDefeated() )
      {
        win     = false;
        allDone = true;
      }
    }
    else
    {
      allDone = true;
      for(PlayerImpl* p : this->players)
      {
        if (p->getIndex() >= BW::PLAYABLE_PLAYER_COUNT)
          continue;
        if ( !p->isDefeated() && !p->isVictorious() && !p->leftGame() )
          allDone = false;
      }
    }
    if ( allDone)
    {
      if (win)
        rn_GameResult = "win";
      this->calledMatchEnd = true;
      events.push_back(Event::MatchFrame());
      events.push_back(Event::MatchEnd(win));
      server.update();
      this->inGame = false;
      events.push_back(Event::MenuFrame());
      server.update();
    }
  }

  //don't have any more MatchFrame events after MatchEnd until MatchStart is called.
  if ( this->calledMatchEnd ) return;

  // Update unit selection
//  if (wantSelectionUpdate && savedUnitSelection != BW::BWDATA::ClientSelectionGroup)
//  {
//    wantSelectionUpdate = false;
//    savedUnitSelection = BW::BWDATA::ClientSelectionGroup;
//    refreshSelectionStates();
//  }

  //update players and check to see if they have just left the game.
  _allies.clear();
  _enemies.clear();
  _observers.clear();
  if ( BWAPIPlayer )
  {
    for(Player p : players)
    {
      if ( p->leftGame() || p->isDefeated() || p == BWAPIPlayer )
        continue;
      if ( BWAPIPlayer->isAlly(p) )
        _allies.insert(p);
      if ( BWAPIPlayer->isEnemy(p) )
        _enemies.insert(p);
      if ( p->isObserver() )
        _observers.insert(p);
    }
  }
  for (int i = 0; i < BW::PLAYER_COUNT; ++i)
  {
    bool prevLeftGame = this->players[i]->leftGame();
    this->players[i]->updateData();
    if (!prevLeftGame && this->players[i]->leftGame())
      events.push_back(Event::PlayerLeft(this->players[i]));
  }
  //update properties of Unit and Bullet objects
  this->updateUnits();
  this->updateBullets();

  //iterate through the list of intercepted messages
  for(std::string &s : sentMessages)
    BroodwarImpl.onSendText(s);
  this->sentMessages.clear();

  //on the first frame we check to see if the client process has connected.
  //if not, then we load the AI dll specified in bwapi.ini
  if ( !this->startedClient )
  {
    this->initializeTournamentModule();
    this->initializeAIModule();

    //push the MatchStart event to the front of the queue so that it is the first event in the queue.
    events.push_front(Event::MatchStart());
    this->startedClient = true;
  }

  if ( !this->bTournamentMessageAppeared &&
        hTournamentModule &&
        this->frameCount > _currentPlayerId()*8 )
  {
    this->bTournamentMessageAppeared = true;
    this->isTournamentCall = true;
    sendText("%s", getTournamentString().c_str());
    if ( this->tournamentController )
      this->tournamentController->onFirstAdvertisement();
    this->isTournamentCall = false;
  }

  //each frame we add a MatchFrame event to the queue
  events.push_back(Event::MatchFrame());

  //if the AI is a client process, this will signal the client to process the next frame
  //if the AI is a DLL, this will translate the events into AIModule callbacks.
  server.update();

  shapes.resize(server.data->shapeCount);
  for (size_t i = 0; i != shapes.size(); ++i) {
    shapes[i] = server.data->shapes[i];
  }
  shapeStrings.resize(server.data->stringCount);
  for (size_t i = 0; i != shapeStrings.size(); ++i) {
    shapeStrings[i] = server.data->strings[i];
  }

  // Process events that were created for interface objects
  this->processInterfaceEvents();

  //Before returning control to starcraft, we clear the unit data for units that are no longer accessible
  for(Unit u : evadeUnits)
    static_cast<UnitImpl*>(u)->updateData();

  //We also kill the units that are dying on this frame.
  //We wait until after server.update() and processEvents() to do this so that the AI can
  //access the last frame of unit data during the onUnitDestroy callback.
  for(Unit ui : dyingUnits)
  {
    UnitImpl *u = static_cast<UnitImpl*>(ui);
    deadUnits.insert(u);
    u16 index = u->getIndex();
    unitArray[index] = new UnitImpl(bwgame.getUnit(index), index);
    u->die();
  }

  /* In case we ever want to add a Flag::UnitPermanence cheat flag...
  bool UnitPermanence = false;
  if ( !UnitPermanence )
  {
    for(Unit ui : evadeUnits)
    {
      UnitImpl *u = static_cast<UnitImpl*>(ui);
      deadUnits.push_back(u);
      int index = u->getIndex();
      unitArray[index] = new UnitImpl(&BW::BWDATA::UnitNodeTable->unit[index],(u16)index);
      u->die();
    }
  }
  */

  //increment frame count if the game is not paused
  if ( bwgame.isGamePaused() == 0 )
    this->frameCount++;

  // Set the replay time, this is a workaround to fixing the replay DIVIDE_BY_ZERO exception bug
//  if ( !this->isReplay() )
//    BW::BWDATA::ReplayHead.frameCount = this->getFrameCount()+20;

  // Execute commands that have been buffered by the command optimizer
  commandOptimizer.flush();

  // Update any graphical/debug overlays
  updateOverlays();
  setTextSize(); // Reset text size
  Broodwar.flush(); // Flush the Broodwar text buffer

  //finally return control to starcraft
}

//------------------------------------------------- STATS -------------------------------------------------
void GameImpl::updateStatistics()
{
  apmCounter.update(getFrameCount());
  fpsCounter.update(getFrameCount());
}

//------------------------------------------------- OVERLAYS -------------------------------------------------
void GameImpl::updateOverlays()
{
  if ( !data->hasGUI )
    return;

  // grid
  if ( grid )
  {
    BWAPI::Position scrPos = getScreenPosition();

    // draw mtx grid
//    for ( int y = scrPos.y/32; y < (scrPos.y + BW::BWDATA::GameScreenBuffer.height())/32 + 1; ++y )
//    {
//      for ( int x = scrPos.x/32; x < (scrPos.x + BW::BWDATA::GameScreenBuffer.width())/32 + 1; ++x )
//      {
//        for ( int i = 0; i < 32; i += 4 )
//        {
//          drawLineMap(x*32 + 32, y*32 + i, x*32 + 32, y*32 + i + 2, BWAPI::Colors::Grey);
//          drawLineMap(x*32 + i, y*32 + 32, x*32 + i + 2, y*32 + 32, BWAPI::Colors::Grey);
//        }
//      }
//    }
    setTextSize(Text::Size::Small);
    drawTextScreen(64, 288, "%c(%u, %u)", Text::White, (scrPos.x+this->getMousePosition().x)/32, (scrPos.y+this->getMousePosition().y)/32);
    setTextSize();
  } // grid

  // fps display
  if ( showfps )
  {
    setTextSize(Text::Size::Small);
    drawTextScreen(1,1, "%cFrame Rate (Logical Frames)\nLFPS: %d\nAvg LFPS:%.3lf", BWAPI::Text::Yellow, this->getFPS(), this->getAverageFPS());
    setTextSize();
  } // fps

}

//--------------------------------------------- TOURNAMENT MODULE ----------------------------------------------
void GameImpl::initializeTournamentModule()
{
  // Declare typedefs for function pointers
  typedef void (*PFNGameInit)(Game *);
  typedef AIModule* (*PFNCreateA1)();
  typedef TournamentModule* (*PFNCreateTournament)();

  // Initialize Tournament Variables
#ifndef _DEBUG
  // Load tournament string and module if string exists
  std::string TournamentDllPath = LoadConfigString("ai", "tournament");
  if ( TournamentDllPath.size() > 0 )
#ifdef _WIN32
    hTournamentModule = LoadLibrary(TournamentDllPath.c_str());
#else
    hTournamentModule = dlopen(TournamentDllPath.c_str(), RTLD_NOW);
#endif

  // If tournament module exists
  if ( hTournamentModule )
  {
    // Obtain our tournament functions
#ifdef _WIN32
    PFNGameInit         newGameInit         = (PFNGameInit)GetProcAddress((HMODULE)hTournamentModule, TEXT("gameInit"));
    PFNCreateA1         newTournamentAI     = (PFNCreateA1)GetProcAddress((HMODULE)hTournamentModule, TEXT("newTournamentAI"));
    PFNCreateTournament newTournamentModule = (PFNCreateTournament)GetProcAddress((HMODULE)hTournamentModule, TEXT("newTournamentModule"));
#else
    PFNGameInit         newGameInit         = (PFNGameInit)dlsym(hTournamentModule, "gameInit");
    PFNCreateA1         newTournamentAI     = (PFNCreateA1)dlsym(hTournamentModule, "newTournamentAI");
    PFNCreateTournament newTournamentModule = (PFNCreateTournament)dlsym(hTournamentModule, "newTournamentModule");
#endif

    // Call the tournament functions if they exist
    if ( newTournamentAI && newTournamentModule && newGameInit )
    {
      newGameInit(this);
      this->tournamentAI         = newTournamentAI();
      this->tournamentController = newTournamentModule();
    }
    else // error when one function is not found
    {
      // Free the tournament module
#ifdef _WIN32
      FreeLibrary((HMODULE)hTournamentModule);
#else
      dlclose(hTournamentModule);
#endif
      hTournamentModule = NULL;

      // Create our error string
      std::string missing;
      if ( !newTournamentAI )
        missing += "newTournamentAI";

      if ( !newTournamentModule )
      {
        if ( !missing.empty() )
          missing += " and ";
        missing += "newTournamentModule";
      }
      missing += " function";

      // print error message
      Broodwar << Text::Red << "ERROR: Failed to find the " << missing << " in tournament module." << std::endl;
    }
  }
  this->bTournamentMessageAppeared = false;
#else
  this->bTournamentMessageAppeared = true;
#endif
}

//--------------------------------------------- AI MODULE ----------------------------------------------
void GameImpl::initializeAIModule()
{
  // Declare typedefs for function pointers
  typedef void (*PFNGameInit)(Game *);
  typedef AIModule* (*PFNCreateA1)();

  // Connect to external module if it exists
  externalModuleConnected = false;
  std::string moduleName("<Nothing>");
  if (this->specifiedModule)
  {
    if ( !hTournamentModule )
      Broodwar << Text::Green << "Using AI Module" << std::endl;
    this->client = this->specifiedModule;
    this->deleteClient = false;
    moduleName = "<Module>";
    externalModuleConnected = true;
  }
  else if ( server.isConnected() ) //check to see if the server is connected to the client
  {
    // assign a blank AI module to our variable
    this->client = new AIModule();
    this->deleteClient = true;
    // Hide success strings in tournament mode
    if ( !hTournamentModule )
      Broodwar << "BWAPI: Connected to AI Client process" << std::endl;
    // Set the module string
    moduleName = "<Client Connection>";
    externalModuleConnected = true;
  }
  else // if not, load the AI module DLL
  {
    // declare/assign variables
    hAIModule         = nullptr;

    std::string dll;
    std::string aicfg;
#ifdef COMPAT
    std::string compatDll;
#endif
    if (!aicfgOverride.empty()) aicfg = aicfgOverride;
    else aicfg = LoadConfigString("ai", BUILD_DEBUG ? "ai_dbg" : "ai", "_NULL");
    if (aicfg == "_NULL")
    {
      Broodwar << Text::Red << "Could not find " << (BUILD_DEBUG ? "ai_dbg" : "ai") << " in \"" << configPath() << "\"." << std::endl;
    }
    else
    {
      std::stringstream aiList(aicfg);

      // Get DLL name
      dll = aicfg.substr(0, aicfg.find_first_of(','));

      // Skip to current intended instance
      for (int i = 0; i < getInstanceNumber() && aiList; ++i)
        std::getline(aiList, dll, ',');

      // trim whitespace outside quotations and then the quotations
      Util::trim(dll, Util::is_whitespace_or_newline);
      Util::trim(dll, [](char c) { return c == '"'; });

#ifdef COMPAT
      auto colon = dll.find(':');
      if (colon != std::string::npos) {
        compatDll = dll.substr(colon + 1);
        dll = dll.substr(0, colon);
      }
#endif

#ifdef _WIN32
      hAIModule = LoadLibraryA(dll.c_str());
#else
      hAIModule = dlopen(dll.c_str(), RTLD_NOW);
      if (!hAIModule) Broodwar << Text::Red << "dlerror: " << dlerror() << std::endl;
#endif
    }

    if ( !hAIModule )
    {
      //if hAIModule is nullptr, there there was a problem when trying to load the AI Module
      this->client = new AIModule();
      this->deleteClient = true;

      // enable flags to allow interaction
      Broodwar->enableFlag(Flag::CompleteMapInformation);
      Broodwar->enableFlag(Flag::UserInput);

      // print error string
      Broodwar << Text::Red << "ERROR: Failed to load the AI Module \"" << dll << "\"." << std::endl;
      externalModuleConnected = false;
    }
    else
    {
      // Obtain the AI module function
#ifdef _WIN32
      auto import = [](void* module, const char* name) {
        return GetProcAddress((HMODULE)module, name);
      };
#else
      auto import = [](void* module, const char* name) {
        return dlsym(module, name);
      };
#endif
      PFNGameInit newGame     = (PFNGameInit)import(hAIModule, "gameInit");
      PFNCreateA1 newAIModule = (PFNCreateA1)import(hAIModule, "newAIModule");
      if ( newAIModule && newGame )
      {
        // Call the AI module function and assign the client variable
#ifdef COMPAT
        if (!compatDll.empty()) {
          auto loadDll = (void(*)(const char*))import(hAIModule, "loadDll");
          loadDll(compatDll.c_str());
        }
        newGame((Game*)&compatGameImpl);
        this->client = compatGameImpl.wrapAIModule(newAIModule());
#ifdef _WIN32
        compatGameImpl.LoadLibrary = [](const char* name) {
          return LoadLibraryA(name);
        });
        compatGameImpl.GetProcAddress = [&](void* module, const char* name) {
          return GetProcAddress(module, name);
        });
#else
      compatGameImpl.LoadLibrary = (void*(*)(const char*))dlsym(hAIModule, "LoadLibrary");
      compatGameImpl.GetProcAddress = (void*(*)(void*, const char*))dlsym(hAIModule, "GetProcAddress");
#endif
#else
        newGame(this);
        this->client = newAIModule();
#endif
        this->deleteClient = true;

        // Hide success strings in tournament mode
        if ( !hTournamentModule )
          Broodwar << Text::Green << "Loaded the AI Module: " << dll << std::endl;
        externalModuleConnected = true;

        // Strip the path from the module name
        size_t slash_pos = dll.find_last_of("/\\");
        if (slash_pos == std::string::npos) moduleName = dll;
        else moduleName = dll.substr(slash_pos + 1);
      }
      else  // If the AIModule function is not found
      {
        // Create a dummy AI module
        this->client = new AIModule();
        this->deleteClient = true;

        // Enable flags to allow interaction
        Broodwar->enableFlag(Flag::CompleteMapInformation);
        Broodwar->enableFlag(Flag::UserInput);

        // Create error string
        std::string missing;
        if ( !newGame )
          missing += "gameInit";

        if ( !newAIModule )
        {
          if ( !missing.empty() )
            missing += " and ";
          missing += "newAIModule";
        }
        missing += " function";

        // Print an error message
        Broodwar << Text::Red << "ERROR: Failed to find the " << missing << " in " << dll << std::endl;
        externalModuleConnected = false;
      }
    }
  }

  if ( !hTournamentModule ) // If tournament mode wasn't initialized
    sendText("BWAPI %s.%d %s is now live using \"%s\".", BWAPI_VER, SVN_REV, BUILD_STR, moduleName.c_str() );
}

