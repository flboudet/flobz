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

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include "PuyoIA.h"

typedef struct {
  int realSuppressionValue;
  int potentialSuppressionValue;
  int criticalHeight;
  int columnScalar[IA_PUYODIMX];
} AIParameters;

static const PuyoCoordinates nullPosition = {0,0};
static const PuyoBinom nullBinom = {PUYO_EMPTY,PUYO_EMPTY,Left,nullPosition};
static const GridEvaluation nullEvaluation = {0,0,0,0,0,0};

#define copyGrid(dst,src) memcpy((void *)dst, (void *)src, sizeof(GridState))
#define resetGrid(dst) memset((void *)dst, PUYO_EMPTY, sizeof(GridState))
#define zeroGrid(dst) memset((void *)dst, 0, sizeof(GridState))

inline int columnHeight(const unsigned int x, const GridState * const grid)
{
  // WARNING: works only if the column has no holes in it
  if ((*grid)[x][0] != PUYO_EMPTY)
  {
    int y = IA_PUYODIMY-1;
    while ((*grid)[x][y] == PUYO_EMPTY) y--;
    return y+1;
  }
  else return 0;
}

inline int stripedColumnHeight(const unsigned int x, const GridState * const grid)
{
  if ((*grid)[x][0] != PUYO_EMPTY)
  {
    int y = IA_PUYODIMY-1;
    while ((y>0) && ((*grid)[x][y] == PUYO_EMPTY || (*grid)[x][y] == PUYO_NEUTRAL)) y--;
    return y+1;
  }
  else return 0;
}

bool removeSamePuyoAround(int X, int Y, const PuyoState color, GridState * const tab, GridEvaluation * const evaluation)
{
  GridState marked;
  unsigned char mx[IA_PUYODIMY*IA_PUYODIMX];
  unsigned char my[IA_PUYODIMX*IA_PUYODIMY];
  int  nFound = 1;

  mx[0] = X;
  my[0] = Y;

  zeroGrid(&marked);
  marked[X][Y] = 1;

  for (int i=0; i<nFound; i++) {
#ifdef SOLO
    if (nFound >= IA_PUYODIMX*IA_PUYODIMY)
    {
      fprintf(stderr,"ERROR 1 in remove routine, nFound=%d\n", nFound);
      exit(0);
    }
#endif

    X = mx[i];
    Y = my[i];

#ifdef SOLO
    if (Y>=IA_PUYODIMY || Y<0 || X >=IA_PUYODIMX || X <0)
    {
      fprintf(stderr,"ERROR 2 in remove routine, X=%d, Y=%d\n", X, Y);
      exit(0);
    }
#endif

    if (Y+1<IA_PUYODIMY) {
      if (marked[X][Y+1] == 0) {
        if ((*tab)[X][Y+1] == color) {
          mx[nFound] = X;
          my[nFound] = Y+1;
          marked[X][Y+1] = 1;
          nFound++;
        } else marked[X][Y+1] = 2;
      }
    }
    if (Y>0) {
      if (marked[X][Y-1] == 0) {
        if ((*tab)[X][Y-1] == color) {
          mx[nFound] = X;
          my[nFound] = Y-1;
          marked[X][Y-1] = 1;
          nFound++;
        } else marked[X][Y-1] = 2;
      }
    }
    if (X+1<IA_PUYODIMX) {
      if (marked[X+1][Y] == 0) {
        if ((*tab)[X+1][Y] == color) {
          mx[nFound] = X+1;
          my[nFound] = Y;
          marked[X+1][Y] = 1;
          nFound++;
        } else marked[X+1][Y] = 2;
      }
    }
    if (X>0) {
      if (marked[X-1][Y] == 0) {
        if ((*tab)[X-1][Y] == color) {
          mx[nFound] = X-1;
          my[nFound] = Y;
          marked[X-1][Y] = 1;
          nFound++;
        } else marked[X-1][Y] = 2;
      }
    }
  }


  if (nFound < 4)
  {
    return false;
  }

  evaluation->puyoSuppressed += nFound;

  for (int i = 0; i < nFound; i++)
  {
    X = mx[i];
    Y = my[i];
#ifdef SOLO
    if (Y>=IA_PUYODIMY || Y<0 || X >=IA_PUYODIMX || X <0)
    {
      fprintf(stderr,"ERROR 3 in remove routine, X=%d, Y=%d\n", X, Y);
      exit(0);
    }
#endif
    (*tab)[X][Y] = PUYO_EMPTY;

    if ((Y+1<IA_PUYODIMY) && ((*tab)[X][Y+1] == PUYO_NEUTRAL)) {
        (*tab)[X][Y+1] = PUYO_EMPTY;
        evaluation->neutralSuppressed++;
    }
    if ((Y>0) && ((*tab)[X][Y-1] == PUYO_NEUTRAL)) {
        (*tab)[X][Y-1] = PUYO_EMPTY;
        evaluation->neutralSuppressed++;
    }
    if ((X+1<IA_PUYODIMX) && ((*tab)[X+1][Y] == PUYO_NEUTRAL)) {
        (*tab)[X+1][Y] = PUYO_EMPTY;
        evaluation->neutralSuppressed++;
    }
    if ((X>0) && ((*tab)[X-1][Y] == PUYO_NEUTRAL)) {
        (*tab)[X-1][Y] = PUYO_EMPTY;
        evaluation->neutralSuppressed++;
    }
  }

  return true;
}


