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

StoryModeGameWidget::StoryModeGameWidget(int lifes, String aiFace)
    : opponentcontroller(NULL),
      faceTicks(0), opponent(aiFace),
      killLeftCheat("killleft", this),
      killRightCheat("killright", this)
{
    setLives(lifes);
}

void StoryModeGameWidget::initWithGUI(GameView &areaA, GameView &areaB,
                                            GamePlayer &playercontroller,
                                            LevelTheme &levelTheme,
                                            int level,
                                            Action *gameOverAction)
{
    // Jeko's corrected_level ensures easier games on easiest levels
    int corrected_level = level;
    if (level > 12) corrected_level = level + (3 - lives) / 2; // Half of medium level, make sure the player do not loose 3 times the same oponent.
    if (level >= 20) corrected_level = level + (3 - lives); // Easy level, make sure the player do always loose the same oponent.
    opponentcontroller = new AIPlayer(corrected_level, areaB);

    GameWidget2P::initWithGUI(areaA, areaB, playercontroller, *opponentcontroller,
                               levelTheme, gameOverAction);
    addSubWidget(&killLeftCheat);
    addSubWidget(&killRightCheat);
	int scoringLevel = 30/level;
	if (scoringLevel < 1) scoringLevel = 1;
	if (scoringLevel > 10) scoringLevel = 10;
	areaA.getAttachedGame()->setScoringLevel(scoringLevel);
	areaB.getAttachedGame()->setScoringLevel(scoringLevel);
}

StoryModeStandardLayoutGameWidget::StoryModeStandardLayoutGameWidget(FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, int level, int nColors, int lifes, String aiFace, Action *gameOverAction)
  : StoryModeGameWidget(lifes, aiFace),
      attachedFloboThemeSet(floboSetTheme),
      attachedRandom(nColors),
      attachedGameFactory(&attachedRandom),
      areaA(&attachedGameFactory, 0, &attachedFloboThemeSet, &levelTheme),
      areaB(&attachedGameFactory, 1, &attachedFloboThemeSet, &levelTheme),
      playercontroller(areaA)

{
    initWithGUI(areaA, areaB, playercontroller, levelTheme, level, gameOverAction);
}

StoryModeGameWidget::~StoryModeGameWidget()
{
    if (opponentcontroller != NULL)
        delete opponentcontroller;
}

void StoryModeGameWidget::cycle()
{
    faceTicks += 1;
    if (faceTicks == 1) {
        AIParameters ai;
        ai.realSuppressionValue = opponent.getIntegerValue("@AI_RealSuppression");
        ai.potentialSuppressionValue = opponent.getIntegerValue("@AI_PotentialSuppression");
        ai.criticalHeight = opponent.getIntegerValue("@AI_CriticalHeight");
        ai.columnScalar[0] = opponent.getIntegerValue("@AI_Column1");
        ai.columnScalar[1] = opponent.getIntegerValue("@AI_Column2");
        ai.columnScalar[2] = opponent.getIntegerValue("@AI_Column3");
        ai.columnScalar[3] = opponent.getIntegerValue("@AI_Column4");
        ai.columnScalar[4] = opponent.getIntegerValue("@AI_Column5");
        ai.columnScalar[5] = opponent.getIntegerValue("@AI_Column6");
        ai.rotationMethod = opponent.getIntegerValue("@AI_RotationMethod");
        ai.fastDropDelta = opponent.getIntegerValue("@AI_FastDropDelta");
        ai.thinkDepth = opponent.getIntegerValue("@AI_ThinkDepth");
        opponentcontroller->setAIParameters(ai);

		opponent.setIntegerValue("@AI_PlayingLevel", opponentcontroller->getLevel());
        opponent.setIntegerValue("@maxHeightLeft", attachedGameA->getColumnHeigth(2));
        opponent.setIntegerValue("@maxHeightRight", attachedGameB->getColumnHeigth(2));
        opponent.setIntegerValue("@maxHeightPlayer", attachedGameA->getColumnHeigth(2));
        opponent.setIntegerValue("@maxHeightAI", attachedGameB->getColumnHeigth(2));
        opponent.setIntegerValue("@neutralsForPlayer", attachedGameA->getNeutralFlobos());
        opponent.setIntegerValue("@neutralsForAI", attachedGameB->getNeutralFlobos());
    }
    if (faceTicks == 100) {
        faceTicks = 0;
    }
    GameWidget2P::cycle();
}

