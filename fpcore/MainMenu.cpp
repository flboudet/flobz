/*
 *  MainMenu.cpp
 *  flobopop
 *
 *  Created by Florent Boudet on 18/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "MainMenu.h"
#include "FPStrings.h"

MainRealMenu::MainRealMenu(MainScreen * mainScreen) :
// Create sub screens
MainScreenMenu(mainScreen),
_soloGameMenu      (mainScreen),
_localGameMenu     (mainScreen),
_local2PlayersGameMenu(mainScreen),
_optionMenu        (mainScreen),
_popFromHallScreenAction(),
_popFromCreditsAction(),
_hiScores("StoryMode", theCommander->getPreferencesManager(), _defaultHiScores),
_hallOfFameScreen(&_popFromHallScreenAction),
_creditsScreen("credits.gsl", &_popFromCreditsAction, false),
// Create action for buttons
_soloGameAction(&_soloGameMenu, mainScreen),
_singlePlayerGameAction(&_localGameMenu, mainScreen),
_twoPlayersGameAction(&_local2PlayersGameMenu, mainScreen),
_optionAction(&_optionMenu, mainScreen),
_hallOfFameAction(&_hallOfFameScreen),
_creditsAction(&_creditsScreen, mainScreen),
// Create buttons
_soloGameButton(theCommander->getLocalizedString("Solo Game"), &_soloGameAction),
_singlePlayerGameButton(theCommander->getLocalizedString("Story-Mode Game"), &_singlePlayerGameAction),
_twoPlayersGameButton(theCommander->getLocalizedString("Two Players Game"), &_twoPlayersGameAction),
_optionButton(theCommander->getLocalizedString("Options"), &_optionAction),
_hallOfFameButton(theCommander->getLocalizedString(kHighScores), &_hallOfFameAction),
_creditsButton(theCommander->getLocalizedString("Credits"), &_creditsAction),
_exitButton(theCommander->getLocalizedString(kExit), &_exitAction)
#ifdef ENABLE_NETWORK
#ifdef ENABLE_NETWORK_INTERNET
, _networkGameMenu   (mainScreen),
_networkGameAction(&_networkGameMenu, mainScreen),
_networkGameButton(theCommander->getLocalizedString(kNetGame), &_networkGameAction)
#else
, _lanGameMenu(mainScreen),
_lanAction(&_lanGameMenu, mainScreen),
_lanGameButton(theCommander->getLocalizedString("Local Area Network Game").c_str(), &_lanAction) // TODO: string
#endif
#endif
{
}


void MainRealMenu::build() {
  _localGameMenu.build();
  _local2PlayersGameMenu.build();
  _optionMenu.build();
  add(&_soloGameButton);
  add(&_singlePlayerGameButton);
  add(&_twoPlayersGameButton);
#ifdef ENABLE_NETWORK
#ifdef ENABLE_NETWORK_INTERNET
  _networkGameMenu.build();
  add(&_networkGameButton);
#else
  _lanGameMenu.build();
  add(&_lanGameButton);
#endif
#endif
  add(&_optionButton);
  // Hall of Fame should be improved to handle multiple boards
  //add(&_hallOfFameButton);
  add(&_creditsButton);
  add(&_exitButton);
}

