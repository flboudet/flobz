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
          _parentScreen(GameUIDefaults::SCREEN_STACK->top()) {}
    void drawAnyway(DrawTarget *dt) {
        _parentScreen->addToGameLoop(_parentScreen->getGameLoop());
        _parentScreen->show();
        _parentScreen->drawAnyway(dt);
        _parentScreen->hide();
        _parentScreen->removeFromGameLoopActive();
    }
    void draw(DrawTarget *dt) {
        _parentScreen->draw(dt);
    }
private:
    gameui::Screen *_parentScreen;
};
void PushScreenState::enterState()
{
    _ghostScreen.reset(new GhostScreen());
    GameUIDefaults::SCREEN_STACK->push(_ghostScreen.get());
}
void PushScreenState::exitState()
{
    GameUIDefaults::GAME_LOOP->garbageCollect(_ghostScreen.release());
}
bool PushScreenState::evaluate()
{
    return true;
}
GameState *PushScreenState::getNextState()
{
    return _nextState;
}

//---------------------------------
// SetupMatchState
//---------------------------------
SetupMatchState::SetupMatchState(GameWidgetFactory  *gameWidgetFactory,
                                 GameOptions         gameOptions,
                                 PlayerNameProvider *nameProvider,
                                 SharedMatchAssets  *sharedMatchAssets,
                                 int nbPlayers)
  : _nbPlayers(nbPlayers),
    _gameWidgetFactory(gameWidgetFactory),
    _gameOptions(gameOptions),
    _nameProvider(nameProvider),
    _sharedAssets(sharedMatchAssets),
    _nextState(NULL),
    _handicapOnVictorious(true),
    _displayVictories(false),
    _accountTotalOnPlayerB(true)
{
}

void SetupMatchState::enterState()
{
    GTLogTrace("SetupMatchState::enterState()");
    // Prepare 1st run
    _sharedAssets->_currentLevelTheme = theCommander->getPreferedLevelTheme(_nbPlayers);
    if (_sharedAssets->_currentLevelTheme.get() == NULL)
        throw std::runtime_error("No matching level theme");
    _sharedAssets->_currentFloboSetTheme = theCommander->getPreferedFloboSetTheme();
    // Create the gamewidget and register as the gamewidget's action
    GameWidget *newGameWidget =
        _gameWidgetFactory->createGameWidget(*(_sharedAssets->_currentFloboSetTheme),
                                              *(_sharedAssets->_currentLevelTheme),
                                              _sharedAssets->_currentLevelTheme->getCentralAnimation2P().c_str(), NULL);
    newGameWidget->setGameOptions(_gameOptions);
    if (_nameProvider != NULL) {
        newGameWidget->setPlayerOneName(_nameProvider->getPlayerName(0));
        newGameWidget->setPlayerTwoName(_nameProvider->getPlayerName(1));
    }
    // Setup total points
    newGameWidget->getStatPlayerOne().total_points = _sharedAssets->_leftTotal;
    if (_accountTotalOnPlayerB)
        newGameWidget->getStatPlayerTwo().total_points = _sharedAssets->_rightTotal;
    // Optionnaly setup victories
    if (_displayVictories)
        newGameWidget->setVictories(_sharedAssets->_leftVictories, _sharedAssets->_rightVictories);
    // Optionnaly setup handicap
    if (_handicapOnVictorious) {
        int victoriesDelta = _sharedAssets->_leftVictories - _sharedAssets->_rightVictories;
        if (victoriesDelta > 0) {
            newGameWidget->addGameAHandicap(victoriesDelta);
        }
        else if (victoriesDelta != 0) {
            newGameWidget->addGameBHandicap(-victoriesDelta);
        }
    }
    GameScreen *newGameScreen = new GameScreen(*(newGameWidget));
    // Handle eventual game enchainment
    if (_sharedAssets->_gameScreen.get() != NULL) {
        GameUIDefaults::GAME_LOOP->garbageCollect(_sharedAssets->_gameWidget.release());
        GameUIDefaults::GAME_LOOP->garbageCollect(_sharedAssets->_gameScreen.release());
    }
    _sharedAssets->_gameWidget.reset(newGameWidget);
    GameUIDefaults::SCREEN_STACK->swap(newGameScreen);
    _sharedAssets->_gameScreen.reset(newGameScreen);
    // Set the game initially paused
    _sharedAssets->_gameScreen->setSuspended(true);
}

