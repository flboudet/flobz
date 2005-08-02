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

PuyoSinglePlayerGameWidget::PuyoSinglePlayerGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, IA_Type type, int level, int lifes, Action *gameOverAction) : attachedPuyoThemeSet(puyoThemeSet),
                                                     attachedGameFactory(&attachedRandom),
                                                     areaA(&attachedGameFactory, &attachedPuyoThemeSet,
                                                     1 + CSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + FSIZE, BSIZE+ESIZE, painter),
                                                     areaB(&attachedGameFactory, &attachedPuyoThemeSet,
                                                     1 + CSIZE + PUYODIMX*TSIZE + DSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + DSIZE - FSIZE - TSIZE, BSIZE+ESIZE, painter),
                                                     playercontroller(areaA, GameControlEvent::kPlayer1Down, GameControlEvent::kPlayer1Left, GameControlEvent::kPlayer1Right,
                                                     GameControlEvent::kPlayer1TurnLeft, GameControlEvent::kPlayer1TurnRight),
                                                     opponentcontroller(type, level, areaB)
{
    initialize(areaA, areaB, playercontroller, opponentcontroller, levelTheme, gameOverAction);
    setLives(lifes);
}

const struct PuyoSingleGameLevelData::StaticLevelDatas PuyoSingleGameLevelData::levelDatas[] = {
// Level 1
{ "story1.gsl", "Herbert the Dog", {RANDOM, 350}, {FLOBO , 190}, {TANIA , 130} },
// Level 2
{ "story2.gsl", "Duke Beary", {FLOBO , 350}, {JEKO  , 180}, {JEKO  , 100} },
// Level 3
{ "story3.gsl", "???", {FLOBO , 250}, {TANIA , 160}, {GYOM  ,  90} },
// Level 4
{ "story4.gsl", "Azrael", {FLOBO , 180}, {FLOBO ,  90}, {JEKO  ,  80} },
// Level 5
{ "story5.gsl", "???", {FLOBO ,  90}, {GYOM  , 210}, {TANIA ,  60} },
// Level 6
{ "story6.gsl", "Zklogh", {JEKO  , 350}, {TANIA ,  90}, {GYOM  ,  60} },
// Level 7
{ "story7.gsl", "MegaPuyo", {TANIA , 320}, {JEKO  ,  80}, {GYOM  ,  40} },
// Level 8
{ "story8.gsl", "Adam McFlurry", {FLOBO ,  62}, {GYOM  ,  90}, {GYOM  ,  30} },
// Level 9
{ "story9.gsl", "MechaPuyo", {RANDOM,   0}, {RANDOM,   0}, {RANDOM,   0} }
};


PuyoSingleGameLevelData::PuyoSingleGameLevelData(int gameLevel, int difficulty) : gameLevel(gameLevel), difficulty(difficulty)
{
    AnimatedPuyoThemeManager * themeManager = getPuyoThemeManger();
    themeToUse = themeManager->getAnimatedPuyoSetTheme();
    levelThemeToUse = themeManager->getPuyoLevelTheme("Level1");
    
    /*
    themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 000.0f);
    themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 072.0f);
    themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 144.0f);
    themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 216.0f);
    themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 288.0f);
    themeToUse->addNeutralPuyo("stone", "round", "round", "normal", 0.0f);

    levelThemeToUse->setLives("heart");
    levelThemeToUse->setBackground("dark");
    levelThemeToUse->setGrid("metal");
    levelThemeToUse->setSpeedMeter("fire");
    */
}

PuyoSingleGameLevelData::~PuyoSingleGameLevelData()
{
}

String PuyoSingleGameLevelData::getStory() const
{
    return String(levelDatas[gameLevel].storyName);
}

String PuyoSingleGameLevelData::getGameLostStory() const
{
    return "gamelost1p.gsl";
}

String PuyoSingleGameLevelData::getGameOverStory() const
{
    return "gameover1p.gsl";
}
    
AnimatedPuyoSetTheme &PuyoSingleGameLevelData::getPuyoTheme() const
{
    return *themeToUse;
}

PuyoLevelTheme &PuyoSingleGameLevelData::getLevelTheme() const
{
    return *levelThemeToUse;
}

IA_Type PuyoSingleGameLevelData::getIAType() const
{
    switch (difficulty) {
        case 0:
            return levelDatas[gameLevel].easySetting.type;
        case 1:
            return levelDatas[gameLevel].mediumSetting.type;
        default:
            return levelDatas[gameLevel].hardSetting.type;
    }
}

