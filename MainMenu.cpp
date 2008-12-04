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
networkGameMenu   (mainScreen),
popFromHallScreenAction(mainScreen, &hallOfFameScreen),
popFromCreditsAction(mainScreen, &creditsScreen),
hallOfFameScreen(*mainScreen, &popFromHallScreenAction),
creditsScreen("credits.gsl", *mainScreen, &popFromCreditsAction, false),
// Create action for buttons
singlePlayerGameAction(&localGameMenu, mainScreen),
twoPlayersGameAction(&local2PlayersGameMenu, mainScreen),
optionAction(&optionMenu, mainScreen),
networkGameAction(&networkGameMenu, mainScreen),
hallOfFameAction(&hallOfFameScreen, mainScreen),
creditsAction(&creditsScreen, mainScreen),
// Create buttons
singlePlayerGameButton(theCommander->getLocalizedString(kSinglePlayerGame), &singlePlayerGameAction),
twoPlayersGameButton(theCommander->getLocalizedString("Two Players Game"), &twoPlayersGameAction),
optionButton(theCommander->getLocalizedString("Options"), &optionAction),
networkGameButton(theCommander->getLocalizedString(kNetGame), &networkGameAction),
hallOfFameButton(theCommander->getLocalizedString(kHighScores), &hallOfFameAction),
creditsButton(theCommander->getLocalizedString("Credits"), &creditsAction),
exitButton(theCommander->getLocalizedString(kExit), &exitAction)
{}


void MainRealMenu::build() {
  localGameMenu.build();
  local2PlayersGameMenu.build();
  optionMenu.build();
  networkGameMenu.build();
  add(&singlePlayerGameButton);
  add(&twoPlayersGameButton);
  add(&networkGameButton);
  add(&optionButton);
  add(&hallOfFameButton);
  add(&creditsButton);
  add(&exitButton);
}

