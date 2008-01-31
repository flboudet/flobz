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

PuyoCombinedEventPlayer::PuyoCombinedEventPlayer(PuyoView &view)
    : PuyoPlayer(view),
      player1controller(view, GameControlEvent::kPlayer1Down, GameControlEvent::kPlayer1Left, GameControlEvent::kPlayer1Right,
                       GameControlEvent::kPlayer1TurnLeft, GameControlEvent::kPlayer1TurnRight),
      player2controller(view, GameControlEvent::kPlayer2Down, GameControlEvent::kPlayer2Left, GameControlEvent::kPlayer2Right,
                       GameControlEvent::kPlayer2TurnLeft, GameControlEvent::kPlayer2TurnRight)
{
}

void PuyoCombinedEventPlayer::eventOccured(GameControlEvent *event)
{
    player1controller.eventOccured(event);
    player2controller.eventOccured(event);
}

void PuyoCombinedEventPlayer::cycle()
{
    player1controller.cycle();
    player2controller.cycle();
}

PuyoSinglePlayerGameWidget::PuyoSinglePlayerGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, int level, int nColors, int lifes, String aiFace, Action *gameOverAction)
    : attachedPuyoThemeSet(puyoThemeSet),
    attachedRandom(nColors),
    attachedGameFactory(&attachedRandom),
    areaA(&attachedGameFactory, &attachedPuyoThemeSet, &levelTheme,
          1 + CSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + FSIZE, BSIZE+ESIZE, painter),
    areaB(&attachedGameFactory, &attachedPuyoThemeSet, &levelTheme,
          1 + CSIZE + PUYODIMX*TSIZE + DSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + DSIZE - FSIZE - TSIZE, BSIZE+ESIZE, painter),
    playercontroller(areaA),
    opponentcontroller(level, areaB), faceTicks(0), opponent(aiFace)
{
    initialize(areaA, areaB, playercontroller, opponentcontroller, levelTheme, gameOverAction);
    setLives(lifes);
}

PuyoSinglePlayerGameWidget::~PuyoSinglePlayerGameWidget()
{
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
        opponentcontroller.setAIParameters(ai);
    }
    if (faceTicks == 100) {
        opponent.setIntegerValue("@maxHeightPlayer", attachedGameA->getMaxColumnHeight());
        opponent.setIntegerValue("@maxHeightAI", attachedGameB->getMaxColumnHeight());
        opponent.setIntegerValue("@neutralsForPlayer", attachedGameA->getNeutralPuyos());
        opponent.setIntegerValue("@neutralsForAI", attachedGameB->getNeutralPuyos());
        faceTicks = 0;
    }
    PuyoGameWidget::cycle();
}