StoryWidget *StoryModeGameWidget::getOpponent()
{
    return &opponent;
}

void StoryModeGameWidget::action(Widget *sender, int actionType,
                                        GameControlEvent *event)
{
    if (sender == static_cast<Widget *>(&killLeftCheat))
        addGameAHandicap(FLOBOBAN_DIMY);
    else if (sender == static_cast<Widget *>(&killRightCheat))
        addGameBHandicap(FLOBOBAN_DIMY);
}

StoryModeLevelsDefinition *StoryModeLevelsDefinition::currentDefinition = NULL;

StoryModeLevelsDefinition::StoryModeLevelsDefinition(String levelDefinitionFile)
{
    GoomSL * gsl = gsl_new();
    if (!gsl) return;
    GSLFA_setupWrapper(gsl, &(theCommander->getDataPathManager()));
    gsl_push_file(gsl, "/lib/levellib.gsl");
    gsl_push_file(gsl, levelDefinitionFile);
    gsl_compile(gsl);
    currentDefinition = this;
    gsl_bind_function(gsl, "end_level",  StoryModeLevelsDefinition::end_level);
    gsl_bind_function(gsl, "getBoolPreference", StoryModeLevelsDefinition::get_BoolPreference);
    gsl_execute(gsl);
    gsl_free(gsl);
}

StoryModeLevelsDefinition::~StoryModeLevelsDefinition()
{
  for (int i = 0 ; i < levelDefinitions.size() ; i++) {
    delete levelDefinitions[i];
  }
}

void StoryModeLevelsDefinition::addLevelDefinition(String levelName, String introStory,
					      String opponentStory, String opponentName,
					      String opponent, String backgroundTheme,
					      String gameLostStory, String gameOverStory,
					      SelIA easySettings,
					      SelIA mediumSettings, SelIA hardSettings)
{
  levelDefinitions.add(new LevelDefinition(levelName, introStory, opponentStory, opponentName,
					   opponent, backgroundTheme, gameLostStory, gameOverStory,
					   easySettings, mediumSettings, hardSettings));
}

StoryModeLevelsDefinition::SelIA::SelIA(int level, int nColors) : level(level), nColors(nColors)
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
  const char * gameOverStory = (const char *) GSL_GLOBAL_PTR(gsl, "level.gameOverStory");
  SelIA easySettings(GSL_GLOBAL_INT(gsl, "level.easySetting.level"),
                     GSL_GLOBAL_INT(gsl, "level.easySetting.nColors"));
  SelIA mediumSettings(GSL_GLOBAL_INT(gsl, "level.mediumSetting.level"),
                       GSL_GLOBAL_INT(gsl, "level.mediumSetting.nColors"));
  SelIA hardSettings(GSL_GLOBAL_INT(gsl, "level.hardSetting.level"),
                     GSL_GLOBAL_INT(gsl, "level.hardSetting.nColors"));
  currentDefinition->addLevelDefinition(levelName, introStory, opponentStory, opponentName,
					opponent, backgroundTheme, gameLostStory, gameOverStory,
					easySettings, mediumSettings, hardSettings);
}