bool SetupMatchState::evaluate()
{
    // This state is just intended to setup the game.
    // No suspending condition.
    return true;
}

GameState *SetupMatchState::getNextState()
{
    return _nextState;
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
: CycledComponent(0.1), _sharedAssets(sharedMatchAssets),
_sharedGetReadyAssets(sharedGetReadyAssets),
_getReadyDisplayed(false), _nextState(NULL)
{
}

void EnterPlayerReadyState::enterState()
{
    GTLogTrace("EnterPlayerReadyState::enterState()");
    if (_sharedAssets._currentLevelTheme->getReadyAnimation2P() == "") {
        return;
    }
    _getReadyDisplayed = false;
    _sharedGetReadyAssets._getReadyWidget.reset(new StoryWidget(_sharedAssets._currentLevelTheme->getReadyAnimation2P().c_str(), this));
    _sharedAssets._gameWidget->setGameOverAction(this);
    _sharedAssets._gameScreen->setOverlayStory(_sharedGetReadyAssets._getReadyWidget.get());
    GameUIDefaults::GAME_LOOP->addIdle(this);
}

void EnterPlayerReadyState::exitState()
{
    _sharedAssets._gameWidget->setGameOverAction(NULL);
    GameUIDefaults::GAME_LOOP->removeIdle(this);
}

bool EnterPlayerReadyState::evaluate()
{
    return _getReadyDisplayed;
}

GameState *EnterPlayerReadyState::getNextState()
{
    return _nextState;
}

void EnterPlayerReadyState::action(Widget *sender, int actionType,
                                   event_manager::GameControlEvent *event)
{
    if (sender == _sharedGetReadyAssets._getReadyWidget.get()) {
        _sharedGetReadyAssets._getReadyWidget.reset(NULL);
    }
    else {
        //_getReadyDisplayed = true;
    }
    evaluateStateMachine();
}

void EnterPlayerReadyState::cycle()
{
    StoryWidget *story = _sharedGetReadyAssets._getReadyWidget.get();
    if (story->getIntegerValue("@getready_displayed") == 1) {
        _getReadyDisplayed = true;
        evaluateStateMachine();
    }
}

void EnterPlayerReadyState::onEvent(GameControlEvent *cevent)
{
    if (!cevent->isUp) {
        switch (cevent->cursorEvent) {
            case kStart:
                _getReadyDisplayed = true;
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
: _sharedAssets(sharedMatchAssets),
_sharedGetReadyAssets(sharedGetReadyAssets),
_nextState(NULL)
{
}

void ExitPlayerReadyState::enterState()
{
    GTLogTrace("ExitPlayerReadyState::enterState()");
    if (_sharedGetReadyAssets._getReadyWidget.get() == NULL)
        return;
    _sharedGetReadyAssets._getReadyWidget->setIntegerValue("@start_pressed", 1);
}

bool ExitPlayerReadyState::evaluate()
{
    return true;
}

GameState *ExitPlayerReadyState::getNextState()
{
    return _nextState;
}

//---------------------------------
// WaitPlayersReadyState
//---------------------------------
WaitPlayersReadyState::WaitPlayersReadyState(SharedMatchAssets &sharedMatchAssets)
    : _sharedAssets(sharedMatchAssets),
      _playersAreReady(false),
      _nextState(NULL)
{
}

void WaitPlayersReadyState::enterState()
{
    GTLogTrace("WaitPlayersReadyState::enterState()");
    _playersAreReady = false;
    if (_sharedAssets._currentLevelTheme->getReadyAnimation2P() == "") {
        _playersAreReady = true;
        return;
    }
    _getReadyWidget.reset(new StoryWidget(_sharedAssets._currentLevelTheme->getReadyAnimation2P().c_str(), this));
    _sharedAssets._gameScreen->setOverlayStory(_getReadyWidget.get());
    _sharedAssets._gameWidget->setGameOverAction(this);
}

void WaitPlayersReadyState::exitState()
{
    _sharedAssets._gameWidget->setGameOverAction(NULL);
}

bool WaitPlayersReadyState::evaluate()
{
    return _playersAreReady;
}

GameState *WaitPlayersReadyState::getNextState()
{
    return _nextState;
}

void WaitPlayersReadyState::action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event)
{
    if (sender == _getReadyWidget.get()) {
        _getReadyWidget.reset(NULL);
    }
    if (_getReadyWidget.get() != NULL) {
        _getReadyWidget->setIntegerValue("@start_pressed", 1);
    }
    _playersAreReady = true;
    evaluateStateMachine();
}

//---------------------------------
// MatchPlayingState
//---------------------------------
MatchPlayingState::MatchPlayingState(SharedMatchAssets &sharedMatchAssets)
    : _sharedAssets(sharedMatchAssets),
      _gameIsOver(false),
      _nextState(NULL),
      _abortedState(NULL)
{
}

void MatchPlayingState::enterState()
{
    GTLogTrace("MatchPlaying::enterState()");
    _gameIsOver = false;
    _sharedAssets._gameWidget->setGameOverAction(this);
    // Resume the game
    _sharedAssets._gameScreen->setSuspended(false);
}

void MatchPlayingState::exitState()
{
    _sharedAssets._gameWidget->setGameOverAction(NULL);
}

bool MatchPlayingState::evaluate()
{
    return _gameIsOver;
}

GameState *MatchPlayingState::getNextState()
{
    if (_sharedAssets._gameWidget->getAborted())
        return _abortedState;
    return _nextState;
}

void MatchPlayingState::action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event)
{
    if (actionType == GameWidget::GAME_IS_OVER) {
        _gameIsOver = true;
        evaluateStateMachine();
    }
}

//---------------------------------
// MatchIsOverState
//---------------------------------
MatchIsOverState::MatchIsOverState(SharedMatchAssets &sharedMatchAssets)
    : _sharedAssets(sharedMatchAssets),
      _aknowledged(false)
{
}

MatchIsOverState::~MatchIsOverState()
{
    if (_gameLostWidget.get() != NULL) {
        _gameLostWidget->getParentScreen()->removeAction(this);
        _gameLostWidget.reset(NULL);
    }
}

void MatchIsOverState::enterState()
{
    GTLogTrace("MatchIsOver::enterState()");
    _aknowledged = false;
    if (_styrolyseName != "") {
        _gameLostWidget.reset(new StoryWidget(_styrolyseName.c_str(), this));
    }
    else if (_sharedAssets._gameWidget->isGameARunning()) {
        _gameLostWidget.reset(new StoryWidget(_sharedAssets._currentLevelTheme->getGameLostRightAnimation2P().c_str(), this));
        _sharedAssets._leftVictories++;
    }
    else {
        _gameLostWidget.reset(new StoryWidget(_sharedAssets._currentLevelTheme->getGameLostLeftAnimation2P().c_str(), this));
        _sharedAssets._rightVictories++;
    }
    _sharedAssets._leftTotal  += _sharedAssets._gameWidget->getStatPlayerOne().points;
    _sharedAssets._rightTotal += _sharedAssets._gameWidget->getStatPlayerTwo().points;
    _sharedAssets._gameWidget->setGameOverAction(this);
    _sharedAssets._gameScreen->setOverlayStory(_gameLostWidget.get());
    _sharedAssets._gameScreen->addAction(this);
}

void MatchIsOverState::exitState()
{
    GTLogTrace("MatchIsOver::exitState()");
    _sharedAssets._gameWidget->setGameOverAction(NULL);
}

bool MatchIsOverState::evaluate()
{
    return _aknowledged;
}

GameState *MatchIsOverState::getNextState()
{
    return _nextState;
}

void MatchIsOverState::action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event)
{
    if (sender == _gameLostWidget.get()) {
        GTLogTrace("MatchIsOver: acknowledged by animation widget");
        _gameLostWidget.reset(NULL);
    }
    else if ((actionType == GameWidget::GAMEOVER_STARTPRESSED)
             || ((_gameLostWidget.get() != NULL)
                 && (sender == (Widget *)(_gameLostWidget->getParentScreen()))))
    {
        GTLogTrace("MatchIsOver: acknowledged by screen action");
        _gameLostWidget->getParentScreen()->removeAction(this);
        _gameLostWidget.reset(NULL);
    }
    else
        return;
    _aknowledged = true;
    evaluateStateMachine();
}

//---------------------------------
// DisplayStatsState
//---------------------------------
DisplayStatsState::DisplayStatsState(SharedMatchAssets &sharedMatchAssets)
    : _sharedAssets(sharedMatchAssets),
      _aknowledged(false),
      _dimensions(416, 194, 50, Vec3(0, 0), Vec3(0, 0))
{
}

DisplayStatsState::~DisplayStatsState()
{
    if (_statsWidget.get() != NULL) {
        _statsWidget->getParentScreen()->removeAction(this);
        _statsWidget.reset(NULL);
    }
}

void DisplayStatsState::enterState()
{
    GTLogTrace("DisplayStats::enterState()");
    _aknowledged = false;
    LevelTheme *lvlTheme = _sharedAssets._currentLevelTheme;
    _dimensions = StatsWidgetDimensions(
        lvlTheme->getStatsHeight(),
        lvlTheme->getStatsLegendWidth(),
        lvlTheme->getStatsComboLineValueWidth(),
        Vec3(lvlTheme->getStatsLeftBackgroundOffsetX(),
             lvlTheme->getStatsLeftBackgroundOffsetY()),
        Vec3(lvlTheme->getStatsRightBackgroundOffsetX(),
             lvlTheme->getStatsRightBackgroundOffsetY()));
    _sharedAssets._gameWidget->setGameOverAction(this);
    _statsWidget.reset(new TwoPlayersStatsWidget(_sharedAssets._gameWidget->getStatPlayerOne(), _sharedAssets._gameWidget->getStatPlayerTwo(), true, true, theCommander->getWindowFramePicture(), _dimensions));
    _sharedAssets._gameScreen->add(_statsWidget.get());
    _sharedAssets._gameScreen->addAction(this);
}

void DisplayStatsState::exitState()
{
    _sharedAssets._gameWidget->setGameOverAction(NULL);
}

bool DisplayStatsState::evaluate()
{
    return _aknowledged;
}

GameState *DisplayStatsState::getNextState()
{
    return _nextState;
}

void DisplayStatsState::action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event)
{
    if (sender == (Widget *)(_statsWidget->getParentScreen())) {
        _statsWidget->getParentScreen()->removeAction(this);
        _statsWidget.reset(NULL);
    }
    else {
        _aknowledged = true;
        evaluateStateMachine();
    }
}

