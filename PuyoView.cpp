#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "PuyoView.h"
#include "PuyoIA.h"
#include "PuyoGame.h"
#include "audio.h"
#include "IosImgProcess.h"

#include "SDL_Painter.h"
SDL_Painter painter;

SDL_Surface *display, *background, *fallingBlue, *fallingRed, *fallingGreen, *fallingViolet, *fallingYellow, *neutral;
SDL_Surface *bigNeutral;
SDL_Surface *shrinkingPuyo[5][5];
SDL_Surface *explodingPuyo[5][5];
SDL_Surface *grid;
SDL_Surface *puyoEyes;
SDL_Surface *puyoEye[3];

SDL_Surface *puyoShadow;
SDL_Surface *puyoFaces[5][16];

static char PuyoGroupImageIndex[2][2][2][2] =
{ {  // empty bottom
    {  // empty right
      {  // empty top
        0, // empty left
        4, // full left
      },
      {  // full top
        3, // empty left
        10, // full left
      }
    },
    {  // full right
      {  // empty top
        2, // empty left
        9, // full left
      },
      {  // full top
        8, // empty left
        14, // full left
      }
    }
  },
  {  // full bottom
    {  // empty right
      {  // empty top
        1, // empty left
        7, // full left
      },
      {  // full top
        6, // empty left
        13, // full left
      }
    },
    {  // full right
      {  // empty top
        5, // empty left
        12, // full left
      },
      {  // full top
        11, // empty left
        15, // full left
      }
    }
  }
};

static const int NB_PERSO_STATE = 2;
SDL_Surface *perso[2];
int currentPerso = 0;

PuyoAnimation::PuyoAnimation()
{
  finishedFlag = false;
}

bool PuyoAnimation::isFinished()
{
  return finishedFlag;
}

NeutralAnimation::NeutralAnimation(int X, int Y, int xOffset, int yOffset)
{
	this->X = (X*TSIZE) + xOffset;
	this->Y = (Y*TSIZE) + yOffset;
	this->currentY = yOffset;
}

void NeutralAnimation::cycle()
{
	currentY += 16;
	if (currentY >= Y)
		finishedFlag = true;
}

void NeutralAnimation::draw(int semiMove)
{
	SDL_Rect drect;
	drect.x = X;
	drect.y = currentY;
	drect.w = neutral->w;
	drect.h = neutral->h;
  painter.requestDraw(neutral, &drect);
}

class TurningAnimation : public PuyoAnimation {
public:
    TurningAnimation(PuyoPuyo *companionPuyo, int vector, int xOffset, int yOffset, SDL_Surface *companionSurface, bool counterclockwise)
    {
        /*
        switch (vector) {
            case 0:
                X = ((x-1)*TSIZE) + xOffset;
                Y = (y*TSIZE) + yOffset;
                break;
            case 1:
                X = (x*TSIZE) + xOffset;
                Y = ((y-1)*TSIZE) + yOffset;
                break;
            case 2:
                X = ((x+1)*TSIZE) + xOffset;
                Y = (y*TSIZE) + yOffset;
                break;
            case 3:
                X = (x*TSIZE) + xOffset;
                Y = ((y+1)*TSIZE) + yOffset;
                break;
        }
         */
         this->counterclockwise = counterclockwise;
        companionVector = vector;
        targetSurface = companionSurface;
        this->companionPuyo = companionPuyo;
        cpt = 0;
        angle = 0;
        step = (3.14 / 2) / 4;
        this->xOffset = xOffset;
        this->yOffset = yOffset;
    }
    void cycle()
    {
        cpt++;
        angle += step;
        if (cpt == 4)
            finishedFlag = true;
    }
    void draw(int semiMove)
    {
        if (targetSurface == NULL)
            return;
        X = (companionPuyo->getPuyoX()*TSIZE) + xOffset;
        Y = (companionPuyo->getPuyoY()*TSIZE) + yOffset;
        
        float offsetA = sin(angle) * TSIZE;
        float offsetB = cos(angle) * TSIZE * (counterclockwise ? -1 : 1);
        SDL_Rect drect;
        //fprintf(stderr, "%x\n", targetSurface);
        drect.w = targetSurface->w;
        drect.h = targetSurface->h;
        drect.y = -semiMove * TSIZE / 2;
        switch (companionVector) {
            case 0:
                drect.x = (short)(X - offsetB);
                drect.y += (short)(Y + offsetA - TSIZE);
                painter.requestDraw(targetSurface, &drect);
                break;
            case 1:
                drect.x = (short)(X - offsetA + TSIZE);
                drect.y += (short)(Y - offsetB);
                painter.requestDraw(targetSurface, &drect);
                break;
            case 2:
                drect.x = (short)(X + offsetB);
                drect.y += (short)(Y - offsetA + TSIZE);
                painter.requestDraw(targetSurface, &drect);
                break;
            case 3:
                drect.x = (short)(X + offsetA - TSIZE);
                drect.y += (short)(Y + offsetB);
                painter.requestDraw(targetSurface, &drect);
                break;
                
            case -3:
                drect.x = (short)(X + offsetB);
                drect.y += (short)(Y + offsetA - TSIZE);
                break;
        }
    }
private:
    PuyoPuyo *companionPuyo;
    int xOffset, yOffset;
    int X, Y, companionVector, cpt;
    float angle;
    float step;
    SDL_Surface *targetSurface;
    bool counterclockwise;
};

