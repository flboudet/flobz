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
#include "FPStrings.h"
#include "StoryModeStarter.h"
#include "GameView.h"
#include "GSLFileAccessWrapper.h"

using namespace event_manager;

StoryModeGameWidget::StoryModeGameWidget(int lifes, const std::string & aiFace)
    : _faceTicks(0), _opponent(aiFace),
      _killLeftCheat("killleft", this),
      _killRightCheat("killright", this)
{
    setLives(lifes);
}

void StoryModeGameWidget::initWithGUI(GameView &areaA, GameView &areaB,
                                            GamePlayer *playercontroller,
                                            LevelTheme &levelTheme,
                                            int level,
                                            Action *gameOverAction)
{
    // Jeko's corrected_level ensures easier games on easiest levels
    int corrected_level = level;
    if (level > 12) corrected_level = level + (3 - _lives) / 2; // Half of medium level, make sure the player do not loose 3 times the same oponent.
    if (level >= 20) corrected_level = level + (3 - _lives); // Easy level, make sure the player do always loose the same oponent.
    _controllerA.reset(playercontroller);
    _controllerB.reset(new AIPlayer(corrected_level, areaB));
    GameWidget2P::initWithGUI(areaA, areaB,
                               levelTheme, gameOverAction);
    addSubWidget(&_killLeftCheat);
    addSubWidget(&_killRightCheat);
	int scoringLevel = 30/level;
	if (scoringLevel < 1) scoringLevel = 1;
	if (scoringLevel > 10) scoringLevel = 10;
	areaA.getAttachedGame()->setScoringLevel(scoringLevel);
	areaB.getAttachedGame()->setScoringLevel(scoringLevel);
}

StoryModeStandardLayoutGameWidget::StoryModeStandardLayoutGameWidget(FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, int level, int nColors, int lifes, const std::string & aiFace, Action *gameOverAction)
  : StoryModeGameWidget(lifes, aiFace),
      _attachedFloboThemeSet(floboSetTheme),
      _attachedRandom(nColors),
      _attachedGameFactory(&_attachedRandom),
      _areaA(&_attachedGameFactory, 0, &_attachedFloboThemeSet, &levelTheme),
      _areaB(&_attachedGameFactory, 1, &_attachedFloboThemeSet, &levelTheme)
{
    initWithGUI(_areaA, _areaB, new CombinedEventPlayer(_areaA), levelTheme, level, gameOverAction);
}

StoryModeGameWidget::~StoryModeGameWidget()
{
}

void StoryModeGameWidget::cycle()
{
    _faceTicks += 1;
    if (_faceTicks == 1) {
        AIParameters ai;
        ai.realSuppressionValue = _opponent.getIntegerValue("@AI_RealSuppression");
        ai.potentialSuppressionValue = _opponent.getIntegerValue("@AI_PotentialSuppression");
        ai.criticalHeight = _opponent.getIntegerValue("@AI_CriticalHeight");
        ai.columnScalar[0] = _opponent.getIntegerValue("@AI_Column1");
        ai.columnScalar[1] = _opponent.getIntegerValue("@AI_Column2");
        ai.columnScalar[2] = _opponent.getIntegerValue("@AI_Column3");
        ai.columnScalar[3] = _opponent.getIntegerValue("@AI_Column4");
        ai.columnScalar[4] = _opponent.getIntegerValue("@AI_Column5");
        ai.columnScalar[5] = _opponent.getIntegerValue("@AI_Column6");
        ai.rotationMethod = _opponent.getIntegerValue("@AI_RotationMethod");
        ai.fastDropDelta = _opponent.getIntegerValue("@AI_FastDropDelta");
        ai.thinkDepth = _opponent.getIntegerValue("@AI_ThinkDepth");
        AIPlayer *opponentcontroller = static_cast<AIPlayer *>(_controllerB.get());
        opponentcontroller->setAIParameters(ai);

		_opponent.setIntegerValue("@AI_PlayingLevel", opponentcontroller->getLevel());
        _opponent.setIntegerValue("@maxHeightLeft", _attachedGameA->getColumnHeigth(2));
        _opponent.setIntegerValue("@maxHeightRight", _attachedGameB->getColumnHeigth(2));
        _opponent.setIntegerValue("@maxHeightPlayer", _attachedGameA->getColumnHeigth(2));
        _opponent.setIntegerValue("@maxHeightAI", _attachedGameB->getColumnHeigth(2));
        _opponent.setIntegerValue("@neutralsForPlayer", _attachedGameA->getNeutralFlobos());
        _opponent.setIntegerValue("@neutralsForAI", _attachedGameB->getNeutralFlobos());
        _opponent.setIntegerValue("@gameOverLeft",  !this->isGameARunning());
        _opponent.setIntegerValue("@gameOverRight", !this->isGameBRunning());
    }
    if (_faceTicks == 100) {
        _faceTicks = 0;
    }
    GameWidget2P::cycle();
}