//---------------------------------
// DisplayStoryScreenState
//---------------------------------
DisplayStoryScreenState::DisplayStoryScreenState(const std::string &screenName)
    : _screenName(screenName), _vp(NULL)
{
}

void DisplayStoryScreenState::enterState()
{
    GTLogTrace("DisplayStoryScreenState(%s)::enterState()", _screenName.c_str());
    _storyScreen.reset(new StoryScreen(_screenName.c_str(),
                                        this));
    // Setup variables value if necessary
    if (_vp != NULL) {
        std::map<std::string, int> ivalues = _vp->getIntValues();
        for (std::map<std::string, int>::iterator iter = ivalues.begin() ;
             iter != ivalues.end() ; ++iter) {
            _storyScreen->getStoryWidget()->setIntegerValue(iter->first.c_str(), iter->second);
        }
        std::map<std::string, float> fvalues = _vp->getFloatValues();
        for (std::map<std::string, float>::iterator iter = fvalues.begin() ;
             iter != fvalues.end() ; ++iter) {
            _storyScreen->getStoryWidget()->setFloatValue(iter->first.c_str(), iter->second);
        }
        std::map<std::string, std::string> svalues = _vp->getStringValues();
        for (std::map<std::string, std::string>::iterator iter = svalues.begin() ;
             iter != svalues.end() ; ++iter) {
            _storyScreen->getStoryWidget()->setStringValue(iter->first.c_str(), iter->second.c_str());
        }
    }
    // Put the story screen on top of the screen stack
    GameUIDefaults::SCREEN_STACK->swap(_storyScreen.get());
    _acknowledged = false;
}

