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
SDL_Surface *puyoEyesSwirl[4];

SDL_Surface *puyoCircle[32];

SDL_Surface *puyoShadow;
SDL_Surface *puyoFaces[5][16];
SDL_Surface *live[4];

static char *AI_NAMES[] = { "Fanzy", "Bob the Killer", "Big Rabbit", "Flying Saucer",
  "Satanas", "Doctor X", "Tanya", "Master Gyom, King of the Puyos",
  "X","Y","Z" };


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
        SDL_Rect drect, drect2;
        drect.w = targetSurface->w;
        drect.h = targetSurface->h;
        drect.y = -semiMove * TSIZE / 2;
        switch (companionVector) {
            case 0:
                drect.x = (short)(X - offsetB);
                drect.y += (short)(Y + offsetA - TSIZE);
                break;
            case 1:
                drect.x = (short)(X - offsetA + TSIZE);
                drect.y += (short)(Y - offsetB);
                break;
            case 2:
                drect.x = (short)(X + offsetB);
                drect.y += (short)(Y - offsetA + TSIZE);
                break;
            case 3:
                drect.x = (short)(X + offsetA - TSIZE);
                drect.y += (short)(Y + offsetB);
                break;
                
            case -3:
                drect.x = (short)(X + offsetB);
                drect.y += (short)(Y + offsetA - TSIZE);
                break;
        }
        drect2 = drect;
        painter.requestDraw(targetSurface, &drect);
        painter.requestDraw(puyoEyes, &drect2);
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
			if (puyoFace != neutral) painter.requestDraw(puyoEyesSwirl[(bouncing/2)%4], &drect);
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
                if (puyoFace != neutral) painter.requestDraw(puyoEyes, &drect);
                
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
    static unsigned int smallTicksCount = 0;

    int mainX = attachedGame->getFallingX();
    int mainY = attachedGame->getFallingY();
    bool falling  = attachedGame->getFallingState() < PUYO_EMPTY;
    
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
            
            /* Main puyo show */
            if (falling && (mainX == i) && (mainY == j))
                painter.requestDraw(puyoCircle[(smallTicksCount++ >> 2) & 0x1F], &drect);
            
            if (currentSurface != neutral) {
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
    for (int i = 0 ; i < PUYODIMX ; i++) {
        for (int j = 0 ; j < PUYODIMY ; j++) {
            puyoEyeState[i][j]++;
            PuyoPuyo *currentPuyo = attachedGame->getPuyoAt(i, j);
            if ((currentPuyo != NULL) && (getSurfaceForPuyo(currentPuyo) != neutral) && (currentPuyo->getAttachedObject() == NULL)) {
                drect.x = xOffset + i * TSIZE;
                drect.y = yOffset + j * TSIZE;
                if (currentPuyo->getPuyoState() < PUYO_EMPTY)
                    drect.y -= attachedGame->getSemiMove() * TSIZE / 2;
                drect.w = puyoShadow->w;
                drect.h = puyoShadow->h;
                if (drect.y + drect.h > vrect.y + vrect.h)
					drect.h -= (drect.y + drect.h - vrect.y - vrect.h);
                if (drect.x + drect.w > vrect.x + vrect.w)
					drect.w -= (drect.x + drect.w - vrect.x - vrect.w);
                painter.requestDraw(puyoShadow, &drect);
            }
        }
    }
    
	for (int i = 0, j = attachedGame->getPuyoCount() ; i < j ; i++) {
        PuyoPuyo *currentPuyo = attachedGame->getPuyoAtIndex(i);
        render(currentPuyo);
    }
    
	drect.x = nXOffset;
	drect.y = nYOffset;
	drect.w = TSIZE;
	drect.h = TSIZE * 2;
	// Drawing next puyos
	SDL_Surface *currentSurface = getSurfaceForState(attachedGame->getNextFalling());
	if (currentSurface != NULL) {
		drect.x = nXOffset;
		drect.y = nYOffset + TSIZE;
		drect.w = currentSurface->w;
		drect.h = currentSurface->h;
		painter.requestDraw(currentSurface, &drect);
		if (currentSurface != neutral) painter.requestDraw(puyoEyes, &drect);
	}
	currentSurface = getSurfaceForState(attachedGame->getNextCompanion());
	if (currentSurface != NULL) {
		drect.x = nXOffset;
		drect.y = nYOffset;
		drect.w = currentSurface->w;
		drect.h = currentSurface->h;
		painter.requestDraw(currentSurface, &drect);
		if (currentSurface != neutral) painter.requestDraw(puyoEyes, &drect);
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

void PuyoStarter::stopRender()
{
	this->stopRendering = true;
	iim_surface_convert_to_gray(painter.gameScreen);
}

void PuyoStarter::restartRender()
{
	this->stopRendering = false;
	painter.redrawAll();
}


extern Menu *menu_pause;
static char *BACKGROUND[NB_MUSIC_THEME] = { "Background.jpg", "BackgroundDark.jpg" };

PuyoStarter::PuyoStarter(PuyoCommander *commander, bool aiLeft, int aiLevel, IA_Type aiType, int theme)
{
	this->stopRendering = false;
	this->paused = false;
	tickCounts = 0;
	this->commander = commander;
	
	background    = IMG_Load_DisplayFormat(BACKGROUND[theme]);
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
	painter.display = display;
	painter.redrawAll(painter.gameScreen);

	static bool firstTime = true;
	if (firstTime) {
		fallingViolet = IMG_Load_DisplayFormatAlpha("v0.png");
		fallingRed   = iim_surface_shift_hue(fallingViolet, 100.0f);
		fallingBlue = iim_surface_shift_hue(fallingViolet, -65.0f);
		fallingGreen    = iim_surface_shift_hue(fallingViolet, -150.0f);
		fallingYellow  = iim_surface_shift_hue(fallingViolet, 140.0f);
		neutral       = IMG_Load_DisplayFormatAlpha("Neutral.png");
		bigNeutral    = IMG_Load_DisplayFormatAlpha("BigNeutral.png");
		
                puyoCircle[0] = IMG_Load_DisplayFormatAlpha("circle.png");
		for (int i = 1 ; i < 32 ; i++) puyoCircle[i] = iim_surface_shift_hue(puyoCircle[i-1],11.25f);
                    
        loadShrinkXplode();
        
        puyoShadow = IMG_Load_DisplayFormatAlpha("Shadow.png");
        puyoEye[0] = IMG_Load_DisplayFormatAlpha("eye0.png");
        puyoEye[1] = IMG_Load_DisplayFormatAlpha("eye1.png");
        puyoEye[2] = IMG_Load_DisplayFormatAlpha("eye2.png");
        puyoEyes = puyoEye[0];
        puyoEyesSwirl[0] = IMG_Load_DisplayFormatAlpha("twirleye0.png");
        puyoEyesSwirl[1] = IMG_Load_DisplayFormatAlpha("twirleye1.png");
        puyoEyesSwirl[2] = IMG_Load_DisplayFormatAlpha("twirleye2.png");
        puyoEyesSwirl[3] = IMG_Load_DisplayFormatAlpha("twirleye3.png");
        
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
		
		live[0] = IMG_Load_DisplayFormatAlpha("0live.png");
		live[1] = IMG_Load_DisplayFormatAlpha("1live.png");
		live[2] = IMG_Load_DisplayFormatAlpha("2live.png");
		live[3] = IMG_Load_DisplayFormatAlpha("3live.png");
		
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
		firstTime = false;
	}
    
	if (fallingBlue == NULL)
	{
		fprintf(stderr, "IMG_Load error:%s\n", SDL_GetError());
		exit(-1);
	}
	attachedGameA = new PuyoGame(&attachedRandom);
	attachedGameB = new PuyoGame(&attachedRandom);
	
	if (aiLeft) {
		randomPlayer = new PuyoIA(aiType, aiLevel, attachedGameA);
		perso[0] = IMG_Load_DisplayFormatAlpha("perso1_1.png");
		perso[1] = IMG_Load_DisplayFormatAlpha("perso1_2.png");
	}
	else {
		randomPlayer = 0;
		perso[0] = NULL;
	}
	
	areaA = new PuyoView(attachedGameA, 1 + CSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + FSIZE, BSIZE+ESIZE);
	areaB = new PuyoView(attachedGameB, 1 + CSIZE + PUYODIMX*TSIZE + DSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + DSIZE - FSIZE - TSIZE, BSIZE+ESIZE);
	
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
	//	SDL_FreeSurface(fallingBlue);
}


#define FPKEY_REPEAT        8
#define FPKEY_DELAY         5

#define FPKEY_keyNumber     10

#define FPKEY_P1_Down       0
#define FPKEY_P1_Left       1
#define FPKEY_P1_Right      2
#define FPKEY_P1_TurnLeft   3
#define FPKEY_P1_TurnRight  4
#define FPKEY_P2_Down       5
#define FPKEY_P2_Left       6
#define FPKEY_P2_Right      7
#define FPKEY_P2_TurnLeft   8
#define FPKEY_P2_TurnRight  9

#define repeatCondition(A) keysDown[A]++; if (((keysDown[A]-FPKEY_DELAY)>0) && ((keysDown[A]-FPKEY_DELAY)%FPKEY_REPEAT == 0)) 


void PuyoStarter::run(int score1, int score2, int lives)
{
	this->lives = lives;
  this->score1 = score1;
  this->score2 = score2;
	SDL_Rect drect;
	SDL_Event event;
	int quit = 0;
	SDL_EnableUNICODE(1);
	int keysDown[FPKEY_keyNumber] = {0,0,0,0,0,0,0,0,0,0};
	
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
		bool left_danger = (attachedGameA->getMaxColumnHeight() > PUYODIMY - 5);
		bool right_danger = (attachedGameB->getMaxColumnHeight() > PUYODIMY - 5);
		bool danger = left_danger || right_danger;
		bool gameover = (!attachedGameA->isGameRunning() || !attachedGameB->isGameRunning());
		
		/*if (paused)
			audio_music_start(0);
		else */if (gameover)
			audio_music_start(3);
		else if (danger)
			audio_music_start(2);
		else
			audio_music_start(1);
		
		if (left_danger)
			currentPerso = 0;
		else
			currentPerso = 1;
		
    while (SDL_PollEvent(&event) == 1)
    {
      if (attachedGameA->isGameRunning() &&
          attachedGameB->isGameRunning()) {
        if (!paused) {

          GameControlEvent controlEvent;
          getControlEvent(event, &controlEvent);

          if (controlEvent.isUp)
          {
            switch (controlEvent.gameEvent) {
              case GameControlEvent::kPlayer1Down:
                if (randomPlayer == 0) keysDown[FPKEY_P1_Down] = 0;
                break;
              case GameControlEvent::kPlayer1Left:
                if (randomPlayer == 0) keysDown[FPKEY_P1_Left] = 0;
                break;
              case GameControlEvent::kPlayer1Right:
                if (randomPlayer == 0) keysDown[FPKEY_P1_Right] = 0;
                break;
              case GameControlEvent::kPlayer1TurnLeft:
                if (randomPlayer == 0) keysDown[FPKEY_P1_TurnLeft] = 0;
                break;
              case GameControlEvent::kPlayer1TurnRight:
                if (randomPlayer == 0) keysDown[FPKEY_P1_TurnRight] = 0;
                break;

              case GameControlEvent::kPlayer2Down:
                keysDown[FPKEY_P2_Down] = 0;
                break;
              case GameControlEvent::kPlayer2Left:
                keysDown[FPKEY_P2_Left] = 0;
                break;
              case GameControlEvent::kPlayer2Right:
                keysDown[FPKEY_P2_Right] = 0;
                break;
              case GameControlEvent::kPlayer2TurnLeft:
                keysDown[FPKEY_P2_TurnLeft] = 0;
                break;
              case GameControlEvent::kPlayer2TurnRight:
                keysDown[FPKEY_P2_TurnRight] = 0;
                break;
            }
          }
          else
            switch (controlEvent.gameEvent) {
              case GameControlEvent::kPlayer1Left:
                if (randomPlayer == 0) {
                  attachedGameA->moveLeft();
                  if (attachedGameA->isEndOfCycle()) keysDown[FPKEY_P1_Left] = 0;
                  else keysDown[FPKEY_P1_Left]++; 
                }
                break;
              case GameControlEvent::kPlayer1Right:
                if (randomPlayer == 0) {
                  attachedGameA->moveRight();
                  if (attachedGameA->isEndOfCycle()) keysDown[FPKEY_P1_Right] = 0;
                  else keysDown[FPKEY_P1_Right]++; 
                }
                break;
              case GameControlEvent::kPlayer1TurnLeft:
                if (randomPlayer == 0) {
                  attachedGameA->rotateLeft();
                  if (attachedGameA->isEndOfCycle()) keysDown[FPKEY_P1_TurnLeft] = 0;
                  else keysDown[FPKEY_P1_TurnLeft]++; 
                }
                break;
              case GameControlEvent::kPlayer1TurnRight:
                if (randomPlayer == 0) {
                  attachedGameA->rotateRight();
                  if (attachedGameA->isEndOfCycle()) keysDown[FPKEY_P1_TurnRight] = 0;
                  else keysDown[FPKEY_P1_TurnRight]++;
                }
                break;
              case GameControlEvent::kPlayer1Down:
                if (randomPlayer == 0) {
                  attachedGameA->cycle();
                  if (attachedGameA->isEndOfCycle()) keysDown[FPKEY_P1_Down] = 0;
                  else keysDown[FPKEY_P1_Down]++;
                }
                break;

              case GameControlEvent::kPlayer2Left:
                attachedGameB->moveLeft();
                if (attachedGameB->isEndOfCycle()) keysDown[FPKEY_P2_Left] = 0;
                else keysDown[FPKEY_P2_Left]++;
                break;
              case GameControlEvent::kPlayer2Right:
                attachedGameB->moveRight();
                if (attachedGameB->isEndOfCycle()) keysDown[FPKEY_P2_Right] = 0;
                else keysDown[FPKEY_P2_Right]++;
                break;
              case GameControlEvent::kPlayer2TurnLeft:
                attachedGameB->rotateLeft();
                if (attachedGameB->isEndOfCycle()) keysDown[FPKEY_P2_TurnLeft] = 0;
                else keysDown[FPKEY_P2_TurnLeft]++;
                break;
              case GameControlEvent::kPlayer2TurnRight:
                attachedGameB->rotateRight();
                if (attachedGameB->isEndOfCycle()) keysDown[FPKEY_P2_TurnRight] = 0;
                else keysDown[FPKEY_P2_TurnRight]++;
                break;
              case GameControlEvent::kPlayer2Down:
                attachedGameB->cycle();
                if (attachedGameB->isEndOfCycle()) keysDown[FPKEY_P2_Down] = 0;
                else keysDown[FPKEY_P2_Down]++;
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
                if (attachedGameA->isEndOfCycle()) {
                  keysDown[FPKEY_P1_Down] = 0;
                  keysDown[FPKEY_P1_Left] = 0;
                  keysDown[FPKEY_P1_Right] = 0;
                  keysDown[FPKEY_P1_TurnLeft] = 0;
                  keysDown[FPKEY_P1_TurnRight] = 0;
                }
                attachedGameB->cycle();
                if (attachedGameB->isEndOfCycle()) {
                  keysDown[FPKEY_P2_Down] = 0;
                  keysDown[FPKEY_P2_Left] = 0;
                  keysDown[FPKEY_P2_Right] = 0;
                  keysDown[FPKEY_P2_TurnLeft] = 0;
                  keysDown[FPKEY_P2_TurnRight] = 0;
                }
              } else {
                if (keysDown[FPKEY_P2_Down]) {
                  attachedGameB->cycle();
                  if (attachedGameB->isEndOfCycle())
                    keysDown[FPKEY_P2_Down] = 0;
                }
                if (keysDown[FPKEY_P2_Left]) {
                  repeatCondition(FPKEY_P2_Left) attachedGameB->moveLeft();
                  if (attachedGameB->isEndOfCycle())
                    keysDown[FPKEY_P2_Left] = 0;
                }
                if (keysDown[FPKEY_P2_Right]) {
                  repeatCondition(FPKEY_P2_Right) attachedGameB->moveRight();
                  if (attachedGameB->isEndOfCycle())
                    keysDown[FPKEY_P2_Right] = 0;
                }
                if (keysDown[FPKEY_P2_TurnLeft]) {
                  repeatCondition(FPKEY_P2_TurnLeft) attachedGameB->rotateLeft();
                  if (attachedGameB->isEndOfCycle())
                    keysDown[FPKEY_P2_TurnLeft] = 0;
                }
                if (keysDown[FPKEY_P2_TurnRight]) {
                  repeatCondition(FPKEY_P2_TurnRight) attachedGameB->rotateRight();
                  if (attachedGameB->isEndOfCycle())
                    keysDown[FPKEY_P2_TurnRight] = 0;
                }

                if (keysDown[FPKEY_P1_Down]) {
                  attachedGameA->cycle();
                  if (attachedGameA->isEndOfCycle())
                    keysDown[FPKEY_P1_Down] = 0;
                }
                if (keysDown[FPKEY_P1_Left]) {
                  repeatCondition(FPKEY_P1_Left) attachedGameA->moveLeft();
                  if (attachedGameA->isEndOfCycle())
                    keysDown[FPKEY_P1_Left] = 0;
                }
                if (keysDown[FPKEY_P1_Right]) {
                  repeatCondition(FPKEY_P1_Right) attachedGameA->moveRight();
                  if (attachedGameA->isEndOfCycle())
                    keysDown[FPKEY_P1_Right] = 0;
                }
                if (keysDown[FPKEY_P1_TurnLeft]) {
                  repeatCondition(FPKEY_P1_TurnLeft) attachedGameA->rotateLeft();
                  if (attachedGameA->isEndOfCycle())
                    keysDown[FPKEY_P1_TurnLeft] = 0;
                }
                if (keysDown[FPKEY_P1_TurnRight]) {
                  repeatCondition(FPKEY_P1_TurnRight) attachedGameA->rotateRight();
                  if (attachedGameA->isEndOfCycle())
                    keysDown[FPKEY_P1_TurnRight] = 0;
                }
              }
              break;
            default:
              break;
          }
        } // !Paused
        else {
          GameControlEvent controlEvent;
          getControlEvent(event, &controlEvent);
          switch (controlEvent.cursorEvent) {
            case GameControlEvent::kUp:
              menu_prev_item(menu_pause);
              break;
            case GameControlEvent::kDown:
              menu_next_item(menu_pause);
              break;
            case GameControlEvent::kStart:
              if (menu_active_is(menu_pause, kContinueGame)) {
                paused = false;
                restartRender();
                menu_hide(menu_pause);
              }
              if (menu_active_is(menu_pause, kAbortGame)) {
                if (menu_active_is(commander->gameOverMenu, "YES"))
                  menu_next_item(commander->gameOverMenu);
                quit = 1;
                menu_hide(menu_pause);
              }
              if (menu_active_is(menu_pause, kOptions)) {
                menu_hide (menu_pause);
                commander->optionMenuLoop(this);
                menu_show (menu_pause);
              }
              break;
          }
        }
      } else // Not GameIsRunning
          {
            if (randomPlayer) {
              if (rightPlayerWin()) {
                if (score2 == 7)
                  commander->gameOverMenu = commander->finishedMenu;
                else
                  commander->gameOverMenu = commander->nextLevelMenu;
              }
              else {
                if (lives == 0) {
                  commander->gameOverMenu = commander->gameOver1PMenu;
                }
                else {
                  commander->gameOverMenu = commander->looserMenu;
                }
              }
            }
            else {
              commander->gameOverMenu = commander->gameOver2PMenu;
            }

            if (!menu_visible(commander->gameOverMenu)) {

              if (leftPlayerWin()) score1++;
              else if (rightPlayerWin()) score2++;

              if (commander->gameOverMenu == commander->gameOver2PMenu) {
                char winner[256];
                char score[256];
                sprintf(winner,"%d Wins!!!",(leftPlayerWin()?1:2));
                sprintf(score, "%d - %d", score1, score2);
                menu_set_value(commander->gameOverMenu, kPlayer, winner);
                menu_set_value(commander->gameOverMenu, kScore,  score);
              }
              else if (commander->gameOverMenu == commander->nextLevelMenu) {
                char level[256];
                sprintf(level, "%d, vs. %s", score2+1, AI_NAMES[score2]);
                menu_set_value(commander->gameOverMenu, kNextLevel, level);
              }
              else if (commander->gameOverMenu == commander->looserMenu) {
                char level[256];
                char cont[256];
                sprintf(level, "%d, vs. %s", score2+1, AI_NAMES[score2]);
                sprintf(cont, "%d", lives);
                menu_set_value(commander->gameOverMenu, kCurrentLevel, level);
                menu_set_value(commander->gameOverMenu, kContinueLeft, cont);
              }
              else if (commander->gameOverMenu == commander->gameOver1PMenu) {
                char level[256];
                sprintf(level, "%d (vs. %s)", score2+1, AI_NAMES[score2]);
                menu_set_value(commander->gameOverMenu, kYouGotToLevel, level);
              }
              commander->showGameOver();
              stopRender();
            } // GameOver Visible
          }
      GameControlEvent controlEvent2;
      getControlEvent(event, &controlEvent2);
      switch (controlEvent2.cursorEvent) {
        case GameControlEvent::kStart:
          if (gameover)
          {
            if (menu_active_is(commander->gameOverMenu, "NO"))
              menu_next_item(commander->gameOverMenu);
            quit = 1;
          }
          break;
        case GameControlEvent::kBack:
          if (!gameover) {
            if (!paused) {
              menu_show(menu_pause);
              paused = true;
              stopRender();
            }
            else {
              paused = false;
              restartRender();
              menu_hide(menu_pause);
            }
          }
          break;
        default:
          break;
      }
      if(event.type == SDL_QUIT) {
        if (menu_active_is(commander->gameOverMenu, "YES"))
          menu_next_item(commander->gameOverMenu);
        quit = 1;
        exit(0);
      }
    }
		commander->updateAll(this);
		
		if (!paused) {
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
		
	}
	commander->hideGameOver();
	if (randomPlayer) {
		for (int i=0; i<NB_PERSO_STATE; ++i)
			SDL_FreeSurface(perso[i]);
	}
	SDL_SetClipRect(display,NULL);
}

void PuyoStarter::draw()
{
	if (stopRendering) {
		SDL_BlitSurface(painter.gameScreen,NULL,display,NULL);
	}
	else {
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
		
		if ((randomPlayer)&&(lives>=0)&&(lives<=3))
		{
			drect.x = painter.gameScreen->w / 2 - live[lives]->w / 2;
			drect.y = 436;
			drect.w = live[lives]->w;
			drect.h = live[lives]->h;
			painter.requestDraw(live[lives], &drect);
		}
		
		painter.draw(painter.gameScreen);
	}
  char text[256];
  if (!randomPlayer)
  {
    sprintf(text, "Win %d", score1);
    SoFont_CenteredString_XY (commander->smallFont, display,
                              50, 460,   text, NULL);
    sprintf(text, "Win %d", score2);
    SoFont_CenteredString_XY (commander->smallFont, display,
                              590, 460, text, NULL);
  }
  else
  {
    SoFont *font = (stopRendering?commander->darkFont:commander->menuFont);
    SoFont_CenteredString_XY (font, display,
                              130, 460,   AI_NAMES[score2], NULL);
  }
  sprintf(text, "<< %d", attachedGameA->getPoints());
  SoFont_CenteredString_XY (commander->smallFont, display,
                            300, 380,   text, NULL);
  sprintf(text, "%d >>", attachedGameB->getPoints());
  SoFont_CenteredString_XY (commander->smallFont, display,
                            340, 395, text, NULL);
}
