#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "PuyoView.h"
#include "PuyoAnimations.h"
#include "AnimatedPuyo.h"
#include "PuyoIA.h"
#include "PuyoGame.h"
#include "audio.h"
#include "IosImgProcess.h"
#include "HiScores.h"
#include "PuyoDoomMelt.h"

#include "SDL_Painter.h"
SDL_Painter painter;

SDL_Surface *display;
IIM_Surface *background, *fallingBlue, *fallingRed, *fallingGreen, *fallingViolet, *fallingYellow, *neutral;
IIM_Surface *gameScreen;
IIM_Surface *bigNeutral;
IIM_Surface *shrinkingPuyo[5][5];
IIM_Surface *explodingPuyo[5][5];
IIM_Surface *grid;
IIM_Surface *speedImg;
IIM_Surface *speedBlackImg;
IIM_Surface *puyoEyes;
IIM_Surface *puyoEye[3];
IIM_Surface *puyoEyesSwirl[4];

IIM_Surface *puyoCircle[32];

IIM_Surface *puyoShadow;
IIM_Surface *puyoFaces[5][16];
IIM_Surface *live[4];

const char *p1name;
const char *p2name;

int GAME_ACCEL = 1250;
int gameLevel;

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
IIM_Surface *perso[2];
int currentPerso = 0;


PuyoView::PuyoView(PuyoRandomSystem *attachedRandom, int xOffset, int yOffset, int nXOffset, int nYOffset)
{
	attachedGame = new PuyoGame(attachedRandom, &attachedPuyoFactory);
    attachedGame->setDelegate(this);
    
	this->xOffset = xOffset;
	this->yOffset = yOffset - TSIZE;
	this->nXOffset = nXOffset;
	this->nYOffset = nYOffset;
	gameRunning = true;
	enemyGame = NULL;
}

void PuyoView::setEnemyGame(PuyoGame *enemyGame)
{
	this->enemyGame = enemyGame;
}

IIM_Surface *PuyoView::getSurfaceForState(PuyoState state)
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

IIM_Surface *PuyoView::getSurfaceForPuyo(PuyoPuyo *puyo)
{
    int i = puyo->getPuyoX();
    int j = puyo->getPuyoY();
    PuyoState currentPuyoState = puyo->getPuyoState();
    AnimatedPuyo *down  = (AnimatedPuyo *)(attachedGame->getPuyoAt(i, j+1));
    AnimatedPuyo *right = (AnimatedPuyo *)(attachedGame->getPuyoAt(i+1, j));
    AnimatedPuyo *up    = (AnimatedPuyo *)(attachedGame->getPuyoAt(i, j-1));
    AnimatedPuyo *left  = (AnimatedPuyo *)(attachedGame->getPuyoAt(i-1, j));
    
    PuyoState downState = (down == NULL)   || (down->isRenderingAnimation())  ? PUYO_EMPTY  : down->getPuyoState();
    PuyoState rightState = (right == NULL) || (right->isRenderingAnimation()) ? PUYO_EMPTY  : right->getPuyoState();
    PuyoState upState = (up == NULL)       || (up->isRenderingAnimation())    ? PUYO_EMPTY  : up->getPuyoState();
    PuyoState leftState = (left == NULL)   || (left->isRenderingAnimation())  ? PUYO_EMPTY  : left->getPuyoState();
    
    int currentIndex = PuyoGroupImageIndex
		[downState == currentPuyoState  ? 1 : 0]
		[rightState == currentPuyoState ? 1 : 0]
		[upState == currentPuyoState    ? 1 : 0]
		[leftState == currentPuyoState  ? 1 : 0];
    switch (currentPuyoState) {
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
			return getSurfaceForState(currentPuyoState);
    }
}