bool countSamePuyoAround(int X, int Y, const PuyoState color, GridState * const tab, GridEvaluation * const evaluation)
{
  GridState marked;
  unsigned char mx[IA_PUYODIMY*IA_PUYODIMX];
  unsigned char my[IA_PUYODIMX*IA_PUYODIMY];
  int  nFound = 1;

  mx[0] = X;
  my[0] = Y;

  zeroGrid(&marked);
  marked[X][Y] = 1;

  for (int i=0; i<nFound; i++) {
#ifdef SOLO
    if (nFound >= IA_PUYODIMX*IA_PUYODIMY)
    {
      fprintf(stderr,"ERROR 1 in remove routine, nFound=%d\n", nFound);
      exit(0);
    }
#endif

    X = mx[i];
    Y = my[i];

#ifdef SOLO
    if (Y>=IA_PUYODIMY || Y<0 || X >=IA_PUYODIMX || X <0)
    {
      fprintf(stderr,"ERROR 2 in remove routine, X=%d, Y=%d\n", X, Y);
      exit(0);
    }
#endif

    if (Y+1<IA_PUYODIMY) {
      if (marked[X][Y+1] == 0) {
        if ((*tab)[X][Y+1] == color) {
          mx[nFound] = X;
          my[nFound] = Y+1;
          marked[X][Y+1] = 1;
          nFound++;
        } else marked[X][Y+1] = 2;
      }
    }
    if (Y>0) {
      if (marked[X][Y-1] == 0) {
        if ((*tab)[X][Y-1] == color) {
          mx[nFound] = X;
          my[nFound] = Y-1;
          marked[X][Y-1] = 1;
          nFound++;
        } else marked[X][Y-1] = 2;
      }
    }
    if (X+1<IA_PUYODIMX) {
      if (marked[X+1][Y] == 0) {
        if ((*tab)[X+1][Y] == color) {
          mx[nFound] = X+1;
          my[nFound] = Y;
          marked[X+1][Y] = 1;
          nFound++;
        } else marked[X+1][Y] = 2;
      }
    }
    if (X>0) {
      if (marked[X-1][Y] == 0) {
        if ((*tab)[X-1][Y] == color) {
          mx[nFound] = X-1;
          my[nFound] = Y;
          marked[X-1][Y] = 1;
          nFound++;
        } else marked[X-1][Y] = 2;
      }
    }
  }


  if (nFound < 2)
  {
    return false;
  }

  evaluation->puyoSuppressed += nFound;

  for (int i = 0; i < nFound; i++)
  {
    X = mx[i];
    Y = my[i];
#ifdef SOLO
    if (Y>=IA_PUYODIMY || Y<0 || X >=IA_PUYODIMX || X <0)
    {
      fprintf(stderr,"ERROR 3 in remove routine, X=%d, Y=%d\n", X, Y);
      exit(0);
    }
#endif
    (*tab)[X][Y] = PUYO_EMPTY;

    if ((Y+1<IA_PUYODIMY) && ((*tab)[X][Y+1] == PUYO_NEUTRAL)) {
        (*tab)[X][Y+1] = PUYO_EMPTY;
        evaluation->neutralSuppressed++;
    }
    if ((Y>0) && ((*tab)[X][Y-1] == PUYO_NEUTRAL)) {
        (*tab)[X][Y-1] = PUYO_EMPTY;
        evaluation->neutralSuppressed++;
    }
    if ((X+1<IA_PUYODIMX) && ((*tab)[X+1][Y] == PUYO_NEUTRAL)) {
        (*tab)[X+1][Y] = PUYO_EMPTY;
        evaluation->neutralSuppressed++;
    }
    if ((X>0) && ((*tab)[X-1][Y] == PUYO_NEUTRAL)) {
        (*tab)[X-1][Y] = PUYO_EMPTY;
        evaluation->neutralSuppressed++;
    }
  }

  return true;
}


