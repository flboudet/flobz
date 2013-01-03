/* FloboPop
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

#ifndef _GAME_WIDGET_H
#define _GAME_WIDGET_H

#include "gameui.h"
#include "FloboGame.h"
#include "GamePlayer.h"
#include "CheatCodeManager.h"
#include "Theme.h"
#include "Story.h"


class StyrolysePainterClient
{
    public:
    StyrolysePainterClient(LevelTheme *theme);
    virtual ~StyrolysePainterClient();
    void update();
    void draw(DrawTarget *dt);
private:
    struct ExtendedClient {
        StyrolyseClient m_styroClient;
        DrawTarget *m_painter;
        LevelTheme *m_theme;
    };
    ExtendedClient m_client;
    Styrolyse *m_animation;
private:
    // Styrolyse methods
    static void *styro_loadImage(StyrolyseClient *_this, const char *path);
    static void styro_drawImage(StyrolyseClient *_this,
                                void *image, int x, int y, int w, int h,
                                int clipx, int clipy, int clipw, int cliph, int flipped, float scaleX, float scaleY, float alpha);
    static void styro_freeImage(StyrolyseClient *_this, void *image);
};


/**
 * The standard difficulty levels, affecting the speed of the game
 */
typedef enum GameDifficulty {
    EASY=0,
    MEDIUM=1,
    HARD=2
} GameDifficulty;

/**
 * This object represents all the game options needed as a parameter
 * to the GameWidget
 */
struct GameOptions
{
    GameOptions() {
        MIN_SPEED = 2;
        MAX_SPEED = 20;
        CYCLES_BEFORE_SPEED_INCREASES = 240;
    }

    static GameOptions fromDifficulty(GameDifficulty difficulty);

    int MIN_SPEED;
    int MAX_SPEED;
    int CYCLES_BEFORE_SPEED_INCREASES;
};

class VictoryDisplay {
public:
    VictoryDisplay(Vec3 position, IosSurface *trophy, int victories = 0);
    virtual ~VictoryDisplay() {}
    void draw(DrawTarget *dt);
    void setValue(int victories) { m_victories = victories; }
private:
    Vec3 m_position;
    IosSurface *m_trophy;
    int m_victories;
};

/**
 * Represents the full featured game widget, with the two players game views.
 * Handles user input.
 */
class GameWidget : public GarbageCollectableItem,
                   public gameui::Widget
{
public:
    enum {
        PAUSED_STARTPRESSED = 1,
        GAMEOVER_STARTPRESSED,
        GAME_IS_OVER,
    };
public:
    GameWidget();
    virtual ~GameWidget();
public:
    // A deplacer
    void setAssociatedScreen(GameScreen *associatedScreen) { this->associatedScreen = associatedScreen; associatedScreenHasBeenSet(associatedScreen); };
    virtual void associatedScreenHasBeenSet(GameScreen *associatedScreen) {}
    virtual void setGameOverAction(gameui::Action *gameOverAction) {
        this->gameOverAction = gameOverAction;
    }
    //
    virtual std::vector<VisualFX*> &getVisualFX()  { return m_visualFX; }
public:
    virtual void setGameOptions(GameOptions options) = 0;
    //
    virtual void pause(bool obscureScreen = true);
    virtual void resume();
    bool isPaused() const { return m_paused; }
    // A deplacer
    virtual void setScreenToPaused(bool fromControls);
    virtual void setScreenToResumed(bool fromControls);
    // Callbacks
    virtual bool backPressed()  {return false;}
    virtual bool startPressed() {return false;}
    virtual void abort() { m_abortedFlag = true; }
    virtual bool getAborted() const { return m_abortedFlag; }
    //
    virtual StoryWidget *getOpponent() = 0;
    //
    // TODO: Make this N-players generic (N from 1 to +inf)
    virtual void setPlayerOneName(String newName) = 0;
    virtual void setPlayerTwoName(String newName) = 0;
    virtual void setVictories(int left, int right) {};
    virtual PlayerGameStat &getStatPlayerOne() = 0;
    virtual PlayerGameStat &getStatPlayerTwo()  = 0;
    virtual void addGameAHandicap(int handicap) = 0;
    virtual void addGameBHandicap(int handicap) = 0;
    virtual bool isGameARunning() const = 0;
    // Widget methods
    bool isFocusable() { return !m_paused; }
protected:
    void setLevelTheme(LevelTheme *levelTheme);
    LevelTheme *getLevelTheme() const { return m_levelTheme; }
protected:
    std::auto_ptr<StyrolysePainterClient> m_styroPainter;
private:
    LevelTheme *m_levelTheme;
protected:
    gameui::Action *gameOverAction;
    GameScreen *associatedScreen;
    std::vector<VisualFX*> m_visualFX;
protected:
    bool m_paused, m_obscureScreenOnPause;
    IosSurface *m_painterGameScreen;
    bool m_abortedFlag;
};

