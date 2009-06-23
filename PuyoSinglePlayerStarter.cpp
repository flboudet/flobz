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

#include "PuyoSinglePlayerStarter.h"
#include "PuyoView.h"

PuyoSinglePlayerGameWidget::PuyoSinglePlayerGameWidget(int lifes, String aiFace)
    : opponentcontroller(NULL),
      faceTicks(0), opponent(aiFace),
      killLeftCheat("killleft", this),
      killRightCheat("killright", this)
{
    setLives(lifes);
}

void PuyoSinglePlayerGameWidget::initWithGUI(PuyoView &areaA, PuyoView &areaB,
                                            PuyoPlayer &playercontroller,
                                            PuyoLevelTheme &levelTheme,
                                            int level,
                                            Action *gameOverAction)
{
    opponentcontroller = new PuyoIA(level, areaB);
    PuyoGameWidget::initWithGUI(areaA, areaB, playercontroller, *opponentcontroller,
                               levelTheme, gameOverAction);
    addSubWidget(&killLeftCheat);
    addSubWidget(&killRightCheat);
}

SinglePlayerStandardLayoutGameWidget::SinglePlayerStandardLayoutGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, int level, int nColors, int lifes, String aiFace, Action *gameOverAction)
  : PuyoSinglePlayerGameWidget(lifes, aiFace),
      attachedPuyoThemeSet(puyoThemeSet),
      attachedRandom(nColors),
      attachedGameFactory(&attachedRandom),
      areaA(&attachedGameFactory, &attachedPuyoThemeSet, &levelTheme,
            1 + CSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + FSIZE, BSIZE+ESIZE),
      areaB(&attachedGameFactory, &attachedPuyoThemeSet, &levelTheme,
            1 + CSIZE + PUYODIMX*TSIZE + DSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + DSIZE - FSIZE - TSIZE, BSIZE+ESIZE),
      playercontroller(areaA)

{
    initWithGUI(areaA, areaB, playercontroller, levelTheme, level, gameOverAction);
}

PuyoSinglePlayerGameWidget::~PuyoSinglePlayerGameWidget()
{
    if (opponentcontroller != NULL)
        delete opponentcontroller;
}

void PuyoSinglePlayerGameWidget::cycle()
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
    PuyoGameWidget::cycle();
}

StoryWidget *PuyoSinglePlayerGameWidget::getOpponent()
{
    return &opponent;
}

void PuyoSinglePlayerGameWidget::action(Widget *sender, int actionType,
                                        GameControlEvent *event)
{
    if (sender == static_cast<Widget *>(&killLeftCheat))
        addGameAHandicap(PUYODIMY);
    else if (sender == static_cast<Widget *>(&killRightCheat))
        addGameBHandicap(PUYODIMY);
}

PuyoSinglePlayerGameWidget *SinglePlayerStandardLayoutFactory::createGameWidget
          (AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme,
           int level, int nColors, int lifes, String aiFace,
           Action *gameOverAction)
{
    return new SinglePlayerStandardLayoutGameWidget(puyoThemeSet, levelTheme,
                                          level, nColors, lifes,
                                          aiFace, gameOverAction);
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
    AnimatedPuyoThemeManager * themeManager = getPuyoThemeManger();
    themeToUse = themeManager->getAnimatedPuyoSetTheme();
    if (levelDefinition->backgroundTheme == "Prefs.DefaultTheme")
        levelThemeToUse = themeManager->getPuyoLevelTheme();
    else
        levelThemeToUse = themeManager->getPuyoLevelTheme(levelDefinition->backgroundTheme);
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

AnimatedPuyoSetTheme &PuyoSingleGameLevelData::getPuyoTheme() const
{
    return *themeToUse;
}

PuyoLevelTheme &PuyoSingleGameLevelData::getLevelTheme() const
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

PuyoGameOver1PScreen::PuyoGameOver1PScreen(String screenName, Screen &previousScreen,
        Action *finishedAction, String playerName, const PlayerGameStat &playerPoints, bool initialTransition)
        : StoryScreen(screenName, previousScreen, finishedAction, initialTransition),
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

    hiScoreBox.add(&hiScoreNameBox);
    hiScoreBox.add(&hiScorePointBox);

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
    m_gameWonScreen = new StoryScreen("gamewon_1p.gsl", *(GameUIDefaults::SCREEN_STACK->top()), this);
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
		     *(GameUIDefaults::SCREEN_STACK->top()),
                     this, m_spFactory->getPlayerName(),
                     m_playerStat);
  Screen *screenToTrans = GameUIDefaults::SCREEN_STACK->top();
  m_hiScoreScreen->transitionFromScreen(*screenToTrans);
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
  Screen *screenToTrans = GameUIDefaults::SCREEN_STACK->top();
  if (m_mainScreen != NULL)
    m_mainScreen->transitionFromScreen(*screenToTrans);
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
  if (sender == m_introStory->getStoryWidget()) {
    stateMachine();
  }
  else if (sender == m_opponentStory->getStoryWidget()) {
    stateMachine();
  }
  else if (sender == m_gameWidget) {
    stateMachine();
  }
  else if ((sender == m_matchLostAnimation)
           && (m_state == kMatchLostAnimation)) {
    stateMachine();
  }
}

void SinglePlayerMatch::performStoryIntroduction()
{
  m_introStory = new StoryScreen(m_levelData->getIntroStory(),
			*(GameUIDefaults::SCREEN_STACK->top()), this);
  if (m_popScreen)
    GameUIDefaults::SCREEN_STACK->pop();
  GameUIDefaults::SCREEN_STACK->push(m_introStory);
  m_state = kStoryIntroduction;
}

void SinglePlayerMatch::performOpponentStory()
{
  m_opponentStory = new StoryScreen(m_levelData->getStory(),
                         *(GameUIDefaults::SCREEN_STACK->top()), this);
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

void SinglePlayerMatch::performMatchPlaying()
{
  m_gameWidget = m_spFactory->createGameWidget
        (m_levelData->getPuyoTheme(), m_levelData->getLevelTheme(),
         m_levelData->getIALevel(), m_levelData->getNColors(), m_remainingLifes,
         m_levelData->getIAFace(), this);
  m_gameWidget->setGameOptions(m_levelData->getGameOptions());
  m_gameScreen = new PuyoGameScreen(*m_gameWidget, *m_opponentStory);
  m_gameWidget->setPlayerOneName(m_spFactory->getPlayerName());
  m_gameWidget->setPlayerTwoName(m_levelData->getIAName());
  m_gameWidget->getStatPlayerOne().total_points = m_playerStat.total_points;
  GameUIDefaults::SCREEN_STACK->pop();
  GameUIDefaults::SCREEN_STACK->push(m_gameScreen);
  delete m_opponentStory;
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
  m_statsWidget = new PuyoTwoPlayersStatsWidget(this->m_gameWidget->getStatPlayerOne(), this->m_gameWidget->getStatPlayerTwo(), true, false, theCommander->getWindowFramePicture());
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
    performMatchPlaying();
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