static const int BOUNCING_OFFSET_NUM = 12;
static const int BOUNCING_OFFSET[] = { -1, -3, -5, -4, -2, 0, -6, -9, -11, -9, -6, 0 };

class FallingAnimation : public PuyoAnimation {
public:
    FallingAnimation(PuyoPuyo *puyo, int originY, int xOffset, int yOffset, int step)
    {
        attachedPuyo  = puyo;
        this->xOffset = xOffset;
        this->yOffset = yOffset;
        this->step    = step;
        this->X  = (puyo->getPuyoX()*TSIZE) + xOffset;
        this->Y  = (originY*TSIZE) + yOffset;
        puyoFace = PuyoView::getSurfaceForState(puyo->getPuyoState());
		bouncing = BOUNCING_OFFSET_NUM - 1;
		if (originY == puyo->getPuyoY()) {
			bouncing = -1;
		}
    }
    void cycle()
    {
        Y += step;
        if (Y >= (attachedPuyo->getPuyoY()*TSIZE) + yOffset)
		{
			bouncing--;
			if (bouncing < 0) {
				finishedFlag = true;
        audio_sound_play(sound_bam1);
      }
      else {
        if (BOUNCING_OFFSET[bouncing] == 0)
          audio_sound_play(sound_bam1);
      }
			Y = (attachedPuyo->getPuyoY()*TSIZE) + yOffset;
		}
    }
    void draw(int semiMove)
    {
        if (puyoFace) {
                SDL_Rect drect;
                drect.x = X;
                drect.y = Y + (bouncing>=0?BOUNCING_OFFSET[bouncing]:0);
                // drect.y = -semiMove() * TSIZE / 2;
                drect.w = puyoFace->w;
                drect.h = puyoFace->h;
                painter.requestDraw(puyoFace, &drect);
                painter.requestDraw(puyoEyes, &drect);
        }
    }
private:
    PuyoPuyo *attachedPuyo;
    int xOffset, yOffset, step;
    int X, Y;
	int bouncing;
    SDL_Surface *puyoFace;
};

