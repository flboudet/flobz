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
    
    int update(PuyoGame *game, PuyoState f_color, PuyoState c_color, bool deep)
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
        
//#define toto(X,Y) (game->getPuyoAt(X,Y))->getPuyoState()
        
        if ((f_y >= 6) && (c_y >= 6)) {
            score = 
            game->getSamePuyoAround(f_x, f_y, f_color)*10
            + game->getSamePuyoAround(c_x, c_y, c_color)*10;
            if (deep)
            {
		if (c_y < PUYODIMY-1) {
                    if ((c_x<(PUYODIMX - 1)) && (c_color == (game->getPuyoAt(c_x+1,c_y))->getPuyoState())) {
                        PuyoState cColorA = (game->getPuyoAt(c_x+1,c_y+1))->getPuyoState();
                        if ((c_color!=cColorA) && (cColorA>5) && (cColorA<11) && (cColorA == (game->getPuyoAt(c_x+1,c_y-1))->getPuyoState())) {
                            score += game->getSamePuyoAround(c_x+1, c_y, cColorA)*3;
                            //(printf("1 - x %d / y %d - s %d - C %d/%d/%d\n",c_x,c_y,score,c_color,f_color,cColorA));
                            //(printf("  - %2d %2d %2d\n",toto(c_x-1,c_y-1),toto(c_x,c_y-1),toto(c_x+1,c_y-1)));
                            //(printf("  - %2d %2d %2d\n",toto(c_x-1,c_y  ),toto(c_x,c_y  ),toto(c_x+1,c_y)));
                            //(printf("  - %2d %2d %2d\n",toto(c_x-1,c_y+1),toto(c_x,c_y+1),toto(c_x+1,c_y+1)));
                        }
                    }
                    
                    if ((c_x>0) && (c_color == (game->getPuyoAt(c_x-1,c_y))->getPuyoState())) {
                        PuyoState cColorB = (game->getPuyoAt(c_x-1,c_y+1))->getPuyoState();
                        if ((c_color!=cColorB) && (cColorB>5) && (cColorB<11) && (cColorB == (game->getPuyoAt(c_x-1,c_y-1))->getPuyoState())) {
                            score += game->getSamePuyoAround(c_x-1, c_y, cColorB)*3;
                            //(printf("3 - x %d / y %d - s %d - C %d/%d/%d\n",c_x,c_y,score,c_color,f_color,cColorB));
                            //(printf("  - %2d %2d %2d\n",toto(c_x-1,c_y-1),toto(c_x,c_y-1),toto(c_x+1,c_y-1)));
                            //(printf("  - %2d %2d %2d\n",toto(c_x-1,c_y  ),toto(c_x,c_y  ),toto(c_x+1,c_y)));
                            //(printf("  - %2d %2d %2d\n",toto(c_x-1,c_y+1),toto(c_x,c_y+1),toto(c_x+1,c_y+1)));
                        }
                    }
		}
                
		if (f_y < PUYODIMY-1) {
                    if ((f_x<(PUYODIMX - 1)) && (f_color == (game->getPuyoAt(f_x+1,f_y))->getPuyoState())) {
                        PuyoState fColorA = (game->getPuyoAt(f_x+1,f_y+1))->getPuyoState();
                        if ((f_color!=fColorA) && (fColorA>5) && (fColorA<11) && (fColorA == (game->getPuyoAt(f_x+1,f_y-1))->getPuyoState())) {
                            score += game->getSamePuyoAround(f_x+1, f_y, fColorA)*3;
                            //(printf("2 - x %d / y %d - s %d - C %d/%d/%d\n",f_x,f_y,score,f_color,c_color,fColorA));
                            //(printf("  - %2d %2d %2d\n",toto(f_x-1,f_y-1),toto(f_x,f_y-1),toto(f_x+1,f_y-1)));
                            //(printf("  - %2d %2d %2d\n",toto(f_x-1,f_y  ),toto(f_x,f_y  ),toto(f_x+1,f_y)));
                            //(printf("  - %2d %2d %2d\n",toto(f_x-1,f_y+1),toto(f_x,f_y+1),toto(f_x+1,f_y+1)));
                        }
                    }
                    
                    
                    if ((f_x>0) && (f_color == (game->getPuyoAt(f_x-1,f_y))->getPuyoState())) {
                        PuyoState fColorB = (game->getPuyoAt(f_x-1,f_y+1))->getPuyoState();
                        if ((f_color!=fColorB) && (fColorB>5) && (fColorB<11) && (fColorB == (game->getPuyoAt(f_x-1,f_y-1))->getPuyoState())) {
                            score += game->getSamePuyoAround(f_x-1, f_y, fColorB)*3;
                            //(printf("4 - x %d / y %d - s %d - C %d/%d/%d\n",f_x,f_y,score,f_color,c_color,fColorB));
                            //(printf("  - %2d %2d %2d\n",toto(f_x-1,f_y-1),toto(f_x,f_y-1),toto(f_x+1,f_y-1)));
                            //(printf("  - %2d %2d %2d\n",toto(f_x-1,f_y  ),toto(f_x,f_y  ),toto(f_x+1,f_y)));
                            //(printf("  - %2d %2d %2d\n",toto(f_x-1,f_y+1),toto(f_x,f_y+1),toto(f_x+1,f_y+1)));
                        }
                    }
		}
		score += f_y+c_y;
            }
        }
        else if ((f_y >= 0) && (c_y >= 0))
        {
            score = -1000
            + game->getSamePuyoAround(f_x, f_y, f_color)*10
            + game->getSamePuyoAround(c_x, c_y, c_color)*10;
            if (deep) score += f_y+c_y;
	}
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
        case POLLUX:
        {
            if (!firstLine && (attachedGame->getFallingY() == 1) && (choosenMove != -1))
                choosenMove = -1;
            firstLine = (attachedGame->getFallingY() == 1);
            if ((choosenMove >= 0) && (random() % level < 10))
            {
                if (attachedGame->getFallingCompanionDir() != evaluator[choosenMove].c_direction)
                    attachedGame->rotateLeft();
                else if (attachedGame->getFallingX() < evaluator[choosenMove].f_x)
                    attachedGame->moveRight();
                else if (attachedGame->getFallingX() > evaluator[choosenMove].f_x)
                    attachedGame->moveLeft();
                else
                    attachedGame->cycle();
                
                //if ((attachedGame->getFallingY() == 0) && (choosenMove != -1)) choosenMove = -1;
                /*if (PUYODIMY - attachedGame->getColumnHeigth(evaluator[choosenMove].f_x) - 1
                    < attachedGame->getFallingY()) choosenMove = -1;*/
            }
            else if (choosenMove == -1) 
            {
                PuyoState f_color = extractColor(attachedGame->getFallingState());
                PuyoState c_color = extractColor(attachedGame->getCompanionState());
                int max = -100000;
                for (int i=PUYODIMX*4-3; i>=0; --i)
                {
                    if ((f_color == PUYO_EMPTY)||(c_color==PUYO_EMPTY)) return;
                    int val = evaluator[i].update(attachedGame, f_color, c_color, type == POLLUX);
                    if (val > max) max = val;
                }
                
                int nbKeep = 0;
                for (int i=PUYODIMX*4-3; i>=0; --i)
                {
                    if (evaluator[i].score == max) {
                        nbKeep ++;
                        evaluator[i].keep = true;
                        //(printf("!");
                    }
                    else evaluator[i].keep = false;
                    //(printf("s:%d ",evaluator[i].score));
                }
                
                int tmp = 1 + ((random() / 7) % nbKeep);
                for (int i=PUYODIMX*4-3; i>=0; --i)
                {
                    if (evaluator[i].keep)
                    {
                        if (tmp == 1)
                        {
                            //(printf("\nchoice : %d/%d (%d)\n",i,nbKeep,evaluator[i].score));
                            choosenMove = i;
                            break;
                        }
                        tmp--;
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