int PuyoSingleGameLevelData::getIALevel() const
{
    switch (difficulty) {
        case 0:
            return levelDatas[gameLevel].easySetting.level;
        case 1:
            return levelDatas[gameLevel].mediumSetting.level;
        default:
            return levelDatas[gameLevel].hardSetting.level;
    }
}

const char * PuyoSingleGameLevelData::getIAName() const {
    return levelDatas[gameLevel].iaName;
}

PuyoGameOver1PScreen::PuyoGameOver1PScreen(String screenName, Screen &previousScreen, Action *finishedAction, String playerName, int playerPoints)
        : PuyoStoryScreen(screenName, previousScreen, finishedAction)
{
    static char *AI_NAMES[] = { "Fanzy", "Garou", "Big Rabbit", "Gizmo",
    "Satanas", "Doctor X", "Tania", "Mr Gyom",
    "The Duke","Jeko","--------" };
    
    initHiScores(AI_NAMES);
    int scorePlace = setHiScore(playerPoints, playerName);
    hiscore *scores = getHiScores();
    //hiScoreBox.add(new Text("Game Over"));
    for (int i = 0 ; i < kHiScoresNumber ; i++) {
        char tmp[256];
        sprintf(tmp, "%d", scores[i].score);
        names[i].setValue(scores[i].name);
        points[i].setValue(tmp);
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
}

SinglePlayerStarterAction::SinglePlayerStarterAction(int difficulty, PuyoSingleNameProvider *nameProvider)
    : currentLevel(0), lifes(3), difficulty(difficulty), levelData(NULL),
      story(NULL), gameScreen(NULL), gameLostWidget(NULL), gameOverScreen(NULL),
      nameProvider(nameProvider) {}

void SinglePlayerStarterAction::action()
{
    if (levelData == NULL) {
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
        if (lifes < 0) {
            if (gameOverScreen == NULL)
                gameOver();
            else {
                lifes = 3;
                currentLevel = 0;
                endGameSession();
            }
        }
        else {
            nextLevel();
        }
    }
    else {
        if (gameOverScreen == NULL)
            gameOver();
        else {
            lifes = 3;
            currentLevel = 0;
            endGameSession();
        }
    }
}

void SinglePlayerStarterAction::initiateLevel()
{
    levelData = new PuyoSingleGameLevelData(currentLevel, difficulty);
    story = new PuyoStoryScreen(levelData->getStory(), *(GameUIDefaults::SCREEN_STACK->top()), this);
    GameUIDefaults::SCREEN_STACK->push(story);
}

void SinglePlayerStarterAction::startGame()
{
    gameWidget = new PuyoSinglePlayerGameWidget(levelData->getPuyoTheme(), levelData->getLevelTheme(), levelData->getIAType(), levelData->getIALevel(), lifes, this);
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
    PuyoSingleGameLevelData *tempLevelData = new PuyoSingleGameLevelData(currentLevel, difficulty);
    story = new PuyoStoryScreen(tempLevelData->getStory(), *(GameUIDefaults::SCREEN_STACK->top()), this);
    endGameSession();
    levelData = tempLevelData;
    GameUIDefaults::SCREEN_STACK->push(story);
}

void SinglePlayerStarterAction::gameOver()
{
    gameOverScreen = new PuyoGameOver1PScreen(levelData->getGameOverStory(), *(GameUIDefaults::SCREEN_STACK->top()), this, gameWidget->getPlayerOneName(), gameWidget->getPointsPlayerOne());
    GameUIDefaults::SCREEN_STACK->pop();
    GameUIDefaults::SCREEN_STACK->push(gameOverScreen);
}

void SinglePlayerStarterAction::gameLost()
{
    gameLostWidget = new PuyoStoryWidget(levelData->getGameLostStory(),
					 this);
    gameScreen->setOverlayStory(gameLostWidget);
}

void SinglePlayerStarterAction::endGameSession()
{
    Screen *screenToTrans = GameUIDefaults::SCREEN_STACK->top();
    GameUIDefaults::SCREEN_STACK->pop();
    (static_cast<PuyoRealMainScreen *>(GameUIDefaults::SCREEN_STACK->top()))->transitionFromScreen(*screenToTrans);
    delete gameWidget;
    delete gameScreen;
    delete levelData;
    if (gameLostWidget != NULL)
        delete gameLostWidget;
    if (gameOverScreen != NULL)
        delete gameOverScreen;

    gameScreen = NULL;
    gameWidget = NULL;
    levelData = NULL;
    gameLostWidget = NULL;
    gameOverScreen = NULL;
}

