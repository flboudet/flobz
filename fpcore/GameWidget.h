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
        StyrolyseClient _styroClient;
        DrawTarget *_painter;
        LevelTheme *_theme;
    };
    ExtendedClient _client;
    Styrolyse *_animation;
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
        _MIN_SPEED = 2;
        _MAX_SPEED = 20;
        _CYCLES_BEFORE_SPEED_INCREASES = 240;
    }

    static GameOptions fromDifficulty(GameDifficulty difficulty);

    int _MIN_SPEED;
    int _MAX_SPEED;
    int _CYCLES_BEFORE_SPEED_INCREASES;
};

class VictoryDisplay {
public:
    VictoryDisplay(Vec3 position, IosSurface *trophy, int victories = 0);
    virtual ~VictoryDisplay() {}
    void draw(DrawTarget *dt);
    void setValue(int victories) { _victories = victories; }
private:
    Vec3 _position;
    IosSurface *_trophy;
    int _victories;
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
    void setAssociatedScreen(GameScreen *associatedScreen) { this->_associatedScreen = associatedScreen; associatedScreenHasBeenSet(associatedScreen); };
    virtual void associatedScreenHasBeenSet(GameScreen *associatedScreen) {}
    virtual void setGameOverAction(gameui::Action *gameOverAction) {
        this->_gameOverAction = gameOverAction;
    }
    //
    virtual std::vector<VisualFX*> &getVisualFX()  { return _visualFX; }
public:
    virtual void setGameOptions(GameOptions options) = 0;
    //
    virtual void pause(bool obscureScreen = true);
    virtual void resume();
    bool isPaused() const { return _paused; }
    // A deplacer
    virtual void setScreenToPaused(bool fromControls);
    virtual void setScreenToResumed(bool fromControls);
    // Callbacks
    virtual bool backPressed()  {return false;}
    virtual bool startPressed() {return false;}
    virtual void abort() { _abortedFlag = true; }
    virtual bool getAborted() const { return _abortedFlag; }
    //
    virtual StoryWidget *getOpponent() = 0;
    //
    // TODO: Make this N-players generic (N from 1 to +inf)
    virtual void setPlayerOneName(const std::string & newName) = 0;
    virtual void setPlayerTwoName(const std::string & newName) = 0;
    virtual void setVictories(int left, int right) {};
    virtual PlayerGameStat &getStatPlayerOne() = 0;
    virtual PlayerGameStat &getStatPlayerTwo()  = 0;
    virtual void addGameAHandicap(int handicap) = 0;
    virtual void addGameBHandicap(int handicap) = 0;
    virtual bool isGameARunning() const = 0;
    // Widget methods
    bool isFocusable() const { return !_paused; }
protected:
    void setLevelTheme(LevelTheme *levelTheme);
    LevelTheme *getLevelTheme() const { return _levelTheme; }
protected:
    std::unique_ptr<StyrolysePainterClient> _styroPainter;
private:
    LevelTheme *_levelTheme;
protected:
    gameui::Action *_gameOverAction;
    GameScreen *_associatedScreen;
    std::vector<VisualFX*> _visualFX;
protected:
    bool _paused, _obscureScreenOnPause;
    IosSurface *_painterGameScreen;
    bool _abortedFlag;
};

/**
 * Represents the full featured game widget, with the two players game views.
 * Handles user input.
 */
class GameWidget2P : public GameWidget, CycledComponent
{
public:
    GameWidget2P(GameOptions options = GameOptions());
    virtual ~GameWidget2P();
public:
    void setGameOptions(GameOptions options);
    void initWithGUI(GameView &areaA, GameView &areaB,
                     LevelTheme &levelTheme,
                     gameui::Action *gameOverAction = NULL);
    // Specific methods
    bool backPressed();
    bool startPressed();
    void setLives(int l) { _lives = l; }
    void setVictories(int left, int right);
    bool isGameARunning() const { return _attachedGameA->isGameRunning(); }
    bool isGameBRunning() const { return _attachedGameB->isGameRunning(); }
    void setPlayerOneName(const std::string & newName);
    void setPlayerTwoName(const std::string & newName);
    const std::string &getPlayerOneName() const { return _playerOneName; }
    PlayerGameStat &getStatPlayerOne() { return _attachedGameA->getGameStat(); }
    PlayerGameStat &getStatPlayerTwo() { return _attachedGameB->getGameStat(); }
    void setStatPlayerOne(PlayerGameStat &gameStat) { _attachedGameA->setGameStat(gameStat); }
    void setStatPlayerTwo(PlayerGameStat &gameStat) { _attachedGameB->setGameStat(gameStat); }
    virtual StoryWidget *getOpponent() { return NULL; }
    void addGameAHandicap(int handicap) {_attachedGameA->increaseNeutralFlobos((handicap>10?10:handicap) * FLOBOBAN_DIMX); _attachedGameA->dropNeutrals();}
    void addGameBHandicap(int handicap) {_attachedGameB->increaseNeutralFlobos((handicap>10?10:handicap) * FLOBOBAN_DIMX); _attachedGameB->dropNeutrals();}
    void addSubWidget(Widget *subWidget);
    // Display player names properties
    void setDisplayPlayerOneName(bool display) { _displayPlayerOneName = display; }
    void setDisplayPlayerTwoName(bool display) { _displayPlayerTwoName = display; }

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
    std::unique_ptr<GamePlayer> _controllerA, _controllerB;
protected:
    DrawTarget &_painter;
    GameView *_areaA, *_areaB;
    FloboGame *_attachedGameA, *_attachedGameB;
    int _cyclesBeforeGameCycle;
    int _cyclesBeforeSpeedIncreases; // time between speed increases in units of 20ms
    unsigned int _tickCounts;
    unsigned long long _cycles;
    bool _displayLives;
    int _lives;
    bool _once;
    bool _gameover;
    int _gameSpeed; // from 0 (MinSpeed) to 20 (MaxSpeed)
    int _MinSpeed,_MaxSpeed; // in units of 20ms
    int _blinkingPointsA, _blinkingPointsB, _savePointsA, _savePointsB;
    std::string _playerOneName, _playerTwoName;
    std::vector<gameui::Widget *> _subwidgets;
    bool _skipGameCycleA, _skipGameCycleB;
    double _gameOverDate;
    bool _displayPlayerOneName, _displayPlayerTwoName;
    void priv_initialize();
private:
    std::unique_ptr<VictoryDisplay> _victoryDisplayA, _victoryDisplayB;
};

// Should be moved elsewhere
extern std::vector<VisualFX*> *activeFX;

#endif // _GAME_WIDGET_H