void PuyoView::cycleAnimation()
{
	for (int i = 0, j = attachedGame->getPuyoCount() ; i < j ; i++) {
		AnimatedPuyo *currentPuyo =
		  (AnimatedPuyo *)(attachedGame->getPuyoAtIndex(i));
		currentPuyo->cycleAnimation();
	}
    attachedPuyoFactory.cycleWalhalla();
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
            AnimatedPuyo *currentPuyo = (AnimatedPuyo *)(attachedGame->getPuyoAt(i, j));
            if ((currentPuyo != NULL) && (getSurfaceForPuyo(currentPuyo) != neutral) && (currentPuyo->isRenderingAnimation() == false)) {
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
        AnimatedPuyo *currentPuyo = (AnimatedPuyo *)(attachedGame->getPuyoAtIndex(i));
        currentPuyo->render(painter, this);
    }
    // drawing the walhalla
    attachedPuyoFactory.renderWalhalla(painter, this);
    
	drect.x = nXOffset;
	drect.y = nYOffset;
	drect.w = TSIZE;
	drect.h = TSIZE * 2;
	// Drawing next puyos
	IIM_Surface *currentSurface = getSurfaceForState(attachedGame->getNextFalling());
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

void PuyoView::gameDidAddNeutral(PuyoPuyo *neutralPuyo, int neutralIndex) {
    int x = neutralPuyo->getPuyoX();
    int y = neutralPuyo->getPuyoY();
    ((AnimatedPuyo *)neutralPuyo)->addAnimation(new NeutralAnimation(x, y, neutralIndex * 4, xOffset, yOffset));
}

void PuyoView::companionDidTurn(PuyoPuyo *companionPuyo, int companionVector, bool counterclockwise)
{
    ((AnimatedPuyo *)companionPuyo)->addAnimation(new TurningAnimation(companionPuyo, companionVector,
								       xOffset, yOffset,
								       getSurfaceForState(companionPuyo->getPuyoState()), counterclockwise));
}

void PuyoView::puyoDidFall(PuyoPuyo *puyo, int originX, int originY)
{
    ((AnimatedPuyo *)puyo)->addAnimation(new FallingAnimation(puyo, originY, xOffset, yOffset, 16));
}

void PuyoView::puyoWillVanish(IosVector &puyoGroup)
{
    AnimationSynchronizer *synchronizer = new AnimationSynchronizer();
    for (int i = 0, j = puyoGroup.getSize() ; i < j ; i++) {
        AnimatedPuyo *currentPuyo = (AnimatedPuyo *)(puyoGroup.getElementAt(i));
        currentPuyo->addAnimation(new VanishAnimation(currentPuyo, i*2 , xOffset, yOffset, synchronizer));
    }
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
        shrinkingPuyo[j-1][3] = IIM_Load_DisplayFormatAlpha(f);
        sprintf(f,"Explode%d.png", j);
        explodingPuyo[j-1][3] = IIM_Load_DisplayFormatAlpha(f);
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
    
  blinkingPointsA = 0;
  blinkingPointsB = 0;
  savePointsA = 0;
  savePointsB = 0;
  
	background    = IIM_Load_DisplayFormat(BACKGROUND[theme]);
    
	painter.backGround = background;
	if (painter.gameScreen == NULL)
	{
		SDL_PixelFormat *fmt = background->surf->format;
		SDL_Surface *tmp = SDL_CreateRGBSurface(background->surf->flags,
												background->w, background->h, 32,
												fmt->Rmask, fmt->Gmask,
												fmt->Bmask, fmt->Amask);
		gameScreen = painter.gameScreen = IIM_RegisterImg(SDL_DisplayFormat(tmp), false);
		SDL_FreeSurface(tmp);
	}
	painter.redrawAll(painter.gameScreen->surf);

	static bool firstTime = true;
	if (firstTime) {
		fallingViolet = IIM_Load_DisplayFormatAlpha("v0.png");
		fallingRed    = iim_surface_shift_hue(fallingViolet, 100.0f);
		fallingBlue   = iim_surface_shift_hue(fallingViolet, -65.0f);
		fallingGreen  = iim_surface_shift_hue(fallingViolet, -150.0f);
		fallingYellow = iim_surface_shift_hue(fallingViolet, 140.0f);
		neutral       = IIM_Load_DisplayFormatAlpha("Neutral.png");
		bigNeutral    = IIM_Load_DisplayFormatAlpha("BigNeutral.png");
    speedImg      = IIM_Load_DisplayFormatAlpha("speed.png");
    speedBlackImg = IIM_Load_DisplayFormatAlpha("speed_black.png");
		
    IIM_Surface * tmpsurf = IIM_Load_DisplayFormatAlpha("circle.png");
		for (int i = 0 ; i < 32 ; i++)
      puyoCircle[i] = iim_surface_set_value(tmpsurf,sin(3.14f/2.0f+i*3.14f/64.0f)*0.6f+0.2f);
    IIM_Free(tmpsurf);
        loadShrinkXplode();
        
        puyoShadow = IIM_Load_DisplayFormatAlpha("Shadow.png");
        puyoEye[0] = IIM_Load_DisplayFormatAlpha("eye0.png");
        puyoEye[1] = IIM_Load_DisplayFormatAlpha("eye1.png");
        puyoEye[2] = IIM_Load_DisplayFormatAlpha("eye2.png");
        puyoEyes = puyoEye[0];
        puyoEyesSwirl[0] = IIM_Load_DisplayFormatAlpha("twirleye0.png");
        puyoEyesSwirl[1] = IIM_Load_DisplayFormatAlpha("twirleye1.png");
        puyoEyesSwirl[2] = IIM_Load_DisplayFormatAlpha("twirleye2.png");
        puyoEyesSwirl[3] = IIM_Load_DisplayFormatAlpha("twirleye3.png");
        
        puyoFaces[0][0] = IIM_Load_DisplayFormatAlpha("v0.png");
        puyoFaces[0][1] = IIM_Load_DisplayFormatAlpha("v1a.png");
        puyoFaces[0][2] = IIM_Load_DisplayFormatAlpha("v1b.png");
        puyoFaces[0][3] = IIM_Load_DisplayFormatAlpha("v1c.png");
        puyoFaces[0][4] = IIM_Load_DisplayFormatAlpha("v1d.png");
        puyoFaces[0][5] = IIM_Load_DisplayFormatAlpha("v2ab.png");
        puyoFaces[0][6] = IIM_Load_DisplayFormatAlpha("v2ac.png");
        puyoFaces[0][7] = IIM_Load_DisplayFormatAlpha("v2ad.png");
        puyoFaces[0][8] = IIM_Load_DisplayFormatAlpha("v2bc.png");
        puyoFaces[0][9] = IIM_Load_DisplayFormatAlpha("v2bd.png");
        puyoFaces[0][10] = IIM_Load_DisplayFormatAlpha("v2cd.png");
        puyoFaces[0][11] = IIM_Load_DisplayFormatAlpha("v3abc.png");
        puyoFaces[0][12] = IIM_Load_DisplayFormatAlpha("v3abd.png");
        puyoFaces[0][13] = IIM_Load_DisplayFormatAlpha("v3acd.png");
        puyoFaces[0][14] = IIM_Load_DisplayFormatAlpha("v3bcd.png");
        puyoFaces[0][15] = IIM_Load_DisplayFormatAlpha("v4abcd.png");
		
		live[0] = IIM_Load_DisplayFormatAlpha("0live.png");
		live[1] = IIM_Load_DisplayFormatAlpha("1live.png");
		live[2] = IIM_Load_DisplayFormatAlpha("2live.png");
		live[3] = IIM_Load_DisplayFormatAlpha("3live.png");
		
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
		
		grid          = IIM_Load_DisplayFormatAlpha("grid.png");
		firstTime = false;
	}
    
	if (fallingBlue == NULL)
	{
		fprintf(stderr, "IMG_Load error:%s\n", SDL_GetError());
		exit(-1);
	}
    
    areaA = new PuyoView(&attachedRandom, 1 + CSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + FSIZE, BSIZE+ESIZE);
	areaB = new PuyoView(&attachedRandom, 1 + CSIZE + PUYODIMX*TSIZE + DSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + DSIZE - FSIZE - TSIZE, BSIZE+ESIZE);

	attachedGameA = areaA->getAttachedGame();
	attachedGameB = areaB->getAttachedGame();
	
	if (aiLeft) {
		randomPlayer = new PuyoIA(aiType, aiLevel, attachedGameA);
		perso[0] = IIM_Load_DisplayFormatAlpha("perso1_1.png");
		perso[1] = IIM_Load_DisplayFormatAlpha("perso1_2.png");
	}
	else {
		randomPlayer = 0;
		perso[0] = NULL;
	}
	
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


#define FPKEY_REPEAT        7
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


void PuyoStarter::run(int score1, int score2, int lives, int point1, int point2)
{
	this->lives = lives;
  this->score1 = score1;
  this->score2 = score2;
	SDL_Rect drect;
	SDL_Event event;
	int quit = 0;
	SDL_EnableUNICODE(1);
	int keysDown[FPKEY_keyNumber] = {0,0,0,0,0,0,0,0,0,0};

  gameSpeed = 20;
  attachedGameB->points = point1;
  attachedGameA->points = point2;
  
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
                  keysDown[FPKEY_P1_TurnLeft] = 0;
                  keysDown[FPKEY_P1_TurnRight] = 0;
                }
                attachedGameB->cycle();
                if (attachedGameB->isEndOfCycle()) {
                  keysDown[FPKEY_P2_Down] = 0;
                  keysDown[FPKEY_P2_TurnLeft] = 0;
                  keysDown[FPKEY_P2_TurnRight] = 0;
                }

                switch (gameLevel)
                {
                  case 1:
                    attachedGameB->points += 1;
                    attachedGameA->points += 1;
                    break;
                  case 2:
                    attachedGameB->points += 5;
                    attachedGameA->points += 5;
                    break;
                  case 3:
                    attachedGameB->points += 10;
                    attachedGameA->points += 10;
                    break;
                }
                
                if (attachedGameA->getPoints()/50000 > savePointsA/50000)
                  blinkingPointsA = 10;
                if (attachedGameB->getPoints()/50000 > savePointsB/50000)
                  blinkingPointsB = 10;

                if (blinkingPointsA > 0)
                  blinkingPointsA--;
                if (blinkingPointsB > 0)
                  blinkingPointsB--;

                savePointsB = attachedGameB->getPoints();
                savePointsA = attachedGameA->getPoints();

                if (savePointsA < 50000) blinkingPointsA=0;
                if (savePointsB < 50000) blinkingPointsB=0;

              } else {
                if (keysDown[FPKEY_P2_Down]) {
                  attachedGameB->cycle();
                  if (attachedGameB->isEndOfCycle())
                    keysDown[FPKEY_P2_Down] = 0;
                }
                if (keysDown[FPKEY_P2_Left]) {
                  repeatCondition(FPKEY_P2_Left) attachedGameB->moveLeft();
                }
                if (keysDown[FPKEY_P2_Right]) {
                  repeatCondition(FPKEY_P2_Right) attachedGameB->moveRight();
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
                extern char *AI_NAMES[];
                sprintf(level, "Stage %d... Vs %s", score2+1, AI_NAMES[score2]);
                menu_set_value(commander->gameOverMenu, kNextLevel, level);
              }
              else if (commander->gameOverMenu == commander->looserMenu) {
                char level[256];
                char cont[256];
                sprintf(level, "Stage %d... Vs %s", score2+1, p2name);
                sprintf(cont, "%d Left", lives);
                menu_set_value(commander->gameOverMenu, kCurrentLevel, level);
                menu_set_value(commander->gameOverMenu, kContinueLeft, cont);
              }
              else if (commander->gameOverMenu == commander->gameOver1PMenu) {
                char level[256];
                sprintf(level, "Stage %d... Vs %s", score2+1, p2name);
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
      // Vitesse du jeu
			if (tickCounts % (gameSpeed + 5 * (3 - gameLevel)) == 0)
      {
				event.user.type = SDL_USEREVENT;
				event.user.code = 1;
				SDL_PushEvent(&event);
        if (!paused) {
          if ((tickCounts > GAME_ACCEL) && (gameSpeed > 1)) {
            tickCounts = 0;
            gameSpeed--;
          }
        }
			}
		}
		
	}
	commander->hideGameOver();
	if (randomPlayer) {
		for (int i=0; i<NB_PERSO_STATE; ++i)
			IIM_Free(perso[i]);
	}
	SDL_SetClipRect(display,NULL);
}

