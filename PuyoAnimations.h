/* FloboPuyo
 * Copyright (C) 2004
 *   Florent Boudet        <flobo@ios-software.com>,
 *   Jean-Christophe Hoelt <jeko@ios-software.com>,
 *   Guillaume Borios      <gborios@ios-software.com>
 *
 * iOS Software <http://ios.free.fr>
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

#ifndef _PUYOANIMATIONS
#define _PUYOANIMATIONS

#include <stdlib.h>
#include <math.h>
#include "glSDL.h"
#include "IosImgProcess.h"
#include "PuyoGame.h"

/* Abstract Animation class */
class PuyoAnimation {
  public:
    PuyoAnimation();
    bool isFinished();
    virtual void cycle() = 0;
    virtual void draw(int semiMove) = 0;
  protected:
    bool finishedFlag;
};

/* Neutral falling animation */
class NeutralAnimation : public PuyoAnimation {
  public:
    NeutralAnimation(int X, int Y, int xOffset, int yOffset);
    void cycle();
    void draw(int semiMove);
  private:
    static IIM_Surface *neutral;
    int X, Y, currentY;
};

/* Companion turning around main puyo animation */
class TurningAnimation : public PuyoAnimation {
public:
    TurningAnimation(PuyoPuyo *companionPuyo,
                     int vector, int xOffset, int yOffset,
                     IIM_Surface *companionSurface,
                     bool counterclockwise);
    void cycle();
    void draw(int semiMove);
private:
    PuyoPuyo *companionPuyo;
    int xOffset, yOffset;
    int X, Y, companionVector, cpt;
    float angle;
    float step;
    IIM_Surface *targetSurface;
    bool counterclockwise;
};

/* Puyo falling and bouncing animation */
class FallingAnimation : public PuyoAnimation {
public:
    FallingAnimation(PuyoPuyo *puyo,
                     int originY, int xOffset, int yOffset, int step);
    void cycle();
    void draw(int semiMove);
private:
        PuyoPuyo *attachedPuyo;
    int xOffset, yOffset, step;
    int X, Y;
	int bouncing;
    IIM_Surface *puyoFace;
    static const int BOUNCING_OFFSET_NUM;
    static const int BOUNCING_OFFSET[];
};

/* Puyo exploding and vanishing animation */
class VanishAnimation : public PuyoAnimation {
public:
    VanishAnimation(PuyoPuyo *puyo, int xOffset, int yOffset);
    void cycle();
    void draw(int semiMove);
private:
    IIM_Surface *puyoFace;
    int xOffset, yOffset;
    int X, Y, iter, color;
};

#endif // _PUYOANIMATIONS