class VanishAnimation : public PuyoAnimation {
public:
    VanishAnimation(PuyoPuyo *puyo, int xOffset, int yOffset)
    {
        puyoFace = PuyoView::getSurfaceForState(puyo->getPuyoState());
        this->xOffset = xOffset;
        this->yOffset = yOffset;
        this->X = (puyo->getPuyoX()*TSIZE) + xOffset;
        this->Y = (puyo->getPuyoY()*TSIZE) + yOffset;
        this->color = puyo->getPuyoState();
        if (color > PUYO_EMPTY)
          color -= PUYO_BLUE;
        iter = 0;
    }
    void cycle()
    {
        iter ++;
        if (iter == 50)
            finishedFlag = true;
    }
    void draw(int semiMove)
    {
        if (iter < 10) {
            if (puyoFace && (iter % 2 == 0)) {
                SDL_Rect drect;
                drect.x = X;
                drect.y = Y;
                drect.w = puyoFace->w;
                drect.h = puyoFace->h;
                painter.requestDraw(puyoFace, &drect);
                painter.requestDraw(puyoEyes, &drect);
            }
        }
        else {
            if (puyoFace) {
                SDL_Rect drect, xrect;
                drect.x = X;
                drect.y = Y;// + (2.5 * pow(iter - 16, 2) - 108);
                drect.w = puyoFace->w;
                drect.h = puyoFace->h;
                int iter2 = iter - 10;
                int shrinkingImage = (iter - 10) / 4;
                if (shrinkingImage < 4) {
                    painter.requestDraw(shrinkingPuyo[shrinkingImage][color], &drect);
                    int xrectY = Y + (int)(2.5 * pow(iter - 16, 2) - 108);
                    xrect.w = explodingPuyo[shrinkingImage][color]->w;
                    xrect.h = explodingPuyo[shrinkingImage][color]->h;
                    xrect.x = X - iter2 * iter2;
                    xrect.y = xrectY;
                    painter.requestDraw(explodingPuyo[shrinkingImage][color], &xrect);
                    xrect.x = X - iter2;
                    xrect.y = xrectY + iter2;
                    painter.requestDraw(explodingPuyo[shrinkingImage][color], &xrect);
                    xrect.x = X + iter2;
                    xrect.y = xrectY + iter2;
                    painter.requestDraw(explodingPuyo[shrinkingImage][color], &xrect);
                    xrect.x = X + iter2 * iter2;
                    xrect.y = xrectY;
                    painter.requestDraw(explodingPuyo[shrinkingImage][color], &xrect);
                }
            }
        }
    }
private:
    SDL_Surface *puyoFace;
    int xOffset, yOffset;
    int X, Y, iter, color;
};

PuyoView::PuyoView(PuyoGame *attachedGame, int xOffset, int yOffset, int nXOffset, int nYOffset)
{
	this->attachedGame = attachedGame;
	this->xOffset = xOffset;
	this->yOffset = yOffset - TSIZE;
	this->nXOffset = nXOffset;
	this->nYOffset = nYOffset;
	gameRunning = true;
	enemyGame = NULL;
	for (int i = 0 ; i < PUYODIMX ; i++)
	  for (int j = 0 ; j < PUYODIMY ; j++) {
	    animationBoard[i][j] = NULL;
        puyoEyeState[i][j] = (i+1) * (j+1) * 1234;
      }
}

void PuyoView::setEnemyGame(PuyoGame *enemyGame)
{
	this->enemyGame = enemyGame;
}

SDL_Surface *PuyoView::getSurfaceForState(PuyoState state)
{
	switch (state) {
		case PUYO_BLUE:
			return fallingBlue;
			break;
		case PUYO_RED:
			return fallingRed;
			break;
		case PUYO_GREEN:
			return fallingGreen;
			break;
		case PUYO_VIOLET:
			return fallingViolet;
			break;
		case PUYO_YELLOW:
			return fallingYellow;
			break;
		case PUYO_FALLINGBLUE:
			return fallingBlue;
			break;
		case PUYO_FALLINGRED:
			return fallingRed;
			break;
		case PUYO_FALLINGGREEN:
			return fallingGreen;
			break;
		case PUYO_FALLINGVIOLET:
			return fallingViolet;
			break;
		case PUYO_FALLINGYELLOW:
			return fallingYellow;
			break;
		case PUYO_NEUTRAL:
			return neutral;
			break;
		default:
			return NULL;
			break;
	}
}

SDL_Surface *PuyoView::getSurfaceForPuyo(PuyoPuyo *puyo)
{
    int i = puyo->getPuyoX();
    int j = puyo->getPuyoY();
    PuyoState currentPuyo = puyo->getPuyoState();
    int currentIndex = PuyoGroupImageIndex
      [attachedGame->getPuyoCellAt(i, j+1) == currentPuyo ? 1 : 0]
      [attachedGame->getPuyoCellAt(i+1, j) == currentPuyo ? 1 : 0]
      [attachedGame->getPuyoCellAt(i, j-1) == currentPuyo ? 1 : 0]
      [attachedGame->getPuyoCellAt(i-1, j) == currentPuyo ? 1 : 0];
    switch (currentPuyo) {
    case PUYO_VIOLET:
      return puyoFaces[0][currentIndex];
    case PUYO_RED:
      return puyoFaces[1][currentIndex];
    case PUYO_BLUE:
      return puyoFaces[2][currentIndex];
    case PUYO_GREEN:
      return puyoFaces[3][currentIndex];
    case PUYO_YELLOW:
      return puyoFaces[4][currentIndex];
    default:
      return getSurfaceForState(currentPuyo);
    }
}