GameOver1PScreen::GameOver1PScreen(String screenName,
        Action *finishedAction, String playerName, const PlayerGameStat &playerPoints, bool initialTransition)
        : StoryScreen(screenName, finishedAction, initialTransition),
        playerName(playerName), playerStat(playerPoints)
{
    initHiScores(AI_NAMES);
    int scorePlace = setHiScore(playerStat.total_points, playerName);

    for (int i = 0 ; i < kHiScoresNumber ; i++) {
        hiScoreNameBox.add(&names[i]);
        hiScorePointBox.add(&points[i]);
        if (i == scorePlace) {
            names[i].setFont(GameUIDefaults::FONT);
            points[i].setFont(GameUIDefaults::FONT);
        }
    }
	titleText.setFont(GameUIDefaults::FONT_INACTIVE);
	titleText.setValue(theCommander->getLocalizedString("Your Final Score:"));
	titleScore.setFont(GameUIDefaults::FONT);
	titleBox.add(&titleText);
	titleBox.add(&titleScore);

    hiScoreBox.add(&hiScoreNameBox);
    hiScoreBox.add(&hiScorePointBox);

	add(&titleBox);
    add(&hiScoreBox);
    refresh();
}

void GameOver1PScreen::refresh()
{
    hiscore *scores = getHiScores();
    for (int i = 0 ; i < kHiScoresNumber ; i++) {
        char tmp[256];
        sprintf(tmp, "%d", scores[i].score);
        names[i].setValue(scores[i].name.c_str());
        points[i].setValue(tmp);
    }

	{
		char tmp[256];
		sprintf(tmp, "%d", playerStat.total_points);
		titleScore.setValue(tmp);
		Vec3 titlePos = titleBox.getPosition();
		titlePos.x = storyWidget.getIntegerValue("@hiScoreTopBox.x");
		titlePos.y = storyWidget.getIntegerValue("@hiScoreTopBox.y");
		titleBox.setPosition(titlePos);
		titleBox.setSize(Vec3(storyWidget.getIntegerValue("@hiScoreTopBox.w"),
							  storyWidget.getIntegerValue("@hiScoreTopBox.h"), 0));
	}
    Vec3 hiScorePos = hiScoreBox.getPosition();
    hiScorePos.x = storyWidget.getIntegerValue("@hiScoreBox.x");
    hiScorePos.y = storyWidget.getIntegerValue("@hiScoreBox.y");
    hiScoreBox.setPosition(hiScorePos);

    hiScoreBox.setSize(Vec3(storyWidget.getIntegerValue("@hiScoreBox.w"),
                            storyWidget.getIntegerValue("@hiScoreBox.h"), 0));
}

GameOver1PScreen::~GameOver1PScreen()
{
}

//---------------------------------
// StoryModeMatchIsOverState
//---------------------------------
StoryModeMatchIsOverState::StoryModeMatchIsOverState(SharedGameAssets *sharedGameAssets,
                                                           SharedMatchAssets *sharedMatchAssets)
    : m_sharedGameAssets(sharedGameAssets),
      m_sharedMatchAssets(sharedMatchAssets),
      m_aknowledged(false)
{
}

StoryModeMatchIsOverState::~StoryModeMatchIsOverState()
{
    if (m_gameLostWidget.get() != NULL) {
        m_gameLostWidget->getParentScreen()->removeAction(this);
        m_gameLostWidget.reset(NULL);
    }
}

void StoryModeMatchIsOverState::enterState()
{
    GTLogTrace("StoryModeMatchIsOver::enterState()");
    m_aknowledged = false;
    if (m_sharedMatchAssets->m_gameWidget->isGameARunning()) {
        m_aknowledged = true;
        m_sharedMatchAssets->m_leftVictories++;
    }
    else {
        m_gameLostWidget.reset(new StoryWidget(m_sharedGameAssets->levelDef->gameLostStory, this));
        m_sharedMatchAssets->m_gameScreen->setOverlayStory(m_gameLostWidget.get());
        m_sharedMatchAssets->m_rightVictories++;
        m_sharedMatchAssets->m_gameScreen->addAction(this);
    }
    m_sharedMatchAssets->m_leftTotal  += m_sharedMatchAssets->m_gameWidget->getStatPlayerOne().points;
    m_sharedMatchAssets->m_rightTotal += m_sharedMatchAssets->m_gameWidget->getStatPlayerTwo().points;
    m_sharedMatchAssets->m_gameWidget->setGameOverAction(this);
}