inline void columnCompress(const unsigned int x, GridState * const grid)
{
    int ydst = 0, ysrc = 0;
    int last = IA_PUYODIMY;

	// on oublie les cases vides en haut de colonne
    while (((*grid)[x][last-1] == PUYO_EMPTY) && (last > 0)) last--;

    while (ydst < last) // Tant qu'on a pas rempli dst
    {
        if ( ysrc < last ) // Si on n'a pas épuisé src
        {
            PuyoState state = (PuyoState)(*grid)[x][ysrc];
            if (state != PUYO_EMPTY) // Si src n'est pas vide
            {
                // On recopie et on passe au dst suivant
                if (ysrc != ydst)
                {
#ifdef SOLO
    if (ydst >=IA_PUYODIMY || ydst <0)
    {
      fprintf(stderr,"ERROR 4 in compress routine, ydst=%d\n", ydst);
      exit(0);
    }
#endif
                    (*grid)[x][ydst] = state;
                }
                ydst++;
            }
            // src suivant
            ysrc++;
        }
        else // Fin du remplissage avec PUYO_EMPTY
        {
#ifdef SOLO
    if (ydst >=IA_PUYODIMY || ydst <0)
    {
      fprintf(stderr,"ERROR 5 in compress routine, ydst=%d\n", ydst);
      exit(0);
    }
#endif
            (*grid)[x][ydst] = PUYO_EMPTY;
            ydst++;
        }
    }
}