void PuyoView::render(PuyoPuyo *puyo)
{
    SDL_Rect drect;
    int i = puyo->getPuyoX();
    int j = puyo->getPuyoY();
    PuyoAnimation *animation = (PuyoAnimation *)(puyo->getAttachedObject());
    if (animation == NULL) {
        SDL_Surface *currentSurface = getSurfaceForPuyo(puyo);
        if (currentSurface != NULL) {
            drect.x = (i*TSIZE) + xOffset;
            drect.y = (j*TSIZE) + yOffset;
            if (puyo->getPuyoState() < PUYO_EMPTY)
                drect.y -= attachedGame->getSemiMove() * TSIZE / 2;
            drect.w = currentSurface->w;
            drect.h = currentSurface->h;
            painter.requestDraw(currentSurface, &drect);
            
            while (puyoEyeState[i][j] >= 750) puyoEyeState[i][j] -= 750;
            int eyePhase = puyoEyeState[i][j];
            if (eyePhase < 5) 
                painter.requestDraw(puyoEye[1], &drect);
            else if (eyePhase < 15) 
                painter.requestDraw(puyoEye[2], &drect);
            else if (eyePhase < 20) 
                painter.requestDraw(puyoEye[1], &drect);
            else
                painter.requestDraw(puyoEye[0], &drect);
        }
    }
    else {
        if (animation->isFinished()) {
            delete animation;
            puyo->setAttachedObject(NULL);
            render(puyo);
        } else {
            animation->draw(attachedGame->getSemiMove());
        }
    }
}

void PuyoView::cycleAnimation(PuyoPuyo *puyo)
{
  PuyoAnimation *animation = (PuyoAnimation *)(puyo->getAttachedObject());
  if ((animation != NULL) && (!animation->isFinished()))
    animation->cycle();
}

void PuyoView::cycleAnimation()
{
  for (int i = 0, j = attachedGame->getPuyoCount() ; i < j ; i++) {
    PuyoPuyo *currentPuyo = attachedGame->getPuyoAtIndex(i);
    cycleAnimation(currentPuyo);
  }
}

void PuyoView::render()
{
	SDL_Rect drect;
    SDL_Rect vrect;
	vrect.x = xOffset;
	vrect.y = yOffset;
	vrect.w = TSIZE * PUYODIMX;
	vrect.h = TSIZE * PUYODIMY;
    SDL_SetClipRect(display, &vrect);
    for (int i = 0 ; i < PUYODIMX ; i++) {
        for (int j = 0 ; j < PUYODIMY ; j++) {
            puyoEyeState[i][j]++;
            PuyoPuyo *currentPuyo = attachedGame->getPuyoAt(i, j);
            if ((currentPuyo != NULL) && (currentPuyo->getAttachedObject() == NULL)) {
                drect.x = xOffset + i * TSIZE + 10;
                drect.y = yOffset + j * TSIZE + 10;
                if (currentPuyo->getPuyoState() < PUYO_EMPTY)
                    drect.y -= attachedGame->getSemiMove() * TSIZE / 2;
                drect.w = puyoShadow->w;
                drect.h = puyoShadow->h;
                painter.requestDraw(puyoShadow, &drect);
            }
        }
    }
    SDL_SetClipRect(display, NULL);
    
	for (int i = 0, j = attachedGame->getPuyoCount() ; i < j ; i++) {
        PuyoPuyo *currentPuyo = attachedGame->getPuyoAtIndex(i);
        render(currentPuyo);
    }
    
	drect.x = nXOffset;
	drect.y = nYOffset;
	drect.w = TSIZE;
	drect.h = TSIZE * 2;
	//SDL_FillRect(display, &drect, 1000);
    // Drawing next puyos
	SDL_Surface *currentSurface = getSurfaceForState(attachedGame->getNextFalling());
	if (currentSurface != NULL) {
		drect.x = nXOffset;
		drect.y = nYOffset;
		drect.w = currentSurface->w;
		drect.h = currentSurface->h;
		painter.requestDraw(currentSurface, &drect);
    painter.requestDraw(puyoEyes, &drect);
	}
	currentSurface = getSurfaceForState(attachedGame->getNextCompanion());
	if (currentSurface != NULL) {
		drect.x = nXOffset;
		drect.y = nYOffset + TSIZE;
		drect.w = currentSurface->w;
		drect.h = currentSurface->h;
		painter.requestDraw(currentSurface, &drect);
    painter.requestDraw(puyoEyes, &drect);
	}
}

