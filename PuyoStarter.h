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

// Le PuyoStarter moderne
class PuyoPureStarter : public PuyoDrawable {
public:
    PuyoPureStarter(PuyoCommander *commander);
    virtual ~PuyoPureStarter();
    virtual void run(int score1, int score2, int lives, int point1, int point2) = 0;
    virtual void draw() = 0;
    virtual void handleEvent(SDL_Event &event);
    
    virtual void startPressed();
    virtual void backPressed();
    
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
};


class PuyoStarter : public PuyoPureStarter {
public:
    PuyoStarter(PuyoCommander *commander, bool aiLeft, int aiLevel, IA_Type aiType, int theme, ios_fc::MessageBox *mbox = NULL);
    virtual ~PuyoStarter();
    
    virtual void run(int score1, int score2, int lives, int point1, int point2);
    virtual void draw();
    
    virtual bool leftPlayerWin() const  { return attachedGameA->isGameRunning(); }
    virtual bool rightPlayerWin() const { return attachedGameB->isGameRunning(); }
    
    virtual int leftPlayerPoints() const { return attachedGameA->getPoints(); }
    virtual int rightPlayerPoints() const { return attachedGameB->getPoints(); }
    
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
    ios_fc::MessageBox *mbox;
    AnimatedPuyoThemeManager attachedThemeManager;
};

#endif // _PUYOSTARTER

