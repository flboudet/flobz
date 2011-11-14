//  FloboPuyo/AIPlayer.h AI header for FloboPuyo
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

// Redefine the table size to our needs
// same columns number
#define IA_FLOBOBAN_DIMX (FLOBOBAN_DIMX)
#define IA_TABLEDIMX (FLOBOBAN_DIMX)
// only visible rows (=rows-2), + one to keep colmun heights
#define IA_FLOBOBAN_DIMY (FLOBOBAN_DIMY-2)
#define IA_TABLEDIMY (IA_FLOBOBAN_DIMY+1)

typedef struct {
  int realSuppressionValue;
  int potentialSuppressionValue;
  int criticalHeight;
  int columnScalar[IA_FLOBOBAN_DIMX];
  int rotationMethod;
  int fastDropDelta;
  int thinkDepth;
  int speedFactor;
} AIParameters;

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
  FloboState falling;
  FloboState companion;
  PuyoOrientation orientation;
  PuyoCoordinates position;
} PuyoBinom;

typedef struct {
  int puyoSuppressedPotential;
  int neutralSuppressedPotential;
  int floboGrouped;
  int puyoSuppressed;
  int neutralSuppressed;
  int height;
} GridEvaluation;

typedef unsigned char GridState[IA_TABLEDIMX][IA_TABLEDIMY];

class AIPlayer : public virtual PuyoPlayer {

  public:
    AIPlayer(int level, PuyoView &targetView);
    virtual ~AIPlayer();
    virtual void cycle();
    void setAIParameters(const AIParameters &ai);
	int getLevel() const { return this->level; }
	
  private:
    FloboState extractColor(FloboState A) const;
    PuyoOrientation extractOrientation(int D) const;
    int revertOrientation(PuyoOrientation D) const;
    void extractGrid(void);
    void decide(int partial, int depth);
    int makeEvaluation(const GridEvaluation * const referenceOne, const PuyoBinom flobos, const GridState * const grid);
    bool selectIfBetterEvaluation(int * const referenceOne, const GridEvaluation * const newOne, const PuyoBinom flobos, const GridState * const grid);

    GridState * internalGrid;
    int decisionMade;
    bool shouldRedecide;
    int lastNumberOfBadPuyos;
    int totalNumberOfBadPuyos;
    PuyoBinom objective;
    int lastLineSeen;
    int currentCycle;
    bool readyToDrop;

    AIParameters params;
	int level;
  
    PuyoBinom current, next;
    unsigned int bestl1;
    bool foundOne;
    PuyoBinom originalPuyo;
    int bestEvaluation;
  
};

#endif // PUYOIA_H

