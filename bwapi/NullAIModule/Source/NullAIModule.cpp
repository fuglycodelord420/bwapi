#include "NullAIModule.h"

using namespace BWAPI;
using namespace Filter;

void NullAIModule::onStart()
{

  // Enable the UserInput flag, which allows us to control the bot and type messages.
  Broodwar->enableFlag(Flag::UserInput);

}

void NullAIModule::onEnd(bool isWinner)
{
}

void NullAIModule::onFrame()
{
}

void NullAIModule::onSendText(std::string text)
{

  // Send the text to the game if it is not being processed.
  Broodwar->sendText("%s", text.c_str());

}

void NullAIModule::onReceiveText(BWAPI::Player player, std::string text)
{
  // Parse the received text
  Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
}

void NullAIModule::onPlayerLeft(BWAPI::Player player)
{
}

void NullAIModule::onNukeDetect(BWAPI::Position target)
{
}

void NullAIModule::onUnitDiscover(BWAPI::Unit unit)
{
}

void NullAIModule::onUnitEvade(BWAPI::Unit unit)
{
}

void NullAIModule::onUnitShow(BWAPI::Unit unit)
{
}

void NullAIModule::onUnitHide(BWAPI::Unit unit)
{
}

void NullAIModule::onUnitCreate(BWAPI::Unit unit)
{
}

void NullAIModule::onUnitDestroy(BWAPI::Unit unit)
{
}

void NullAIModule::onUnitMorph(BWAPI::Unit unit)
{
}

void NullAIModule::onUnitRenegade(BWAPI::Unit unit)
{
}

void NullAIModule::onSaveGame(std::string gameName)
{
}

void NullAIModule::onUnitComplete(BWAPI::Unit unit)
{
}