void StoryModeMatchIsOverState::exitState()
{
    m_sharedMatchAssets->m_gameWidget->setGameOverAction(NULL);
}

bool StoryModeMatchIsOverState::evaluate()
{
    return m_aknowledged;
}

GameState *StoryModeMatchIsOverState::getNextState()
{
    return m_nextState;
}

void StoryModeMatchIsOverState::action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event)
{
    if (sender == (Widget *)(m_gameLostWidget->getParentScreen())) {
        m_gameLostWidget->getParentScreen()->removeAction(this);
        m_gameLostWidget.reset(NULL);
        return;
    }
    m_aknowledged = true;
    evaluateStateMachine();
}

//---------------------------------
// StoryModeMatchState
//---------------------------------
StoryModeMatchState::StoryModeMatchState(SharedGameAssets *sharedGameAssets)
    : m_sharedGameAssets(sharedGameAssets),
      m_gameWidgetFactory(this)
{
}

void StoryModeMatchState::enterState()
{
    m_nextState = NULL;
    // Creating the different game states
    if (m_sharedGameAssets->levelDef->introStory == "")
        m_introStoryScreen.reset(NULL);
    else
        m_introStoryScreen.reset(new DisplayStoryScreenState(m_sharedGameAssets->levelDef->introStory));
    m_opponentStoryScreen.reset(new DisplayStoryScreenState(m_sharedGameAssets->levelDef->opponentStory));
    m_setupMatch.reset(new SetupMatchState(m_gameWidgetFactory, m_sharedGameAssets->gameOptions, this, &m_sharedAssets));
    m_enterPlayersReady.reset(new EnterPlayerReadyState(m_sharedAssets, m_sharedGetReadyAssets));
    m_exitPlayersReady.reset(new ExitPlayerReadyState(m_sharedAssets, m_sharedGetReadyAssets));
    m_matchPlaying.reset(new MatchPlayingState(m_sharedAssets));
    m_matchIsOver.reset(new StoryModeMatchIsOverState(m_sharedGameAssets, &m_sharedAssets));
    m_displayStats.reset(new DisplayStatsState(m_sharedAssets));
    m_leaveMatch.reset(new CallActionState(this, LEAVE_MATCH));
    m_abortGame.reset(new CallActionState(this, ABORT_GAME));
    // Additional state setup
    m_setupMatch->setHandicapOnVictorious(false);
    m_setupMatch->setAccountTotalOnPlayerB(false);
    // Linking the states together
    if (m_introStoryScreen.get() != NULL)
        m_introStoryScreen->setNextState(m_opponentStoryScreen.get());
    m_opponentStoryScreen->setNextState(m_setupMatch.get());
    m_setupMatch->setNextState(m_enterPlayersReady.get());
    m_enterPlayersReady->setNextState(m_exitPlayersReady.get());
    m_exitPlayersReady->setNextState(m_matchPlaying.get());
    m_matchPlaying->setNextState(m_matchIsOver.get());
    m_matchPlaying->setAbortedState(m_abortGame.get());
    m_matchIsOver->setNextState(m_displayStats.get());
    m_displayStats->setNextState(m_leaveMatch.get());
    // Initializing the state machine
    if (m_introStoryScreen.get() != NULL)
        m_stateMachine.setInitialState(m_introStoryScreen.get());
    else
        m_stateMachine.setInitialState(m_opponentStoryScreen.get());
    // Run the state machine
    m_stateMachine.reset();
    m_stateMachine.evaluate();
}

void StoryModeMatchState::exitState()
{
}

bool StoryModeMatchState::evaluate()
{
    if (m_nextState == NULL)
        return false;
    return true;
}