StoryWidget *StoryModeGameWidget::getOpponent()
{
    return &_opponent;
}

void StoryModeGameWidget::action(Widget *sender, int actionType,
                                        GameControlEvent *event)
{
    if (sender == static_cast<Widget *>(&_killLeftCheat))
        addGameAHandicap(FLOBOBAN_DIMY);
    else if (sender == static_cast<Widget *>(&_killRightCheat))
        addGameBHandicap(FLOBOBAN_DIMY);
}

StoryModeLevelsDefinition *StoryModeLevelsDefinition::_currentDefinition = NULL;

StoryModeLevelsDefinition::StoryModeLevelsDefinition(const std::string & levelDefinitionFile)
{
    GoomSL * gsl = gsl_new();
    if (!gsl) return;
    GSLFA_setupWrapper(gsl, &(theCommander->getDataPathManager()));
    gsl_push_file(gsl, "/lib/levellib.gsl");
    gsl_push_file(gsl, levelDefinitionFile.c_str());
    gsl_compile(gsl);
    _currentDefinition = this;
    gsl_bind_function(gsl, "end_level",  StoryModeLevelsDefinition::end_level);
    gsl_bind_function(gsl, "getBoolPreference", StoryModeLevelsDefinition::get_BoolPreference);
    gsl_execute(gsl);
    gsl_free(gsl);
}

StoryModeLevelsDefinition::~StoryModeLevelsDefinition()
{
  for (int i = 0 ; i < _levelDefinitions.size() ; i++) {
    delete _levelDefinitions[i];
  }
}

void StoryModeLevelsDefinition::addLevelDefinition(const std::string & levelName, const std::string & introStory,
					      const std::string & opponentStory, const std::string & opponentName,
					      const std::string & opponent, const std::string & backgroundTheme,
					      const std::string & gameLostStory, const std::string & gameWonStory, const std::string & gameOverStory,
					      SelIA easySettings,
					      SelIA mediumSettings, SelIA hardSettings)
{
  _levelDefinitions.add(new LevelDefinition(levelName, introStory, opponentStory, opponentName,
					   opponent, backgroundTheme, gameLostStory, gameWonStory, gameOverStory,
					   easySettings, mediumSettings, hardSettings));
}

StoryModeLevelsDefinition::SelIA::SelIA(int level, int nColors) : _level(level), _nColors(nColors)
{
}

void StoryModeLevelsDefinition::get_BoolPreference(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    char *name  = (char*)GSL_LOCAL_PTR(gsl, local, "name");
    int def     = GSL_LOCAL_INT(gsl, local, "default");
    GSL_GLOBAL_INT(gsl, "getBoolPreference")
        = theCommander->getPreferencesManager()->getBoolPreference(name, def);
}

void StoryModeLevelsDefinition::end_level(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
  const char * levelName = (const char *) GSL_GLOBAL_PTR(gsl, "level.levelName");
  const char * introStory = (const char *) GSL_GLOBAL_PTR(gsl, "level.introStory");
  const char * opponentStory = (const char *) GSL_GLOBAL_PTR(gsl, "level.opponentStory");
  const char * opponentName = (const char *) GSL_GLOBAL_PTR(gsl, "level.opponentName");
  const char * opponent = (const char *) GSL_GLOBAL_PTR(gsl, "level.opponentAI");
  const char * backgroundTheme = (const char *) GSL_GLOBAL_PTR(gsl, "level.backgroundTheme");
  const char * gameLostStory = (const char *) GSL_GLOBAL_PTR(gsl, "level.gameLostStory");
  const char * gameWonStory = (const char *) GSL_GLOBAL_PTR(gsl, "level.gameWonStory");
  const char * gameOverStory = (const char *) GSL_GLOBAL_PTR(gsl, "level.gameOverStory");
  if (gameLostStory == NULL)
      gameLostStory = "";
  if (gameWonStory == NULL)
      gameWonStory = "";
  SelIA easySettings(GSL_GLOBAL_INT(gsl, "level.easySetting.level"),
                     GSL_GLOBAL_INT(gsl, "level.easySetting.nColors"));
  SelIA mediumSettings(GSL_GLOBAL_INT(gsl, "level.mediumSetting.level"),
                       GSL_GLOBAL_INT(gsl, "level.mediumSetting.nColors"));
  SelIA hardSettings(GSL_GLOBAL_INT(gsl, "level.hardSetting.level"),
                     GSL_GLOBAL_INT(gsl, "level.hardSetting.nColors"));
  _currentDefinition->addLevelDefinition(levelName, introStory, opponentStory, opponentName,
					opponent, backgroundTheme, gameLostStory, gameWonStory, gameOverStory,
					easySettings, mediumSettings, hardSettings);
}