void DisplayStoryScreenState::exitState()
{
    GameUIDefaults::GAME_LOOP->garbageCollect(_storyScreen.release());
}

bool DisplayStoryScreenState::evaluate()
{
    return _acknowledged;
}

GameState *DisplayStoryScreenState::getNextState()
{
    return _nextState;
}

void DisplayStoryScreenState::action(Widget *sender, int actionType,
                                     event_manager::GameControlEvent *event)
{
    _acknowledged = true;
    evaluateStateMachine();
}

//---------------------------------
// ManageHiScoresState
//---------------------------------
ManageHiScoresState::ManageHiScoresState(SharedMatchAssets  *sharedMatchAssets,
                                         PlayerNameProvider *nameProvider,
                                         const char         *scoreBoardId,
                                         const char         *storyName,
                                         StoryNameProvider  *storyNameProvider)
    : _boardId(scoreBoardId),
      _sharedMatchAssets(sharedMatchAssets),
      _nameProvider(nameProvider)
{
    _newHiScore.reset(new DisplayStoryScreenState("new_hiscore.gsl"));
    _displayHallOfFame.reset(new DisplayHallOfFameState(storyName, storyNameProvider));
    _endOfStateMachine.reset(new CallActionState(this, 0));
    _newHiScore->setNextState(_displayHallOfFame.get());
    _displayHallOfFame->setNextState(_endOfStateMachine.get());
}

