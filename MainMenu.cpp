/*
 *  MainMenu.cpp
 *  flobopuyo
 *
 *  Created by Florent Boudet on 18/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "MainMenu.h"

MainRealMenu::MainRealMenu(MainScreen * mainScreen) :
// Create sub screens
MainScreenMenu(mainScreen),
localGameMenu     (mainScreen),
local2PlayersGameMenu(mainScreen),
optionMenu        (mainScreen),
popFromHallScreenAction(mainScreen, &hallOfFameScreen),
popFromCreditsAction(mainScreen, &creditsScreen),
hallOfFameScreen(*mainScreen, &popFromHallScreenAction),
creditsScreen("credits.gsl", *mainScreen, &popFromCreditsAction, false),
// Create action for buttons
singlePlayerGameAction(&localGameMenu, mainScreen),
twoPlayersGameAction(&local2PlayersGameMenu, mainScreen),
optionAction(&optionMenu, mainScreen),
hallOfFameAction(&hallOfFameScreen, mainScreen),
creditsAction(&creditsScreen, mainScreen),
// Create buttons
singlePlayerGameButton(theCommander->getLocalizedString(kSinglePlayerGame), &singlePlayerGameAction),
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
{}


void MainRealMenu::build() {
  localGameMenu.build();
  local2PlayersGameMenu.build();
  optionMenu.build();
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
  add(&hallOfFameButton);
  add(&creditsButton);
  add(&exitButton);
}

