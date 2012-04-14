/* FloboPop
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

#include "GTLog.h"
#include "LocalGameStates.h"

using namespace event_manager;

//---------------------------------
// PushScreenState
//---------------------------------
class PushScreenState::GhostScreen : public gameui::Screen
{
public:
    GhostScreen(GameLoop *loop = NULL)
        : gameui::Screen(loop),
          m_parentScreen(GameUIDefaults::SCREEN_STACK->top()) {}
    void drawAnyway(DrawTarget *dt) {
        m_parentScreen->addToGameLoop(m_parentScreen->getGameLoop());
        m_parentScreen->show();
        m_parentScreen->drawAnyway(dt);
        m_parentScreen->hide();
        m_parentScreen->removeFromGameLoopActive();
    }
    void draw(DrawTarget *dt) {
        m_parentScreen->draw(dt);
    }
private:
    gameui::Screen *m_parentScreen;
};
void PushScreenState::enterState()
{
    m_ghostScreen.reset(new GhostScreen());
    GameUIDefaults::SCREEN_STACK->push(m_ghostScreen.get());
}
void PushScreenState::exitState()
{
    GameUIDefaults::GAME_LOOP->garbageCollect(m_ghostScreen.release());
}
bool PushScreenState::evaluate()
{
    return true;
}
GameState *PushScreenState::getNextState()
{
    return m_nextState;
}

//---------------------------------
// SetupMatchState
//---------------------------------
SetupMatchState::SetupMatchState(GameWidgetFactory  *gameWidgetFactory,
                                 GameOptions         gameOptions,
                                 PlayerNameProvider *nameProvider,
                                 SharedMatchAssets  *sharedMatchAssets,
                                 int nbPlayers)
  : m_nbPlayers(nbPlayers),
    m_gameWidgetFactory(gameWidgetFactory),
    m_gameOptions(gameOptions),
    m_nameProvider(nameProvider),
    m_sharedAssets(sharedMatchAssets),
    m_nextState(NULL),
    m_handicapOnVictorious(true),
    m_accountTotalOnPlayerB(true)
{
}

void SetupMatchState::enterState()
{
    GTLogTrace("SetupMatchState::enterState()");
    // Prepare 1st run
    m_sharedAssets->m_currentLevelTheme = theCommander->getPreferedLevelTheme(m_nbPlayers);
    if (m_sharedAssets->m_currentLevelTheme.get() == NULL)
        throw ios_fc::Exception("No matching level theme");
    m_sharedAssets->m_currentFloboSetTheme = theCommander->getPreferedFloboSetTheme();
    // Create the gamewidget and register as the gamewidget's action
    GameWidget *newGameWidget =
        m_gameWidgetFactory->createGameWidget(*(m_sharedAssets->m_currentFloboSetTheme),
                                              *(m_sharedAssets->m_currentLevelTheme),
                                              m_sharedAssets->m_currentLevelTheme->getCentralAnimation2P().c_str(), NULL);
    newGameWidget->setGameOptions(m_gameOptions);
    if (m_nameProvider != NULL) {
        newGameWidget->setPlayerOneName(m_nameProvider->getPlayerName(0));
        newGameWidget->setPlayerTwoName(m_nameProvider->getPlayerName(1));
    }
    // Setup total points
    newGameWidget->getStatPlayerOne().total_points = m_sharedAssets->m_leftTotal;
    if (m_accountTotalOnPlayerB)
        newGameWidget->getStatPlayerTwo().total_points = m_sharedAssets->m_rightTotal;
    // Optionnaly setup handicap
    if (m_handicapOnVictorious) {
        int victoriesDelta = m_sharedAssets->m_leftVictories - m_sharedAssets->m_rightVictories;
        if (victoriesDelta > 0) {
            newGameWidget->addGameAHandicap(victoriesDelta);
        }
        else if (victoriesDelta != 0) {
            newGameWidget->addGameBHandicap(-victoriesDelta);
        }
    }
    GameScreen *newGameScreen = new GameScreen(*(newGameWidget));
    // Handle eventual game enchainment
    if (m_sharedAssets->m_gameScreen.get() != NULL) {
        GameUIDefaults::GAME_LOOP->garbageCollect(m_sharedAssets->m_gameWidget.release());
        GameUIDefaults::GAME_LOOP->garbageCollect(m_sharedAssets->m_gameScreen.release());
    }
    m_sharedAssets->m_gameWidget.reset(newGameWidget);
    GameUIDefaults::SCREEN_STACK->swap(newGameScreen);
    m_sharedAssets->m_gameScreen.reset(newGameScreen);
    // Set the game initially paused
    m_sharedAssets->m_gameScreen->setSuspended(true);
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
    GTLogTrace("EnterPlayerReadyState::enterState()");
    if (m_sharedAssets.m_currentLevelTheme->getReadyAnimation2P() == "") {
        return;
    }
    m_getReadyDisplayed = false;
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

void EnterPlayerReadyState::onEvent(GameControlEvent *cevent)
{
    if (!cevent->isUp) {
        switch (cevent->cursorEvent) {
            case kStart:
                m_getReadyDisplayed = true;
                evaluateStateMachine();
                break;
            default:
                break;
        }
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
    GTLogTrace("ExitPlayerReadyState::enterState()");
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
    GTLogTrace("WaitPlayersReadyState::enterState()");
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
    GTLogTrace("MatchPlaying::enterState()");
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

MatchIsOverState::~MatchIsOverState()
{
    if (m_gameLostWidget.get() != NULL) {
        m_gameLostWidget->getParentScreen()->removeAction(this);
        m_gameLostWidget.reset(NULL);
    }
}

void MatchIsOverState::enterState()
{
    GTLogTrace("MatchIsOver::enterState()");
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
    m_sharedAssets.m_gameScreen->addAction(this);
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
    else if (sender == (Widget *)(m_gameLostWidget->getParentScreen())) {
        m_gameLostWidget->getParentScreen()->removeAction(this);
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
      m_aknowledged(false),
      m_dimensions(416, 194, 50, Vec3(0, 0), Vec3(0, 0))
{
}

DisplayStatsState::~DisplayStatsState()
{
    if (m_statsWidget.get() != NULL) {
        m_statsWidget->getParentScreen()->removeAction(this);
        m_statsWidget.reset(NULL);
    }
}

void DisplayStatsState::enterState()
{
    GTLogTrace("DisplayStats::enterState()");
    m_aknowledged = false;
    LevelTheme *lvlTheme = m_sharedAssets.m_currentLevelTheme;
    m_dimensions = StatsWidgetDimensions(
        lvlTheme->getStatsHeight(),
        lvlTheme->getStatsLegendWidth(),
        lvlTheme->getStatsComboLineValueWidth(),
        Vec3(lvlTheme->getStatsLeftBackgroundOffsetX(),
             lvlTheme->getStatsLeftBackgroundOffsetY()),
        Vec3(lvlTheme->getStatsRightBackgroundOffsetX(),
             lvlTheme->getStatsRightBackgroundOffsetY()));
    m_sharedAssets.m_gameWidget->setGameOverAction(this);
    m_statsWidget.reset(new TwoPlayersStatsWidget(m_sharedAssets.m_gameWidget->getStatPlayerOne(), m_sharedAssets.m_gameWidget->getStatPlayerTwo(), true, true, theCommander->getWindowFramePicture(), m_dimensions));
    m_sharedAssets.m_gameScreen->add(m_statsWidget.get());
    m_sharedAssets.m_gameScreen->addAction(this);
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
    if (sender == (Widget *)(m_statsWidget->getParentScreen())) {
        m_statsWidget->getParentScreen()->removeAction(this);
        m_statsWidget.reset(NULL);
    }
    else {
        m_aknowledged = true;
        evaluateStateMachine();
    }
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
    GTLogTrace("DisplayStoryScreenState(%s)::enterState()", m_screenName.c_str());
    m_storyScreen.reset(new StoryScreen(m_screenName.c_str(),
                                        this));
    GameUIDefaults::SCREEN_STACK->swap(m_storyScreen.get());
    m_acknowledged = false;
}

void DisplayStoryScreenState::exitState()
{
    GameUIDefaults::GAME_LOOP->garbageCollect(m_storyScreen.release());
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
// DisplayHallOfFameState
//---------------------------------
DisplayHallOfFameState::DisplayHallOfFameState(SharedMatchAssets  *sharedMatchAssets,
                                               PlayerNameProvider *nameProvider,
                                               const char         *scoreBoardId,
                                               const char         *storyName,
                                               StoryNameProvider  *storyNameProvider)
    : m_boardId(scoreBoardId),
      m_storyName(storyName), m_sharedMatchAssets(sharedMatchAssets),
      m_nameProvider(nameProvider),
      m_storyNameProvider(storyNameProvider)
{
}

void DisplayHallOfFameState::enterState()
{
    if (m_storyNameProvider != NULL)
        m_storyName = m_storyNameProvider->getStoryName();
    GTLogTrace("StoryModeDisplayHallOfFameState(%s)::enterState()", m_storyName.c_str());

    const PlayerGameStat &playerPoints = m_sharedMatchAssets->m_gameWidget->getStatPlayerOne();
    m_gameOverScreen.reset(new GameOverScreen(m_storyName.c_str(),
                                              this));
    m_scoreBoard.reset(new LocalStorageHiScoreBoard(m_boardId.c_str(), theCommander->getPreferencesManager(), m_defaultScoreBoard));
    m_gameOverScreen->setScoreBoard(m_scoreBoard.get());
    m_gameOverScreen->setFinalScore(m_nameProvider->getPlayerName(0),
                                    playerPoints.points +
                                    playerPoints.total_points);
    GameUIDefaults::SCREEN_STACK->swap(m_gameOverScreen.get());
    m_gameOverScreen->refresh();
    m_acknowledged = false;
}

void DisplayHallOfFameState::exitState()
{
    GameUIDefaults::GAME_LOOP->garbageCollect(m_gameOverScreen.release());
}

bool DisplayHallOfFameState::evaluate()
{
    return m_acknowledged;
}

GameState *DisplayHallOfFameState::getNextState()
{
    return m_nextState;
}

void DisplayHallOfFameState::action(Widget *sender, int actionType,
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
    GTLogTrace("LeaveGame::enterState()");
    GameUIDefaults::SCREEN_STACK->pop();
    //MainScreen *menuScreen = dynamic_cast<MainScreen *>(GameUIDefaults::SCREEN_STACK->top());
    //if (menuScreen != NULL)
    //    menuScreen->transitionFromScreen(*(m_sharedAssets.m_gameScreen));
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

//---------------------------------
// CallActionState
//---------------------------------
CallActionState::CallActionState(Action *actionToCall, int actionType)
    : m_actionToCall(actionToCall), m_actionType(actionType)
{
}

void CallActionState::enterState()
{
    GTLogTrace("CallAction::enterState()");
    m_actionToCall->action(NULL, m_actionType, NULL);
}

bool CallActionState::evaluate()
{
    return false;
}

GameState *CallActionState::getNextState()
{
    return NULL;
}
