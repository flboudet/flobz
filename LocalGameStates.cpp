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

#include "LocalGameStates.h"

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
    m_sharedAssets.m_currentLevelTheme = theCommander->getPreferedLevelTheme();
    m_sharedAssets.m_currentPuyoSetTheme = theCommander->getPreferedPuyoSetTheme();
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
        GameUIDefaults::GAME_LOOP->garbageCollect(m_sharedAssets.m_gameWidget.release());
        GameUIDefaults::GAME_LOOP->garbageCollect(m_sharedAssets.m_gameScreen.release());
    }
    m_sharedAssets.m_gameWidget.reset(newGameWidget);
    m_sharedAssets.m_gameScreen.reset(newGameScreen);
    GameUIDefaults::SCREEN_STACK->push(m_sharedAssets.m_gameScreen.get());
    // Set the game initially paused
    m_sharedAssets.m_gameScreen->setSuspended(true);
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
// EnterPlayerReadyState
//---------------------------------
EnterPlayerReadyState::EnterPlayerReadyState(SharedMatchAssets &sharedMatchAssets,
                                             SharedGetReadyAssets &sharedGetReadyAssets)
: CycledComponent(0.1), m_sharedAssets(sharedMatchAssets),
m_sharedGetReadyAssets(sharedGetReadyAssets),
m_getReadyDisplayed(false), m_nextState(NULL)
{
}

void EnterPlayerReadyState::enterState()
{
    cout << "EnterPlayerReadyState::enterState()" << endl;
    if (m_sharedAssets.m_currentLevelTheme->getReadyAnimation2P() == "") {
        return;
    }
    m_sharedGetReadyAssets.m_getReadyWidget.reset(new StoryWidget(m_sharedAssets.m_currentLevelTheme->getReadyAnimation2P().c_str(), this));
    m_sharedAssets.m_gameWidget->setGameOverAction(this);
    m_sharedAssets.m_gameScreen->setOverlayStory(m_sharedGetReadyAssets.m_getReadyWidget.get());
    GameUIDefaults::GAME_LOOP->addIdle(this);
}

void EnterPlayerReadyState::exitState()
{
    m_sharedAssets.m_gameWidget->setGameOverAction(NULL);
    GameUIDefaults::GAME_LOOP->removeIdle(this);
}

bool EnterPlayerReadyState::evaluate()
{
    return m_getReadyDisplayed;
}

GameState *EnterPlayerReadyState::getNextState()
{
    return m_nextState;
}

void EnterPlayerReadyState::action(Widget *sender, int actionType,
                                   event_manager::GameControlEvent *event)
{
    if (sender == m_sharedGetReadyAssets.m_getReadyWidget.get()) {
        m_sharedGetReadyAssets.m_getReadyWidget.reset(NULL);
    }
    else {
        //m_getReadyDisplayed = true;
    }

    evaluateStateMachine();
}

void EnterPlayerReadyState::cycle()
{
    StoryWidget *story = m_sharedGetReadyAssets.m_getReadyWidget.get();
    if (story->getIntegerValue("@getready_displayed") == 1) {
        m_getReadyDisplayed = true;
        evaluateStateMachine();
    }
}

//---------------------------------
// ExitPlayerReadyState
//---------------------------------
ExitPlayerReadyState::ExitPlayerReadyState(SharedMatchAssets &sharedMatchAssets,
                                           SharedGetReadyAssets &sharedGetReadyAssets)
: m_sharedAssets(sharedMatchAssets),
m_sharedGetReadyAssets(sharedGetReadyAssets),
m_nextState(NULL)
{
}

void ExitPlayerReadyState::enterState()
{
    cout << "ExitPlayerReadyState::enterState()" << endl;
    if (m_sharedGetReadyAssets.m_getReadyWidget.get() == NULL)
        return;
    m_sharedGetReadyAssets.m_getReadyWidget->setIntegerValue("@start_pressed", 1);
}

bool ExitPlayerReadyState::evaluate()
{
    return true;
}

GameState *ExitPlayerReadyState::getNextState()
{
    return m_nextState;
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
    m_gameLostWidget.reset(NULL);
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
    m_statsWidget.reset(NULL);
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
// DisplayStoryScreenState
//---------------------------------
DisplayStoryScreenState::DisplayStoryScreenState(const char *screenName)
    : m_screenName(screenName)
{
}

void DisplayStoryScreenState::enterState()
{
    cout << "DisplayStoryScreenState::enterState()" << endl;
    m_storyScreen.reset(new StoryScreen(m_screenName.c_str(),
                                        *(GameUIDefaults::SCREEN_STACK->top()),
                                        this));
    GameUIDefaults::SCREEN_STACK->push(m_storyScreen.get());
    m_acknowledged = false;
}

void DisplayStoryScreenState::exitState()
{
    GameUIDefaults::SCREEN_STACK->pop();
    m_storyScreen.reset(NULL);
}

bool DisplayStoryScreenState::evaluate()
{
    return m_acknowledged;
}

GameState *DisplayStoryScreenState::getNextState()
{
    return m_nextState;
}

void DisplayStoryScreenState::action(Widget *sender, int actionType,
                                     event_manager::GameControlEvent *event)
{
    m_acknowledged = true;
    evaluateStateMachine();
}

//---------------------------------
// LeaveGameState
//---------------------------------
LeaveGameState::LeaveGameState(SharedMatchAssets &sharedMatchAssets,
                               Action *actionToCallWhenLeft)
    : m_sharedAssets(sharedMatchAssets),
      m_actionToCallWhenLeft(actionToCallWhenLeft)
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
    if (m_actionToCallWhenLeft != NULL)
        m_actionToCallWhenLeft->action(NULL, 0, NULL);
}

bool LeaveGameState::evaluate()
{
    return false;
}

GameState *LeaveGameState::getNextState()
{
    return NULL;
}

