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
#include "HiScores.h"
#include "goomsl/goomsl.h"
#include "goomsl/goomsl_hash.h"
#include <vector>

class PuyoSingleNameProvider {
public:
    virtual String getPlayerName() const = 0;
    virtual ~PuyoSingleNameProvider() {};
};

class PuyoCombinedEventPlayer : public PuyoPlayer {
public:
    PuyoCombinedEventPlayer(PuyoView &view);
    void eventOccured(GameControlEvent *event);
    void cycle();
private:
    PuyoEventPlayer player1controller;
    PuyoEventPlayer player2controller;
};

class PuyoSinglePlayerGameWidget : public PuyoGameWidget {
public:
    PuyoSinglePlayerGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, int level, int nColors, int lifes, String aiFace, Action *gameOverAction = NULL);
    virtual ~PuyoSinglePlayerGameWidget();
    bool didPlayerWon() const { return isGameARunning(); }
    void cycle();
    PuyoStoryWidget *getOpponent();
private:
    AnimatedPuyoSetTheme &attachedPuyoThemeSet;
    PuyoRandomSystem attachedRandom;
    PuyoLocalGameFactory attachedGameFactory;
    PuyoView areaA, areaB;
    PuyoCombinedEventPlayer playercontroller;
    PuyoIA opponentcontroller;
    int faceTicks;
    PuyoStoryWidget opponent;
};

class PuyoLevelDefinitions {
public:
    struct SelIA {
      SelIA(int level, int nColors);
        int level;
        int nColors;
    };
    struct LevelDefinition {
      LevelDefinition(String levelName, String introStory,
		      String opponentStory, String opponentName, String opponent,
              String backgroundTheme, String gameLostStory, String gameOverStory,
		      SelIA easySettings, SelIA mediumSettings, SelIA hardSettings)
	: levelName(levelName), introStory(introStory),
	   opponentStory(opponentStory),  opponentName(opponentName), opponent(opponent),
       backgroundTheme(backgroundTheme), gameLostStory(gameLostStory), gameOverStory(gameOverStory),
	   easySettings(easySettings), mediumSettings(mediumSettings), hardSettings(hardSettings) {}
      String levelName;
      String introStory;
      String opponentStory;
      String opponentName;
      String opponent;
      String backgroundTheme;
      String gameLostStory;
      String gameOverStory;
      SelIA easySettings;
      SelIA mediumSettings;
      SelIA hardSettings;
    };
    PuyoLevelDefinitions(String levelDefinitionFile);
    LevelDefinition *getLevelDefinition(int levelNumber) { return levelDefinitions[levelNumber]; }
    int getNumLevels() const { return levelDefinitions.size(); }
    virtual ~PuyoLevelDefinitions();
private:
    void addLevelDefinition(String levelName, String introStory,
			    String opponentStory, String opponentName, String opponent,
                String backgroundTheme, String gameLostStory, String gameOverStory,
			    SelIA easySettings,
			    SelIA mediumSettings, SelIA hardSettings);
    static void end_level(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static PuyoLevelDefinitions *currentDefinition;
    AdvancedBuffer<LevelDefinition *> levelDefinitions;
};

class PuyoSingleGameLevelData {
public:
    PuyoSingleGameLevelData(int gameLevel, int difficulty, PuyoLevelDefinitions &levelDefinitions);
    ~PuyoSingleGameLevelData();
    String getIntroStory() const;
    String getStory() const;
    String getGameLostStory() const;
    String getGameOverStory() const;
    AnimatedPuyoSetTheme &getPuyoTheme() const;
    PuyoLevelTheme &getLevelTheme() const;
    int getIALevel() const;
    String getIAName() const;
    String getIAFace() const;
    int getNColors() const;
private:
    int gameLevel, difficulty;
    PuyoLevelDefinitions::LevelDefinition *levelDefinition;
    AnimatedPuyoSetTheme *themeToUse;
    PuyoLevelTheme *levelThemeToUse;
};

class PuyoGameOver1PScreen : public PuyoStoryScreen {
public:
    PuyoGameOver1PScreen(String screenName, Screen &previousScreen, Action *finishedAction,
            String playerName, const PlayerGameStat &playerPoints);
    void refresh();
    virtual ~PuyoGameOver1PScreen();
private:
    Text names[kHiScoresNumber], points[kHiScoresNumber];
    VBox hiScoreNameBox, hiScorePointBox;
    HBox hiScoreBox;
    String playerName;
    PlayerGameStat playerStat;
};

class SinglePlayerStarterAction : public Action {
public:
    SinglePlayerStarterAction(int difficulty, PuyoSingleNameProvider *nameProvider = NULL);
    void action();
    
private:
    void initiateLevel();
    
    void startGame();
    
    void nextLevel();

    void gameLost();
    
    void gameOver();
    
    void gameWon();
    
    void resetGameSession();
    
    void endGameSession();
    
    int currentLevel, lifes, difficulty;
    PuyoSingleGameLevelData *levelData;
    PuyoStoryScreen *story;
    PuyoGameScreen *gameScreen;
    PuyoSinglePlayerGameWidget *gameWidget;
    PuyoStoryWidget *gameLostWidget;
    PuyoGameOver1PScreen *gameOverScreen;
    PuyoStoryScreen *gameWonScreen;
    PuyoSingleNameProvider *nameProvider;
    PuyoLevelDefinitions levelDefinitions;
    bool inIntroduction;
};

#endif // _PUYOSINGLEPLAYERSTARTER
