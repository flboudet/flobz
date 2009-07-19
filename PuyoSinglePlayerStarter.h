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
#include "PuyoStatsWidget.h"
#include "goomsl/goomsl.h"
#include "goomsl/goomsl_hash.h"
#include <vector>

class SinglePlayerGameWidget : public GameWidget, public Action {
public:
    SinglePlayerGameWidget(int lifes, String aiFace);
    void initWithGUI(PuyoView &areaA, PuyoView &areaB, PuyoPlayer &playercontroller, PuyoLevelTheme &levelTheme, int level, Action *gameOverAction);
    virtual ~SinglePlayerGameWidget();
    bool didPlayerWon() const { return isGameARunning(); }
    void cycle();
    StoryWidget *getOpponent();
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType,
                        GameControlEvent *event);
protected:
    PuyoIA *opponentcontroller;
    int faceTicks;
    StoryWidget opponent;
    PuyoCheatCodeManager killLeftCheat, killRightCheat;
};

class SinglePlayerStandardLayoutGameWidget : public SinglePlayerGameWidget
{
public:
    SinglePlayerStandardLayoutGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, int level, int nColors, int lifes, String aiFace, Action *gameOverAction = NULL);
private:
    AnimatedPuyoSetTheme &attachedPuyoThemeSet;
    PuyoRandomSystem attachedRandom;
    PuyoLocalGameFactory attachedGameFactory;
    PuyoView areaA, areaB;
    PuyoCombinedEventPlayer playercontroller;
};

/**
 * This class provides objects that are used during the game,
 * such as the player name, the player controller,
 * and the screen layout.
 */
class SinglePlayerFactory {
public:
    virtual String getPlayerName() const = 0;
    virtual SinglePlayerGameWidget *createGameWidget
        (AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme,
         int level, int nColors, int lifes, String aiFace,
         Action *gameOverAction) = 0;
    virtual ~SinglePlayerFactory() {};
};

/**
 * Implementation of SinglePlayerFactory with the standard game layout
 */
class SinglePlayerStandardLayoutFactory : public SinglePlayerFactory {
    virtual SinglePlayerGameWidget *createGameWidget
    (AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme,
     int level, int nColors, int lifes, String aiFace,
     Action *gameOverAction);
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
    GameOptions getGameOptions() const;
private:
    int gameLevel, difficulty;
    PuyoLevelDefinitions::LevelDefinition *levelDefinition;
    AnimatedPuyoSetTheme *themeToUse;
    PuyoLevelTheme *levelThemeToUse;
};

class PuyoGameOver1PScreen : public StoryScreen {
public:
    PuyoGameOver1PScreen(String screenName, Screen &previousScreen, Action *finishedAction,
            String playerName, const PlayerGameStat &playerPoints, bool initialTransition=false);
    void refresh();
    virtual ~PuyoGameOver1PScreen();
private:
    Text names[kHiScoresNumber], points[kHiScoresNumber];
    VBox hiScoreNameBox, hiScorePointBox;
    HBox hiScoreBox;
    String playerName;
    PlayerGameStat playerStat;
};

class SinglePlayerMatch;

/**
 * State machine managing a single player game session
 */
class SinglePlayerStarterAction : public Action {
public:
    SinglePlayerStarterAction(MainScreen *mainScreen, int difficulty,
			      SinglePlayerFactory *spFactory);
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType,
			GameControlEvent *event);
private:
    void stateMachine();
    void performMatchPlaying(bool skipIntroduction = false,
			     bool popScreen = false);
    void performEndOfMatch();
    void performHiScoreScreen(String gameOverStoryName);
    void performBackToMenu();
    void performGameWon();

    enum State {
      kGameNotStarted,
      kMatchPlaying,
      kGameWon,
      kGameOver,
      kHiScoreScreen
    };
    MainScreen *m_mainScreen;
    State m_state;
    SinglePlayerFactory *m_spFactory;
    int m_currentLevel, m_lifes, m_difficulty;
    SinglePlayerMatch *m_currentMatch;
    PuyoGameOver1PScreen *m_hiScoreScreen;
    StoryScreen *m_gameWonScreen;
    PuyoLevelDefinitions m_levelDefinitions;
    PlayerGameStat m_playerStat;
};

/**
 * State machine managing a single player match against an opponent
 */
class SinglePlayerMatch : public Action, public Widget {
public:
    SinglePlayerMatch(PlayerGameStat &playerStat,
              Action *gameOverAction,
		      PuyoSingleGameLevelData *levelData,
		      bool skipIntroduction = false,
                      bool popScreen = false,
		      SinglePlayerFactory *spFactory = NULL,
		      int remainingLifes = 0);
    virtual ~SinglePlayerMatch();
    void run();
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType,
			GameControlEvent *event);
    enum State {
      kNotRunning,
      kStoryIntroduction,
      kStory,
      kMatchPlaying,
      kMatchLostAnimation,
      kMatchWonScores,
      kMatchLostScores,
      kMatchOverWon,
      kMatchOverLost,
      kMatchOverAborted
    };
    /**
     * Returns the current state of the match
     */
    State getState() const { return m_state; }
    /**
     * Returns the game over story name of the opponent
     */
    String getGameOverStoryName() const
      { return m_levelData->getGameOverStory(); }
private:
    void performStoryIntroduction();
    void performOpponentStory();
    void performMatchPlaying();
    void performEndOfMatch();
    void performMatchLostAnimation();
    void performMatchScores(State scoreState);
    void trigMatchOverAction();
    /**
     * Performs a step in the match state machine
     */
    void stateMachine();
    State m_state;
    Action *m_matchOverAction;
    PuyoSingleGameLevelData *m_levelData;
    bool m_skipIntroduction;
    bool m_popScreen;
    SinglePlayerFactory *m_spFactory;
    int m_remainingLifes;
    StoryScreen *m_introStory, *m_opponentStory;
    GameScreen *m_gameScreen;
    SinglePlayerGameWidget *m_gameWidget;
    StoryWidget *m_matchLostAnimation;
    PuyoTwoPlayersStatsWidget *m_statsWidget;
    PlayerGameStat &m_playerStat;
};

#endif // _PUYOSINGLEPLAYERSTARTER


