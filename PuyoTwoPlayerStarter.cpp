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

//---------------------------------
// SetupMatchState
//---------------------------------
SetupMatchState::SetupMatchState(GameWidgetFactory &gameWidgetFactory,
                                 int difficulty,
                                 PuyoTwoNameProvider *nameProvider,
                                 SharedMatchAssets &sharedMatchAssets)
  : m_gameWidgetFactory(gameWidgetFactory),
    m_difficulty(difficulty),
    m_nameProvider(nameProvider),
    m_sharedAssets(sharedMatchAssets),
    m_nextState(NULL)
{
}

void SetupMatchState::enterState()
{
    cout << "SetupMatchState::enterState()" << endl;
    // Prepare 1st run
    m_sharedAssets.m_currentLevelTheme = theCommander->getDefaultLevelTheme();
    m_sharedAssets.m_currentPuyoSetTheme = theCommander->getDefaultPuyoSetTheme();
    // Create the gamewidget and register as the gamewidget's action
    GameWidget *newGameWidget =
        m_gameWidgetFactory.createGameWidget(*(m_sharedAssets.m_currentPuyoSetTheme),
                                             *(m_sharedAssets.m_currentLevelTheme),
                                             m_sharedAssets.m_currentLevelTheme->getCentralAnimation2P().c_str(), NULL);
    newGameWidget->setGameOptions(GameOptions::FromLevel(m_difficulty));
    if (m_nameProvider != NULL) {
        newGameWidget->setPlayerOneName(m_nameProvider->getPlayer1Name());
        newGameWidget->setPlayerTwoName(m_nameProvider->getPlayer2Name());
    }
    // Setup total points
    newGameWidget->getStatPlayerOne().total_points = m_sharedAssets.m_leftTotal;
    newGameWidget->getStatPlayerTwo().total_points = m_sharedAssets.m_rightTotal;
    // Setup handicap
    int victoriesDelta = m_sharedAssets.m_leftVictories - m_sharedAssets.m_rightVictories;
    cout << "victories delta=" << victoriesDelta << endl;
    if (victoriesDelta > 0) {
        newGameWidget->addGameAHandicap(victoriesDelta);
    }
    else if (victoriesDelta != 0) {
        newGameWidget->addGameBHandicap(-victoriesDelta);
    }
    GameScreen *newGameScreen =
        new GameScreen(*(newGameWidget), *(GameUIDefaults::SCREEN_STACK->top()));
    // Handle eventual game enchainment
    if (m_sharedAssets.m_gameScreen.get() != NULL) {
        GameUIDefaults::SCREEN_STACK->pop();
    }
    m_sharedAssets.m_gameWidget.reset(newGameWidget);
    m_sharedAssets.m_gameScreen.reset(newGameScreen);
    GameUIDefaults::SCREEN_STACK->push(m_sharedAssets.m_gameScreen.get());
    // Set the game initially paused
    m_sharedAssets.m_gameScreen->setSuspended(true);
}

void SetupMatchState::exitState()
{
}

bool SetupMatchState::evaluate()
{
    // This state is just intended to setup the game.
    // No suspending condition.
    return true;
}

GameState *SetupMatchState::getNextState()
{
    return m_nextState;
}

void SetupMatchState::action(Widget *sender, int actionType,
                             event_manager::GameControlEvent *event)
{
}

//---------------------------------
// WaitPlayersReadyState
//---------------------------------
WaitPlayersReadyState::WaitPlayersReadyState(SharedMatchAssets &sharedMatchAssets)
    : m_sharedAssets(sharedMatchAssets),
      m_playersAreReady(false),
      m_nextState(NULL)
{
}

void WaitPlayersReadyState::enterState()
{
    cout << "WaitPlayersReadyState::enterState()" << endl;
    m_playersAreReady = false;
    if (m_sharedAssets.m_currentLevelTheme->getReadyAnimation2P() == "") {
        m_playersAreReady = true;
        return;
    }
    m_getReadyWidget.reset(new StoryWidget(m_sharedAssets.m_currentLevelTheme->getReadyAnimation2P().c_str(), this));
    m_sharedAssets.m_gameScreen->setOverlayStory(m_getReadyWidget.get());
    m_sharedAssets.m_gameWidget->setGameOverAction(this);
}

