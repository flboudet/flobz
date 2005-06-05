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
    PuyoSingleGameLevelData(int gameLevel)
    {
        themeToUse = new AnimatedPuyoSetTheme("", "Classic.fptheme");
        themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 000.0f);
        themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 060.0f);
        themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 120.0f);
        themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 180.0f);
        themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 240.0f);
        themeToUse->addAnimatedPuyoTheme("stone", "round", "round", "normal", 300.0f);
        themeToUse->addNeutralPuyo("stone", "round", "round", "normal", 0.0f);
        
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
    
    int getStory() const { return 1; }
    AnimatedPuyoSetTheme &getPuyoTheme() const { return *themeToUse; }
    PuyoLevelTheme &getLevelTheme() const { return *levelThemeToUse; }
    
private:
    AnimatedPuyoSetTheme *themeToUse;
    PuyoLevelTheme *levelThemeToUse;
};

class ExperimentalPlayerGameAction : public Action {
public:
    ExperimentalPlayerGameAction() : currentLevel(0), levelData(NULL), story(NULL), gameScreen(NULL) {}
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
        levelData = new PuyoSingleGameLevelData(currentLevel);
        story = new PuyoStoryScreen(levelData->getStory(), *(GameUIDefaults::SCREEN_STACK->top()), this);
        GameUIDefaults::SCREEN_STACK->push(story);
    }
    
    void startGame()
    {
        gameWidget = new PuyoTwoPlayerGameWidget(levelData->getPuyoTheme(), levelData->getLevelTheme(), this);
        gameScreen = new PuyoGameScreen(*gameWidget, *story);
        GameUIDefaults::SCREEN_STACK->pop();
        delete story;
        story = NULL;
        GameUIDefaults::SCREEN_STACK->push(gameScreen);
    }
    
    void nextLevel()
    {
        PuyoSingleGameLevelData *tempLevelData = new PuyoSingleGameLevelData(++currentLevel);
        story = new PuyoStoryScreen(tempLevelData->getStory(), *(GameUIDefaults::SCREEN_STACK->top()), this);
        endGameSession();
        levelData = tempLevelData;
        GameUIDefaults::SCREEN_STACK->push(story);
    }
    
    void endGameSession()
    {
        GameUIDefaults::SCREEN_STACK->pop();
        delete gameWidget;
        delete gameScreen;
        delete levelData;
        
        gameScreen = NULL;
        gameWidget = NULL;
        levelData = NULL;
    }
    
private:
    int currentLevel;
    PuyoSingleGameLevelData *levelData;
    PuyoStoryScreen *story;
    PuyoGameScreen *gameScreen;
    PuyoTwoPlayerGameWidget *gameWidget;
};

void LocalGameMenu::build() {
    add(new EditFieldWithLabel("Player Name:", "flobo"));
    add(new Text("Choose Game Level"));
    add(new Button("Easy", &easyAction));
    add(new Button("Medium", new ExperimentalPlayerGameAction()));
    add(new Button("Hard", new SinglePlayerGameActionz(HARD)));
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