void PuyoView::renderNeutral()
{
  SDL_Rect drect;

  // Drawing neutral puyos
  int numBigNeutral = attachedGame->getNeutralPuyos() / PUYODIMX;
  int numNeutral = attachedGame->getNeutralPuyos() % PUYODIMX;
  int drect_x = xOffset;
  int compressor = 0;

  int width = numBigNeutral * bigNeutral->w + numNeutral * neutral->w;
  if (width > TSIZE * PUYODIMX) {
    compressor = (width - TSIZE * PUYODIMX) / (numNeutral + numBigNeutral);
  } 

  for (int cpt = 0 ; cpt < numBigNeutral ; cpt++) {
    drect.x = drect_x;
    drect.y = yOffset + 3 + TSIZE;
    drect.w = bigNeutral->w;
    drect.h = bigNeutral->h;
    painter.requestDraw(bigNeutral, &drect);
    drect_x += bigNeutral->w - compressor;
  }
  for (int cpt = 0 ; cpt < numNeutral ; cpt++) {
    drect.x = drect_x;
    drect.y = yOffset + 3 + TSIZE;
    drect.w = neutral->w;
    drect.h = neutral->h;
    painter.requestDraw(neutral, &drect);
    drect_x += neutral->w - compressor;
  }
}

void PuyoView::gameDidAddNeutral(PuyoPuyo *neutralPuyo) {
    int x = neutralPuyo->getPuyoX();
    int y = neutralPuyo->getPuyoY();
	neutralPuyo->setAttachedObject(new NeutralAnimation(x, y, xOffset, yOffset));
}

void PuyoView::companionDidTurn(PuyoPuyo *companionPuyo, int companionVector, bool counterclockwise)
{
    PuyoAnimation *animation = (PuyoAnimation *)(companionPuyo->getAttachedObject());
    if (animation != NULL)
        delete animation;
    companionPuyo->setAttachedObject(new TurningAnimation(companionPuyo, companionVector,
                                                          xOffset, yOffset,
                                                          getSurfaceForState(companionPuyo->getPuyoState()), counterclockwise));
}

void PuyoView::puyoDidFall(PuyoPuyo *puyo, int originX, int originY)
{
    PuyoAnimation *animation = (PuyoAnimation *)(puyo->getAttachedObject());
    if (animation != NULL)
        delete animation;
    puyo->setAttachedObject(new FallingAnimation(puyo, originY, xOffset, yOffset, 16));
}

void PuyoView::puyoWillVanish(PuyoPuyo *puyo)
{
    PuyoAnimation *animation = (PuyoAnimation *)(puyo->getAttachedObject());
    if (animation != NULL)
        delete animation;
    puyo->setAttachedObject(new VanishAnimation(puyo, xOffset, yOffset));
}

void PuyoView::gameDidEndCycle()
{
	if (enemyGame != NULL) {
		if (attachedGame->getNeutralPuyos() < 0)
			enemyGame->increaseNeutralPuyos(- attachedGame->getNeutralPuyos());
	}
}

void PuyoView::gameLost()
{
	gameRunning = false;
}

void PuyoView::scheduleAnimations(int tickCount)
{
  //	render();
#ifdef PLUSDEDRAW
	for (int i = 0 ; i < PUYODIMX ; i++) {
		for (int j = 0 ; j < PUYODIMY ; j++) {
			if (animationBoard[i][j] != NULL) {
				if (animationBoard[i][j]->isFinished()) {
					delete animationBoard[i][j];
					animationBoard[i][j] = NULL;
                    //render(i, j);
				} else {
					animationBoard[i][j]->draw();
					animationBoard[i][j]->cycle();
				}
			}
		}
	}
#endif
}
        

static void loadShrinkXplode2(int i, float dec)
{
    for (int j=1;j<=4;++j)
    {
        shrinkingPuyo[j-1][i] = iim_surface_shift_hue(shrinkingPuyo[j-1][3],dec);
        explodingPuyo[j-1][i] = iim_surface_shift_hue(explodingPuyo[j-1][3],dec);
    }
}