void WaitPlayersReadyState::exitState()
{
    m_sharedAssets.m_gameWidget->setGameOverAction(NULL);
}

bool WaitPlayersReadyState::evaluate()
{
    return m_playersAreReady;
}

GameState *WaitPlayersReadyState::getNextState()
{
    return m_nextState;
}

void WaitPlayersReadyState::action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event)
{
    if (sender == m_getReadyWidget.get()) {
        m_getReadyWidget.reset(NULL);
    }
    if (m_getReadyWidget.get() != NULL) {
        m_getReadyWidget->setIntegerValue("@start_pressed", 1);
    }
    m_playersAreReady = true;
    evaluateStateMachine();
}

//---------------------------------
// MatchPlayingState
//---------------------------------
MatchPlayingState::MatchPlayingState(SharedMatchAssets &sharedMatchAssets)
    : m_sharedAssets(sharedMatchAssets),
      m_gameIsOver(false),
      m_nextState(NULL),
      m_abortedState(NULL)
{
}

void MatchPlayingState::enterState()
{
    cout << "MatchPlaying::enterState()" << endl;
    m_gameIsOver = false;
    m_sharedAssets.m_gameWidget->setGameOverAction(this);
    // Resume the game
    m_sharedAssets.m_gameScreen->setSuspended(false);
}

void MatchPlayingState::exitState()
{
    m_sharedAssets.m_gameWidget->setGameOverAction(NULL);
}

bool MatchPlayingState::evaluate()
{
    return m_gameIsOver;
}

GameState *MatchPlayingState::getNextState()
{
    if (m_sharedAssets.m_gameWidget->getAborted())
        return m_abortedState;
    return m_nextState;
}

void MatchPlayingState::action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event)
{
    if (actionType == GameWidget::GAME_IS_OVER) {
        m_gameIsOver = true;
        evaluateStateMachine();
    }
}

//---------------------------------
// MatchIsOverState
//---------------------------------
MatchIsOverState::MatchIsOverState(SharedMatchAssets &sharedMatchAssets)
    : m_sharedAssets(sharedMatchAssets),
      m_aknowledged(false)
{
}

void MatchIsOverState::enterState()
{
    cout << "MatchIsOver::enterState()" << endl;
    m_aknowledged = false;
    if (m_sharedAssets.m_gameWidget->isGameARunning()) {
        m_gameLostWidget.reset(new StoryWidget(m_sharedAssets.m_currentLevelTheme->getGameLostRightAnimation2P().c_str(), this));
        m_sharedAssets.m_leftVictories++;
    }
    else {
        m_gameLostWidget.reset(new StoryWidget(m_sharedAssets.m_currentLevelTheme->getGameLostLeftAnimation2P().c_str(), this));
        m_sharedAssets.m_rightVictories++;
    }
    m_sharedAssets.m_leftTotal  += m_sharedAssets.m_gameWidget->getStatPlayerOne().points;
    m_sharedAssets.m_rightTotal += m_sharedAssets.m_gameWidget->getStatPlayerTwo().points;
    m_sharedAssets.m_gameWidget->setGameOverAction(this);
    m_sharedAssets.m_gameScreen->setOverlayStory(m_gameLostWidget.get());
    // TODO: m_gameLostWidget must be released when the screen is removed
}

void MatchIsOverState::exitState()
{
    m_sharedAssets.m_gameWidget->setGameOverAction(NULL);
}

bool MatchIsOverState::evaluate()
{
    return m_aknowledged;
}

GameState *MatchIsOverState::getNextState()
{
    return m_nextState;
}

