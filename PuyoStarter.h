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

#ifndef _PUYOSTARTER
#define _PUYOSTARTER

class PuyoCycled;

// Le PuyoStarter moderne
class PuyoPureStarter : public Screen { //DrawableComponent {
public:
    PuyoPureStarter(PuyoCommander *commander);
    virtual ~PuyoPureStarter();
    virtual void run(int score1, int score2, int lives, int point1, int point2) = 0;
    virtual void draw() = 0;
    virtual void handleEvent(SDL_Event &event);
    
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
};


class PuyoStarter : public PuyoPureStarter { // , public virtual IdleComponent {
public:
    PuyoStarter(PuyoCommander *commander, int theme);
    virtual ~PuyoStarter();
    
    virtual void run(int score1, int score2, int lives, int point1, int point2);
    void cycle();
    
    virtual void draw();
    void draw(SDL_Surface *surf) const;
    
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
    AnimatedPuyoThemeManager attachedThemeManager;
    int quit;
    bool gameover;
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

#endif // _PUYOSTARTER