static void loadShrinkXplode(void)
{
    for (int j=1;j<=4;++j)
    {
        char f[20];
        sprintf(f,"Shrink%d.png", j);
        shrinkingPuyo[j-1][3] = IMG_Load_DisplayFormatAlpha(f);
        sprintf(f,"Explode%d.png", j);
        explodingPuyo[j-1][3] = IMG_Load_DisplayFormatAlpha(f);
    }
    
    loadShrinkXplode2(0,-65.0f);
    loadShrinkXplode2(1,100.0f);
    loadShrinkXplode2(2,-150.0f);
    loadShrinkXplode2(4,140.0f);
}


PuyoStarter::PuyoStarter(PuyoCommander *commander, bool aiLeft, int aiLevel)
{
  tickCounts = 0;
  this->commander = commander;

	background    = IMG_Load_DisplayFormat("Background.jpg");
	painter.backGround = background;
	if (painter.gameScreen == NULL)
	{
    SDL_PixelFormat *fmt = background->format;
    SDL_Surface *tmp = SDL_CreateRGBSurface(background->flags,
        background->w, background->h, 32,
        fmt->Rmask, fmt->Gmask,
        fmt->Bmask, fmt->Amask);
    painter.gameScreen = SDL_DisplayFormat(tmp);
    SDL_FreeSurface(tmp);
  }
	
  fallingViolet = IMG_Load_DisplayFormatAlpha("v0.png");
  fallingRed   = iim_surface_shift_hue(fallingViolet, 100.0f);
  fallingBlue = iim_surface_shift_hue(fallingViolet, -65.0f);
  fallingGreen    = iim_surface_shift_hue(fallingViolet, -150.0f);
  fallingYellow  = iim_surface_shift_hue(fallingViolet, 140.0f);
  neutral       = IMG_Load_DisplayFormatAlpha("Neutral.png");
  bigNeutral    = IMG_Load_DisplayFormatAlpha("BigNeutral.png");

        loadShrinkXplode();
        
        puyoShadow = IMG_Load_DisplayFormatAlpha("Shadow.png");
        puyoEye[0] = IMG_Load_DisplayFormatAlpha("eye0.png");
        puyoEye[1] = IMG_Load_DisplayFormatAlpha("eye1.png");
        puyoEye[2] = IMG_Load_DisplayFormatAlpha("eye2.png");
        puyoEyes = puyoEye[0];
        
        puyoFaces[0][0] = IMG_Load_DisplayFormatAlpha("v0.png");
        puyoFaces[0][1] = IMG_Load_DisplayFormatAlpha("v1a.png");
        puyoFaces[0][2] = IMG_Load_DisplayFormatAlpha("v1b.png");
        puyoFaces[0][3] = IMG_Load_DisplayFormatAlpha("v1c.png");
        puyoFaces[0][4] = IMG_Load_DisplayFormatAlpha("v1d.png");
        puyoFaces[0][5] = IMG_Load_DisplayFormatAlpha("v2ab.png");
        puyoFaces[0][6] = IMG_Load_DisplayFormatAlpha("v2ac.png");
        puyoFaces[0][7] = IMG_Load_DisplayFormatAlpha("v2ad.png");
        puyoFaces[0][8] = IMG_Load_DisplayFormatAlpha("v2bc.png");
        puyoFaces[0][9] = IMG_Load_DisplayFormatAlpha("v2bd.png");
        puyoFaces[0][10] = IMG_Load_DisplayFormatAlpha("v2cd.png");
        puyoFaces[0][11] = IMG_Load_DisplayFormatAlpha("v3abc.png");
        puyoFaces[0][12] = IMG_Load_DisplayFormatAlpha("v3abd.png");
        puyoFaces[0][13] = IMG_Load_DisplayFormatAlpha("v3acd.png");
        puyoFaces[0][14] = IMG_Load_DisplayFormatAlpha("v3bcd.png");
        puyoFaces[0][15] = IMG_Load_DisplayFormatAlpha("v4abcd.png");
    
	for (int i = 0 ; i < 16 ; i++) {
	  puyoFaces[1][i] = iim_surface_shift_hue(puyoFaces[0][i], 100);
	}
	for (int i = 0 ; i < 16 ; i++) {
	  puyoFaces[2][i] = iim_surface_shift_hue(puyoFaces[0][i], -65);
	}
	for (int i = 0 ; i < 16 ; i++) {
	  puyoFaces[3][i] = iim_surface_shift_hue(puyoFaces[0][i], -150);
	}
	for (int i = 0 ; i < 16 ; i++) {
	  puyoFaces[4][i] = iim_surface_shift_hue(puyoFaces[0][i], 140);
	}

    grid          = IMG_Load_DisplayFormatAlpha("grid.png");
	if (fallingBlue == NULL)
	{
		fprintf(stderr, "IMG_Load error:%s\n", SDL_GetError());
		exit(-1);
	}
	attachedGameA = new PuyoGame(&attachedRandom);
	attachedGameB = new PuyoGame(&attachedRandom);

  if (aiLeft) {
    randomPlayer = new PuyoIA(POLLUX, aiLevel, attachedGameA);
    perso[0] = IMG_Load_DisplayFormatAlpha("perso1_1.png");
    perso[1] = IMG_Load_DisplayFormatAlpha("perso1_2.png");
  }
  else {
    randomPlayer = 0;
    perso[0] = NULL;
  }

	areaA = new PuyoView(attachedGameA, CSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + FSIZE, BSIZE+ESIZE);
	areaB = new PuyoView(attachedGameB, CSIZE + PUYODIMX*TSIZE + DSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + DSIZE - FSIZE - TSIZE, BSIZE+ESIZE);
        
	attachedGameA->setDelegate(areaA);
	attachedGameB->setDelegate(areaB);

	areaA->setEnemyGame(attachedGameB);
	areaB->setEnemyGame(attachedGameA);
}

