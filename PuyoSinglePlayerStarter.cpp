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

#include "GTLog.h"
#include "PuyoSinglePlayerStarter.h"
#include "PuyoView.h"
#include "preferences.h"

using namespace event_manager;

SinglePlayerGameWidget::SinglePlayerGameWidget(int lifes, String aiFace)
    : opponentcontroller(NULL),
      faceTicks(0), opponent(aiFace),
      killLeftCheat("killleft", this),
      killRightCheat("killright", this)
{
    setLives(lifes);
}

void SinglePlayerGameWidget::initWithGUI(PuyoView &areaA, PuyoView &areaB,
                                            PuyoPlayer &playercontroller,
                                            LevelTheme &levelTheme,
                                            int level,
                                            Action *gameOverAction)
{
    opponentcontroller = new PuyoIA(level, areaB);
    GameWidget::initWithGUI(areaA, areaB, playercontroller, *opponentcontroller,
                               levelTheme, gameOverAction);
    addSubWidget(&killLeftCheat);
    addSubWidget(&killRightCheat);
	int scoringLevel = 30/level;
	if (scoringLevel < 1) scoringLevel = 1;
	if (scoringLevel > 10) scoringLevel = 10;
	areaA.getAttachedGame()->setScoringLevel(scoringLevel);
	areaB.getAttachedGame()->setScoringLevel(scoringLevel);
}

SinglePlayerStandardLayoutGameWidget::SinglePlayerStandardLayoutGameWidget(PuyoSetTheme &puyoThemeSet, LevelTheme &levelTheme, int level, int nColors, int lifes, String aiFace, Action *gameOverAction)
  : SinglePlayerGameWidget(lifes, aiFace),
      attachedPuyoThemeSet(puyoThemeSet),
      attachedRandom(nColors),
      attachedGameFactory(&attachedRandom),
      areaA(&attachedGameFactory, 0, &attachedPuyoThemeSet, &levelTheme),
      areaB(&attachedGameFactory, 1, &attachedPuyoThemeSet, &levelTheme),
      playercontroller(areaA)

{
    initWithGUI(areaA, areaB, playercontroller, levelTheme, level, gameOverAction);
}

SinglePlayerGameWidget::~SinglePlayerGameWidget()
{
    if (opponentcontroller != NULL)
        delete opponentcontroller;
}

void SinglePlayerGameWidget::cycle()
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
        opponent.setIntegerValue("@neutralsForPlayer", attachedGameA->getNeutralPuyos());
        opponent.setIntegerValue("@neutralsForAI", attachedGameB->getNeutralPuyos());
    }
    if (faceTicks == 100) {
        faceTicks = 0;
    }
    GameWidget::cycle();
}

StoryWidget *SinglePlayerGameWidget::getOpponent()
{
    return &opponent;
}

void SinglePlayerGameWidget::action(Widget *sender, int actionType,
                                        GameControlEvent *event)
{
    if (sender == static_cast<Widget *>(&killLeftCheat))
        addGameAHandicap(PUYODIMY);
    else if (sender == static_cast<Widget *>(&killRightCheat))
        addGameBHandicap(PUYODIMY);
}

SinglePlayerGameWidget *SinglePlayerStandardLayoutFactory::createGameWidget
          (PuyoSetTheme &puyoThemeSet, LevelTheme &levelTheme,
           int level, int nColors, int lifes, String aiFace,
           Action *gameOverAction)
{
    return new SinglePlayerStandardLayoutGameWidget(puyoThemeSet, levelTheme,
                                          level, nColors, lifes,
                                          aiFace, gameOverAction);
}

StatsWidgetDimensions SinglePlayerStandardLayoutFactory::getStatsWidgetDimensions() const
{
    return StatsWidgetDimensions(416, 194, 50, Vec3(0, 0), Vec3(0, 0));
}

PuyoLevelDefinitions *PuyoLevelDefinitions::currentDefinition = NULL;

