#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "PuyoIA.h"

#define UP 2
#define LEFT 1
#define DOWN 0
#define RIGHT 3

struct PosEvaluator {
  int c_direction;
  int f_x;
  int score;
  bool keep;

  int update(PuyoGame *game, PuyoState f_color, PuyoState c_color)
  {
    int f_y = 0;
    int c_x = 0;
    int c_y = 0;

    if (f_color == PUYO_EMPTY) return 0;
    if (c_color == PUYO_EMPTY) return 0;

    switch (c_direction) {
      case UP:
        f_y = game->getColumnHeigth(f_x);
        c_x = f_x;
        c_y = f_y - 1;
        break;
      case DOWN:
        c_y = game->getColumnHeigth(f_x);
        c_x = f_x;
        f_y = c_y - 1;
        break;
      case RIGHT:
        f_y = game->getColumnHeigth(f_x);
        c_x = f_x + 1;
        c_y = game->getColumnHeigth(f_x + 1);
        break;
      case LEFT:
        f_y = game->getColumnHeigth(f_x);
        c_x = f_x - 1;
        c_y = game->getColumnHeigth(f_x - 1);
        break;
    }
    f_y = PUYODIMY - f_y - 1; /* - 1 or not -1 ??? TODO */
    c_y = PUYODIMY - c_y - 1;
    if ((f_y >= 6) && (c_y >= 6)) {
      score = 
          game->getSamePuyoAround(f_x, f_y, f_color)
        + game->getSamePuyoAround(c_x, c_y, c_color);
    }
    else if ((f_y >= 0) && (c_y >= 0))
      score = -1000
        + game->getSamePuyoAround(f_x, f_y, f_color)
        + game->getSamePuyoAround(c_x, c_y, c_color);
    else
      score = -50000;
    return score;
  }
  
  void init(int col, int dir)
  {
    f_x = col;
    c_direction = dir;
  }
  PosEvaluator() { f_x = c_direction = 0; }
};

PuyoIA::PuyoIA(IA_Type type, int level, PuyoGame *targetGame)
        : PuyoPlayer(targetGame), type(type), level(level)
{
    evaluator = new PosEvaluator[PUYODIMX * 4 - 2];
    int iEval = 0;
    for (int col=0; col<PUYODIMX; ++col) {
      if (col != 0)
        evaluator[iEval++].init(col, LEFT);
      if (col != PUYODIMX - 1)
        evaluator[iEval++].init(col, RIGHT);
      evaluator[iEval++].init(col, UP);
      evaluator[iEval++].init(col, DOWN);
    }
    choosenMove = -1;
    firstLine = false;
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
    return A;
  }
}

void PuyoIA::cycle()
{
  switch (type) {
    case CASTOR:
      {
        if (!firstLine && (attachedGame->getFallingY() == 1))
          choosenMove = -1;
        firstLine = (attachedGame->getFallingY() == 1);
        if ((choosenMove >= 0) && (random() % level < 10)) {
          if (attachedGame->getFallingCompanionDir() != evaluator[choosenMove].c_direction)
            attachedGame->rotateLeft();
          else if (attachedGame->getFallingX() < evaluator[choosenMove].f_x)
            attachedGame->moveRight();
          else if (attachedGame->getFallingX() > evaluator[choosenMove].f_x)
            attachedGame->moveLeft();
          else
            attachedGame->cycle();

          if (PUYODIMY - attachedGame->getColumnHeigth(evaluator[choosenMove].f_x) - 1
              < attachedGame->getFallingY()) choosenMove = -1;
          }
        else if (choosenMove == -1) {
          PuyoState f_color = extractColor(attachedGame->getFallingState());
          PuyoState c_color = extractColor(attachedGame->getCompanionState());
          int max = -100000;
          for (int i=PUYODIMX*4-3; i>=0; --i) {
            if ((f_color == PUYO_EMPTY)||(c_color==PUYO_EMPTY))
              continue;
            int val = evaluator[i].update(attachedGame, f_color, c_color);
            if (val > max) {
              max = val;
            }
          }
          int nbKeep = 0;
          for (int i=PUYODIMX*4-3; i>=0; --i) {
            evaluator[i].keep = (evaluator[i].score == max);
            nbKeep ++;
          }
          int tmp = (random() / 7) % nbKeep;
          for (int i=PUYODIMX*4-3; i>=0; --i) {
            if (evaluator[i].keep) {
              tmp--;
              if (tmp == 0) {
                choosenMove = i;
                break;
              }
            }
          }
        }
      }
      break;

    case RANDOM:
      switch (random() % 50) {
        case 0:
          attachedGame->rotateLeft();
          break;
        case 1:
          attachedGame->moveLeft();
          break;
        case 2:
          attachedGame->moveRight();
          break;
        default:
          break;
      }
  } 
}