//---------------------------------
// StoryModeMatchIsOverState
//---------------------------------
StoryModeMatchIsOverState::StoryModeMatchIsOverState(SharedGameAssets *sharedGameAssets,
                                                           SharedMatchAssets *sharedMatchAssets)
    : _sharedGameAssets(sharedGameAssets),
      _sharedMatchAssets(sharedMatchAssets),
      _aknowledged(false)
{
}

StoryModeMatchIsOverState::~StoryModeMatchIsOverState()
{
    if (_gameLostWidget.get() != NULL) {
        _gameLostWidget->getParentScreen()->removeAction(this);
        _gameLostWidget.reset(NULL);
    }
}

void StoryModeMatchIsOverState::enterState()
{
    GTLogTrace("StoryModeMatchIsOver::enterState()");
    _aknowledged = false;
    if (_sharedMatchAssets->_gameWidget->isGameARunning()) {
        _sharedMatchAssets->_leftVictories++;
        if (_sharedGameAssets->_levelDef->_gameWonStory == "")
            _aknowledged = true;
        else {
            _gameLostWidget.reset(new StoryWidget(_sharedGameAssets->_levelDef->_gameWonStory, this));
            _sharedMatchAssets->_gameScreen->setOverlayStory(_gameLostWidget.get());
            _sharedMatchAssets->_gameScreen->addAction(this);
        }
    }
    else {
        _sharedMatchAssets->_rightVictories++;
        if (_sharedGameAssets->_levelDef->_gameLostStory == "")
            _aknowledged = true;
        else {
            _gameLostWidget.reset(new StoryWidget(_sharedGameAssets->_levelDef->_gameLostStory, this));
            _sharedMatchAssets->_gameScreen->setOverlayStory(_gameLostWidget.get());
            _sharedMatchAssets->_gameScreen->addAction(this);
        }
    }
    _sharedMatchAssets->_leftTotal  += _sharedMatchAssets->_gameWidget->getStatPlayerOne().points;
    _sharedMatchAssets->_rightTotal += _sharedMatchAssets->_gameWidget->getStatPlayerTwo().points;
    _sharedMatchAssets->_gameWidget->setGameOverAction(this);
}

void StoryModeMatchIsOverState::exitState()
{
    _sharedMatchAssets->_gameWidget->setGameOverAction(NULL);
}

bool StoryModeMatchIsOverState::evaluate()
{
    return _aknowledged;
}

GameState *StoryModeMatchIsOverState::getNextState()
{
    return _nextState;
}

void StoryModeMatchIsOverState::action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event)
{
    if (sender == (Widget *)(_gameLostWidget->getParentScreen())) {
        _gameLostWidget->getParentScreen()->removeAction(this);
        _gameLostWidget.reset(NULL);
        return;
    }
    _aknowledged = true;
    evaluateStateMachine();
}

//---------------------------------
// StoryModeMatchState
//---------------------------------
StoryModeMatchState::StoryModeMatchState(SharedGameAssets *sharedGameAssets)
    : _sharedGameAssets(sharedGameAssets),
      _gameWidgetFactory(this)
{
}

