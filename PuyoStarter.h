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

#ifndef _PUYOSTARTER
#define _PUYOSTARTER

class PuyoCycled;

class PuyoPauseMenu : public VBox {
public:
    PuyoPauseMenu();
private:
    Text menuTitle;
    Button continueButton;
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

class PuyoGameWidget : public Widget, CycledComponent {
public:
    PuyoGameWidget(PuyoView &areaA, PuyoView &areaB, PuyoPlayer &controllerA, PuyoPlayer &controllerB, PuyoLevelTheme &levelTheme);
    virtual ~PuyoGameWidget();
    void cycle();
    void draw(SDL_Surface *screen);
    void pause();
    void resume();
    bool isFocusable() { return !paused; }
    void eventOccured(GameControlEvent *event);
    IdleComponent *getIdleComponent() { return this; }
protected:
    SDL_Painter painter;
private:
    PuyoLevelTheme &attachedLevelTheme;
    IIM_Surface *background, *grid, *liveImage[4];
    PuyoView &areaA, &areaB;
    PuyoPlayer &controllerA, &controllerB;
    PuyoGame *attachedGameA, *attachedGameB;
    int cyclesBeforeGameCycle;
    unsigned int tickCounts;
    bool paused;
    bool displayLives;
    int lives;
};

class PuyoGameScreen : public Screen {
public:
    PuyoGameScreen(PuyoGameWidget &gameWidget, Screen &previousScreen);
    void onEvent(GameControlEvent *cevent);
    virtual void backPressed();
private:
    bool paused;
    PuyoPauseMenu pauseMenu;
    PuyoGameWidget &gameWidget;
    PuyoScreenTransitionWidget transitionWidget;
};

class PuyoTwoPlayerGameWidget : public PuyoGameWidget {
public:
    PuyoTwoPlayerGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme);
private:
    AnimatedPuyoSetTheme &attachedPuyoThemeSet;
    PuyoRandomSystem attachedRandom;
    PuyoLocalGameFactory attachedGameFactory;
    PuyoView areaA, areaB;
    PuyoEventPlayer controllerA, controllerB;
};

// Le PuyoStarter moderne
class PuyoPureStarter : public Screen {
public:
    PuyoPureStarter(PuyoCommander *commander);
    virtual ~PuyoPureStarter();
    virtual void run(int score1, int score2, int lives, int point1, int point2) = 0;
    virtual void draw() = 0;
    
    virtual void startPressed();
    virtual void backPressed();

    virtual void cycle() = 0;

    void idle(double currentTime);
    bool isLate(double currentTime) const;
    
    //virtual bool leftPlayerWin() const  { return attachedGameA->isGameRunning(); }
    //virtual bool rightPlayerWin() const { return attachedGameB->isGameRunning(); }
    
    //virtual int leftPlayerPoints() const { return attachedGameA->getPoints(); }
    //virtual int rightPlayerPoints() const { return attachedGameB->getPoints(); }
    
protected:
    PuyoCommander *commander;
    bool paused;
    bool stopRendering;
    bool gameAborted;
    
    void stopRender();
    void restartRender();

    PuyoCycled *cycled;
    PuyoPauseMenu pauseMenu;
};


class PuyoStarter : public PuyoPureStarter { // , public virtual IdleComponent {
public:
    PuyoStarter(PuyoCommander *commander, int theme);
    virtual ~PuyoStarter();
    
    virtual void run(int score1, int score2, int lives, int point1, int point2);
    void cycle();
    
    virtual void draw();
    void draw(SDL_Surface *surf);
    
    virtual bool leftPlayerWin() const  { return attachedGameA->isGameRunning(); }
    virtual bool rightPlayerWin() const { return attachedGameB->isGameRunning(); }
    
    virtual int leftPlayerPoints() const { return attachedGameA->getPoints(); }
    virtual int rightPlayerPoints() const { return attachedGameB->getPoints(); }
   
    void onEvent(GameControlEvent *event);
    
protected:
    PuyoView *areaA, *areaB;
    PuyoGame *attachedGameA, *attachedGameB;
    PuyoIA *randomPlayer;
    PuyoGameFactory *attachedGameFactory;
    PuyoGameFactory *attachedNetworkGameFactory;
    PuyoRandomSystem attachedRandom;
    int tickCounts;
    int lives;
    int score1;
    int score2;
    int gameSpeed;
    
    int blinkingPointsA, blinkingPointsB, savePointsA, savePointsB;
    AnimatedPuyoSetTheme attachedThemeManager;
    int quit;
    bool gameover;
};

#endif // _PUYOSTARTER

