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
#include "OptionMenu.h"
#include "PuyoLocalMenu.h"
#ifdef ENABLE_NETWORK
#include "NetworkMenu.h"
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
class MainRealMenu : public MainScreenMenu {
public:
    MainRealMenu(MainScreen * mainScreen);
    void build();

private:
    LocalGameMenu         localGameMenu;
    Local2PlayersGameMenu local2PlayersGameMenu;
    OptionMenu            optionMenu;

    PopToMainScreenAction popFromHallScreenAction, popFromCreditsAction;
    HallOfFameScreen      hallOfFameScreen; // Comes from PuyoSinglePlayerStarter.cpp
    StoryScreen           creditsScreen;
    PuyoPushMenuAction    singlePlayerGameAction;
    PuyoPushMenuAction    twoPlayersGameAction;
    PuyoPushMenuAction    optionAction;
    PushHallOfFameAction  hallOfFameAction;
    PushStoryScreenAction creditsAction;
    ExitAction exitAction;

    Button singlePlayerGameButton;
    Button twoPlayersGameButton;
    Button optionButton;
    Button hallOfFameButton;
    Button creditsButton;
    Button exitButton;

#ifdef ENABLE_NETWORK
#ifdef ENABLE_NETWORK_INTERNET
    NetworkGameMenu       networkGameMenu;
    PuyoPushMenuAction    networkGameAction;
    Button                networkGameButton;
#else
    LANGameMenu           lanGameMenu;
    PuyoPushMenuAction    lanAction;
    Button                lanGameButton;
#endif
#endif
};

#endif // _MAIN_MENU_H_

