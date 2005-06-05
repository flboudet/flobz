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

PuyoSinglePlayerGameWidget::PuyoSinglePlayerGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, IA_Type type, int level, Action *gameOverAction) : attachedPuyoThemeSet(puyoThemeSet),
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
}

const struct PuyoSingleGameLevelData::StaticLevelDatas PuyoSingleGameLevelData::levelDatas[] = {
// Level 1
{ "story1.gsl", {RANDOM, 350}, {FLOBO , 190}, {TANIA , 130} },
// Level 2
{ "story2.gsl", {FLOBO , 350}, {JEKO  , 180}, {JEKO  , 100} },
// Level 3
{ "story3.gsl", {FLOBO , 250}, {TANIA , 160}, {GYOM  ,  90} },
// Level 4
{ "story4.gsl", {FLOBO , 180}, {FLOBO ,  90}, {JEKO  ,  80} },
// Level 5
{ "story5.gsl", {FLOBO ,  90}, {GYOM  , 210}, {TANIA ,  60} },
// Level 6
{ "story6.gsl", {JEKO  , 350}, {TANIA ,  90}, {GYOM  ,  60} },
// Level 7
{ "story7.gsl", {TANIA , 320}, {JEKO  ,  80}, {GYOM  ,  40} },
// Level 8
{ "story8.gsl", {FLOBO ,  62}, {GYOM  ,  90}, {GYOM  ,  30} },
// Level 9
{ "story9.gsl", {RANDOM,   0}, {RANDOM,   0}, {RANDOM,   0} }
};


PuyoSingleGameLevelData::PuyoSingleGameLevelData(int gameLevel, int difficulty) : gameLevel(gameLevel), difficulty(difficulty)
{
    themeToUse = new AnimatedPuyoSetTheme("", "Classic.fptheme");
    themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 000.0f);
    themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 060.0f);
    themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 120.0f);
    themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 180.0f);
    themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 240.0f);
    //themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 300.0f);
    themeToUse->addNeutralPuyo("stone", "round", "round", "normal", 0.0f);
    //themeToUse->cache();
    
    levelThemeToUse = new PuyoLevelTheme("", "Classic.fptheme");
    levelThemeToUse->setLives("heart");
    levelThemeToUse->setBackground("dark");
    levelThemeToUse->setGrid("metal");
    levelThemeToUse->setSpeedMeter("fire");
}

PuyoSingleGameLevelData::~PuyoSingleGameLevelData()
{
    delete themeToUse;
    delete levelThemeToUse;
}

String PuyoSingleGameLevelData::getStory() const
{
    return String(levelDatas[gameLevel].storyName);
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


SinglePlayerStarterAction::SinglePlayerStarterAction(int difficulty)
    : currentLevel(0), difficulty(difficulty), levelData(NULL), story(NULL), gameScreen(NULL) {}

void SinglePlayerStarterAction::action()
{
    if (levelData == NULL) {
        initiateLevel();
    }
    else if (gameScreen == NULL) {
        startGame();
    }
    else if (! gameWidget->getAborted()) {
        nextLevel();
    }
    else {
        endGameSession();
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
    gameWidget = new PuyoSinglePlayerGameWidget(levelData->getPuyoTheme(), levelData->getLevelTheme(), levelData->getIAType(), levelData->getIALevel(), this);
    gameScreen = new PuyoGameScreen(*gameWidget, *story);
    GameUIDefaults::SCREEN_STACK->pop();
    delete story;
    story = NULL;
    GameUIDefaults::SCREEN_STACK->push(gameScreen);
}

void SinglePlayerStarterAction::nextLevel()
{
    PuyoSingleGameLevelData *tempLevelData = new PuyoSingleGameLevelData(++currentLevel, difficulty);
    story = new PuyoStoryScreen(tempLevelData->getStory(), *(GameUIDefaults::SCREEN_STACK->top()), this);
    endGameSession();
    levelData = tempLevelData;
    GameUIDefaults::SCREEN_STACK->push(story);
}

void SinglePlayerStarterAction::endGameSession()
{
    GameUIDefaults::SCREEN_STACK->pop();
    ((PuyoRealMainScreen *)(GameUIDefaults::SCREEN_STACK->top()))->transitionFromScreen(*gameScreen);
    delete gameWidget;
    delete gameScreen;
    delete levelData;
    
    gameScreen = NULL;
    gameWidget = NULL;
    levelData = NULL;
}