void StoryModeMatchState::enterState()
{
    _nextState = NULL;
    // Creating the different game states
    if (_sharedGameAssets->_levelDef->_introStory == "")
        _introStoryScreen.reset(NULL);
    else
        _introStoryScreen.reset(new DisplayStoryScreenState(_sharedGameAssets->_levelDef->_introStory));
    _opponentStoryScreen.reset(new DisplayStoryScreenState(_sharedGameAssets->_levelDef->_opponentStory));
    _setupMatch.reset(new SetupMatchState(_gameWidgetFactory, _sharedGameAssets->_gameOptions, this, &_sharedAssets));
    _enterPlayersReady.reset(new EnterPlayerReadyState(_sharedAssets, _sharedGetReadyAssets));
    _exitPlayersReady.reset(new ExitPlayerReadyState(_sharedAssets, _sharedGetReadyAssets));
    _matchPlaying.reset(new MatchPlayingState(_sharedAssets));
    _matchIsOver.reset(new StoryModeMatchIsOverState(_sharedGameAssets, &_sharedAssets));
    _displayStats.reset(new DisplayStatsState(_sharedAssets));
    _leaveMatch.reset(new CallActionState(this, LEAVE_MATCH));
    _abortGame.reset(new CallActionState(this, ABORT_GAME));
    // Additional state setup
    _setupMatch->setHandicapOnVictorious(false);
    _setupMatch->setAccountTotalOnPlayerB(false);
    // Linking the states together
    if (_introStoryScreen.get() != NULL)
        _introStoryScreen->setNextState(_opponentStoryScreen.get());
    _opponentStoryScreen->setNextState(_setupMatch.get());
    _setupMatch->setNextState(_enterPlayersReady.get());
    _enterPlayersReady->setNextState(_exitPlayersReady.get());
    _exitPlayersReady->setNextState(_matchPlaying.get());
    _matchPlaying->setNextState(_matchIsOver.get());
    _matchPlaying->setAbortedState(_abortGame.get());
    _matchIsOver->setNextState(_displayStats.get());
    _displayStats->setNextState(_leaveMatch.get());
    // Initializing the state machine
    if (_introStoryScreen.get() != NULL)
        _stateMachine.setInitialState(_introStoryScreen.get());
    else
        _stateMachine.setInitialState(_opponentStoryScreen.get());
    // Run the state machine
    _stateMachine.reset();
    _stateMachine.evaluate();
}

void StoryModeMatchState::exitState()
{
}

bool StoryModeMatchState::evaluate()
{
    if (_nextState == NULL)
        return false;
    return true;
}

GameState *StoryModeMatchState::getNextState()
{
    return _nextState;
}

std::string StoryModeMatchState::getPlayerName(int playerNumber) const
{
    switch (playerNumber) {
    case 0:
      return _sharedGameAssets->_playerName.c_str();
	break;
    case 1:
    default:
        return _sharedGameAssets->_levelDef->_opponentName;
    }
}

GameWidget *StoryModeMatchState::createGameWidget(FloboSetTheme &floboSetTheme,
                                         LevelTheme &levelTheme,
                                         const std::string & centerFace,
                                         Action *gameOverAction)
{
    _sharedAssets._currentFloboSetTheme = theCommander->getPreferedFloboSetTheme();
    if (_sharedGameAssets->_levelDef->_backgroundTheme == "Prefs.DefaultTheme")
        _sharedAssets._currentLevelTheme = theCommander->getPreferedLevelTheme();
    else
        _sharedAssets._currentLevelTheme = theCommander->getLevelTheme(_sharedGameAssets->_levelDef->_backgroundTheme.c_str()); // TODO string
    return new StoryModeStandardLayoutGameWidget(*(_sharedAssets._currentFloboSetTheme),
                                                    *(_sharedAssets._currentLevelTheme),
                                                    _sharedGameAssets->_levelDef->getAISettings(_sharedGameAssets->_difficulty)._level,
                                                    _sharedGameAssets->_levelDef->getAISettings(_sharedGameAssets->_difficulty)._nColors,
                                                    _sharedGameAssets->_lifes,
                                                    _sharedGameAssets->_levelDef->_opponent, gameOverAction);
}

void StoryModeMatchState::action(Widget *sender, int actionType,
                                    event_manager::GameControlEvent *event)
{
    switch (actionType) {
    case LEAVE_MATCH:
      //_playerStat.total_points += _gameWidget->getStatPlayerOne().points;
        if (_sharedAssets._gameWidget->isGameARunning()) {
            _nextState = _victoriousState;
            // Note achievement if available
            std::string achievementName = std::string("victory_") + _sharedGameAssets->_levelDef->_opponentName;
            if (theCommander->getAchievementsManager() != NULL)
                theCommander->getAchievementsManager()->declareAchievement(achievementName.c_str(), 100.);
        }
        else {
            if (_sharedGameAssets->_lifes == 0) {
                _nextState = _gameLostState;
            }
            else {
                _nextState = _humiliatedState;
                _sharedGameAssets->_lifes--;
            }
        }
        break;
    case ABORT_GAME:
    default:
        _nextState = _abortedState;
        break;
    }
    evaluateStateMachine();
}