void MatchIsOverState::action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event)
{
    if (sender == m_gameLostWidget.get()) {
        m_gameLostWidget.reset(NULL);
    }
    m_aknowledged = true;
    evaluateStateMachine();
}

//---------------------------------
// DisplayStatsState
//---------------------------------
DisplayStatsState::DisplayStatsState(SharedMatchAssets &sharedMatchAssets)
    : m_sharedAssets(sharedMatchAssets),
      m_aknowledged(false)
{
}

void DisplayStatsState::enterState()
{
    cout << "DisplayStats::enterState()" << endl;
    m_aknowledged = false;

    m_sharedAssets.m_gameWidget->setGameOverAction(this);
    StatsWidgetDimensions dimensions(416, 194, 50, Vec3(0, 0), Vec3(0, 0));
    m_statsWidget.reset(new TwoPlayersStatsWidget(m_sharedAssets.m_gameWidget->getStatPlayerOne(), m_sharedAssets.m_gameWidget->getStatPlayerTwo(), true, true, theCommander->getWindowFramePicture(), dimensions));
    m_sharedAssets.m_gameScreen->add(m_statsWidget.get());
    // TODO: the stats widget must be released when the screen is removed
}

void DisplayStatsState::exitState()
{
    m_sharedAssets.m_gameWidget->setGameOverAction(NULL);
}

bool DisplayStatsState::evaluate()
{
    return m_aknowledged;
}

GameState *DisplayStatsState::getNextState()
{
    return m_nextState;
}

void DisplayStatsState::action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event)
{
    m_aknowledged = true;
    evaluateStateMachine();
}

//---------------------------------
// LeaveGameState
//---------------------------------
LeaveGameState::LeaveGameState(SharedMatchAssets &sharedMatchAssets)
    : m_sharedAssets(sharedMatchAssets)
{
}

void LeaveGameState::enterState()
{
    cout << "LeaveGame::enterState()" << endl;
    GameUIDefaults::SCREEN_STACK->pop();
    MainScreen *menuScreen = dynamic_cast<MainScreen *>(GameUIDefaults::SCREEN_STACK->top());
    if (menuScreen != NULL)
        menuScreen->transitionFromScreen(*(m_sharedAssets.m_gameScreen));
    m_sharedAssets.release();
}

bool LeaveGameState::evaluate()
{
    return false;
}

GameState *LeaveGameState::getNextState()
{
    return NULL;
}


//---------------------------------
// Two players local game state machine
//---------------------------------
AltTwoPlayersStarterAction::AltTwoPlayersStarterAction(int difficulty, GameWidgetFactory &gameWidgetFactory, PuyoTwoNameProvider *nameProvider)
{
    // Creating the different game states
    m_setupMatch.reset(new SetupMatchState(gameWidgetFactory, difficulty, nameProvider, m_sharedAssets));
    m_waitPlayersReady.reset(new WaitPlayersReadyState(m_sharedAssets));
    m_matchPlaying.reset(new MatchPlayingState(m_sharedAssets));
    m_matchIsOver.reset(new MatchIsOverState(m_sharedAssets));
    m_displayStats.reset(new DisplayStatsState(m_sharedAssets));
    m_leaveGame.reset(new LeaveGameState(m_sharedAssets));
    // Linking the states together
    m_setupMatch->setNextState(m_waitPlayersReady.get());
    m_waitPlayersReady->setNextState(m_matchPlaying.get());
    m_matchPlaying->setNextState(m_matchIsOver.get());
    m_matchPlaying->setAbortedState(m_leaveGame.get());
    m_matchIsOver->setNextState(m_displayStats.get());
    m_displayStats->setNextState(m_setupMatch.get());
    // Initializing the state machine
    m_stateMachine.setInitialState(m_setupMatch.get());
}

void AltTwoPlayersStarterAction::action(Widget *sender, int actionType,
                                        event_manager::GameControlEvent *event)
{
    m_stateMachine.reset();
    m_stateMachine.evaluate();
}


//---------------------------------
// Legacy state machine, to be removed soon!!!
//---------------------------------
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

