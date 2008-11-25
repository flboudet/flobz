/*
 *  MainMenu.h
 *  flobopuyo
 *
 *  Created by Florent Boudet on 18/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _MAIN_MENU_H_
#define _MAIN_MENU_H_

#include "HallOfFame.h"
#include "PuyoOptionMenu.h"
#include "PuyoLocalMenu.h"
#ifndef DISABLE_NETWORK
#include "PuyoNetworkMenu.h"
#endif

/*
 * MENU ACTIONS
 */
class ExitAction : public Action {
  public:
    void action() { SDL_Quit(); exit(0); }
};

class SinglePlayerGameAction : public Action {
  public: void action();
};


/// Main menu of the game
///
/// Contains buttons for 1P,2P,Option and Network menus.
class MainRealMenu : public PuyoMainScreenMenu {
public:
    MainRealMenu(PuyoMainScreen * mainScreen);
    void build();

private:
    LocalGameMenu         localGameMenu;
    Local2PlayersGameMenu local2PlayersGameMenu;
    OptionMenu            optionMenu;
    NetworkGameMenu       networkGameMenu;

    PopToMainScreenAction popFromHallScreenAction, popFromCreditsAction;
    HallOfFameScreen      hallOfFameScreen; // Comes from PuyoSinglePlayerStarter.cpp
    PuyoStoryScreen       creditsScreen;
    PuyoPushMenuAction    singlePlayerGameAction;
    PuyoPushMenuAction    twoPlayersGameAction;
    PuyoPushMenuAction    optionAction;
    PuyoPushMenuAction    networkGameAction;
    PushHallOfFameAction  hallOfFameAction;
    PushStoryScreenAction creditsAction;
    ExitAction exitAction;

    Button singlePlayerGameButton;
    Button twoPlayersGameButton;
    Button optionButton;
    Button networkGameButton;
    Button hallOfFameButton;
    Button creditsButton;
    Button exitButton;
};

#endif // _MAIN_MENU_H_