GameState *StoryModeMatchState::getNextState()
{
    return m_nextState;
}

String StoryModeMatchState::getPlayerName(int playerNumber) const
{
    switch (playerNumber) {
    case 0:
      return m_sharedGameAssets->playerName.c_str();
	break;
    case 1:
    default:
        return m_sharedGameAssets->levelDef->opponentName;
    }
}

GameWidget *StoryModeMatchState::createGameWidget(FloboSetTheme &floboSetTheme,
                                         LevelTheme &levelTheme,
                                         String centerFace,
                                         Action *gameOverAction)
{
    m_sharedAssets.m_currentFloboSetTheme = theCommander->getPreferedFloboSetTheme();
    if (m_sharedGameAssets->levelDef->backgroundTheme == "Prefs.DefaultTheme")
        m_sharedAssets.m_currentLevelTheme = theCommander->getPreferedLevelTheme();
    else
        m_sharedAssets.m_currentLevelTheme = theCommander->getLevelTheme(m_sharedGameAssets->levelDef->backgroundTheme);
    return new StoryModeStandardLayoutGameWidget(*(m_sharedAssets.m_currentFloboSetTheme),
                                                    *(m_sharedAssets.m_currentLevelTheme),
                                                    m_sharedGameAssets->levelDef->getAISettings(m_sharedGameAssets->difficulty).level,
                                                    m_sharedGameAssets->levelDef->getAISettings(m_sharedGameAssets->difficulty).nColors,
                                                    m_sharedGameAssets->lifes,
                                                    m_sharedGameAssets->levelDef->opponent, gameOverAction);
}

void StoryModeMatchState::action(Widget *sender, int actionType,
                                    event_manager::GameControlEvent *event)
{
    switch (actionType) {
    case LEAVE_MATCH:
      //m_playerStat.total_points += m_gameWidget->getStatPlayerOne().points;
        if (m_sharedAssets.m_gameWidget->isGameARunning()) {
            m_nextState = m_victoriousState;
        }
        else {
            if (m_sharedGameAssets->lifes == 0) {
                m_nextState = m_gameLostState;
            }
            else {
                m_nextState = m_humiliatedState;
                m_sharedGameAssets->lifes--;
            }
        }
        break;
    case ABORT_GAME:
    default:
        m_nextState = m_abortedState;
        break;
    }
    evaluateStateMachine();
}

//---------------------------------
// StoryModePrepareNextMatchState
//---------------------------------
std::auto_ptr<StoryModeLevelsDefinition> StoryModePrepareNextMatchState::m_levelDefProvider;
StoryModePrepareNextMatchState::StoryModePrepareNextMatchState(SharedGameAssets *sharedGameAssets)
    : m_sharedGameAssets(sharedGameAssets)
{
    reset();
}

void StoryModePrepareNextMatchState::enterState()
{
    if (m_levelDefProvider.get() == NULL)
        m_levelDefProvider.reset(new StoryModeLevelsDefinition("/story/levels.gsl"));
    ++m_currentLevel;
    if (m_levelDefProvider->getNumLevels() > m_currentLevel) {
        m_sharedGameAssets->levelDef = m_levelDefProvider->getLevelDefinition(m_currentLevel);
        m_nextState = m_nextMatchState;
    }
    else { // The game is won, there is no more levels
        m_nextState = m_gameWonState;
    }
}

bool StoryModePrepareNextMatchState::evaluate()
{
    return true;
}

GameState *StoryModePrepareNextMatchState::getNextState()
{
    return m_nextState;
}

void StoryModePrepareNextMatchState::reset()
{
    m_currentLevel = -1;
}

//---------------------------------
// StoryModeDisplayHallOfFameState
//---------------------------------
StoryModeDisplayHallOfFameState::StoryModeDisplayHallOfFameState(SharedGameAssets *sharedGameAssets,
                                                                 SharedMatchAssets *sharedMatchAssets,
                                                                 const char *storyName)
    : m_storyName(storyName), m_sharedGameAssets(sharedGameAssets), m_sharedMatchAssets(sharedMatchAssets)
{
}

