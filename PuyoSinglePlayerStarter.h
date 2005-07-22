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
#ifndef _PUYOSINGLEPLAYERSTARTER
#define _PUYOSINGLEPLAYERSTARTER

#include "PuyoStarter.h"

class PuyoSinglePlayerGameWidget : public PuyoGameWidget {
public:
    PuyoSinglePlayerGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, IA_Type type, int level, int lifes, Action *gameOverAction = NULL);
    bool didPlayerWon() const { return isGameARunning(); }
private:
    AnimatedPuyoSetTheme &attachedPuyoThemeSet;
    PuyoRandomSystem attachedRandom;
    PuyoLocalGameFactory attachedGameFactory;
    PuyoView areaA, areaB;
    PuyoEventPlayer playercontroller;
    PuyoIA opponentcontroller;
};


class PuyoSingleGameLevelData {
public:
    PuyoSingleGameLevelData(int gameLevel, int difficulty);
    ~PuyoSingleGameLevelData();
    String getStory() const;
    String getGameLostStory() const;
    String getGameOverStory() const;
    AnimatedPuyoSetTheme &getPuyoTheme() const;
    PuyoLevelTheme &getLevelTheme() const;
    IA_Type getIAType() const;
    int getIALevel() const;
    
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


class SinglePlayerStarterAction : public Action {
public:
    SinglePlayerStarterAction(int difficulty);
    void action();
    
private:
    void initiateLevel();
    
    void startGame();
    
    void nextLevel();

    void gameLost();
    
    void gameOver();
    
    void endGameSession();
    
    int currentLevel, lifes, difficulty;
    PuyoSingleGameLevelData *levelData;
    PuyoStoryScreen *story;
    PuyoGameScreen *gameScreen;
    PuyoSinglePlayerGameWidget *gameWidget;
    PuyoStoryWidget *gameLostWidget;
    PuyoStoryScreen *gameOverScreen;
};

#endif // _PUYOSINGLEPLAYERSTARTER
