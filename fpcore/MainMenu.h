/*
 *  MainMenu.h
 *  flobopop
 *
 *  Created by Florent Boudet on 18/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _MAIN_MENU_H_
#define _MAIN_MENU_H_

#include "MainScreen.h"
#include "HallOfFame.h"
#include "OptionMenu.h"
#include "LocalMenu.h"
#ifdef ENABLE_NETWORK
#include "NetworkMenu.h"
#endif

/*
 * MENU ACTIONS
 */
class ExitAction : public Action {
  public:
    void action() {
		// TODO: Fix
		// SDL_Quit();
		exit(0);
	}
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
    SoloGameMenu          soloGameMenu;
    LocalGameMenu         localGameMenu;
    Local2PlayersGameMenu local2PlayersGameMenu;
    OptionMenu            optionMenu;

    PopToMainScreenAction popFromHallScreenAction, popFromCreditsAction;
    HallOfFameScreen      hallOfFameScreen; // Comes from StoryModeStarter.cpp
    StoryScreen           creditsScreen;
    PushMainScreenMenuAction    soloGameAction;
    PushMainScreenMenuAction    singlePlayerGameAction;
    PushMainScreenMenuAction    twoPlayersGameAction;
    PushMainScreenMenuAction    optionAction;
    PushHallOfFameAction  hallOfFameAction;
    PushStoryScreenAction creditsAction;
    ExitAction exitAction;

    Button soloGameButton;
    Button singlePlayerGameButton;
    Button twoPlayersGameButton;
    Button optionButton;
    Button hallOfFameButton;
    Button creditsButton;
    Button exitButton;

#ifdef ENABLE_NETWORK
#ifdef ENABLE_NETWORK_INTERNET
    NetworkGameMenu       networkGameMenu;
    PushMainScreenMenuAction    networkGameAction;
    Button                networkGameButton;
#else
    LANGameMenu           lanGameMenu;
    PushMainScreenMenuAction    lanAction;
    Button                lanGameButton;
#endif
#endif
};

#endif // _MAIN_MENU_H_

