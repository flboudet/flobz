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
soloGameMenu      (mainScreen),
localGameMenu     (mainScreen),
local2PlayersGameMenu(mainScreen),
optionMenu        (mainScreen),
popFromHallScreenAction(),
popFromCreditsAction(),
hiScores("StoryMode", theCommander->getPreferencesManager(), defaultHiScores),
hallOfFameScreen(&popFromHallScreenAction),
creditsScreen("credits.gsl", &popFromCreditsAction, false),
// Create action for buttons
soloGameAction(&soloGameMenu, mainScreen),
singlePlayerGameAction(&localGameMenu, mainScreen),
twoPlayersGameAction(&local2PlayersGameMenu, mainScreen),
optionAction(&optionMenu, mainScreen),
hallOfFameAction(&hallOfFameScreen),
creditsAction(&creditsScreen, mainScreen),
// Create buttons
soloGameButton(theCommander->getLocalizedString("Solo Game"), &soloGameAction),
singlePlayerGameButton(theCommander->getLocalizedString("Story-Mode Game"), &singlePlayerGameAction),
twoPlayersGameButton(theCommander->getLocalizedString("Two Players Game"), &twoPlayersGameAction),
optionButton(theCommander->getLocalizedString("Options"), &optionAction),
hallOfFameButton(theCommander->getLocalizedString(kHighScores), &hallOfFameAction),
creditsButton(theCommander->getLocalizedString("Credits"), &creditsAction),
exitButton(theCommander->getLocalizedString(kExit), &exitAction)
#ifdef ENABLE_NETWORK
#ifdef ENABLE_NETWORK_INTERNET
, networkGameMenu   (mainScreen),
networkGameAction(&networkGameMenu, mainScreen),
networkGameButton(theCommander->getLocalizedString(kNetGame), &networkGameAction)
#else
, lanGameMenu(mainScreen),
lanAction(&lanGameMenu, mainScreen),
lanGameButton(theCommander->getLocalizedString("Local Area Network Game"), &lanAction)
#endif
#endif
{
}


void MainRealMenu::build() {
  localGameMenu.build();
  local2PlayersGameMenu.build();
  optionMenu.build();
  add(&soloGameButton);
  add(&singlePlayerGameButton);
  add(&twoPlayersGameButton);
#ifdef ENABLE_NETWORK
#ifdef ENABLE_NETWORK_INTERNET
  networkGameMenu.build();
  add(&networkGameButton);
#else
  lanGameMenu.build();
  add(&lanGameButton);
#endif
#endif
  add(&optionButton);
  // Hall of Fame should be improved to handle multiple boards
  //add(&hallOfFameButton);
  add(&creditsButton);
  add(&exitButton);
}