/**
 * Represents the full featured game widget, with the two players game views.
 * Handles user input.
 */
class GameWidget2P : public GameWidget, CycledComponent
{
public:
    GameWidget2P(GameOptions options = GameOptions(), bool withGUI = true);
    virtual ~GameWidget2P();
public:
    void setGameOptions(GameOptions options);
    void initWithGUI(GameView &areaA, GameView &areaB,
                     GamePlayer &controllerA, GamePlayer &controllerB,
                     LevelTheme &levelTheme,
                     gameui::Action *gameOverAction = NULL);
    void initWithoutGUI(GameView &areaA, GameView &areaB,
                     GamePlayer &controllerA, GamePlayer &controllerB,
                     gameui::Action *gameOverAction = NULL);
    // Specific methods
    bool backPressed();
    bool startPressed();
    void setLives(int l) { lives = l; }
    void setVictories(int left, int right);
    bool isGameARunning() const { return attachedGameA->isGameRunning(); }
    bool isGameBRunning() const { return attachedGameB->isGameRunning(); }
    void setPlayerOneName(String newName);
    void setPlayerTwoName(String newName);
    String getPlayerOneName() const { return playerOneName; }
    PlayerGameStat &getStatPlayerOne() { return attachedGameA->getGameStat(); }
    PlayerGameStat &getStatPlayerTwo() { return attachedGameB->getGameStat(); }
    void setStatPlayerOne(PlayerGameStat &gameStat) { attachedGameA->setGameStat(gameStat); }
    void setStatPlayerTwo(PlayerGameStat &gameStat) { attachedGameB->setGameStat(gameStat); }
    virtual StoryWidget *getOpponent() { return NULL; }
    void addGameAHandicap(int handicap) {attachedGameA->increaseNeutralFlobos((handicap>10?10:handicap) * FLOBOBAN_DIMX); attachedGameA->dropNeutrals();}
    void addGameBHandicap(int handicap) {attachedGameB->increaseNeutralFlobos((handicap>10?10:handicap) * FLOBOBAN_DIMX); attachedGameB->dropNeutrals();}
    void addSubWidget(Widget *subWidget);
    // Display player names properties
    void setDisplayPlayerOneName(bool display) { m_displayPlayerOneName = display; }
    void setDisplayPlayerTwoName(bool display) { m_displayPlayerTwoName = display; }

    // CycledComponent methods
    void cycle();
    void eventOccured(event_manager::GameControlEvent *event);

    // Widget methods
    void draw(DrawTarget *dt);
    IdleComponent *getIdleComponent() { return this; }

    // Draw subfunctions (in order to subclass GameWidget with modified look)
    virtual void drawBackground(DrawTarget *dt);
    virtual void drawGameAreas(DrawTarget *dt);
    virtual void drawGameNeutrals(DrawTarget *dt);

    // A deplacer
    virtual void actionAfterGameOver(bool fromControls, int actionType);

protected:

    bool withGUI;
    DrawTarget &painter;
    GameView *areaA, *areaB;
    GamePlayer *controllerA, *controllerB;
    FloboGame *attachedGameA, *attachedGameB;
    int cyclesBeforeGameCycle;
    int cyclesBeforeSpeedIncreases; // time between speed increases in units of 20ms
    unsigned int tickCounts;
    unsigned long long cycles;
    bool displayLives;
    int lives;
    bool once;
    bool gameover;
    int gameSpeed; // from 0 (MinSpeed) to 20 (MaxSpeed)
    int MinSpeed,MaxSpeed; // in units of 20ms
    int blinkingPointsA, blinkingPointsB, savePointsA, savePointsB;
    String playerOneName, playerTwoName;
    std::vector<gameui::Widget *> m_subwidgets;
    bool skipGameCycleA, skipGameCycleB;
    double gameOverDate;
    bool m_displayPlayerOneName, m_displayPlayerTwoName;
    void priv_initialize();
private:
    std::auto_ptr<VictoryDisplay> m_victoryDisplayA, m_victoryDisplayB;
};

// Should be moved elsewhere
extern std::vector<VisualFX*> *activeFX;

#endif // _GAME_WIDGET_H

