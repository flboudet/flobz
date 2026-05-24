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
    SoloGameMenu          _soloGameMenu;
    LocalGameMenu         _localGameMenu;
    Local2PlayersGameMenu _local2PlayersGameMenu;
    OptionMenu            _optionMenu;

    PopToMainScreenAction _popFromHallScreenAction, _popFromCreditsAction;
    HiScoreDefaultBoard   _defaultHiScores;
    LocalStorageHiScoreBoard _hiScores;
    HallOfFameScreen      _hallOfFameScreen; // Comes from StoryModeStarter.cpp
    StoryScreen           _creditsScreen;
    PushMainScreenMenuAction    _soloGameAction;
    PushMainScreenMenuAction    _singlePlayerGameAction;
    PushMainScreenMenuAction    _twoPlayersGameAction;
    PushMainScreenMenuAction    _optionAction;
    PushHallOfFameAction  _hallOfFameAction;
    PushStoryScreenAction _creditsAction;
    ExitAction _exitAction;

    Button _soloGameButton;
    Button _singlePlayerGameButton;
    Button _twoPlayersGameButton;
    Button _optionButton;
    Button _hallOfFameButton;
    Button _creditsButton;
    Button _exitButton;

#ifdef ENABLE_NETWORK
#ifdef ENABLE_NETWORK_INTERNET
    NetworkGameMenu       _networkGameMenu;
    PushMainScreenMenuAction    _networkGameAction;
    Button                _networkGameButton;
#else
    LANGameMenu           _lanGameMenu;
    PushMainScreenMenuAction    _lanAction;
    Button                _lanGameButton;
#endif
#endif
};

#endif // _MAIN_MENU_H_

