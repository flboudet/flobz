//  FloboPuyo/PuyoIA.h AI header for FloboPuyo
//  Copyright (C) 2007 Guillaume Borios <gyom@ios-software.com>
//
//  iOS-Software <http://www.ios-software.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef PUYOIA_H
#define PUYOIA_H

#include "PuyoPlayer.h"

enum IA_Type {
  RANDOM,
  FLOBO,
  JEKO,
  TANIA,
  GYOM
};

#define IA_PUYODIMX (PUYODIMX)
#define IA_PUYODIMY (PUYODIMY-2)

typedef enum {
  Left = 0,
  Above = 1,
  Below = 2,
  Right = 3
} PuyoOrientation;

typedef struct {
  unsigned char x;
  unsigned char y;
} PuyoCoordinates;


typedef struct {
  PuyoState falling;
  PuyoState companion;
  PuyoOrientation orientation;
  PuyoCoordinates position;
} PuyoBinom;

typedef struct {
  int puyoSuppressedPotential;
  int neutralSuppressedPotential;
  int puyoGrouped;
  int puyoSuppressed;
  int neutralSuppressed;
  int height;
} GridEvaluation;

typedef unsigned char GridState[IA_PUYODIMX][IA_PUYODIMY];

class PuyoIA : public virtual PuyoPlayer {

  public:
    PuyoIA(IA_Type type, int level, PuyoView &targetView);
    ~PuyoIA();
    virtual void cycle();

  private:
    PuyoState extractColor(PuyoState A) const;
    PuyoOrientation extractOrientation(int D) const;
    void extractGrid(void);
    void decide();
    int makeEvaluation(const GridEvaluation * const referenceOne, const PuyoBinom puyos);
    bool selectIfBetterEvaluation(int * const referenceOne, const GridEvaluation * const newOne, const PuyoBinom puyos);

    GridState * internalGrid;
    bool decisionMade;
    IA_Type type;
    int level;
    PuyoBinom objective;

    void * special;
};

#endif // PUYOIA_H