bool dropPuyos(const PuyoBinom binom, GridState * const grid)
{
  int x, h, g;

  x = binom.position.x;
  h = columnHeight(x, grid);
  switch (binom.orientation)
  {
  case Above:
    if (h + 1 >= IA_PUYODIMY) return false;
#ifdef SOLO
    if (x >=IA_PUYODIMX || x <0)
    {
      fprintf(stderr,"ERROR 6 in drop routine, x=%d\n", x);
      exit(0);
    }
#endif
    (*grid)[x][h]   = binom.falling;
    (*grid)[x][h+1] = binom.companion;
    break;

  case Below:
    if (h + 1 >= IA_PUYODIMY) return false;
#ifdef SOLO
    if (x >=IA_PUYODIMX || x <0)
    {
      fprintf(stderr,"ERROR 7 in drop routine, x=%d\n", x);
      exit(0);
    }
#endif
    (*grid)[x][h+1] = binom.falling;
    (*grid)[x][h]   = binom.companion;
    break;

  case Left:
    if (h >= IA_PUYODIMY) return false;
    g = columnHeight(x-1, grid);
    if (g >= IA_PUYODIMY) return false;
#ifdef SOLO
    if (x >=IA_PUYODIMX || x-1 <0)
    {
      fprintf(stderr,"ERROR 6 in drop routine, x=%d\n", x);
      exit(0);
    }
#endif
    (*grid)[x][h]   = binom.falling;
    (*grid)[x-1][g] = binom.companion;
    break;

  case Right:
#ifdef SOLO
    if (x +1 >=IA_PUYODIMX || x <0)
    {
      fprintf(stderr,"ERROR 6 in drop routine, x=%d\n", x);
      exit(0);
    }
#endif
    if (h >= IA_PUYODIMY) return false;
    g = columnHeight(x+1, grid);
    if (g >= IA_PUYODIMY) return false;
    (*grid)[x][h]   = binom.falling;
    (*grid)[x+1][g] = binom.companion;
    break;
  }

  return true;
}


// Those combinations can place the binoms everywhere on x
// except x=0 with companion left and x=IA_PUYODIMX-1 with companion right
#define MAXCOMBINATION (IA_PUYODIMX*4)-2
inline void serialPosition(unsigned int serialnr /* from 1 to MAXCOMBINATION */, PuyoBinom * binom)
{
  binom->orientation = (PuyoOrientation)(serialnr % 4);
  binom->position.x = serialnr / 4;
}


bool suppressGroups(GridState * const dst, GridEvaluation * const evaluation)
{
  bool didSomething = false;
  // for each position, try to remove a group
  for (unsigned int x = 0;  x < IA_PUYODIMX; x++)
  {
    for (unsigned int y = 0; y < IA_PUYODIMY; y++)
    {
      PuyoState color = (PuyoState)(*dst)[x][y];
      if ((color != PUYO_EMPTY) && (color != PUYO_NEUTRAL))
      {
        didSomething |= removeSamePuyoAround(x, y, color, dst, evaluation);
      }
    }
  }

  // compress the columns by dropping floatting puyos left
  for (unsigned int x = 0;  x < IA_PUYODIMX; x++)
  {
    columnCompress(x, dst);
  }  

  // Tell if we remove any puyo
  return didSomething;
}


void evalWith(GridState * grid, GridState * gridOrigin, GridEvaluation * const realEvaluation)
{
  GridState tmp;
  GridEvaluation evaluation;

  // Evaluate the potential to destroy more puyos

  // for each puyo except the lower line and the columns top
  evaluation = nullEvaluation;
  for (int x = 0;  x < IA_PUYODIMX; x++)
  {
    int h = columnHeight(x, grid);
    if (h > realEvaluation->height) realEvaluation->height = h;
    h = stripedColumnHeight(x, grid);

    for (int y = 1;  y < h-1; y++)
    {
      // if free on the left or free on the right
      if ( ((x > 0) && ((*grid)[x-1][y] == PUYO_EMPTY)) || ((x < IA_PUYODIMX-1) && ((*grid)[x+1][y] == PUYO_EMPTY)))
      {
        // then try to remove it and see what happens
        copyGrid(&tmp,grid);
        tmp[x][y] = PUYO_EMPTY;
        columnCompress(x,&tmp);
        evaluation = nullEvaluation;
        while (suppressGroups(&tmp, &evaluation) == true) {};
        if (evaluation.puyoSuppressed > realEvaluation->puyoSuppressedPotential)
        {
          realEvaluation->puyoSuppressedPotential = evaluation.puyoSuppressed;
        }
      }
    }
  }
  
    // count the groups of more than 1 puyo
  evaluation = nullEvaluation;
  copyGrid(&tmp,grid);
  for (int x = 0;  x < IA_PUYODIMX; x++)
  {
    int h = stripedColumnHeight(x, grid);
    for (int y = 0;  y < h; y++)
    {
      if ((tmp[x][y] != PUYO_EMPTY) && (tmp[x][y] != PUYO_NEUTRAL))
      {
        countSamePuyoAround(x, y, (PuyoState)tmp[x][y], &tmp, &evaluation);
      }
    }
  }
  int d = evaluation.puyoSuppressed;
  evaluation = nullEvaluation;
  copyGrid(&tmp,gridOrigin);
  for (int x = 0;  x < IA_PUYODIMX; x++)
  {
    int h = stripedColumnHeight(x, grid);
    for (int y = 0;  y < h; y++)
    {
      if ((tmp[x][y] != PUYO_EMPTY) && (tmp[x][y] != PUYO_NEUTRAL))
      {
        countSamePuyoAround(x, y, (PuyoState)tmp[x][y], &tmp, &evaluation);
      }
    }
  }
  d -= evaluation.puyoSuppressed;
  if (d>0) realEvaluation->puyoGrouped = d;

}

