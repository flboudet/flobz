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

  int update(PuyoGame *game, PuyoState f_color, PuyoState c_color,
             int IAPuyoValue,
             int IASecondLevelPuyoValue,
             int IAConstructorLevel,
             int IAHeightAgressivity,
             int IAHeightFactor, int myRand)
  {
    int f_y = 0;
    int c_x = 0;
    int c_y = 0;
    
    if (f_color == PUYO_EMPTY) return 0;
    if (c_color == PUYO_EMPTY) return 0;
    
    /* Calcul de la posisition finale des deux puyos */
    switch (c_direction) {
    case UP:
        f_y = game->getColumnHeigth(f_x);
        c_x = f_x;
        c_y = f_y + 1;
        break;
    case DOWN:
      c_y = game->getColumnHeigth(f_x);
      c_x = f_x;
      f_y = c_y + 1;
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
    f_y = PUYODIMY - f_y - 1;
    c_y = PUYODIMY - c_y - 1;

    /* Si l'un des deux puyos est tout en haut => pire cas ! */
    if ((f_y < 0) || (c_y < 0))
    {
      score = -10000;
      return score;
    }

    /* On favorise les cas ou on regroupe les couleurs ! */
    score = (game->getSamePuyoAround(f_x, f_y, f_color) + game->getSamePuyoAround(c_x, c_y, c_color) )*IAPuyoValue;

    /* On favorise les cas ou on ne separe pas deux puyos de couleurs identiques qui tombent ! */
    if ( (f_color == c_color) && ( (c_x==f_x) || (c_y==f_y) ) ) score += IAPuyoValue;

    /* Si l'un des deux puyos est vraiment tres haut => vraiment pas bien ! */
    if ((f_y < 2) || (c_y < 2)) score -= 1000;

    /* Si les deux puyos sont suffisament bas => mode attaque ! */
    if ((f_y >= IAHeightAgressivity) && (c_y >= IAHeightAgressivity))
    {
      /* On va defavoriser les cas ou on fait des trops petites constructions : */
      PuyoState aboveStateF, aboveStateC;
	  bool F,C;

	  /* Main puyo may only make a block of 4 */
      aboveStateF = (c_direction == UP) ? c_color : PUYO_EMPTY;
	  F = ((game->getSamePuyoAround(f_x, f_y, f_color) + ((aboveStateF == f_color)?1:0)) == 4);

	  /* Companion puyo may only make a block of 4 */
      aboveStateC = (c_direction == DOWN) ? f_color : PUYO_EMPTY;
	  C = ((game->getSamePuyoAround(c_x, c_y, c_color) + ((aboveStateC == c_color)?1:0)) == 4);

      if ((F || C) && !(F && C))
      {
         score -= IAConstructorLevel*IAPuyoValue;
         /* fprintf(stderr,"--o--"); */
      }
    }

    /* On favorise les cas ou les puyos sont envoyes le plus bas possible */
    score += (f_y+c_y)*IAHeightFactor;

    /* On favorise les regroupements verticaux apres demolition : */

    /* xx2 */
    /* xx3 Favoriser le cas ou on pose un '3' au milieu pour que les '2' se groupent apres destruction du '3' */
    /* 152 */

    /* xx2 */
    /* x43 Favoriser aussi un peu le cas ou on pose un '5' sur le '4' */
    /* 152 */

    /* Puyo companion */
    if (c_y < PUYODIMY-1) /* Si on est pas completement en bas */
    {
      if ((c_x<(PUYODIMX - 1)) && (c_color == (game->getPuyoAt(c_x+1,c_y))->getPuyoState())) /* Test du pattern a droite */
      {
        PuyoState cColorA = (game->getPuyoAt(c_x+1,c_y+1))->getPuyoState();
        if ((c_color!=cColorA) && (cColorA>5) && (cColorA<11) && (cColorA == (game->getPuyoAt(c_x+1,c_y-1))->getPuyoState()))
        {
          score += game->getSamePuyoAround(c_x+1, c_y, cColorA)*IASecondLevelPuyoValue;
        }
      }
                    
      if ((c_x>0) && (c_color == (game->getPuyoAt(c_x-1,c_y))->getPuyoState())) /* Test du pattern a gauche */
      {
        PuyoState cColorB = (game->getPuyoAt(c_x-1,c_y+1))->getPuyoState();
        if ((c_color!=cColorB) && (cColorB>5) && (cColorB<11) && (cColorB == (game->getPuyoAt(c_x-1,c_y-1))->getPuyoState()))
        {
          score += game->getSamePuyoAround(c_x-1, c_y, cColorB)*IASecondLevelPuyoValue;
        }
      }
    }
    if (c_y < PUYODIMY-2) /* Si on est pas trop en bas */
    {
      if (c_color == (game->getPuyoAt(c_x,c_y+2))->getPuyoState())
      {
        score += game->getSamePuyoAround(c_x, c_y+2, c_color)*IASecondLevelPuyoValue;
      }
    }
                
    /* Puyo principal */
    if (f_y < PUYODIMY-1)  /* Si on est pas completement en bas */
    {
      if ((f_x<(PUYODIMX - 1)) && (f_color == (game->getPuyoAt(f_x+1,f_y))->getPuyoState())) /* Test du pattern a droite */
      {
        PuyoState fColorA = (game->getPuyoAt(f_x+1,f_y+1))->getPuyoState();
        if ((f_color!=fColorA) && (fColorA>5) && (fColorA<11) && (fColorA == (game->getPuyoAt(f_x+1,f_y-1))->getPuyoState()))
        {
          score += game->getSamePuyoAround(f_x+1, f_y, fColorA)*IASecondLevelPuyoValue;
        }
      }
                                  
      if ((f_x>0) && (f_color == (game->getPuyoAt(f_x-1,f_y))->getPuyoState())) /* Test du pattern a gauche */
      {
        PuyoState fColorB = (game->getPuyoAt(f_x-1,f_y+1))->getPuyoState();
        if ((f_color!=fColorB) && (fColorB>5) && (fColorB<11) && (fColorB == (game->getPuyoAt(f_x-1,f_y-1))->getPuyoState()))
        {
          score += game->getSamePuyoAround(f_x-1, f_y, fColorB)*IASecondLevelPuyoValue;
        }
      }
    }
    if (f_y < PUYODIMY-2) /* Si on est pas trop en bas */
    {
      if (f_color == (game->getPuyoAt(f_x,f_y+2))->getPuyoState())
      {
        score += game->getSamePuyoAround(f_x, f_y+2, f_color)*IASecondLevelPuyoValue;
      }
    }

    score += myRand;
    return score;
  }
    
  void init(int col, int dir)
  {
    f_x = col;
    c_direction = dir;
  }

  PosEvaluator() { f_x = c_direction = 0; }

};