PuyoLevelDefinitions::PuyoLevelDefinitions(String levelDefinitionFile)
{
    GoomSL * gsl = gsl_new();
    if (!gsl) return;
    String libPath = theCommander->getDataPathManager().getPath("/lib/levellib.gsl");
    char * fbuffer = gsl_init_buffer((const char *)libPath);
    gsl_append_file_to_buffer(levelDefinitionFile, &fbuffer);
    gsl_compile(gsl,fbuffer);
    currentDefinition = this;
    gsl_bind_function(gsl, "end_level",  PuyoLevelDefinitions::end_level);
    gsl_bind_function(gsl, "getBoolPreference", PuyoLevelDefinitions::get_BoolPreference);
    gsl_execute(gsl);
    free(fbuffer);
    gsl_free(gsl);
}

PuyoLevelDefinitions::~PuyoLevelDefinitions()
{
  for (int i = 0 ; i < levelDefinitions.size() ; i++) {
    delete levelDefinitions[i];
  }
}

void PuyoLevelDefinitions::addLevelDefinition(String levelName, String introStory,
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

PuyoLevelDefinitions::SelIA::SelIA(int level, int nColors) : level(level), nColors(nColors)
{
}

void PuyoLevelDefinitions::get_BoolPreference(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    char *name  = (char*)GSL_LOCAL_PTR(gsl, local, "name");
    int def     = GSL_LOCAL_INT(gsl, local, "default");
    GSL_GLOBAL_INT(gsl, "getBoolPreference")
        = GetBoolPreference(name, def);
}

void PuyoLevelDefinitions::end_level(GoomSL *gsl, GoomHash *global, GoomHash *local)
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

PuyoSingleGameLevelData::PuyoSingleGameLevelData(int gameLevel, int difficulty,
						 PuyoLevelDefinitions &levelDefinitions)
  : gameLevel(gameLevel), difficulty(difficulty),
    levelDefinition(levelDefinitions.getLevelDefinition(gameLevel))
{
    themeToUse = theCommander->getPreferedPuyoSetTheme();
    if (levelDefinition->backgroundTheme == "Prefs.DefaultTheme")
        levelThemeToUse = theCommander->getPreferedLevelTheme();
    else
        levelThemeToUse = theCommander->getLevelTheme(levelDefinition->backgroundTheme);
}

PuyoSingleGameLevelData::~PuyoSingleGameLevelData()
{
}

String PuyoSingleGameLevelData::getIntroStory() const
{
    return levelDefinition->introStory;
}

String PuyoSingleGameLevelData::getStory() const
{
    return levelDefinition->opponentStory;
}

String PuyoSingleGameLevelData::getGameLostStory() const
{
    return levelDefinition->gameLostStory;
}

String PuyoSingleGameLevelData::getGameOverStory() const
{
    return levelDefinition->gameOverStory;
}

PuyoSetTheme &PuyoSingleGameLevelData::getPuyoTheme() const
{
    return *themeToUse;
}

LevelTheme &PuyoSingleGameLevelData::getLevelTheme() const
{
    return *levelThemeToUse;
}

int PuyoSingleGameLevelData::getIALevel() const
{
    switch (difficulty) {
        case 0:
            return levelDefinition->easySettings.level;
        case 1:
            return levelDefinition->mediumSettings.level;
        default:
            return levelDefinition->hardSettings.level;
    }
}

int PuyoSingleGameLevelData::getNColors() const
{
    switch (difficulty) {
        case 0:
            return levelDefinition->easySettings.nColors;
        case 1:
            return levelDefinition->mediumSettings.nColors;
        default:
            return levelDefinition->hardSettings.nColors;
    }
}

String PuyoSingleGameLevelData::getIAName() const {
  return levelDefinition->opponentName;
}

String PuyoSingleGameLevelData::getIAFace() const
{
    return levelDefinition->opponent;
}

GameOptions PuyoSingleGameLevelData::getGameOptions() const
{
  return GameOptions::FromLevel(difficulty);
}

PuyoGameOver1PScreen::PuyoGameOver1PScreen(String screenName,
        Action *finishedAction, String playerName, const PlayerGameStat &playerPoints, bool initialTransition)
        : StoryScreen(screenName, finishedAction, initialTransition),
        playerName(playerName), playerStat(playerPoints)
{
    static const char *AI_NAMES[] = { "Fanzy", "Garou", "Big Rabbit", "Gizmo",
    "Satanas", "Doctor X", "Tania", "Mr Gyom",
    "The Duke","Jeko","--------" };

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
    //add(transitionWidget);
    refresh();
}

void PuyoGameOver1PScreen::refresh()
{
    hiscore *scores = getHiScores();
    for (int i = 0 ; i < kHiScoresNumber ; i++) {
        char tmp[256];
        sprintf(tmp, "%d", scores[i].score);
        names[i].setValue(scores[i].name);
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

PuyoGameOver1PScreen::~PuyoGameOver1PScreen()
{
}

SinglePlayerStarterAction::SinglePlayerStarterAction(MainScreen *mainScreen, int difficulty, SinglePlayerFactory *spFactory)
    : m_mainScreen(mainScreen),
      m_state(kGameNotStarted),
      m_spFactory(spFactory),
      m_currentLevel(0), m_lifes(3), m_difficulty(difficulty),
      m_currentMatch(NULL),
      m_hiScoreScreen(NULL), m_gameWonScreen(NULL),
      m_levelDefinitions(theCommander->getDataPathManager().getPath("/story/levels.gsl"))
{}

void SinglePlayerStarterAction::action(Widget *sender, int actionType,
			GameControlEvent *event)
{
  if (m_state == kGameNotStarted)
    stateMachine();
  else if (sender == m_currentMatch)
    stateMachine();
  else if (sender == m_hiScoreScreen->getStoryWidget())
    stateMachine();
  else if (sender == m_gameWonScreen->getStoryWidget())
    stateMachine();
}

void SinglePlayerStarterAction::stateMachine()
{
  switch (m_state) {
  case kGameNotStarted:
    performMatchPlaying(false, false);
    break;
  case kMatchPlaying:
    performEndOfMatch();
    break;
  case kGameOver:
    performHiScoreScreen(m_currentMatch->getGameOverStoryName());
    break;
  case kGameWon:
    performHiScoreScreen("gamewon_highscores_1p.gsl");
    break;
  case kHiScoreScreen:
    performBackToMenu();
  default:
    break;
  }
}

void SinglePlayerStarterAction::performMatchPlaying(bool skipIntroduction,
						    bool popScreen)
{
    m_state = kMatchPlaying;
    SinglePlayerMatch *previousMatch = m_currentMatch;
    PuyoSingleGameLevelData *levelData = new PuyoSingleGameLevelData(m_currentLevel, m_difficulty, m_levelDefinitions);
    m_currentMatch = new SinglePlayerMatch(m_playerStat, this, levelData,
                                           skipIntroduction, popScreen,
                                           m_spFactory, m_lifes);
    m_currentMatch->run();
    if (previousMatch != NULL) {
      delete previousMatch;
    }
}

void SinglePlayerStarterAction::performEndOfMatch()
{
  switch (m_currentMatch->getState()) {
  case SinglePlayerMatch::kMatchOverWon:
    m_currentLevel++;
    if (m_levelDefinitions.getNumLevels() > m_currentLevel) {
      performMatchPlaying(false, true);
    }
    else { // The game is won, there is no more levels
      performGameWon();
    }
    break;
  case SinglePlayerMatch::kMatchOverLost:
    m_lifes--;
    if (m_lifes >= 0)
      performMatchPlaying(true, true);
    else {
      m_state = kGameOver;
      stateMachine();
    }
    break;
  case SinglePlayerMatch::kMatchOverAborted:
  default:
    m_state = kGameOver;
    stateMachine();
    break;
  }
}

void SinglePlayerStarterAction::performGameWon()
{
    m_state = kGameWon;
    m_gameWonScreen = new StoryScreen("gamewon_1p.gsl", this);
    GameUIDefaults::SCREEN_STACK->pop();
    GameUIDefaults::SCREEN_STACK->push(m_gameWonScreen);
    if (m_currentMatch != NULL) {
      delete m_currentMatch;
      m_currentMatch = NULL;
    }
}

void SinglePlayerStarterAction::performHiScoreScreen(String gameOverStoryName)
{
  m_state = kHiScoreScreen;
  m_hiScoreScreen = new PuyoGameOver1PScreen(gameOverStoryName,
                     this, m_spFactory->getPlayerName(),
                     m_playerStat);
  m_hiScoreScreen->refresh();
  GameUIDefaults::SCREEN_STACK->pop();
  GameUIDefaults::SCREEN_STACK->push(m_hiScoreScreen);
  if (m_currentMatch != NULL) {
    delete m_currentMatch;
    m_currentMatch = NULL;
  }
  if (m_gameWonScreen != NULL) {
    delete m_gameWonScreen;
    m_gameWonScreen = NULL;
  }
}

void SinglePlayerStarterAction::performBackToMenu()
{
  // Rewind screen stack
  GameUIDefaults::SCREEN_STACK->pop();
  delete m_hiScoreScreen;
  // Restore initial values to the reused action
  m_lifes = 3;
  m_currentLevel = 0;
  m_state = kGameNotStarted;
  m_playerStat.total_points = 0;
}

SinglePlayerMatch::SinglePlayerMatch
                     (PlayerGameStat &playerStat,
                      Action *gameOverAction,
                      PuyoSingleGameLevelData *levelData,
                      bool skipIntroduction,
                      bool popScreen,
                      SinglePlayerFactory *spFactory,
                      int remainingLifes)
		       : m_playerStat(playerStat),
                 m_state(kNotRunning),
                 m_matchOverAction(gameOverAction),
                 m_levelData(levelData),
                 m_skipIntroduction(skipIntroduction),
                 m_popScreen(popScreen),
                 m_spFactory(spFactory),
                 m_remainingLifes(remainingLifes),
                 m_introStory(NULL),
                 m_opponentStory(NULL),
                 m_getReadyWidget(NULL),
                 m_matchLostAnimation(NULL),
                 m_statsWidget(NULL)
{
}

SinglePlayerMatch::~SinglePlayerMatch()
{
  delete m_gameScreen;
  delete m_gameWidget;
  delete m_levelData;
  if (m_matchLostAnimation != NULL)
    delete m_matchLostAnimation;
  if (m_statsWidget != NULL)
    delete m_statsWidget;
}

void SinglePlayerMatch::run()
{
  if (m_state == kNotRunning)
    stateMachine();
}

void SinglePlayerMatch::action(Widget *sender, int actionType,
				     GameControlEvent *event)
{
  //  cout << "SinglePlayerAction!!! " << actionType << endl;
  if (sender == m_introStory->getStoryWidget()) {
    stateMachine();
  }
  else if (sender == m_opponentStory->getStoryWidget()) {
    stateMachine();
  }
  else if (((sender == m_gameWidget)
            && (actionType == GameWidget::GAMEOVER_STARTPRESSED))
           || (actionType == GameWidget::GAME_IS_OVER)) {
    stateMachine();
  }
  else if ((sender == m_matchLostAnimation)
           && (m_state == kMatchLostAnimation)) {
    stateMachine();
  }
  else if (m_state == kMatchGettingStarted)
      stateMachine();
}

void SinglePlayerMatch::performStoryIntroduction()
{
  m_introStory = new StoryScreen(m_levelData->getIntroStory(), this);
  if (m_popScreen)
    GameUIDefaults::SCREEN_STACK->pop();
  GameUIDefaults::SCREEN_STACK->push(m_introStory);
  m_state = kStoryIntroduction;
}

void SinglePlayerMatch::performOpponentStory()
{
  m_opponentStory = new StoryScreen(m_levelData->getStory(), this);
  // If we went from an introduction story, remove it from display
  if (m_introStory != NULL)
    GameUIDefaults::SCREEN_STACK->pop();
  else if (m_popScreen)
    GameUIDefaults::SCREEN_STACK->pop();
  GameUIDefaults::SCREEN_STACK->push(m_opponentStory);
  // Delete the intro story if needed
  if (m_introStory != NULL) {
    delete m_introStory;
    m_introStory = NULL;
  }
  m_state = kStory;
}

void SinglePlayerMatch::performMatchPrepare()
{
  m_gameWidget = m_spFactory->createGameWidget
        (m_levelData->getPuyoTheme(), m_levelData->getLevelTheme(),
         m_levelData->getIALevel(), m_levelData->getNColors(), m_remainingLifes,
         m_levelData->getIAFace(), this);
  m_gameWidget->setGameOptions(m_levelData->getGameOptions());
  m_gameScreen = new GameScreen(*m_gameWidget);
  m_gameWidget->setPlayerOneName(m_spFactory->getPlayerName());
  m_gameWidget->setPlayerTwoName(m_levelData->getIAName());
  m_gameWidget->getStatPlayerOne().total_points = m_playerStat.total_points;
  GameUIDefaults::SCREEN_STACK->pop();
  GameUIDefaults::SCREEN_STACK->push(m_gameScreen);
  delete m_opponentStory;
  prepareGame();
}

void SinglePlayerMatch::prepareGame()
{
    LevelTheme &currentLevelTheme = m_levelData->getLevelTheme();
    m_gameScreen->setSuspended(true);
    m_state = kMatchGettingStarted;
    if (currentLevelTheme.getReadyAnimation2P() == "") {
        performMatchStart();
        return;
    }
    m_getReadyWidget = new StoryWidget(currentLevelTheme.getReadyAnimation2P().c_str(), this);
    m_gameScreen->setOverlayStory(m_getReadyWidget);
}

void SinglePlayerMatch::performMatchStart()
{
    if (m_getReadyWidget != NULL) {
        m_getReadyWidget->setIntegerValue("@start_pressed", 1);
    }
    m_gameScreen->setSuspended(false);
    m_state = kMatchPlaying;
}

void SinglePlayerMatch::performEndOfMatch()
{
  m_playerStat.total_points += m_gameWidget->getStatPlayerOne().points;
  if (m_gameWidget->getAborted()) {
    m_state = kMatchOverAborted;
    trigMatchOverAction();
  }
  else if (m_gameWidget->didPlayerWon()) {
    performMatchScores(kMatchWonScores);
  }
  else { // Match has been lost
    performMatchLostAnimation();
  }
}

void SinglePlayerMatch::performMatchLostAnimation()
{
  m_matchLostAnimation = new StoryWidget(m_levelData->getGameLostStory(),
					    this);
  m_gameScreen->setOverlayStory(m_matchLostAnimation);
  m_state = kMatchLostAnimation;
}

void SinglePlayerMatch::performMatchScores(State scoreState)
{
  m_state = scoreState;
    StatsWidgetDimensions dimensions = m_spFactory->getStatsWidgetDimensions();
  m_statsWidget = new TwoPlayersStatsWidget(this->m_gameWidget->getStatPlayerOne(), this->m_gameWidget->getStatPlayerTwo(), true, false, theCommander->getWindowFramePicture(), dimensions);
  m_gameScreen->add(m_statsWidget);
}

void SinglePlayerMatch::trigMatchOverAction()
{
  m_matchOverAction->action(this, 0, NULL);
}

void SinglePlayerMatch::stateMachine()
{
  switch (m_state) {
  case kNotRunning:
    if ((m_levelData->getIntroStory() != "")
        && (!m_skipIntroduction))
      performStoryIntroduction();
    else
      performOpponentStory();
    break;
  case kStoryIntroduction:
    performOpponentStory();
    break;
  case kStory:
    performMatchPrepare();
    break;
  case kMatchGettingStarted:
    performMatchStart();
    break;
  case kMatchPlaying:
    performEndOfMatch();
    break;
  case kMatchLostAnimation:
    performMatchScores(kMatchLostScores);
    break;
  case kMatchWonScores:
    m_state = kMatchOverWon;
    trigMatchOverAction();
    break;
  case kMatchLostScores:
    m_state = kMatchOverLost;
    trigMatchOverAction();
    break;
  default:
    break;
  }
}

AltTweakedGameWidgetFactory::AltTweakedGameWidgetFactory(PuyoLevelDefinitions::LevelDefinition *levelDef)
    : m_levelDef(levelDef)
{
}

GameWidget *AltTweakedGameWidgetFactory::createGameWidget(PuyoSetTheme &puyoThemeSet,
                                         LevelTheme &levelTheme,
                                         String centerFace,
                                         Action *gameOverAction)
{
    return new SinglePlayerStandardLayoutGameWidget(puyoThemeSet, levelTheme,
                                                    1/*level*/, m_levelDef->easySettings.nColors, 3,
                                                    m_levelDef->opponent, gameOverAction);
}

//---------------------------------
// SinglePlayerMatchIsOverState
//---------------------------------
SinglePlayerMatchIsOverState::SinglePlayerMatchIsOverState(SharedGameAssets *sharedGameAssets,
                                                           SharedMatchAssets *sharedMatchAssets)
    : m_sharedGameAssets(sharedGameAssets),
      m_sharedMatchAssets(sharedMatchAssets),
      m_aknowledged(false)
{
}

void SinglePlayerMatchIsOverState::enterState()
{
    cout << "SinglePlayerMatchIsOver::enterState()" << endl;
    m_aknowledged = false;
    if (m_sharedMatchAssets->m_gameWidget->isGameARunning()) {
        m_aknowledged = true;
        m_sharedMatchAssets->m_leftVictories++;
    }
    else {
        m_gameLostWidget.reset(new StoryWidget(m_sharedGameAssets->levelDef->gameLostStory, this));
	m_sharedMatchAssets->m_gameScreen->setOverlayStory(m_gameLostWidget.get());
        m_sharedMatchAssets->m_rightVictories++;
    }
    m_sharedMatchAssets->m_leftTotal  += m_sharedMatchAssets->m_gameWidget->getStatPlayerOne().points;
    m_sharedMatchAssets->m_rightTotal += m_sharedMatchAssets->m_gameWidget->getStatPlayerTwo().points;
    m_sharedMatchAssets->m_gameWidget->setGameOverAction(this);
}

void SinglePlayerMatchIsOverState::exitState()
{
    m_sharedMatchAssets->m_gameWidget->setGameOverAction(NULL);
    m_gameLostWidget.reset(NULL);
}

bool SinglePlayerMatchIsOverState::evaluate()
{
    return m_aknowledged;
}

GameState *SinglePlayerMatchIsOverState::getNextState()
{
    return m_nextState;
}

void SinglePlayerMatchIsOverState::action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event)
{
    if (sender == m_gameLostWidget.get()) {
        m_gameLostWidget.reset(NULL);
    }
    m_aknowledged = true;
    evaluateStateMachine();
}

//---------------------------------
// SinglePlayerMatchState
//---------------------------------
SinglePlayerMatchState::SinglePlayerMatchState(SharedGameAssets *sharedGameAssets)
    : m_sharedGameAssets(sharedGameAssets)
{
}

void SinglePlayerMatchState::enterState()
{
    m_nextState = NULL;
    // Creating the different game states
    if (m_sharedGameAssets->levelDef->introStory == "")
        m_introStoryScreen.reset(NULL);
    else
        m_introStoryScreen.reset(new DisplayStoryScreenState(m_sharedGameAssets->levelDef->introStory));
    m_opponentStoryScreen.reset(new DisplayStoryScreenState(m_sharedGameAssets->levelDef->opponentStory));
    m_setupMatch.reset(new SetupMatchState(*this, m_sharedGameAssets->difficulty, this, m_sharedAssets));
    m_enterPlayersReady.reset(new EnterPlayerReadyState(m_sharedAssets, m_sharedGetReadyAssets));
    m_exitPlayersReady.reset(new ExitPlayerReadyState(m_sharedAssets, m_sharedGetReadyAssets));
    m_matchPlaying.reset(new MatchPlayingState(m_sharedAssets));
    m_matchIsOver.reset(new SinglePlayerMatchIsOverState(m_sharedGameAssets, &m_sharedAssets));
    m_displayStats.reset(new DisplayStatsState(m_sharedAssets));
    m_leaveMatch.reset(new CallActionState(this, LEAVE_MATCH));
    m_abortGame.reset(new CallActionState(this, ABORT_GAME));
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

void SinglePlayerMatchState::exitState()
{
}

bool SinglePlayerMatchState::evaluate()
{
    if (m_nextState == NULL)
        return false;
    return true;
}

GameState *SinglePlayerMatchState::getNextState()
{
    return m_nextState;
}

String SinglePlayerMatchState::getPlayerName(int playerNumber) const
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

GameWidget *SinglePlayerMatchState::createGameWidget(PuyoSetTheme &puyoThemeSet,
                                         LevelTheme &levelTheme,
                                         String centerFace,
                                         Action *gameOverAction)
{
    return new SinglePlayerStandardLayoutGameWidget(puyoThemeSet, levelTheme,
                                                    m_sharedGameAssets->difficulty,
                                                    m_sharedGameAssets->levelDef->easySettings.nColors,
                                                    m_sharedGameAssets->lifes,
                                                    m_sharedGameAssets->levelDef->opponent, gameOverAction);
}

void SinglePlayerMatchState::action(Widget *sender, int actionType,
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
StoryModePrepareNextMatchState::StoryModePrepareNextMatchState(SharedGameAssets *sharedGameAssets)
    : m_sharedGameAssets(sharedGameAssets)
{
    m_levelDefProvider.reset(new PuyoLevelDefinitions(theCommander->getDataPathManager().getPath("/story/levels.gsl")));
    reset();
}

void StoryModePrepareNextMatchState::enterState()
{
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
    m_gameOverScreen.reset(new PuyoGameOver1PScreen(m_storyName.c_str(),
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

AltSinglePlayerStarterAction::AltSinglePlayerStarterAction(MainScreen *mainScreen, int difficulty,
                                                             SinglePlayerFactory *spFactory)
{
    // Initializing the shared game assets with random data
    m_sharedGameAssets.difficulty = 1;//difficulty;
    // Creating the different game states
    m_pushGameScreen.reset(new PushScreenState());
    m_prepareNextMatch.reset(new StoryModePrepareNextMatchState(&m_sharedGameAssets));
    m_playMatch.reset(new SinglePlayerMatchState(&m_sharedGameAssets));
    m_gameWon.reset(new DisplayStoryScreenState("gamewon_1p.gsl"));
    m_gameLostHoF.reset(new StoryModeDisplayHallOfFameState(&m_sharedGameAssets, m_playMatch->getMatchAssets()));
    m_gameWonHoF.reset(new StoryModeDisplayHallOfFameState(&m_sharedGameAssets, m_playMatch->getMatchAssets(), "gamewon_highscore_1p.gsl"));
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

void AltSinglePlayerStarterAction::action(Widget *sender, int actionType,
                                        event_manager::GameControlEvent *event)
{
    m_sharedGameAssets.playerName = "Bob l'eponge";
    m_sharedGameAssets.lifes = 3;
    m_prepareNextMatch->reset();
    m_stateMachine.reset();
    m_stateMachine.evaluate();
}

