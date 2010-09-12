/* FloboPuyo
 * Copyright (C) 2004
 *   Florent Boudet        <flobo@ios-software.com>,
 *   Jean-Christophe Hoelt <jeko@ios-software.com>,
 *   Guillaume Borios      <gyom@ios-software.com>
 *
 * iOS Software <http://www.ios-software.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *
 */

#include "PuyoTwoPlayerStarter.h"
using namespace event_manager;

TwoPlayersGameWidget::TwoPlayersGameWidget(PuyoSetTheme &puyoThemeSet, LevelTheme &levelTheme, String aiFace, Action *gameOverAction) : attachedPuyoThemeSet(puyoThemeSet),
                                                     attachedRandom(5), attachedGameFactory(&attachedRandom),
                                                     areaA(&attachedGameFactory, 0, &attachedPuyoThemeSet, &levelTheme),
                                                     areaB(&attachedGameFactory, 1, &attachedPuyoThemeSet, &levelTheme),
                                                     playercontrollerA(areaA, kPlayer1Down, kPlayer1Left, kPlayer1Right,
                                                     kPlayer1TurnLeft, kPlayer1TurnRight),
                                                     playercontrollerB(areaB, kPlayer2Down, kPlayer2Left, kPlayer2Right,
                                                     kPlayer2TurnLeft, kPlayer2TurnRight),
                                                     opponentFace(aiFace)
{
    initWithGUI(areaA, areaB, playercontrollerA, playercontrollerB, levelTheme, gameOverAction);
    setLives(-1);
}

StoryWidget *TwoPlayersGameWidget::getOpponent()
{
    return &opponentFace;
}

void TwoPlayersGameWidget::cycle()
{
    opponentFace.setIntegerValue("@maxHeightLeft", attachedGameA->getColumnHeigth(2));
    opponentFace.setIntegerValue("@maxHeightRight", attachedGameB->getColumnHeigth(2));
    opponentFace.setIntegerValue("@neutralsForLeft", attachedGameA->getNeutralPuyos());
    opponentFace.setIntegerValue("@neutralsForRight", attachedGameB->getNeutralPuyos());
    opponentFace.setIntegerValue("@comboPhaseLeft", attachedGameA->getComboPhase());
    opponentFace.setIntegerValue("@comboPhaseRight", attachedGameB->getComboPhase());
    GameWidget::cycle();
}

TwoPlayersStarterAction::TwoPlayersStarterAction(int difficulty, GameWidgetFactory &gameWidgetFactory, PuyoTwoNameProvider *nameProvider)
  : m_state(kNotRunning), difficulty(difficulty), gameWidgetFactory(gameWidgetFactory), gameScreen(NULL),
    nameProvider(nameProvider), gameLostWidget(NULL), m_getReadyWidget(NULL), currentLevelTheme(NULL), leftVictories(0), rightVictories(0),
    m_statsWidget(NULL), totalPointsPlayerOne_(0), totalPointsPlayerTwo_(0) {}

void TwoPlayersStarterAction::action(Widget *sender, int actionType,
                                     GameControlEvent *event)
{
    //cout << "TwoPlayerAction!!! " << actionType << endl;
  if (m_state == kNotRunning) {
      stateMachine();
  }
  else if (sender == gameWidget) {
    if (gameWidget->getAborted())
      endGameSession();
    else if (m_state == kMatchGettingStarted)
        stateMachine();
    else if (actionType == GameWidget::GAMEOVER_STARTPRESSED)
      stateMachine();
    else if (actionType == GameWidget::GAME_IS_OVER)
      stateMachine();
  }
  else if (sender == gameLostWidget)
      stateMachine();
  else if (m_state == kMatchGettingStarted)
      stateMachine();
}

void TwoPlayersStarterAction::stateMachine()
{
  switch (m_state) {
  case kNotRunning:
      prepareGame1stRun();
      break;
  case kMatchGettingStarted:
      startGame();
      break;
  case kMatchPlaying:
      gameOver();
      break;
  case kMatchWonP1Animation:
      gameScores();
      break;
  case kMatchWonP2Animation:
      gameScores();
      break;
  case kMatchScores:
      prepareGameNextRun();
      break;
  default:
      break;
  }
}