PuyoIA::PuyoIA(IA_Type type, int level, PuyoView *targetView)
: PuyoPlayer(targetView), type(type), level(level)
{
    attachedGame = targetView->getAttachedGame();
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
    int IAPuyoValue = 100;
    int IASecondLevelPuyoValue = 30;
    int IAConstructorLevel = 3;
    int IAHeightAgressivity = 6;
    int IAHeightFactor = 2;

    switch (type) {
        case GYOM:
            break;
        case TANIA:
            IAConstructorLevel = 1;
            IAHeightAgressivity = 9;
            break;
        case JEKO:
            IAConstructorLevel = 1;
            IAHeightAgressivity = 3;
            IAHeightFactor = 1;
            break;
        case FLOBO:
            IAConstructorLevel = 0; 
            IAHeightAgressivity = 0xffff; // Inhibits agressivity.
            IAHeightFactor = 0;
            IAPuyoValue = 50;
            IASecondLevelPuyoValue = 0;
            break;
        default:
            break;
    }

    switch (type) {
            
        case RANDOM:
            switch (random() % 50) {
                case 0:
                    attachedView->rotateLeft();
                    break;
                case 1:
                    attachedView->moveLeft();
                    break;
                case 2:
                    attachedView->moveRight();
                    break;
                default:
                    break;
            }
            break;

        default:
        {
            if (!firstLine && (attachedGame->getFallingY() == 1) && (choosenMove != -1))
                choosenMove = -1;
            firstLine = (attachedGame->getFallingY() == 1);
            if ((choosenMove >= 0) && (random() % level < 10))
            {
                if (attachedGame->getFallingCompanionDir() != evaluator[choosenMove].c_direction)
                    attachedView->rotateLeft();
                else if (attachedGame->getFallingX() < evaluator[choosenMove].f_x)
                    attachedView->moveRight();
                else if (attachedGame->getFallingX() > evaluator[choosenMove].f_x)
                    attachedView->moveLeft();
                else
                    attachedView->cycleGame();
            }
            else if (choosenMove == -1) 
            {
                PuyoState f_color = extractColor(attachedGame->getFallingState());
                PuyoState c_color = extractColor(attachedGame->getCompanionState());
                int max = -100000;
                for (int i=PUYODIMX*4-3; i>=0; --i)
                {
                    if ((f_color == PUYO_EMPTY)||(c_color==PUYO_EMPTY)) return;
                    int myRand = 0;

                    if (type == FLOBO) {
                        myRand = (int) ((double)IAPuyoValue*2*(double)random()/(RAND_MAX+1.0));
                    }
                    else if (type == JEKO) {
                        myRand = (int) ((double)IAPuyoValue*(double)random()/(RAND_MAX+1.0)/2.0);
                    }
                    

                    /* IAPuyoValue / IASecondLevelPuyoValue / IAConstructorLevel / IAHeightAgressivity / IAHeightFactor */
                    int val = evaluator[i].update(attachedGame, f_color, c_color,
                            IAPuyoValue, IASecondLevelPuyoValue, IAConstructorLevel, IAHeightAgressivity, IAHeightFactor,
                            myRand);
                    
                    if (val >= max) max = val;
                }
                
                int nbKeep = 0;
                for (int i=PUYODIMX*4-3; i>=0; --i)
                {
                    if (evaluator[i].score == max) {
                        nbKeep ++;
                        evaluator[i].keep = true;
                    }
                    else evaluator[i].keep = false;
                }

                if (nbKeep > 0) {
                    int tmp = 1 + ((random() / 7) % nbKeep);

                    for (int i=PUYODIMX*4-3; i>=0; --i)
                    {
                        if (evaluator[i].keep)
                        {
                            if (tmp == 1)
                            {
                                /*fprintf(stderr,"\nchoice : %d/%d (%d)\n",i,nbKeep,evaluator[i].score);*/
                                choosenMove = i;
                                break;
                            }
                            tmp--;
                        }
                    }
                }
            }
        }
    } 
}