bool dropBinom(const PuyoBinom binom, const GridState * const src, GridState * const dst, GridEvaluation * const evaluation)
{
  // display debug info
#ifdef SOLOPLUS
  displaySep("+++");
  displayBinom(binom);
  displayGrid(src==NULL?dst:src);
#endif

  // Verify quickly if we can reach the point
  for (int i = 2; i != binom.position.x; (binom.position.x>i)?i++:i--)
    if (columnHeight(i, src)>=IA_PUYODIMY) return false;

  // Copy the matrix
  if (src != NULL) copyGrid(dst,src);

  // Drop the Puyos and return false if not possible
  if (dropPuyos(binom, dst) == false) return false;

  // Delete all eligible groups
  while (suppressGroups(dst, evaluation) == true) {};

  // display debug info
#ifdef SOLOPLUS
  displayGrid(dst);
  fprintf(stdout,"Suppressions -> {%d, %d - %d}\n", evaluation->puyoSuppressed, evaluation->neutralSuppressed, evaluation->puyoSuppressedPotential);
  displaySep("---");
#endif

  // return
  return true;
}

int PuyoIA::makeEvaluation(const GridEvaluation * const referenceOne, const PuyoBinom puyos)
{
  int rR,rP;
  
  rR = (referenceOne->puyoSuppressed + referenceOne->neutralSuppressed + referenceOne->puyoGrouped);
  rR *= ((AIParameters*)special)->realSuppressionValue;

  if (((AIParameters*)special)->criticalHeight > referenceOne->height)
  {
    rP = (referenceOne->neutralSuppressedPotential + referenceOne->puyoSuppressedPotential);
    rP *= ((AIParameters*)special)->potentialSuppressionValue;
  }
  else rP = 0;

  int pos1, pos2;
  pos1 = puyos.position.x;
  pos2 = pos1;
  switch (puyos.orientation)
  {
    case Right:
      pos2 = pos1+1;
      break;
    case Left:
      pos2 = pos1-1;
      break;
  }
  
  int c = ((AIParameters*)special)->columnScalar[pos1] + ((AIParameters*)special)->columnScalar[pos2];

  int r = c * (1+rR+rP);
  return r;
}

bool PuyoIA::selectIfBetterEvaluation(int * const best, const GridEvaluation * const newOne, const PuyoBinom puyos)
{
  int n = makeEvaluation(newOne, puyos);
  int r = *best;
  if (n > r)
  {
    *best = n;
    return true;
  }
  return false;
}