void TwoPlayersStarterAction::prepareGame1stRun()
{
    currentLevelTheme = theCommander->getDefaultLevelTheme();

    gameWidget = gameWidgetFactory.createGameWidget(*(theCommander->getDefaultPuyoSetTheme()), *currentLevelTheme, currentLevelTheme->getCentralAnimation2P().c_str(), this);
    gameWidget->setGameOptions(GameOptions::FromLevel(difficulty));
    gameScreen = new GameScreen(*gameWidget, *(GameUIDefaults::SCREEN_STACK->top()));
    if (nameProvider != NULL) {
        gameWidget->setPlayerOneName(nameProvider->getPlayer1Name());
        gameWidget->setPlayerTwoName(nameProvider->getPlayer2Name());
    }
    leftVictories = 0; rightVictories = 0;
    GameUIDefaults::SCREEN_STACK->push(gameScreen);
    prepareGame();
}
void TwoPlayersStarterAction::prepareGame()
{
    gameScreen->setSuspended(true);
    m_state = kMatchGettingStarted;
    if (currentLevelTheme->getReadyAnimation2P() == "") {
        startGame();
        return;
    }
    m_getReadyWidget = new StoryWidget(currentLevelTheme->getReadyAnimation2P().c_str(), this);
    gameScreen->setOverlayStory(m_getReadyWidget);
}

void TwoPlayersStarterAction::startGame()
{
    if (m_getReadyWidget != NULL) {
        m_getReadyWidget->setIntegerValue("@start_pressed", 1);
    }
    gameScreen->setSuspended(false);
    m_state = kMatchPlaying;
}

void TwoPlayersStarterAction::gameOver()
{
    if (gameWidget->isGameARunning()) {
        gameLostWidget = new StoryWidget(currentLevelTheme->getGameLostRightAnimation2P().c_str(), this);
        leftVictories++;
        m_state = kMatchWonP1Animation;
    }
    else {
        gameLostWidget = new StoryWidget(currentLevelTheme->getGameLostLeftAnimation2P().c_str(), this);
        rightVictories++;
        m_state = kMatchWonP2Animation;
    }
    gameScreen->setOverlayStory(gameLostWidget);
}

void TwoPlayersStarterAction::gameScores()
{
  m_state = kMatchScores;
  StatsWidgetDimensions dimensions(416, 194, 50, Vec3(0, 0), Vec3(0, 0));
  m_statsWidget = new TwoPlayersStatsWidget(this->gameWidget->getStatPlayerOne(), this->gameWidget->getStatPlayerTwo(), true, true, theCommander->getWindowFramePicture(), dimensions);
  gameScreen->add(m_statsWidget);
}

void TwoPlayersStarterAction::prepareGameNextRun()
{
    currentLevelTheme = theCommander->getDefaultLevelTheme();

    GameWidget *newGameWidget = gameWidgetFactory.createGameWidget(*(theCommander->getDefaultPuyoSetTheme()), *currentLevelTheme, currentLevelTheme->getCentralAnimation2P().c_str(), this);
    GameScreen *newGameScreen = new GameScreen(*newGameWidget, *(GameUIDefaults::SCREEN_STACK->top()));
    if (nameProvider != NULL) {
        newGameWidget->setPlayerOneName(nameProvider->getPlayer1Name());
        newGameWidget->setPlayerTwoName(nameProvider->getPlayer2Name());
    }

    GameUIDefaults::SCREEN_STACK->pop();
    if (gameWidget != NULL) {
        totalPointsPlayerOne_ += gameWidget->getStatPlayerOne().points;
        totalPointsPlayerTwo_ += gameWidget->getStatPlayerTwo().points;
        GameUIDefaults::GAME_LOOP->garbageCollect(gameWidget);
        GameUIDefaults::GAME_LOOP->garbageCollect(gameScreen);
    }
    if (gameLostWidget != NULL)
        delete gameLostWidget;
    gameLostWidget = NULL;
    gameScreen = newGameScreen;
    gameWidget = newGameWidget;
    gameWidget->getStatPlayerOne().total_points = totalPointsPlayerOne_;
    gameWidget->getStatPlayerTwo().total_points = totalPointsPlayerTwo_;
    GameUIDefaults::SCREEN_STACK->push(gameScreen);
    int victoriesDelta = leftVictories - rightVictories;
    if (victoriesDelta > 0) {
        gameWidget->addGameAHandicap(victoriesDelta);
    }
    else if (victoriesDelta != 0) {
        gameWidget->addGameBHandicap(-victoriesDelta);
    }
    if (m_statsWidget != NULL)
      delete m_statsWidget;
    m_statsWidget = NULL;
    prepareGame();
}

void TwoPlayersStarterAction::endGameSession()
{
    GameUIDefaults::SCREEN_STACK->pop();
    MainScreen *menuScreen = dynamic_cast<MainScreen *>(GameUIDefaults::SCREEN_STACK->top());
    if (menuScreen != NULL)
        menuScreen->transitionFromScreen(*gameScreen);

    delete gameWidget;
    delete gameScreen;

    gameScreen = NULL;
    gameWidget = NULL;

    if (m_statsWidget != NULL)
      delete m_statsWidget;
    m_statsWidget = NULL;

    m_state = kNotRunning;
}