void StoryModeDisplayHallOfFameState::enterState()
{
    if (m_storyName == "") {
        m_storyName = m_sharedGameAssets->levelDef->gameOverStory;
    }
    GTLogTrace("StoryModeDisplayHallOfFameState(%s)::enterState()", m_storyName.c_str());
    std::string &playerName = m_sharedGameAssets->playerName;
    const PlayerGameStat &playerPoints = m_sharedMatchAssets->m_gameWidget->getStatPlayerOne();
    m_gameOverScreen.reset(new GameOver1PScreen(m_storyName.c_str(),
                                        this, playerName.c_str(), playerPoints));
    GameUIDefaults::SCREEN_STACK->swap(m_gameOverScreen.get());
    m_gameOverScreen->refresh();
    m_acknowledged = false;
}

void StoryModeDisplayHallOfFameState::exitState()
{
    GameUIDefaults::GAME_LOOP->garbageCollect(m_gameOverScreen.release());
}

bool StoryModeDisplayHallOfFameState::evaluate()
{
    return m_acknowledged;
}

GameState *StoryModeDisplayHallOfFameState::getNextState()
{
    return m_nextState;
}

void StoryModeDisplayHallOfFameState::action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event)
{
    m_acknowledged = true;
    evaluateStateMachine();
}

StoryModeStarterAction::StoryModeStarterAction(GameDifficulty difficulty,
                                                           PlayerNameProvider *nameProvider)
    : m_nameProvider(nameProvider)
{
    // Initializing the shared game assets
    m_sharedGameAssets.difficulty = difficulty;
    m_sharedGameAssets.gameOptions = GameOptions::fromDifficulty(difficulty);
    // Creating the different game states
    m_pushGameScreen.reset(new PushScreenState());
    m_prepareNextMatch.reset(new StoryModePrepareNextMatchState(&m_sharedGameAssets));
    m_playMatch.reset(new StoryModeMatchState(&m_sharedGameAssets));
    m_gameWon.reset(new DisplayStoryScreenState("gamewon_1p.gsl"));
    m_gameLostHoF.reset(new StoryModeDisplayHallOfFameState(&m_sharedGameAssets, m_playMatch->getMatchAssets()));
    m_gameWonHoF.reset(new StoryModeDisplayHallOfFameState(&m_sharedGameAssets, m_playMatch->getMatchAssets(), "gamewon_highscores_1p.gsl"));
    m_leaveGame.reset(new LeaveGameState(*(m_playMatch->getMatchAssets())));
    // Linking the states together
    m_pushGameScreen->setNextState(m_prepareNextMatch.get());
    m_prepareNextMatch->setNextMatchState(m_playMatch.get());
    m_prepareNextMatch->setGameWonState(m_gameWon.get());
    m_playMatch->setVictoriousState(m_prepareNextMatch.get());
    m_playMatch->setHumiliatedState(m_playMatch.get());
    m_playMatch->setGameLostState(m_gameLostHoF.get());
    m_playMatch->setAbortedState(m_leaveGame.get());
    m_gameWon->setNextState(m_gameWonHoF.get());
    m_gameWonHoF->setNextState(m_leaveGame.get());
    m_gameLostHoF->setNextState(m_leaveGame.get());
    // Initializing the state machine
    m_stateMachine.setInitialState(m_pushGameScreen.get());
}

void StoryModeStarterAction::action(Widget *sender, int actionType,
                                    event_manager::GameControlEvent *event)
{
    if (m_nameProvider == NULL)
        m_sharedGameAssets.playerName = "Player";
    else
        m_sharedGameAssets.playerName = m_nameProvider->getPlayerName(0);
    m_sharedGameAssets.lifes = 3;
    m_prepareNextMatch->reset();
    m_stateMachine.reset();
    m_stateMachine.evaluate();
}

