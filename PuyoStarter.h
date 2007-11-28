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

#include "PuyoGame.h"
#include "PuyoIA.h"
#include "PuyoCommander.h"
#include "ios_messagebox.h"
#include "AnimatedPuyoTheme.h"
#include "PuyoScreenTransition.h"
#include "PuyoOptionMenu.h"
#include "PuyoStrings.h"

#ifndef _PUYOSTARTER
#define _PUYOSTARTER

#include <vector>

class PuyoCycled;

class PuyoPauseMenu : public VBox {
public:
    PuyoPauseMenu(Action *continueAction, Action *abortAction);
    virtual ~PuyoPauseMenu();
    virtual void finishLayout();
    int pauseMenuTop, pauseMenuLeft;
    void toggleSoundFx();
    void toggleMusic();
    void toggleFullScreen();
private:
    IIM_Surface   *menuBG;
    SliderContainer pauseContainer;
    VBox pauseVBox;
    Text menuTitle;
    Button continueButton;
    ToggleSoundFxAction toggleSoundFxAction;
    ToggleMusicAction toggleMusicAction;
    ToggleFullScreenAction toggleFullScreenAction;
    ToggleButton audioButton, musicButton, fullScreenButton;
    Button abortButton;
};

class PuyoLocalGameFactory : public PuyoGameFactory {
public:
    PuyoLocalGameFactory(PuyoRandomSystem *attachedRandom): attachedRandom(attachedRandom) {}
    PuyoGame *createPuyoGame(PuyoFactory *attachedPuyoFactory) {
        return new PuyoLocalGame(attachedRandom, attachedPuyoFactory);
    }
private:
    PuyoRandomSystem *attachedRandom;
};

class PuyoEventPlayer : public PuyoPlayer {
public:
    PuyoEventPlayer(PuyoView &view, int downEvent, int leftEvent, int rightEvent, int turnLeftEvent, int turnRightEvent);
    void eventOccured(GameControlEvent *event);
    void cycle();
private:
    bool keyShouldRepeat(int &key);
    const int downEvent, leftEvent, rightEvent, turnLeftEvent, turnRightEvent;
    int fpKey_Down, fpKey_Left, fpKey_Right, fpKey_TurnLeft, fpKey_TurnRight;
    int fpKey_Repeat, fpKey_Delay;
};

class PuyoCheatCodeManager {
public:
    PuyoCheatCodeManager(String cheatCode, Action *cheatAction) : cheatCode(cheatCode), cheatAction(cheatAction), cheatCodeLength(cheatCode.length()), currentPosition(0) {}
    void eventOccured(GameControlEvent *event);
private:
    String cheatCode;
    Action *cheatAction;
    int cheatCodeLength;
    int currentPosition;
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

class PuyoGameWidget : public GarbageCollectableItem, public Widget, CycledComponent {
public:
    PuyoGameWidget(PuyoView &areaA, PuyoView &areaB, PuyoPlayer &controllerA, PuyoPlayer &controllerB, PuyoLevelTheme &levelTheme, Action *gameOverAction = NULL);
    PuyoGameWidget();
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
    IdleComponent *getIdleComponent() { return this; }
    virtual void abort() { abortedFlag = true; }
    bool getAborted() const { return abortedFlag; }
    void setLives(int l) { lives = l; }
    bool isGameARunning() const { return attachedGameA->isGameRunning(); }
    bool isGameBRunning() const { return attachedGameB->isGameRunning(); }
    void setPlayerOneName(String newName) { playerOneName = newName; }
    void setPlayerTwoName(String newName) { playerTwoName = newName; }
    String getPlayerOneName() const { return playerOneName; }
    int getPointsPlayerOne() { return attachedGameA->getPoints(); }
    virtual PuyoStoryWidget *getOpponentFace() { return NULL; }
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
};

class ContinueAction : public Action {
public:
    ContinueAction(PuyoGameScreen &screen) : screen(screen) {}
    void action();
private:
    PuyoGameScreen &screen;
};

class AbortAction : public Action {
public:
    AbortAction(PuyoGameScreen &screen) : screen(screen) {}
    void action();
private:
    PuyoGameScreen &screen;
};

class PuyoGameScreen : public Screen {
public:
    PuyoGameScreen(PuyoGameWidget &gameWidget, Screen &previousScreen);
    ~PuyoGameScreen();
    void onEvent(GameControlEvent *cevent);
    virtual bool backPressed();
    virtual void abort();
    void setOverlayStory(PuyoStoryWidget *story);
    virtual void setPaused(bool fromControls);
    virtual void setResumed(bool fromControls);
    PuyoPauseMenu & getPauseMenu() { return pauseMenu; }
    /* Notification on screen visibility change
     * @param visible  true if the scren is visible, otherwise false
     */
    virtual void onScreenVisibleChanged(bool visible);
private:
    bool paused;
    ContinueAction continueAction;
    AbortAction abortAction;
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