void ManageHiScoresState::enterState()
{
    _finished = false;
    // Initializes the score board
    const PlayerGameStat &playerPoints = _sharedMatchAssets->_gameWidget->getStatPlayerOne();
    _scoreBoard.reset(new LocalStorageHiScoreBoard(_boardId.c_str(), theCommander->getPreferencesManager(), _defaultScoreBoard));
    // Adds the latest score
    int rank = _scoreBoard->setHiScore(_nameProvider->getPlayerName(0).c_str(),
                                        playerPoints.points +
                                        playerPoints.total_points);
    // Declare score to the achievements manager
    if (theCommander->getAchievementsManager() != NULL)
        theCommander->getAchievementsManager()->declareScore(_boardId.c_str(),
                                                             playerPoints.points +
                                                             playerPoints.total_points);
    // Updates the displayHallOfFame state
    _displayHallOfFame->setHiScoreBoard(_scoreBoard.get());
    _displayHallOfFame->setFinalScore(_nameProvider->getPlayerName(0).c_str(),
                                       playerPoints.points +
                                       playerPoints.total_points);
    _displayHallOfFame->setRank(rank);
    // Initializing the state machine
    if (rank == -1)
        _stateMachine.setInitialState(_displayHallOfFame.get());
    else
        _stateMachine.setInitialState(_newHiScore.get());
    // Run the state machine
    _stateMachine.reset();
    _stateMachine.evaluate();
}

void ManageHiScoresState::exitState()
{
}

bool ManageHiScoresState::evaluate()
{
    return _finished;
}

GameState *ManageHiScoresState::getNextState()
{
    return _nextState;
}

void ManageHiScoresState::action(Widget *sender, int actionType,
                                 event_manager::GameControlEvent *event)
{
    _finished = true;
    evaluateStateMachine();
}

//---------------------------------
// DisplayHallOfFameState
//---------------------------------
DisplayHallOfFameState::DisplayHallOfFameState(const char         *storyName,
                                               StoryNameProvider  *storyNameProvider)
    : _storyName(storyName),
      _storyNameProvider(storyNameProvider)
{
}

