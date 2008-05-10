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

#ifndef _PUYOSTARTER
#define _PUYOSTARTER

#include <vector>

#include "PuyoGame.h"
#include "PuyoEventPlayer.h"
#include "PuyoIA.h"
#include "PuyoCheatCodeManager.h"
#include "PuyoPauseMenu.h"
#include "PuyoCommander.h"
#include "ios_messagebox.h"
#include "AnimatedPuyoTheme.h"
#include "PuyoScreenTransition.h"
#include "PuyoStrings.h"

class PuyoLocalGameFactory : public PuyoGameFactory {
public:
    PuyoLocalGameFactory(PuyoRandomSystem *attachedRandom): attachedRandom(attachedRandom) {}
    PuyoGame *createPuyoGame(PuyoFactory *attachedPuyoFactory) {
        return new PuyoLocalGame(attachedRandom, attachedPuyoFactory);
    }
private:
    PuyoRandomSystem *attachedRandom;
};

class PuyoGameWidget;
class PuyoKillPlayerLeftAction : public Action {
public:
    PuyoKillPlayerLeftAction(PuyoGameWidget &target) : target(target) {}
    void action();
private:
    PuyoGameWidget &target;
};

class PuyoKillPlayerRightAction : public Action {
public:
    PuyoKillPlayerRightAction(PuyoGameWidget &target) : target(target) {}
    void action();
private:
    PuyoGameWidget &target;
};

class PuyoGameScreen;
struct GameOptions;

struct GameOptions
{
    GameOptions() {
        MIN_SPEED = 2;
        MAX_SPEED = 20;
        CYCLES_BEFORE_SPEED_INCREASES = 240;
    }

    static GameOptions FromLevel(int level);

    int MIN_SPEED;
    int MAX_SPEED;
    int CYCLES_BEFORE_SPEED_INCREASES;
};

class PuyoGameWidget : public GarbageCollectableItem, public Widget, CycledComponent {
public:
    PuyoGameWidget(GameOptions options = GameOptions());
    void setGameOptions(GameOptions options);
    virtual ~PuyoGameWidget();
    void initialize(PuyoView &areaA, PuyoView &areaB, PuyoPlayer &controllerA, PuyoPlayer &controllerB, PuyoLevelTheme &levelTheme, Action *gameOverAction = NULL);
    void initialize();
    void cycle();
    void draw(SDL_Surface *screen);
    void pause();
    void resume();
    bool isFocusable() { return !paused; }
    void eventOccured(GameControlEvent *event);
    bool backPressed();
    bool startPressed();
    IdleComponent *getIdleComponent() { return this; }
    virtual void abort() { abortedFlag = true; }
    bool getAborted() const { return abortedFlag; }
    void setLives(int l) { lives = l; }
    bool isGameARunning() const { return attachedGameA->isGameRunning(); }
    bool isGameBRunning() const { return attachedGameB->isGameRunning(); }
    void setPlayerOneName(String newName) { playerOneName = newName; }
    void setPlayerTwoName(String newName) { playerTwoName = newName; }
    String getPlayerOneName() const { return playerOneName; }
    PlayerGameStat getStatPlayerOne() { return attachedGameA->getGameStat(); }
    virtual PuyoStoryWidget *getOpponent() { return NULL; }
    virtual std::vector<PuyoFX*> &getPuyoFX() { return puyoFX; }
    void addGameAHandicap(int handicap) {attachedGameA->increaseNeutralPuyos(handicap * PUYODIMX); attachedGameA->dropNeutrals();}
    void addGameBHandicap(int handicap) {attachedGameB->increaseNeutralPuyos(handicap * PUYODIMX); attachedGameB->dropNeutrals();}
    // A deplacer
    void setAssociatedScreen(PuyoGameScreen *associatedScreen) { this->associatedScreen = associatedScreen; associatedScreenHasBeenSet(associatedScreen); };
    virtual void setScreenToPaused(bool fromControls);
    virtual void setScreenToResumed(bool fromControls);
    virtual void actionAfterGameOver(bool fromControls);
protected:
    virtual void associatedScreenHasBeenSet(PuyoGameScreen *associatedScreen) {}