PuyoStoryWidget *PuyoSinglePlayerGameWidget::getOpponent()
{
    return &opponent;
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

PuyoGameOver1PScreen::PuyoGameOver1PScreen(String screenName, Screen &previousScreen,
        Action *finishedAction, String playerName, const PlayerGameStat &playerPoints)
        : PuyoStoryScreen(screenName, previousScreen, finishedAction, false),
        playerName(playerName), playerStat(playerPoints)
{
    static char *AI_NAMES[] = { "Fanzy", "Garou", "Big Rabbit", "Gizmo",
    "Satanas", "Doctor X", "Tania", "Mr Gyom",
    "The Duke","Jeko","--------" };
    
    initHiScores(AI_NAMES);
    int scorePlace = setHiScore(playerStat.points, playerName);

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
    add(transitionWidget);
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

SinglePlayerStarterAction::SinglePlayerStarterAction(int difficulty, PuyoSingleNameProvider *nameProvider)
    : currentLevel(0), lifes(3), difficulty(difficulty), levelData(NULL),
      story(NULL), gameScreen(NULL), gameLostWidget(NULL), gameOverScreen(NULL), gameWonScreen(NULL),
      nameProvider(nameProvider), levelDefinitions(theCommander->getDataPathManager().getPath("/story/levels.gsl")),
      inIntroduction(false) {}

void SinglePlayerStarterAction::action()
{
    if ((levelData == NULL) || (inIntroduction)) {
        initiateLevel();
    }
    else if (gameScreen == NULL) {
        startGame();
    }
    else if (! gameWidget->getAborted()) {
        if (! gameWidget->didPlayerWon()) {
            if (gameLostWidget == NULL) {
                lifes--;
                gameLost();
                return;
            }
        }
        else
            currentLevel++;
        
        // Gameover management
        if (lifes < 0) {
            if (gameOverScreen == NULL)
                gameOver();
            else {
                endGameSession();
            }
        }
        else {
            // Game won management
            if (levelDefinitions.getNumLevels() <= currentLevel) {
                if (this->gameWonScreen == NULL)
                    gameWon();
                else {
                    if (gameOverScreen == NULL)
                        gameOver();
                    else
                        endGameSession();
                }
            }
            else
                nextLevel();
        }
    }
    else {
        if (gameOverScreen == NULL)
            gameOver();
        else {
            endGameSession();
        }
    }
}

void SinglePlayerStarterAction::initiateLevel()
{
    if (levelData == NULL) {
        levelData = new PuyoSingleGameLevelData(currentLevel, difficulty, levelDefinitions);
        String introductionStory = levelData->getIntroStory();
        if (introductionStory != "") {
            story = new PuyoStoryScreen(introductionStory, *(GameUIDefaults::SCREEN_STACK->top()), this);
            inIntroduction = true;
        }
        else
            story = new PuyoStoryScreen(levelData->getStory(), *(GameUIDefaults::SCREEN_STACK->top()), this);
    }
    else {
        PuyoStoryScreen *previousStory = story;
        story = new PuyoStoryScreen(levelData->getStory(), *(GameUIDefaults::SCREEN_STACK->top()), this);
        if (inIntroduction) {
            inIntroduction = false;
            GameUIDefaults::SCREEN_STACK->pop();
            delete previousStory;
        }
    }
    GameUIDefaults::SCREEN_STACK->push(story);
}

void SinglePlayerStarterAction::startGame()
{
    gameWidget = new PuyoSinglePlayerGameWidget(levelData->getPuyoTheme(), levelData->getLevelTheme(), levelData->getIALevel(), levelData->getNColors(), lifes, levelData->getIAFace(), this);
    gameScreen = new PuyoGameScreen(*gameWidget, *story);
    if (nameProvider != NULL)
        gameWidget->setPlayerOneName(nameProvider->getPlayerName());
    gameWidget->setPlayerTwoName(levelData->getIAName());
    GameUIDefaults::SCREEN_STACK->pop();
    delete story;
    story = NULL;
    GameUIDefaults::SCREEN_STACK->push(gameScreen);
}

void SinglePlayerStarterAction::nextLevel()
{
    PuyoSingleGameLevelData *tempLevelData = new PuyoSingleGameLevelData(currentLevel, difficulty, levelDefinitions);
    story = new PuyoStoryScreen(tempLevelData->getStory(), *(GameUIDefaults::SCREEN_STACK->top()), this);
    resetGameSession();
    levelData = tempLevelData;
    GameUIDefaults::SCREEN_STACK->push(story);
}

void SinglePlayerStarterAction::gameOver()
{
    String gameOverStory;
    if (gameWonScreen == NULL)
        gameOverStory = levelData->getGameOverStory();
    else
        gameOverStory = "gamewon_highscores_1p.gsl";
    gameOverScreen = new PuyoGameOver1PScreen(gameOverStory, *(GameUIDefaults::SCREEN_STACK->top()), this, gameWidget->getPlayerOneName(), gameWidget->getStatPlayerOne());
    GameUIDefaults::SCREEN_STACK->pop();
    GameUIDefaults::SCREEN_STACK->push(gameOverScreen);
}

void SinglePlayerStarterAction::gameWon()
{
    gameWonScreen = new PuyoStoryScreen("gamewon_1p.gsl", *(GameUIDefaults::SCREEN_STACK->top()), this);
    GameUIDefaults::SCREEN_STACK->pop();
    GameUIDefaults::SCREEN_STACK->push(gameWonScreen);
}

void SinglePlayerStarterAction::gameLost()
{
    gameLostWidget = new PuyoStoryWidget(levelData->getGameLostStory(),
					 this);
    gameScreen->setOverlayStory(gameLostWidget);
}

void SinglePlayerStarterAction::endGameSession()
{
    // Restore initial values to the reused action
    lifes = 3;
    currentLevel = 0;
    resetGameSession();
}
    
void SinglePlayerStarterAction::resetGameSession()
{
    // Rewind screen stack
    Screen *screenToTrans = GameUIDefaults::SCREEN_STACK->top();
    GameUIDefaults::SCREEN_STACK->pop();
    (static_cast<PuyoMainScreen *>(GameUIDefaults::SCREEN_STACK->top()))->transitionFromScreen(*screenToTrans);
    delete gameWidget;
    delete gameScreen;
    delete levelData;
    if (gameLostWidget != NULL)
        delete gameLostWidget;
    if (gameOverScreen != NULL)
        delete gameOverScreen;
    if (gameWonScreen != NULL)
        delete gameWonScreen;

    gameScreen = NULL;
    gameWidget = NULL;
    levelData = NULL;
    gameLostWidget = NULL;
    gameOverScreen = NULL;
    gameWonScreen = NULL;
}