//---------------------------------
// StoryModePrepareNextMatchState
//---------------------------------
std::unique_ptr<StoryModeLevelsDefinition> StoryModePrepareNextMatchState::_levelDefProvider;
StoryModePrepareNextMatchState::StoryModePrepareNextMatchState(SharedGameAssets *sharedGameAssets)
    : _sharedGameAssets(sharedGameAssets)
{
    reset();
}

void StoryModePrepareNextMatchState::enterState()
{
    if (_levelDefProvider.get() == NULL)
        _levelDefProvider.reset(new StoryModeLevelsDefinition("/story/levels.gsl"));
    ++_currentLevel;
    if (_levelDefProvider->getNumLevels() > _currentLevel) {
        _sharedGameAssets->_levelDef = _levelDefProvider->getLevelDefinition(_currentLevel);
        _nextState = _nextMatchState;
    }
    else { // The game is won, there is no more levels
        _nextState = _gameWonState;
    }
}

bool StoryModePrepareNextMatchState::evaluate()
{
    return true;
}

GameState *StoryModePrepareNextMatchState::getNextState()
{
    return _nextState;
}

void StoryModePrepareNextMatchState::reset()
{
    _currentLevel = -1;
}

//---------------------------------
// StoryModeStarterAction
//---------------------------------

StoryModeStarterAction::StoryModeStarterAction(GameDifficulty difficulty,
                                                           PlayerNameProvider *nameProvider)
    : _nameProvider(nameProvider)
{
    // Initializing the shared game assets
    _sharedGameAssets._difficulty = difficulty;
    _sharedGameAssets._gameOptions = GameOptions::fromDifficulty(difficulty);
    // Creating the different game states
    _pushGameScreen.reset(new PushScreenState());
    _prepareNextMatch.reset(new StoryModePrepareNextMatchState(&_sharedGameAssets));
    _playMatch.reset(new StoryModeMatchState(&_sharedGameAssets));
    _gameWon.reset(new DisplayStoryScreenState("gamewon_1p.gsl"));
    _gameLostHoF.reset(new ManageHiScoresState(_playMatch->getMatchAssets(), nameProvider, STORY_SCOREBOARD_ID, "", &_sharedGameAssets));
    _gameWonHoF.reset(new ManageHiScoresState(_playMatch->getMatchAssets(), nameProvider, STORY_SCOREBOARD_ID, "gamewon_highscores_1p.gsl"));
    _leaveGame.reset(new LeaveGameState(*(_playMatch->getMatchAssets())));
    // Linking the states together
    _pushGameScreen->setNextState(_prepareNextMatch.get());
    _prepareNextMatch->setNextMatchState(_playMatch.get());
    _prepareNextMatch->setGameWonState(_gameWon.get());
    _playMatch->setVictoriousState(_prepareNextMatch.get());
    _playMatch->setHumiliatedState(_playMatch.get());
    _playMatch->setGameLostState(_gameLostHoF.get());
    _playMatch->setAbortedState(_leaveGame.get());
    _gameWon->setNextState(_gameWonHoF.get());
    _gameWonHoF->setNextState(_leaveGame.get());
    _gameLostHoF->setNextState(_leaveGame.get());
    // Initializing the state machine
    _stateMachine.setInitialState(_pushGameScreen.get());
}

void StoryModeStarterAction::action(Widget *sender, int actionType,
                                    event_manager::GameControlEvent *event)
{
    if (_nameProvider == NULL)
        _sharedGameAssets._playerName = "Player";
    else
        _sharedGameAssets._playerName = _nameProvider->getPlayerName(0);
    _sharedGameAssets._lifes = 3;
    _prepareNextMatch->reset();
    _stateMachine.reset();
    _stateMachine.evaluate();
}

void StoryModeStarterAction::pauseGameIfPossible()
{
    if (_playMatch->getMatchAssets() == NULL)
        return;
    if (_playMatch->getMatchAssets()->_gameScreen.get() == NULL)
        return;
    _playMatch->getMatchAssets()->_gameScreen->setPaused(true);
}