void PuyoStarter::draw()
{
	if (stopRendering) {
		SDL_BlitSurface(painter.gameScreen->surf,NULL,display,NULL);
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
		
		painter.draw(painter.gameScreen->surf);
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

  SoFont *font = (stopRendering?commander->darkFont:commander->menuFont);
  SoFont_CenteredString_XY (font, display, 510, 460,   p1name, NULL);
  SoFont_CenteredString_XY (font, display, 130, 460,   p2name, NULL);

  int gameSpeedCpy = gameSpeed;
  if (gameSpeed == 1) gameSpeedCpy = 0;
  
  SDL_Rect speedRect;
  speedRect.x = 0;
  speedRect.w = speedImg->w;
  speedRect.h = (20 - gameSpeedCpy) * 6;
  speedRect.y = speedImg->h - speedRect.h;

  SDL_Rect drect;
  drect.x = 320 - speedRect.w / 2;
  drect.y = 170 - speedRect.h;
  drect.w = speedRect.w;
  drect.h = speedRect.h;

  SDL_Rect speedBlackRect = speedRect;
  SDL_Rect drectBlack     = drect;
  
  speedBlackRect.h = speedImg->h - speedRect.h;
  speedBlackRect.y = 0;
  drectBlack.y = 50;
  drectBlack.h = speedBlackRect.h;
  
  SDL_BlitSurface(speedBlackImg->surf,&speedBlackRect,display,&drectBlack);
  if (stopRendering)
    SDL_BlitSurface(speedBlackImg->surf,&speedRect,display,&drect);
  else
    SDL_BlitSurface(speedImg->surf,&speedRect,display,&drect);

  SoFont *fontBl = NULL;
  if ((blinkingPointsA % 2) == 0)
    fontBl = commander->smallFont;
  else
    fontBl = commander->menuFont;

  sprintf(text, "<< %d", attachedGameA->getPoints());
  SoFont_CenteredString_XY (fontBl, display,
                            300, 380,   text, NULL);
  
  if ((blinkingPointsB % 2) == 0)
    fontBl = commander->smallFont;
  else
    fontBl = commander->menuFont;

  sprintf(text, "%d >>", attachedGameB->getPoints());
  SoFont_CenteredString_XY (fontBl, display,
                            340, 395, text, NULL);
}
