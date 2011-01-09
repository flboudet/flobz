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

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include "AIPlayer.h"

static const PuyoCoordinates nullPosition = {0,0};
static const PuyoBinom nullBinom = {PUYO_EMPTY,PUYO_EMPTY,Left,nullPosition};
static const GridEvaluation nullEvaluation = {0,0,0,0,0,0};

#define copyGrid(dst,src) memcpy((void *)dst, (void *)src, sizeof(GridState))
#define zeroGrid(dst) memset((void *)dst, 0, sizeof(GridState))

#define HEIGHTS_ROW (IA_TABLEDIMY-1)

inline int columnHeight(const unsigned int x, const GridState * const grid)
{
  return (*grid)[x][HEIGHTS_ROW];
}

inline int stripedColumnHeight(const unsigned int x, const GridState * const grid)
{
  int h = (*grid)[x][HEIGHTS_ROW];
  while ((h>0) && ((*grid)[x][h-1] == PUYO_NEUTRAL)) h--;
  return h;
}


inline int normalColumnHeight(const unsigned int x, const GridState * const grid)
{
  if ((*grid)[x][0] != PUYO_EMPTY)
  {
    int y = IA_PUYODIMY-1;
    while ((y>0) && ((*grid)[x][y] == PUYO_EMPTY)) y--;
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

    X = mx[i];
    Y = my[i];

    if (Y+1<IA_PUYODIMY) {
      if (marked[X][Y+1] == 0) {
        if ((*tab)[X][Y+1] == color) {
          mx[nFound] = X;
          my[nFound] = Y+1;
          nFound++;
        }
        marked[X][Y+1] = 1;
      }
    }
    if (Y>0) {
      if (marked[X][Y-1] == 0) {
        if ((*tab)[X][Y-1] == color) {
          mx[nFound] = X;
          my[nFound] = Y-1;
          nFound++;
        }
        marked[X][Y-1] = 1;
      }
    }
    if (X+1<IA_PUYODIMX) {
      if (marked[X+1][Y] == 0) {
        if ((*tab)[X+1][Y] == color) {
          mx[nFound] = X+1;
          my[nFound] = Y;
          nFound++;
        }
        marked[X+1][Y] = 1;
      }
    }
    if (X>0) {
      if (marked[X-1][Y] == 0) {
        if ((*tab)[X-1][Y] == color) {
          mx[nFound] = X-1;
          my[nFound] = Y;
          nFound++;
        }
        marked[X-1][Y] = 1;
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

    X = mx[i];
    Y = my[i];

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
  int height = 0;
  
	// on oublie les cases vides en haut de colonne
  while ( (last > 0) && ((*grid)[x][last-1] == PUYO_EMPTY) ) last--;

  while (ydst < last) // Tant qu'on a pas rempli dst
  {
    if ( ysrc < last ) // Si on n'a pas épuisé src
    {
      PuyoState state = (PuyoState)(*grid)[x][ysrc];
      if (state != PUYO_EMPTY) // Si src n'est pas vide
      {
        // On recopie et on passe au dst suivant
        if (ysrc != ydst) (*grid)[x][ydst] = state;
        height++;
        ydst++;
      }
      // src suivant
      ysrc++;
    }
    else // Fin du remplissage avec PUYO_EMPTY
    {
      (*grid)[x][ydst] = PUYO_EMPTY;
      ydst++;
    }
  }
  (*grid)[x][HEIGHTS_ROW] = height;
}

static int fallingTable[PUYODIMX] = {0, 3, 1, 4, 2, 5};

void dropNeutrals(int number, int totalFromStart, GridState * const grid, GridState * const src)
{
    copyGrid(grid, src);
    while (number > 0)
    {
        int cycleNeutral;
        if (number >= IA_PUYODIMX)
            cycleNeutral = IA_PUYODIMX;
        else
            cycleNeutral = number;
        for (int i = 0 ; i < cycleNeutral ; i++)
        {
            int posX = fallingTable[(totalFromStart++) % IA_PUYODIMX];
            int posY = (*grid)[posX][HEIGHTS_ROW];
            number--;
            if (((*grid)[posX][posY] = PUYO_EMPTY) || (posY >= IA_PUYODIMY))
                continue;
            // Creating a new neutral puyo
            (*grid)[posX][posY] = PUYO_NEUTRAL;
            (*grid)[posX][HEIGHTS_ROW]++;
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
    (*grid)[x][h]   = binom.falling;
    (*grid)[x][h+1] = binom.companion;
    (*grid)[x][HEIGHTS_ROW] = h + 2;
    break;

  case Below:
    if (h + 1 >= IA_PUYODIMY) return false;
    (*grid)[x][h+1] = binom.falling;
    (*grid)[x][h]   = binom.companion;
    (*grid)[x][HEIGHTS_ROW] = h + 2;
    break;

  case Left:
    if (h >= IA_PUYODIMY) return false;
    g = columnHeight(x-1, grid);
    if (g >= IA_PUYODIMY) return false;
    (*grid)[x][h]   = binom.falling;
    (*grid)[x-1][g] = binom.companion;
    ((*grid)[x][HEIGHTS_ROW]) = h + 1;
    ((*grid)[x-1][HEIGHTS_ROW]) = g + 1;
    break;

  case Right:
    if (h >= IA_PUYODIMY) return false;
    g = columnHeight(x+1, grid);
    if (g >= IA_PUYODIMY) return false;
    (*grid)[x][h]   = binom.falling;
    (*grid)[x+1][g] = binom.companion;
    ((*grid)[x][HEIGHTS_ROW]) = h + 1;
    ((*grid)[x+1][HEIGHTS_ROW]) = g + 1;
    break;
  }

  return true;
}


// Those combinations can place the binoms everywhere on x
// except x=0 with companion left and x=IA_PUYODIMX-1 with companion right
#define MAXCOMBINATION (IA_PUYODIMX*4)-2
#define DISPATCHCYCLES 4
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


void evalWith(const GridState * const grid, const GridEvaluation * const originEvaluation, GridEvaluation * const realEvaluation)
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
        int r;
        for (r=0; suppressGroups(&tmp, &evaluation); r++) {};
        // if we used more than 1 round more neutrals willbe dropped to the ennemy
        if (r>1) evaluation.puyoSuppressed += (r-1)*PUYODIMX;
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
  int d = evaluation.puyoSuppressed - originEvaluation->puyoSuppressed;
  if (d>0) realEvaluation->puyoGrouped = d;
}

bool dropBinom(const PuyoBinom binom, const GridState * const src, GridState * const dst, GridEvaluation * const evaluation)
{
  // Copy the matrix
  if (src != NULL) copyGrid(dst,src);

  // Drop the Puyos and return false if not possible
  if (dropPuyos(binom, dst) == false) return false;

  // Delete all eligible groups
  while (suppressGroups(dst, evaluation) == true) {};

  // return
  return true;
}

int AIPlayer::makeEvaluation(const GridEvaluation * const referenceOne, const PuyoBinom puyos, const GridState * const grid)
{
  int rR,rP;
  
  rR = (2 * referenceOne->puyoSuppressed + 2 * referenceOne->neutralSuppressed + referenceOne->puyoGrouped);
  rR *= params.realSuppressionValue;

  if (params.criticalHeight > referenceOne->height)
  {
    rP = (referenceOne->neutralSuppressedPotential + referenceOne->puyoSuppressedPotential);
    rP *= params.potentialSuppressionValue;
  }
  else
  {
    rP = 0;
    rR *= 2;
  }

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
    default:
      break;
  }
  
  int c = 0;
  
  if (params.criticalHeight > referenceOne->height)
  {

  for (int x = 0;  x < IA_PUYODIMX; x++)
  {
    c += IA_PUYODIMY - abs(params.columnScalar[x] - (int)(*grid)[x][HEIGHTS_ROW]);
  }
  c*=IA_PUYODIMX;
  }
  else c = IA_PUYODIMY * IA_PUYODIMX * IA_PUYODIMX + 2 * IA_PUYODIMY;
    c += params.columnScalar[pos1] - (int)(*grid)[pos1][HEIGHTS_ROW];
    c += params.columnScalar[pos2] - (int)(*grid)[pos2][HEIGHTS_ROW];

  int r = c * (1+rR+rP);
  return r;
}

bool AIPlayer::selectIfBetterEvaluation(int * const best, const GridEvaluation * const newOne, const PuyoBinom puyos, const GridState * const grid)
{
  int n = makeEvaluation(newOne, puyos, grid);
  int r = *best;
  if (n > r)
  {
    *best = n;
    return true;
  }
  return false;
}


AIPlayer::AIPlayer(int level, PuyoView &targetView)
: PuyoPlayer(targetView)
{
  internalGrid = NULL;
  decisionMade = 0;
  shouldRedecide = false;
  lastNumberOfBadPuyos = 0;
  totalNumberOfBadPuyos = 0;
  attachedGame = targetView.getAttachedGame();
  objective = nullBinom;
  lastLineSeen = PUYODIMY+1;
  currentCycle = 0;
  readyToDrop = false;

  params.realSuppressionValue = 1;
  params.potentialSuppressionValue = 3;
  params.criticalHeight = 10;
  params.columnScalar[5] = 1;
  params.columnScalar[0] = 1;
  params.columnScalar[1] = 1;
  params.columnScalar[2] = 1;
  params.columnScalar[3] = 1;
  params.columnScalar[4] = 1;
  params.columnScalar[5] = 1;
  params.rotationMethod = 0; // negative (right), null (shortest), positive(left)
  params.fastDropDelta = PUYODIMY; // puyo height relative to column height before fast drop
  params.thinkDepth = 2;
  params.speedFactor = level>0?level:1;
  this->level = level;
}

void AIPlayer::setAIParameters(const AIParameters &ai)
{
  int a = params.speedFactor;
  params = ai;
  params.speedFactor = a;
}

AIPlayer::~AIPlayer()
{
  if (internalGrid != NULL) free(internalGrid);
}

PuyoState AIPlayer::extractColor(PuyoState A) const
{
  switch (A) {
    case PUYO_FALLINGBLUE:
    case PUYO_BLUE:
      return PUYO_BLUE;
    case PUYO_FALLINGRED:
    case PUYO_RED:
      return PUYO_RED;
    case PUYO_FALLINGGREEN:
    case PUYO_GREEN:
      return PUYO_GREEN;
    case PUYO_FALLINGVIOLET:
    case PUYO_VIOLET:
      return PUYO_VIOLET;
    case PUYO_FALLINGYELLOW:
    case PUYO_YELLOW:
      return PUYO_YELLOW;
    default:
      fprintf(stderr,"Error in AI : unknown puyo color %d %d!!\nExiting...\n",(int)A,(int)attachedGame->isGameRunning());
      exit(0);
  }
  return PUYO_EMPTY;
}

static const PuyoOrientation GameToIAOrientation[4] = {Below,Left,Above,Right};
PuyoOrientation AIPlayer::extractOrientation(int D) const
{
  if (D<0 || D>3) {
    fprintf(stderr,"Error in AI : unknown puyo orientation!!\nExiting...\n");
    exit(0);  
  }
  return GameToIAOrientation[(int)D];
}

static const int IAToGameOrientation[4] = {1,2,0,3};
int AIPlayer::revertOrientation(PuyoOrientation D) const
{
  if (D<0 || D>3) {
    fprintf(stderr,"Error in AI : unknown puyo orientation bis!!\nExiting...\n");
    exit(0);
  }
  return IAToGameOrientation[D];
}

void AIPlayer::extractGrid(void)
{
  // Alloc a grid if not already done
  if (internalGrid == NULL) internalGrid = (GridState *)malloc(sizeof(GridState));

  // Fill the grid with current data
  for (int i = 0; i < IA_PUYODIMX; i++)
  {
    int height = 0;
    for (int j = 0; j < IA_PUYODIMY; j++)
    {
      PuyoPuyo * thePuyo = attachedGame->getPuyoAt(i,(PUYODIMY-1)-j);
      if (thePuyo != NULL)
      {
        PuyoState state = thePuyo->getPuyoState();
        if (state >= PUYO_STILL)
        {
          (* internalGrid)[i][j] = state;
          height++;
        } else (* internalGrid)[i][j] = PUYO_EMPTY;
      }
      else
      {
        (* internalGrid)[i][j] = PUYO_EMPTY;
      }
    }
    (* internalGrid)[i][HEIGHTS_ROW] = height;
  }
}

bool canReach(const PuyoBinom binom, const PuyoBinom dest, GridState * const internalGrid, int duration)
{
  int minBinomY = binom.position.y;
  int minBinomX = binom.position.x;
  int maxBinomX = binom.position.x;
  switch (binom.orientation)
  {
    case Left:
      minBinomX--;
      break;
    case Right:
      maxBinomX++;
      break;
    case Below:
      minBinomY--;
      break;
    default:
      break;
  }
 
    minBinomY = ((minBinomY-duration) >= 0) ? minBinomY-duration : 0;
  //if (minBinomY >= IA_PUYODIMY) return true;
  
  int minDestX = dest.position.x;
  int maxDestX = dest.position.x;
  switch (dest.orientation)
  {
    case Left:
      minDestX--;
      break;
    case Right:
      maxDestX++;
      break;
    default:
      break;
  }
  
  if (minBinomX < maxDestX)
    for (int i = minBinomX;  i <= maxDestX; i++) if ((*internalGrid)[i][minBinomY] != PUYO_EMPTY) return false;
  
  if (maxBinomX > minDestX)
    for (int i = minDestX;  i <= maxBinomX; i++) if ((*internalGrid)[i][minBinomY] != PUYO_EMPTY) return false;
  
  return true;
}

void AIPlayer::decide(int partial, int depth)
{
  //fprintf(stderr, "  Decision %d on %d - Depth = %d\n",partial+1,DISPATCHCYCLES,depth);
  if (partial == 0)
  {
    // get puyo binoms to drop
    PuyoState etat;
    etat = attachedGame->getFallingState();
    if (etat == PUYO_EMPTY) return;
    current.falling     = extractColor(etat);
    etat = attachedGame->getCompanionState();
    if (etat == PUYO_EMPTY) return;
    current.companion   = extractColor(etat);
    current.orientation = extractOrientation(attachedGame->getFallingCompanionDir());
    current.position.x  = attachedGame->getFallingX();
    current.position.y  = PUYODIMY - attachedGame->getFallingY();
    
    originalPuyo = current;
    
    next.falling        = extractColor(attachedGame->getNextFalling());
    next.companion      = extractColor(attachedGame->getNextCompanion());
    next.orientation    = Left;
    next.position.x     = 0;
    next.position.y     = IA_PUYODIMY+1;

    bestl1=1;
    foundOne = false;
    bestEvaluation = 0;
  }
  
  switch (depth)
  {
    case 1:
      for (unsigned int l1 = 1+partial; l1 <= MAXCOMBINATION; l1+=DISPATCHCYCLES)
      {
        // set position of binom 1
        serialPosition(l1,&current);
        
        // reset evaluation
        GridEvaluation evaluation1 = nullEvaluation;
        
        GridState state1;
        
        // drop the binom (including destroying eligible groups) and continue if game not lost
        if (canReach(originalPuyo, current, internalGrid, 1) && dropBinom(current, internalGrid, &state1, &evaluation1))
        {
          evalWith(&state1, &nullEvaluation, &evaluation1);
          
          if (foundOne == false || selectIfBetterEvaluation(&bestEvaluation, &evaluation1, current, &state1))
          {
            bestl1 = l1;
          }
          foundOne = true;
        }
      }
      if (foundOne) serialPosition(bestl1,&current);
      objective = current;
      break;
      
    case 2:
      for (unsigned int l1 = 1+partial; l1 <= MAXCOMBINATION; l1+=DISPATCHCYCLES)
      {
        // set position of binom 1
        serialPosition(l1,&current);
        
        // reset evaluation
        GridEvaluation evaluation1 = nullEvaluation;
        
        GridState state1;
        
        // drop the binom (including destroying eligible groups) and continue if game not lost
        if (canReach(originalPuyo, current, internalGrid, 1) && dropBinom(current, internalGrid, &state1, &evaluation1))
        {
          for (unsigned int l2 = 1; l2 <= MAXCOMBINATION; l2++)
          {
            // set position of binom 1
            serialPosition(l2,&next);
            
            // copy evaluation
            GridEvaluation evaluation2 = evaluation1;
            
            GridState state1bis;
            GridState state2;
            
            dropNeutrals(lastNumberOfBadPuyos-evaluation1.puyoSuppressed, totalNumberOfBadPuyos, &state1bis, &state1);  
            
            // drop the binom (including destroying eligible groups) and eval board if game not lost
            if (canReach(originalPuyo, next, &state1bis, 1) && dropBinom(next, &state1bis, &state2, &evaluation2))
            {
              evalWith(&state2, &evaluation1, &evaluation2);
              
              if (foundOne == false || selectIfBetterEvaluation(&bestEvaluation, &evaluation2, current, &state2))
              {
                bestl1 = l1;
              }
              foundOne = true;
            }
          }
        }
      }
      if (foundOne) serialPosition(bestl1,&current);
      objective = current;
      break;
      
    default:
      objective.position.x = (random() % IA_PUYODIMX);
      objective.orientation = (PuyoOrientation)(random() % 4);
      break;
  }

}


// 0 no need to rotate, 1 rotate clockwise, 2 both directions are equal, -1 rotate counterclockwise
// positions are expression as Game references
static const signed char rotationMatrix[4 /*target*/][4 /*current*/] = {{0,-1,2,1},{1,0,-1,2},{2,1,0,-1},{-1,2,1,0}};

void AIPlayer::cycle()
{
  // If no falling puyo, no need to play
  if (attachedGame->getFallingPuyo() == NULL || !attachedGame->isGameRunning()) 
  {
    return;
  }
  
  int currentLine = attachedGame->getFallingY();
  int currentColumn = attachedGame->getFallingX();
  int currentNumberOfBadPuyos = attachedGame->getNeutralPuyos();
  int currentTotalNumberOfBadPuyos = attachedGame->getGameTotalNeutralPuyos();
    
  // If we start with new puyos or restart because new bad Puyos came
  if (attachedGame->isPhaseReady())
  {
    //fprintf(stderr, "Thinking\n");
    // Reset the cycle counter
    currentCycle = 0;
    // Save we did make any decision yet
    decisionMade = 0;
    // Should not decide again if we didn't decide yet !
    shouldRedecide = false;
    lastNumberOfBadPuyos = currentNumberOfBadPuyos;
    totalNumberOfBadPuyos = currentTotalNumberOfBadPuyos;
  } else {
      // If we did not start, test if we should restart because new bad puyos arrived
      if (currentNumberOfBadPuyos != lastNumberOfBadPuyos) {
          //fprintf(stderr, "RE-Thinking\n");
          // Reset the cycle counter
          currentCycle = 0;
          // Save we did make any decision yet
          decisionMade = 0;
          // Should not decide again if we didn't decide yet !
          shouldRedecide = true;
          lastNumberOfBadPuyos = currentNumberOfBadPuyos;
          totalNumberOfBadPuyos = currentTotalNumberOfBadPuyos;
      }
  }



  
  // increment the cycle counter
  currentCycle++;
  
  // Test if we have to decide where to play
  if (decisionMade < DISPATCHCYCLES)
  {
    // if so update the internal grid the first time
    if (decisionMade == 0 && !shouldRedecide) extractGrid();

    // then start to think
    decide(decisionMade, params.thinkDepth);
 
    // remember what we decided
    decisionMade++;
    
    // don't drop yet!!
    readyToDrop = false;
    
    // Do not move until we are sure
    //if (decisionMade < DISPATCHCYCLES) return;
      //if (decisionMade == DISPATCHCYCLES) fprintf(stderr, "Thought\n");
  }

  // Now move to the position we decided :

  // If we can drop, then go on
  if (readyToDrop)
  {
    if (internalGrid == NULL) extractGrid();
    if ((PUYODIMY-1-currentLine) - ((* internalGrid)[currentColumn][HEIGHTS_ROW] + (objective.orientation == Below)?1:0) <= params.fastDropDelta) targetView.cycleGame();
  }
  
  // Else try to move at the specified frequency
  else if ((currentCycle % params.speedFactor) == 0)
  {
    bool shouldMove;
    bool shouldRotate;
    
    bool couldntMove = false;
    bool couldntRotate = false;
  
    int curOrientation = attachedGame->getFallingCompanionDir();

    // should we move or rotate
    if (random()%2 == 0)
    {
      shouldMove = (currentColumn != objective.position.x);
      shouldRotate = (!shouldMove) && (extractOrientation(curOrientation) != objective.orientation);
    } else {
      shouldRotate = (extractOrientation(curOrientation) != objective.orientation);
      shouldMove = (!shouldRotate) && (currentColumn != objective.position.x);
    }
    // if no need to move or rotate we're ready to drop
    readyToDrop = !(shouldMove || shouldRotate);
    
    // Move if useful
    if (shouldMove)
    {
      if (currentColumn < objective.position.x) targetView.moveRight();
      else targetView.moveLeft();

      // or decide again if not possible
      couldntMove = (currentColumn == attachedGame->getFallingX());
    }
    
    // Rotate if useful
    if (shouldRotate)
    {
      if (params.rotationMethod == 0)
      {
        if (rotationMatrix[revertOrientation(objective.orientation)][curOrientation] > 0)
          targetView.rotateRight();
        else
          targetView.rotateLeft();
        
      }
      else if (params.rotationMethod < 0) targetView.rotateRight();
      else if (params.rotationMethod > 0) targetView.rotateLeft();
        
      // or decide again if not possible
      couldntRotate = (curOrientation == attachedGame->getFallingCompanionDir());
    }

    // if need to move but impossible, decide again
      if (couldntMove) {
          decisionMade = 0;
          //fprintf(stderr, "ReThink because couldn't move\n");;
      }
  }
  lastLineSeen = currentLine;
}