PuyoIA::PuyoIA(IA_Type type, int level, PuyoView &targetView)
: PuyoPlayer(targetView), type(type), level(level)
{
  internalGrid = NULL;
  decisionMade = false;
  attachedGame = targetView.getAttachedGame();
  objective = nullBinom;

  special = NULL;

  // Select IA
  switch (type)
  {
    case GYOM:
      special = malloc(sizeof(AIParameters));
      ((AIParameters*)special)->realSuppressionValue = 1;
      ((AIParameters*)special)->potentialSuppressionValue = 3;
      ((AIParameters*)special)->criticalHeight = 10;
      ((AIParameters*)special)->columnScalar[0] = 2;
      ((AIParameters*)special)->columnScalar[1] = 1;
      ((AIParameters*)special)->columnScalar[2] = 1;
      ((AIParameters*)special)->columnScalar[3] = 4;
      ((AIParameters*)special)->columnScalar[4] = 4;
      ((AIParameters*)special)->columnScalar[5] = 4;
      break;
  
    case FLOBO:
      special = malloc(sizeof(AIParameters));
      ((AIParameters*)special)->realSuppressionValue = 3;
      ((AIParameters*)special)->potentialSuppressionValue = 1;
      ((AIParameters*)special)->criticalHeight = 3;
      ((AIParameters*)special)->columnScalar[0] = 1;
      ((AIParameters*)special)->columnScalar[1] = 1;
      ((AIParameters*)special)->columnScalar[2] = 1;
      ((AIParameters*)special)->columnScalar[3] = 1;
      ((AIParameters*)special)->columnScalar[4] = 1;
      ((AIParameters*)special)->columnScalar[5] = 1;
      break;
  
    case TANIA:
      special = malloc(sizeof(AIParameters));
      ((AIParameters*)special)->realSuppressionValue = 2;
      ((AIParameters*)special)->potentialSuppressionValue = 1;
      ((AIParameters*)special)->criticalHeight = 5;
      ((AIParameters*)special)->columnScalar[0] = 1;
      ((AIParameters*)special)->columnScalar[1] = 2;
      ((AIParameters*)special)->columnScalar[2] = 1;
      ((AIParameters*)special)->columnScalar[3] = 2;
      ((AIParameters*)special)->columnScalar[4] = 1;
      ((AIParameters*)special)->columnScalar[5] = 2;
      break;
  
    case JEKO:
      special = malloc(sizeof(AIParameters));
      ((AIParameters*)special)->realSuppressionValue = 1;
      ((AIParameters*)special)->potentialSuppressionValue = 3;
      ((AIParameters*)special)->criticalHeight = 8;
      ((AIParameters*)special)->columnScalar[0] = 2;
      ((AIParameters*)special)->columnScalar[1] = 1;
      ((AIParameters*)special)->columnScalar[2] = 1;
      ((AIParameters*)special)->columnScalar[3] = 1;
      ((AIParameters*)special)->columnScalar[4] = 1;
      ((AIParameters*)special)->columnScalar[5] = 2;
      break;

    default:
      break;
  }
}

PuyoIA::~PuyoIA()
{
  if (internalGrid != NULL) free(internalGrid);
  if (special != NULL) free(special);
}

PuyoState PuyoIA::extractColor(PuyoState A) const
{
  switch (A) {
    case PUYO_FALLINGBLUE:
      return PUYO_BLUE;
    case PUYO_FALLINGRED:
      return PUYO_RED;
    case PUYO_FALLINGGREEN:
      return PUYO_GREEN;
    case PUYO_FALLINGVIOLET:
      return PUYO_VIOLET;
    case PUYO_FALLINGYELLOW:
      return PUYO_YELLOW;
    default:
      fprintf(stderr,"Error in AI : unknown puyo color %d!!\nExiting...\n",(int)A);
      exit(0);
  }
  return PUYO_EMPTY;
}

PuyoOrientation PuyoIA::extractOrientation(int D) const
{
  switch (D) {
    case 0:
      return Below;
    case 1:
      return Left;
    case 2:
      return Above;
    case 3:
      return Right;
    default:
      fprintf(stderr,"Error in AI : unknown puyo orientation!!\nExiting...\n");
      exit(0);
  }
  return Left;
}

