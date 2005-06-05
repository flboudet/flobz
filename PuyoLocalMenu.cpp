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

#include "PuyoLocalMenu.h"
#include "PuyoSinglePlayerStarter.h"
#include "AnimatedPuyoTheme.h"
#include "PuyoScreenTransition.h"

class SinglePlayerGameActionz : public Action {
public:
    SinglePlayerGameActionz(SinglePlayerLevel level) {;}
    void action();
};

/**
 * Launches a single player game
 */
void SinglePlayerGameActionz::action()
{
  PuyoStarter *starter = new PuyoSinglePlayerStarter(theCommander, 250, FLOBO, 0);
  starter->run(0,0,0,0,0);
  GameUIDefaults::SCREEN_STACK->push(starter);
}

void NewSinglePlayerGameAction::action()
{
    AnimatedPuyoSetTheme *themeToUse = new AnimatedPuyoSetTheme("", "Classic.fptheme");
    themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 000.0f);
    themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 060.0f);
    themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 120.0f);
    themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 180.0f);
    themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 240.0f);
    themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 300.0f);
    themeToUse->addNeutralPuyo("stone", "round", "round", "normal", 0.0f);
    //themeToUse->cache();
    
    PuyoLevelTheme *levelThemeToUse = new PuyoLevelTheme("", "Classic.fptheme");
    levelThemeToUse->setLives("heart");
    levelThemeToUse->setBackground("dark");
    levelThemeToUse->setGrid("metal");
    levelThemeToUse->setSpeedMeter("fire");
    //levelThemeToUse->cache();
    
    PuyoGameScreen *starter = new PuyoGameScreen(*(new PuyoTwoPlayerGameWidget(*themeToUse, *levelThemeToUse)), *(GameUIDefaults::SCREEN_STACK->top()));
    GameUIDefaults::SCREEN_STACK->push(starter);
}

class PuyoSingleGameLevelData {
public:
    PuyoSingleGameLevelData(int gameLevel, int difficulty) : gameLevel(gameLevel), difficulty(difficulty)
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
    
    ~PuyoSingleGameLevelData()
    {
        delete themeToUse;
        delete levelThemeToUse;
    }
    
    String getStory() const { return String(levelDatas[gameLevel].storyName); }
    AnimatedPuyoSetTheme &getPuyoTheme() const { return *themeToUse; }
    PuyoLevelTheme &getLevelTheme() const { return *levelThemeToUse; }
    IA_Type getIAType() const
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
    int getIALevel() const
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
    
private:
    int gameLevel, difficulty;
    AnimatedPuyoSetTheme *themeToUse;
    PuyoLevelTheme *levelThemeToUse;
    struct SelIA {
        IA_Type type;
        int level;
    };
    struct StaticLevelDatas {
        char *storyName;
        SelIA easySetting, mediumSetting, hardSetting;
    };
    static const struct StaticLevelDatas levelDatas[];
};


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

class ExperimentalPlayerGameAction : public Action {
public:
    ExperimentalPlayerGameAction(int difficulty) : currentLevel(0), difficulty(difficulty), levelData(NULL), story(NULL), gameScreen(NULL) {}
    void action()
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
    
    void initiateLevel()
    {
        levelData = new PuyoSingleGameLevelData(currentLevel, difficulty);
        story = new PuyoStoryScreen(levelData->getStory(), *(GameUIDefaults::SCREEN_STACK->top()), this);
        GameUIDefaults::SCREEN_STACK->push(story);
    }
    
    void startGame()
    {
        gameWidget = new PuyoSinglePlayerGameWidget(levelData->getPuyoTheme(), levelData->getLevelTheme(), levelData->getIAType(), levelData->getIALevel(), this);
        gameScreen = new PuyoGameScreen(*gameWidget, *story);
        GameUIDefaults::SCREEN_STACK->pop();
        delete story;
        story = NULL;
        GameUIDefaults::SCREEN_STACK->push(gameScreen);
    }
    
    void nextLevel()
    {
        PuyoSingleGameLevelData *tempLevelData = new PuyoSingleGameLevelData(++currentLevel, difficulty);
        story = new PuyoStoryScreen(tempLevelData->getStory(), *(GameUIDefaults::SCREEN_STACK->top()), this);
        endGameSession();
        levelData = tempLevelData;
        GameUIDefaults::SCREEN_STACK->push(story);
    }
    
    void endGameSession()
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
    
private:
    int currentLevel, difficulty;
    PuyoSingleGameLevelData *levelData;
    PuyoStoryScreen *story;
    PuyoGameScreen *gameScreen;
    PuyoGameWidget *gameWidget;
};

void LocalGameMenu::build() {
    add(new EditFieldWithLabel("Player Name:", "flobo"));
    add(new Text("Choose Game Level"));
    add(new Button("Easy", new ExperimentalPlayerGameAction(0)));
    add(new Button("Medium", new ExperimentalPlayerGameAction(1)));
    add(new Button("Hard", new ExperimentalPlayerGameAction(2)));
    add(new Button("Cancel", new PuyoPopMenuAction(mainScreen)));
}


void Local2PlayersGameMenu::build()
{
    add(new EditFieldWithLabel("Player Name:", "flobo"));
    add(new Text("Choose Game Level"));
    add(new Button("Easy", new SinglePlayerGameActionz(EASY)));
    add(new Button("Medium", new SinglePlayerGameActionz(MEDIUM)));
    add(new Button("Hard", new SinglePlayerGameActionz(HARD)));
    add(new Button("Cancel", new PuyoPopMenuAction(mainScreen)));
}