void DisplayHallOfFameState::enterState()
{
    if (_storyNameProvider != NULL)
        _storyName = _storyNameProvider->getStoryName();
    GTLogTrace("StoryModeDisplayHallOfFameState(%s)::enterState()", _storyName.c_str());
    _gameOverScreen.reset(new GameOverScreen(_storyName.c_str(),
                                              this));

    //const PlayerGameStat &playerPoints = _sharedMatchAssets->_gameWidget->getStatPlayerOne();

    _gameOverScreen->setScoreBoard(_scoreBoard);
    _gameOverScreen->setFinalScore(_playerName.c_str(), _playerScore);
    _gameOverScreen->highlightRank(_rank);
    GameUIDefaults::SCREEN_STACK->swap(_gameOverScreen.get());
    _gameOverScreen->refresh();
    _acknowledged = false;
}

void DisplayHallOfFameState::exitState()
{
    GameUIDefaults::GAME_LOOP->garbageCollect(_gameOverScreen.release());
}

bool DisplayHallOfFameState::evaluate()
{
    return _acknowledged;
}

GameState *DisplayHallOfFameState::getNextState()
{
    return _nextState;
}

void DisplayHallOfFameState::action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event)
{
    _acknowledged = true;
    evaluateStateMachine();
}

//---------------------------------
// LeaveGameState
//---------------------------------
LeaveGameState::LeaveGameState(SharedMatchAssets &sharedMatchAssets,
                               Action *actionToCallWhenLeft)
    : _sharedAssets(sharedMatchAssets),
      _actionToCallWhenLeft(actionToCallWhenLeft)
{
}

void LeaveGameState::enterState()
{
    GTLogTrace("LeaveGame::enterState()");
    GameUIDefaults::SCREEN_STACK->pop();
    //MainScreen *menuScreen = dynamic_cast<MainScreen *>(GameUIDefaults::SCREEN_STACK->top());
    //if (menuScreen != NULL)
    //    menuScreen->transitionFromScreen(*(_sharedAssets._gameScreen));
    _sharedAssets.release();
    if (_actionToCallWhenLeft != NULL)
        _actionToCallWhenLeft->action(NULL, 0, NULL);
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
    : _actionToCall(actionToCall), _actionType(actionType)
{
}

void CallActionState::enterState()
{
    GTLogTrace("CallAction::enterState()");
    _actionToCall->action(NULL, _actionType, NULL);
}

bool CallActionState::evaluate()
{
    return false;
}

GameState *CallActionState::getNextState()
{
    return NULL;
}

//---------------------------------
// ManageMultiSetsState
//---------------------------------
ManageMultiSetsState::ManageMultiSetsState(SharedMatchAssets  *sharedMatchAssets, int nbSets, PlayerNameProvider *nameProvider)
    : _sharedAssets(sharedMatchAssets), _nameProvider(nameProvider), _nbSets(nbSets)
{
}

void ManageMultiSetsState::enterState()
{
}

void ManageMultiSetsState::exitState()
{
}

bool ManageMultiSetsState::evaluate()
{
    return true;
}

GameState *ManageMultiSetsState::getNextState()
{
    if ((_sharedAssets->_leftVictories >= _nbSets)
        || (_sharedAssets->_rightVictories >= _nbSets))
        return _endOfGameState;
    return _nextSetState;
}

std::map<std::string, int> ManageMultiSetsState::getIntValues() const
{
    std::map<std::string, int> result;
    return result;
}

std::map<std::string, float> ManageMultiSetsState::getFloatValues() const
{
    std::map<std::string, float> result;
    result["@leftVictories"] = _sharedAssets->_leftVictories;
    result["@rightVictories"] = _sharedAssets->_rightVictories;
    return result;
}

std::map<std::string, std::string> ManageMultiSetsState::getStringValues() const
{
    std::map<std::string, std::string> result;
    if (_nameProvider != NULL) {
        result["@leftName"] = _nameProvider->getPlayerName(0);
        result["@rightName"] = _nameProvider->getPlayerName(1);
    }
    return result;
}