void PuyoIA::extractGrid(void)
{
  // Alloc a grid if not already done
  if (internalGrid == NULL) internalGrid = (GridState *)malloc(sizeof(GridState));

  // Fill the grid with current data
  for (int i = 0; i < IA_PUYODIMX; i++)
    for (int j = 0; j < IA_PUYODIMY; j++)
      (* internalGrid)[i][j] = attachedGame->getPuyoAt(i,(PUYODIMY-1)-j)->getPuyoState();
}

void PuyoIA::decide()
{
  PuyoBinom current, next;

  GridState state1,state2;
  GridEvaluation evaluation1, evaluation2;

  unsigned int bestl1=1, bestl2=1;
  int bestEvaluation = 0;

  bool foundOne = false;

  // get puyo binoms to drop
  current.falling     = extractColor(attachedGame->getFallingState());
  current.companion   = extractColor(attachedGame->getCompanionState());
  current.orientation = Left;
  current.position.x  = 0;
  current.position.y  = IA_PUYODIMY+1;
  next.falling        = extractColor(attachedGame->getNextFalling());
  next.companion      = extractColor(attachedGame->getNextCompanion());
  next.orientation    = Left;
  next.position.x     = 0;
  next.position.y     = IA_PUYODIMY+1;

  for (unsigned int l1 = 1; l1 <= MAXCOMBINATION; l1++)
  {
    // set position of binom 1
    serialPosition(l1,&current);

    // reset evaluation
    evaluation1 = nullEvaluation;

    // drop the binom (including destroying eligible groups) and continue if game not lost
    if (dropBinom(current, internalGrid, &state1, &evaluation1))
    {
      for (unsigned int l2 = 1; l2 <= MAXCOMBINATION; l2++)
      {
        // set position of binom 1
        serialPosition(l2,&next);

        // copy evaluation
        evaluation2 = evaluation1;

        // drop the binom (including destroying eligible groups) and eval board if game not lost
        if (dropBinom(next, &state1, &state2, &evaluation2))
        {
          evalWith(&state2, &state1, &evaluation2);

          if (foundOne == false || selectIfBetterEvaluation(&bestEvaluation, &evaluation2, current))
          {
            bestl1 = l1;
            bestl2 = l2;
          }
          foundOne = true;
        }
      }
    }
  }

#ifdef SOLOPLUS
  displaySep(NULL);
  fprintf(stdout,"Best Suppressions @ {%d, %d} : {%d, %d - %d}\n",bestl1, bestl2, bestEvaluation.puyoSuppressed, bestEvaluation.neutralSuppressed, bestEvaluation.puyoSuppressedPotential);
#endif

  // set position of best binom
  if (foundOne == false) current.position.x=IA_PUYODIMX;
  else serialPosition(bestl1,&current);

  objective = current;
}



void PuyoIA::cycle()
{
  // Test if we have to decide where to play
  if ((decisionMade == false) && (attachedGame->getFallingY() == 1))
  {
    // if so update the internal grid
    extractGrid();

    // Select IA
    switch (type)
    {
      case RANDOM: // Random move
        objective.position.x = (random() % IA_PUYODIMX);
        objective.orientation = (PuyoOrientation)(random() % 4);
        break;

      case GYOM:
      case TANIA:
      case JEKO:
      case FLOBO:
        decide();
        break;

      default:
        objective.position.x = 3;
        objective.orientation = Above;
        break;        
    }

    // remember we decided
    decisionMade = true;
  }
  if (attachedGame->getFallingY() != 1) decisionMade = false;

  // Move to the position we decided
  if (random() % level < 10)
  {
    if (extractOrientation(attachedGame->getFallingCompanionDir()) != objective.orientation) targetView.rotateLeft();
    else if (attachedGame->getFallingX() < objective.position.x) targetView.moveRight();
    else if (attachedGame->getFallingX() > objective.position.x) targetView.moveLeft();
    else targetView.cycleGame();
  }
}