    // Styrolyse methods
    static void *styro_loadImage(StyrolyseClient *_this, const char *path);
    static void styro_drawImage(StyrolyseClient *_this,
				void *image, int x, int y,
				int clipx, int clipy, int clipw, int cliph);
    static void styro_freeImage(StyrolyseClient *_this, void *image);

    PuyoGameScreen *associatedScreen;
    SDL_Painter painter;
    PuyoLevelTheme *attachedLevelTheme;
    PuyoView *areaA, *areaB;
    PuyoPlayer *controllerA, *controllerB;
    PuyoGame *attachedGameA, *attachedGameB;
    int cyclesBeforeGameCycle;
    int cyclesBeforeSpeedIncreases; // time between speed increases in units of 20ms
    unsigned int tickCounts;
    unsigned long long cycles;
    bool paused;
    bool displayLives;
    int lives;
    bool once;
    Action *gameOverAction;
    bool gameover;
    bool abortedFlag;
    int gameSpeed; // from 0 (MinSpeed) to 20 (MaxSpeed)
    int MinSpeed,MaxSpeed; // in units of 20ms
    int blinkingPointsA, blinkingPointsB, savePointsA, savePointsB;
    String playerOneName, playerTwoName;
    PuyoKillPlayerLeftAction killLeftAction;
    PuyoKillPlayerRightAction killRightAction;
    PuyoCheatCodeManager killLeftCheat, killRightCheat;
    std::vector<PuyoFX*> puyoFX;
    bool skipGameCycleA, skipGameCycleB;
    double gameOverDate;
    // Foreground animation
    struct StyrolysePainterClient {
      StyrolyseClient m_styroClient;
      SDL_Painter *m_painter;
      PuyoLevelTheme *m_theme;
    };
    Styrolyse *m_foregroundAnimation;
    StyrolysePainterClient m_styroPainter;
};

class PuyoGameScreen : public Screen, public Action {
public:
    PuyoGameScreen(PuyoGameWidget &gameWidget, Screen &previousScreen);
    ~PuyoGameScreen();
    void onEvent(GameControlEvent *cevent);
    virtual bool backPressed();
    virtual bool startPressed();
    virtual void abort();
    void setOverlayStory(PuyoStoryWidget *story);
    /**
     * Sets the game to paused
     * @param fromControls true if the action leading to the pause comes
     *                     in response to a local input control. In this case,
     *                     the action will get forwarded to the opponent in
     *                     a network game. Otherwise it won't.
     */
    virtual void setPaused(bool fromControls);
    /**
     * Sets the game to resumed
     * @param fromControls true if the action leading to the pause comes
     *                     in response to a local input control. In this case,
     *                     the action will get forwarded to the opponent in
     *                     a network game. Otherwise it won't.
     */
    virtual void setResumed(bool fromControls);
    /** Returns the ingame pause menu widget
     */
    PuyoPauseMenu & getPauseMenu() { return pauseMenu; }
    /** Notification on screen visibility change
     * @param visible  true if the scren is visible, otherwise false
     */
    virtual void onScreenVisibleChanged(bool visible);
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType, GameControlEvent *event);
private:
    bool paused;
    PuyoPauseMenu pauseMenu;
    PuyoGameWidget &gameWidget;
    PuyoScreenTransitionWidget transitionWidget;
    PuyoStoryWidget *overlayStory;
};

class PuyoTwoPlayerGameWidget : public PuyoGameWidget {
public:
    PuyoTwoPlayerGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, Action *gameOverAction = NULL);
private:
    AnimatedPuyoSetTheme &attachedPuyoThemeSet;
    PuyoRandomSystem attachedRandom;
    PuyoLocalGameFactory attachedGameFactory;
    PuyoView areaA, areaB;
    PuyoEventPlayer controllerA, controllerB;
};

#endif // _PUYOSTARTER