PuyoStarter::~PuyoStarter()
{
	delete areaA;
	delete areaB;
	delete attachedGameA;
	delete attachedGameB;
	SDL_FreeSurface(fallingBlue);
}

void PuyoStarter::run(int score1, int score2, int lives)
{
  SDL_Rect drect;
  SDL_Event event;
  int quit = 0;
  SDL_EnableUNICODE( 1 );
  bool downIsDown = false;
  bool downLeftIsDown = false;

  if (!randomPlayer) {
    int sc1=score1,sc2=score2;
    while ((sc1>0)&&(sc2>0)) { sc1--; sc2--; }
    attachedGameA->increaseNeutralPuyos(sc1 * PUYODIMX);
    attachedGameB->increaseNeutralPuyos(sc2 * PUYODIMX);
    attachedGameA->dropNeutrals();
    attachedGameB->dropNeutrals();
    attachedGameA->cycle();
    attachedGameB->cycle();
  }
  while (!quit) {
    bool left_danger = (attachedGameA->getMaxColumnHeight() > PUYODIMY - 4);
    bool right_danger = (attachedGameB->getMaxColumnHeight() > PUYODIMY - 4);
    bool danger = left_danger || right_danger;
    bool gameover = (!attachedGameA->isGameRunning() || !attachedGameB->isGameRunning());
    
    if (gameover)
      audio_music_start(3);
    else if (danger)
      audio_music_start(2);
    else
      audio_music_start(1);

    if (left_danger)
      currentPerso = 0;
    else
      currentPerso = 1;

    while (SDL_PollEvent(&event) == 1) {
      if (attachedGameA->isGameRunning() &&
	  attachedGameB->isGameRunning()) {

    GameControlEvent controlEvent;
    commander->getControlEvent(event, &controlEvent);
    switch (controlEvent.gameEvent) {
        case GameControlEvent::kPlayer1Left:
            attachedGameA->moveLeft();
            break;
        case GameControlEvent::kPlayer1Right:
            attachedGameA->moveRight();
            break;
        case GameControlEvent::kPlayer1TurnLeft:
            attachedGameA->rotateLeft();
            break;
        case GameControlEvent::kPlayer1TurnRight:
            attachedGameA->rotateRight();
            break;
        case GameControlEvent::kPlayer1Down:
            attachedGameA->cycle();
            if (attachedGameA->isEndOfCycle())
                downLeftIsDown = false;
            else
                downLeftIsDown = true;
            break;
        case GameControlEvent::kPlayer1DownUp:
            downLeftIsDown = false;
            break;
        case GameControlEvent::kPlayer2Left:
            attachedGameB->moveLeft();
            break;
        case GameControlEvent::kPlayer2Right:
            attachedGameB->moveRight();
            break;
        case GameControlEvent::kPlayer2TurnLeft:
            attachedGameB->rotateLeft();
            break;
        case GameControlEvent::kPlayer2TurnRight:
            attachedGameB->rotateRight();
            break;
        case GameControlEvent::kPlayer2Down:
            attachedGameB->cycle();
            if (attachedGameB->isEndOfCycle())
                downIsDown = false;
            else
                downIsDown = true;
            break;
        case GameControlEvent::kPlayer2DownUp:
            downIsDown = false;
            break;
        default:
            break;
    }

	switch (event.type) {
	case SDL_USEREVENT:
          if (randomPlayer)
        	  randomPlayer->cycle();
	  if (event.user.code == 1) {
	    drect.x = 0;
	    drect.y = 0;
	    drect.w = 640;
	    drect.h = 480;
	    attachedGameA->cycle();
            if (attachedGameA->isEndOfCycle())
              downLeftIsDown = false;
	    attachedGameB->cycle();
            if (attachedGameB->isEndOfCycle())
              downIsDown = false;
	  } else {
	    if (downIsDown) {
	      attachedGameB->cycle();
              if (attachedGameB->isEndOfCycle())
                downIsDown = false;
            }
	    if (downLeftIsDown) {
	      attachedGameA->cycle();
              if (attachedGameA->isEndOfCycle())
                downLeftIsDown = false;
            }
	  }
	  break;
	default:
	  break;
	}
      } else {
        if (!menu_visible(commander->gameOverMenu)) {
          const char *winner = 0;
          char score[256];
          if (leftPlayerWin())
            if (rightPlayerWin())
              winner = "Draw Game";
            else {
              winner = "Left Player";
              score1 ++;
            }
          else {
            winner = "Right Player";
            score2 ++;
          }
          if (randomPlayer)
            sprintf(score, "Level  : %d  ( %d Lives Left )", score2 + 1, lives);
          else
            sprintf(score, "Score  : %d - %d", score1, score2);
          commander->showGameOver();
          menu_set_value(commander->gameOverMenu, "Winner: ", winner);
          menu_set_value(commander->gameOverMenu, "Current", score);
        }
      }
      switch (event.type) {
      case SDL_KEYDOWN:
	switch (event.key.keysym.sym) {
	case SDLK_y:
	case SDLK_RETURN:
    if (gameover)
    {
      if (menu_active_is(commander->gameOverMenu, "NO"))
        menu_next_item(commander->gameOverMenu);
      quit = 1;
    }
	break;
	case SDLK_n:
    if (!gameover) break;
  case SDLK_ESCAPE:
    if (menu_active_is(commander->gameOverMenu, "YES"))
      menu_next_item(commander->gameOverMenu);
    quit = 1;
	  break;
	default:
	  break;
	}
	break;
      case SDL_QUIT:	/* SDL_QUIT event (window close) */
	quit = 1;
	break;
      default:
	break;
      }
    }
    commander->updateAll(this);

    areaA->cycleAnimation();
    areaB->cycleAnimation();

    tickCounts++;
    event.user.type = SDL_USEREVENT;
    event.user.code = 0;
    SDL_PushEvent(&event);
    if (tickCounts % 20 == 0) { // Vitesse du jeu
      event.user.type = SDL_USEREVENT;
      event.user.code = 1;
      SDL_PushEvent(&event);
    }

  }
  //SDL_RemoveTimer(myTimer);
  commander->hideGameOver();
  if (randomPlayer) {
    for (int i=0; i<NB_PERSO_STATE; ++i)
      SDL_FreeSurface(perso[i]);
  }
}

void PuyoStarter::draw()
{
  SDL_Rect drect;

  areaA->render();
  areaB->render();
  
  drect.x = 21;
  drect.y = -1;
  drect.w = grid->w;
  drect.h = grid->h;
  painter.requestDraw(grid, &drect);
  drect.x = 407;
  drect.y = -1;
  drect.w = grid->w;
  drect.h = grid->h;
  painter.requestDraw(grid, &drect);

  areaA->renderNeutral();
  areaB->renderNeutral();
  if ((randomPlayer)&&(currentPerso>=0))
  {
    drect.x = 320 - perso[currentPerso]->w/2;
    drect.y = 280 - perso[currentPerso]->h/2;
    drect.w = perso[currentPerso]->w;
    drect.h = perso[currentPerso]->h;
    painter.requestDraw(perso[currentPerso], &drect);
  }
  painter.draw();
  SDL_BlitSurface(painter.gameScreen, NULL, display, NULL);
}
