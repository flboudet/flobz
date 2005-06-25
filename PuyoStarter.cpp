/* FloboPuyo
 * Copyright (C) 2004
 *   Florent Boudet        <flobo@ios-software.com>,
 *   Jean-Christophe Hoelt <jeko@ios-software.com>,
 *   Guillaume Borios      <gyom@ios-software.com>
 *
 * iOS Software <http://www.ios-software.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *
 */

#include "PuyoStarter.h"
#include "PuyoView.h"
#include "PuyoNetworkView.h"
#include "PuyoNetworkGame.h"

#include "SDL_Painter.h"
#include "IosImgProcess.h"
#include "audio.h"

using namespace ios_fc;

extern SDL_Painter painter;

IIM_Surface *grid;
IIM_Surface *perso[2];
int currentPerso;
IIM_Surface *live[4];
IIM_Surface *speedImg;
IIM_Surface *speedBlackImg;
IIM_Surface *gameScreen;
IIM_Surface *shrinkingPuyo[5][5];
IIM_Surface *explodingPuyo[5][5];
extern IIM_Surface *bigNeutral;
IIM_Surface *puyoEyesSwirl[4];

int gameLevel;
int GAME_ACCEL = 1250;
static const int NB_PERSO_STATE = 2;

const char *p1name = "Player1";
const char *p2name = "Player2";

static char *BACKGROUND[NB_MUSIC_THEME] = { "Background.jpg", "BackgroundDark.jpg" };
extern IIM_Surface *background, *neutral;

PuyoEventPlayer::PuyoEventPlayer(PuyoView &view,
						     int downEvent, int leftEvent, int rightEvent,
						     int turnLeftEvent, int turnRightEvent)
  : PuyoPlayer(view), downEvent(downEvent), leftEvent(leftEvent), rightEvent(rightEvent),
    turnLeftEvent(turnLeftEvent), turnRightEvent(turnRightEvent),
    fpKey_Down(0), fpKey_Left(0), fpKey_Right(0), fpKey_TurnLeft(0), fpKey_TurnRight(0),
    fpKey_Repeat(7), fpKey_Delay(5)
{
}

void PuyoEventPlayer::eventOccured(GameControlEvent *event)
{
    int curGameEvent = event->gameEvent;
    if (event->isUp) {
        if (curGameEvent == downEvent) {
	    fpKey_Down = 0;
	}
	else if (curGameEvent == leftEvent) {
	    fpKey_Left = 0;
	}
	else if (curGameEvent == rightEvent) {
	    fpKey_Right = 0;
	}
	else if (curGameEvent == turnLeftEvent) {
	    fpKey_TurnLeft = 0;
	}
	else if (curGameEvent == turnRightEvent) {
	    fpKey_TurnRight = 0;
	}
    }
    else {
        if (curGameEvent == downEvent) {
	    fpKey_Down++;
	}
	else if (curGameEvent == leftEvent) {
	    targetView.moveLeft();
	    fpKey_Left++;
	}
	else if (curGameEvent == rightEvent) {
	    targetView.moveRight();
	    fpKey_Right++;
	}
	else if (curGameEvent == turnLeftEvent) {
	    targetView.rotateLeft();
	    fpKey_TurnLeft++;
	}
	else if (curGameEvent == turnRightEvent) {
	    targetView.rotateRight();
	    fpKey_TurnRight++;
	}
    }
}

void PuyoEventPlayer::cycle()
{
    // Key repetition
    if (fpKey_Down) {
    if (attachedGame->isEndOfCycle())
        fpKey_Down = 0;
    else
        targetView.cycleGame();
    }
    if (keyShouldRepeat(fpKey_Left))
        targetView.moveLeft();
    if (keyShouldRepeat(fpKey_Right))
        targetView.moveRight();
    if (keyShouldRepeat(fpKey_TurnLeft)) {
        if (attachedGame->isEndOfCycle())
	    fpKey_TurnLeft = 0;
	targetView.rotateLeft();
    }
    if (keyShouldRepeat(fpKey_TurnRight)) {
        if (attachedGame->isEndOfCycle())
	    fpKey_TurnRight = 0;
	targetView.rotateRight();
    }
}

bool PuyoEventPlayer::keyShouldRepeat(int &key)
{
    if (key == 0) return false;
    key++;
    return ((key - fpKey_Delay) > 0) && ((key - fpKey_Delay) % fpKey_Repeat == 0);
}

PuyoGameWidget::PuyoGameWidget(PuyoView &areaA, PuyoView &areaB, PuyoPlayer &controllerA, PuyoPlayer &controllerB, PuyoLevelTheme &levelTheme, Action *gameOverAction)
    : CycledComponent(0.02), attachedLevelTheme(&levelTheme), areaA(&areaA), areaB(&areaB), controllerA(&controllerA), controllerB(&controllerB),
      cyclesBeforeGameCycle(50), cyclesBeforeSpeedIncreases(500), cyclesBeforeGameCycleV(cyclesBeforeGameCycle), tickCounts(0), paused(false), displayLives(true), lives(3), gameOverAction(gameOverAction), abortedFlag(false), gameSpeed(20)
{
    initialize();
}

PuyoGameWidget::PuyoGameWidget()
    : CycledComponent(0.02), cyclesBeforeGameCycle(50), cyclesBeforeSpeedIncreases(500), cyclesBeforeGameCycleV(cyclesBeforeGameCycle), tickCounts(0), paused(false), displayLives(true), lives(3), abortedFlag(false), gameSpeed(20)
{
}

void PuyoGameWidget::initialize(PuyoView &areaA, PuyoView &areaB, PuyoPlayer &controllerA, PuyoPlayer &controllerB, PuyoLevelTheme &levelTheme, Action *gameOverAction)
{
    this->areaA = &areaA;
    this->areaB = &areaB;
    this->controllerA = &controllerA;
    this->controllerB = &controllerB;
    this->attachedLevelTheme = &levelTheme;
    this->gameOverAction = gameOverAction;
    initialize();
}

void PuyoGameWidget::initialize()
{
    once = false;
    gameover = false;
    // Affreux, a degager absolument
    if (neutral == NULL)
        neutral = IIM_Load_DisplayFormatAlpha("Neutral.png");
    if (bigNeutral == NULL)
        bigNeutral = IIM_Load_DisplayFormatAlpha("BigNeutral.png");
    static bool firstTime = true;
    if (firstTime) {
        /*speedImg      = IIM_Load_DisplayFormatAlpha("speed.png");
        speedBlackImg = IIM_Load_DisplayFormatAlpha("speed_black.png");
        */
        NeutralPopAnimation::initResources();
        firstTime = false;
    }
    background = attachedLevelTheme->getBackground();
    speedFront = attachedLevelTheme->getSpeedMeter(true);
    speedBack  = attachedLevelTheme->getSpeedMeter(false);
    grid       = IIM_Load_DisplayFormatAlpha("grid.png");
    liveImage[0] = IIM_Load_DisplayFormatAlpha("0live.png");
    liveImage[1] = IIM_Load_DisplayFormatAlpha("1live.png");
    liveImage[2] = IIM_Load_DisplayFormatAlpha("2live.png");
    liveImage[3] = IIM_Load_DisplayFormatAlpha("3live.png");
    SDL_PixelFormat *fmt = background->surf->format;
    SDL_Surface *tmp = SDL_CreateRGBSurface(background->surf->flags,
                                            background->w, background->h, 32,
                                            fmt->Rmask, fmt->Gmask,
                                            fmt->Bmask, fmt->Amask);
    painter.gameScreen = IIM_RegisterImg(SDL_DisplayFormat(tmp), false);
    SDL_FreeSurface(tmp);
    
    painter.backGround = background;
    painter.redrawAll(painter.gameScreen->surf);
    
    // Setting up games
    attachedGameA = this->areaA->getAttachedGame();
    attachedGameB = this->areaB->getAttachedGame();
    this->areaA->setEnemyGame(attachedGameB);
    this->areaB->setEnemyGame(attachedGameA);
    printf("Ennemis positionnes! %x, %x\n", attachedGameA, attachedGameB);
    
    setReceiveUpEvents(true);
    setFocusable(true);
}

PuyoGameWidget::~PuyoGameWidget()
{
    IIM_Free(painter.gameScreen);
}

void PuyoGameWidget::cycle()
{
    if (!paused) {
        tickCounts++;
        
        // Controls
        controllerA->cycle();
        controllerB->cycle();
        
        // Animations
        areaA->cycleAnimation();
        areaB->cycleAnimation();
        
        // Game cycles
        if (tickCounts % cyclesBeforeGameCycleV  == 0) {
            areaA->cycleGame();
            areaB->cycleGame();
        }
        if ((cyclesBeforeSpeedIncreases != -1) && (gameSpeed > 0) && (tickCounts % cyclesBeforeSpeedIncreases == 0)) {
            gameSpeed--;
            cyclesBeforeGameCycleV = (int)((float)cyclesBeforeGameCycle * ((float)gameSpeed / 20.));
        }
        
        requestDraw();
    }
    gameover = (areaA->isGameOver() || areaB->isGameOver());
    if ((gameover || abortedFlag) && !once) {
        once = true;
        if (gameOverAction)
            gameOverAction->action();
    }
}

void PuyoGameWidget::draw(SDL_Surface *screen)
{
  //printf("DRAW()\n");
    if (!paused) {
        // Rendering puyo views
        areaA->render();
        areaB->render();
        
        // Rendering the grids
        SDL_Rect drect;
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
        
        // Rendering the neutral puyos
        areaA->renderNeutral();
        areaB->renderNeutral();
        
        // Rendering the lives        
        if (displayLives && (lives>=0) && (lives<=3))
        {
            drect.x = painter.gameScreen->w / 2 - liveImage[lives]->w / 2;
            drect.y = 436;
            drect.w = liveImage[lives]->w;
            drect.h = liveImage[lives]->h;
            painter.requestDraw(liveImage[lives], &drect);
        }
        
        // Drawing the painter
        painter.draw();
    }
    SDL_BlitSurface(painter.gameScreen->surf, NULL, screen, NULL);
    
    // Rendering the game speed meter
    // Should be moved to the painter
    int gameSpeedCpy = gameSpeed;
    if (gameSpeed == 1) gameSpeedCpy = 0;
    
    SDL_Rect speedRect;
    speedRect.x = 0;
    speedRect.w = speedFront->w;
    speedRect.h = (20 - gameSpeedCpy) * 6;
    speedRect.y = speedFront->h - speedRect.h;
    
    SDL_Rect drect;
    drect.x = 320 - speedRect.w / 2;
    drect.y = 170 - speedRect.h;
    drect.w = speedRect.w;
    drect.h = speedRect.h;
    
    SDL_Rect speedBlackRect = speedRect;
    SDL_Rect drectBlack     = drect;
    
    speedBlackRect.h = speedFront->h - speedRect.h;
    speedBlackRect.y = 0;
    drectBlack.y = 50;
    drectBlack.h = speedBlackRect.h;

    SDL_BlitSurface(speedBack->surf,&speedBlackRect, screen, &drectBlack);
    if (!paused)
        SDL_BlitSurface(speedFront->surf,&speedRect, screen, &drect);
    else
        SDL_BlitSurface(speedBack->surf,&speedRect, screen, &drect);
    
    // Rendering the player names
    //SoFont *font = (paused?commander->darkFont:commander->menuFont);
    //SoFont_CenteredString_XY (font, screen, 510, 460,   "bla", NULL);
    //SoFont_CenteredString_XY (font, screen, 130, 460,   "bli", NULL);
}

void PuyoGameWidget::pause()
{
    paused = true;
    iim_surface_convert_to_gray(painter.gameScreen);
    requestDraw();
}

void PuyoGameWidget::resume()
{
    paused = false;
    painter.redrawAll();
}

void PuyoGameWidget::eventOccured(GameControlEvent *event)
{
    if (paused)
        lostFocus();
    else {
        controllerA->eventOccured(event);
        controllerB->eventOccured(event);
    }
}

PuyoPauseMenu::PuyoPauseMenu(Action *continueAction, Action *abortAction) : menuTitle("Pause"), continueButton("Continue game", continueAction), abortButton("Abort game", abortAction)
{
    add(&menuTitle);
    add(&continueButton);
    add(&abortButton);
}

void ContinueAction::action()
{
    screen.backPressed();
}

void AbortAction::action()
{
    screen.abort();
}

PuyoGameScreen::PuyoGameScreen(PuyoGameWidget &gameWidget, Screen &previousScreen)
    : Screen(0, 0, 640, 480), paused(false), continueAction(*this), abortAction(*this),
      pauseMenu(&continueAction, &abortAction), gameWidget(gameWidget), transitionWidget(previousScreen, NULL)
{
    add(&gameWidget);
    add(&transitionWidget);
}

void PuyoGameScreen::onEvent(GameControlEvent *cevent)
{
    switch (cevent->cursorEvent) {
    case GameControlEvent::kStart:
        break;
    case GameControlEvent::kBack:
        backPressed();
        break;
    }
    Screen::onEvent(cevent);
}

void PuyoGameScreen::backPressed()
{
    printf("Back pressed!\n");
    if (!paused) {
        this->add(&pauseMenu);
        this->focus(&pauseMenu);
        paused = true;
        gameWidget.pause();
        //stopRender();
    }
    else {
        paused = false;
        this->remove(&pauseMenu);
        this->focus(&gameWidget);
        gameWidget.resume();
        //restartRender();
    }
}

void PuyoGameScreen::abort()
{
    gameWidget.abort();
}


PuyoTwoPlayerGameWidget::PuyoTwoPlayerGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, Action *gameOverAction) : attachedPuyoThemeSet(puyoThemeSet),
                                                     attachedGameFactory(&attachedRandom),
                                                     areaA(&attachedGameFactory, &attachedPuyoThemeSet,
                                                     1 + CSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + FSIZE, BSIZE+ESIZE, painter),
                                                     areaB(&attachedGameFactory, &attachedPuyoThemeSet,
                                                     1 + CSIZE + PUYODIMX*TSIZE + DSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + DSIZE - FSIZE - TSIZE, BSIZE+ESIZE, painter),
                                                     controllerA(areaA, GameControlEvent::kPlayer1Down, GameControlEvent::kPlayer1Left, GameControlEvent::kPlayer1Right,
                                                     GameControlEvent::kPlayer1TurnLeft, GameControlEvent::kPlayer1TurnRight),
                                                     controllerB(areaB, GameControlEvent::kPlayer2Down, GameControlEvent::kPlayer2Left, GameControlEvent::kPlayer2Right,
                                                     GameControlEvent::kPlayer2TurnLeft, GameControlEvent::kPlayer2TurnRight)
{
    initialize(areaA, areaB, controllerA, controllerB, levelTheme, gameOverAction);
}

class PuyoCycled : public CycledComponent
{
  public:
    PuyoPureStarter *starter;
    
    PuyoCycled(PuyoPureStarter *starter) : CycledComponent(0.02), starter(starter) {}
    void cycle() {
      starter->cycle();
    }
};

PuyoPureStarter::PuyoPureStarter(PuyoCommander *commander) : Screen(0,0,640,480), commander(commander), pauseMenu(NULL, NULL)
{
    paused = false;
    stopRendering = false;
    gameAborted = false;
    cycled = new PuyoCycled(this);
}

PuyoPureStarter::~PuyoPureStarter()
{
}

void PuyoPureStarter::idle(double currentTime)
{
  cycled->idle(currentTime);
}

bool PuyoPureStarter::isLate(double currentTime) const
{
  return cycled->isLate(currentTime);
}

void PuyoPureStarter::startPressed()
{
    //if (gameover) {
        //if (menu_active_is(commander->gameOverMenu, "NO"))
        //    menu_next_item(commander->gameOverMenu);
        //quit = true;
    //}
}

void PuyoPureStarter::backPressed()
{
    printf("Back pressed!\n");

    if (1/*!gameover*/) {
        if (!paused) {
            this->add(&pauseMenu);
            paused = true;
            stopRender();
        }
        else {
            paused = false;
            this->remove(&pauseMenu);
            restartRender();
        }
    }
    else {
        //if (menu_active_is(commander->gameOverMenu, "NO"))
        //    menu_next_item(commander->gameOverMenu);
        //quit = 1;
    }
}
    
void PuyoPureStarter::stopRender()
{
    this->stopRendering = true;
    iim_surface_convert_to_gray(painter.gameScreen);
}


void PuyoPureStarter::restartRender()
{
    this->stopRendering = false;
    painter.redrawAll();
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

void PuyoStarter::draw(SDL_Surface *surf) {
  const_cast<PuyoStarter*>(this)->draw();
  Screen::draw(surf);
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
        SDL_BlitSurface(painter.gameScreen->surf, NULL, display, NULL);
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


PuyoStarter::PuyoStarter(PuyoCommander *commander, int theme):attachedThemeManager("path", "name"), PuyoPureStarter(commander)
{
    tickCounts = 0;
    
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

        neutral       = IIM_Load_DisplayFormatAlpha("Neutral.png");
        bigNeutral    = IIM_Load_DisplayFormatAlpha("BigNeutral.png");
        speedImg      = IIM_Load_DisplayFormatAlpha("speed.png");
        speedBlackImg = IIM_Load_DisplayFormatAlpha("speed_black.png");

        loadShrinkXplode();
        
        puyoEyesSwirl[0] = IIM_Load_DisplayFormatAlpha("twirleye0.png");
        puyoEyesSwirl[1] = IIM_Load_DisplayFormatAlpha("twirleye1.png");
        puyoEyesSwirl[2] = IIM_Load_DisplayFormatAlpha("twirleye2.png");
        puyoEyesSwirl[3] = IIM_Load_DisplayFormatAlpha("twirleye3.png");

        live[0] = IIM_Load_DisplayFormatAlpha("0live.png");
        live[1] = IIM_Load_DisplayFormatAlpha("1live.png");
        live[2] = IIM_Load_DisplayFormatAlpha("2live.png");
        live[3] = IIM_Load_DisplayFormatAlpha("3live.png");
        
        NeutralPopAnimation::initResources();
        
        grid          = IIM_Load_DisplayFormatAlpha("grid.png");
        firstTime = false;
    }
}

PuyoStarter::~PuyoStarter()
{
    delete areaA;
    delete areaB;
    delete attachedGameA;
    delete attachedGameB;
    delete attachedGameFactory;
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

int keysDown[FPKEY_keyNumber] = {0,0,0,0,0,0,0,0,0,0};

void PuyoStarter::onEvent(GameControlEvent *cevent)
{
  GameControlEvent controlEvent = *cevent;
  SDL_Event event = cevent->sdl_event;
  if (attachedGameA->isGameRunning() &&
      attachedGameB->isGameRunning()) {
    if (!paused) {

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
      else {
        switch (controlEvent.gameEvent) {
          case GameControlEvent::kPlayer1Left:
            if (randomPlayer == 0) {
              areaA->moveLeft();
              if (attachedGameA->isEndOfCycle()) keysDown[FPKEY_P1_Left] = 0;
              else keysDown[FPKEY_P1_Left]++; 
            }
            break;
          case GameControlEvent::kPlayer1Right:
            if (randomPlayer == 0) {
              areaA->moveRight();
              if (attachedGameA->isEndOfCycle()) keysDown[FPKEY_P1_Right] = 0;
              else keysDown[FPKEY_P1_Right]++; 
            }
            break;
          case GameControlEvent::kPlayer1TurnLeft:
            if (randomPlayer == 0) {
              areaA->rotateLeft();
              if (attachedGameA->isEndOfCycle()) keysDown[FPKEY_P1_TurnLeft] = 0;
              else keysDown[FPKEY_P1_TurnLeft]++; 
            }
            break;
          case GameControlEvent::kPlayer1TurnRight:
            if (randomPlayer == 0) {
              areaA->rotateRight();
              if (attachedGameA->isEndOfCycle()) keysDown[FPKEY_P1_TurnRight] = 0;
              else keysDown[FPKEY_P1_TurnRight]++;
            }
            break;
          case GameControlEvent::kPlayer1Down:
            if (randomPlayer == 0) {
              //attachedGameA->cycle(); desact flobo
              if (attachedGameA->isEndOfCycle()) keysDown[FPKEY_P1_Down] = 0;
              else keysDown[FPKEY_P1_Down]++;
            }
            break;

          case GameControlEvent::kPlayer2Left:
            areaB->moveLeft();
            if (attachedGameB->isEndOfCycle()) keysDown[FPKEY_P2_Left] = 0;
            else keysDown[FPKEY_P2_Left]++;
            break;
          case GameControlEvent::kPlayer2Right:
            areaB->moveRight();
            if (attachedGameB->isEndOfCycle()) keysDown[FPKEY_P2_Right] = 0;
            else keysDown[FPKEY_P2_Right]++;
            break;
          case GameControlEvent::kPlayer2TurnLeft:
            areaB->rotateLeft();
            if (attachedGameB->isEndOfCycle()) keysDown[FPKEY_P2_TurnLeft] = 0;
            else keysDown[FPKEY_P2_TurnLeft]++;
            break;
          case GameControlEvent::kPlayer2TurnRight:
            areaB->rotateRight();
            if (attachedGameB->isEndOfCycle()) keysDown[FPKEY_P2_TurnRight] = 0;
            else keysDown[FPKEY_P2_TurnRight]++;
            break;
          case GameControlEvent::kPlayer2Down:
            //attachedGameB->cycle(); desact flobo
            if (attachedGameB->isEndOfCycle()) keysDown[FPKEY_P2_Down] = 0;
            else keysDown[FPKEY_P2_Down]++;
            break;
          default:
            break;
        }
      }

      switch (event.type) {
        case SDL_USEREVENT:
          if (randomPlayer)
            randomPlayer->cycle();
          if (event.user.code == 1) {

            if (attachedGameA->isEndOfCycle()) {
              keysDown[FPKEY_P1_Down] = 0;
              keysDown[FPKEY_P1_TurnLeft] = 0;
              keysDown[FPKEY_P1_TurnRight] = 0;
            }

            areaA->cycleGame(); // a voir

            if (attachedGameB->isEndOfCycle()) {
              keysDown[FPKEY_P2_Down] = 0;
              keysDown[FPKEY_P2_TurnLeft] = 0;
              keysDown[FPKEY_P2_TurnRight] = 0;
            }
            areaB->cycleGame(); // a voir

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
              if (attachedGameB->isEndOfCycle())
                keysDown[FPKEY_P2_Down] = 0;
              else
                areaB->cycleGame(); // a voir
            }
            if (keysDown[FPKEY_P2_Left]) {
              repeatCondition(FPKEY_P2_Left) areaB->moveLeft();
            }
            if (keysDown[FPKEY_P2_Right]) {
              repeatCondition(FPKEY_P2_Right) areaB->moveRight();
            }
            if (keysDown[FPKEY_P2_TurnLeft]) {
              repeatCondition(FPKEY_P2_TurnLeft) areaB->rotateLeft();
              if (attachedGameB->isEndOfCycle())
                keysDown[FPKEY_P2_TurnLeft] = 0;
            }
            if (keysDown[FPKEY_P2_TurnRight]) {
              repeatCondition(FPKEY_P2_TurnRight) areaB->rotateRight();
              if (attachedGameB->isEndOfCycle())
                keysDown[FPKEY_P2_TurnRight] = 0;
            }

            if (keysDown[FPKEY_P1_Down]) {
              if (attachedGameA->isEndOfCycle())
                keysDown[FPKEY_P1_Down] = 0;
              else
                areaA->cycleGame(); // a voir
            }
            if (keysDown[FPKEY_P1_Left]) {
              repeatCondition(FPKEY_P1_Left) areaA->moveLeft();
              if (attachedGameA->isEndOfCycle())
                keysDown[FPKEY_P1_Left] = 0;
            }
            if (keysDown[FPKEY_P1_Right]) {
              repeatCondition(FPKEY_P1_Right) areaA->moveRight();
              if (attachedGameA->isEndOfCycle())
                keysDown[FPKEY_P1_Right] = 0;
            }
            if (keysDown[FPKEY_P1_TurnLeft]) {
              repeatCondition(FPKEY_P1_TurnLeft) areaA->rotateLeft();
              if (attachedGameA->isEndOfCycle())
                keysDown[FPKEY_P1_TurnLeft] = 0;
            }
            if (keysDown[FPKEY_P1_TurnRight]) {
              repeatCondition(FPKEY_P1_TurnRight) areaA->rotateRight();
              if (attachedGameA->isEndOfCycle())
                keysDown[FPKEY_P1_TurnRight] = 0;
            }
          }
          break;
        case SDL_KEYDOWN:
          /* check for cheat-codes */
          static int cheatcode = 0;
          if (event.key.keysym.sym == SDLK_k) cheatcode  = 0;
          if (event.key.keysym.sym == SDLK_i) cheatcode += 1;
          if (event.key.keysym.sym == SDLK_e) cheatcode += 10;
          if (event.key.keysym.sym == SDLK_f) cheatcode += 100;
          if (event.key.keysym.sym == SDLK_t) cheatcode += 1000;
          if (event.key.keysym.sym == SDLK_l) cheatcode += 10000;
          if (cheatcode == 31111) {
            attachedGameA->increaseNeutralPuyos(PUYODIMX * 12);
            attachedGameA->dropNeutrals();
            attachedGameA->increaseNeutralPuyos(PUYODIMX * 12);
          }
        default:
          break;
      }
    } // Game is paused
    else {
//      GameControlEvent controlEvent;
//      getControlEvent(event, &controlEvent);
      /*                        switch (controlEvent.cursorEvent) {
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
                                */
    }
  } else // Not GameIsRunning
      {
        quit = 1;
        /*
           if (randomPlayer) {
           if (rightPlayerWin()) {
           if ((_score2 == 7) && (rightPlayerWin()))
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

           if (leftPlayerWin()) score1 = _score1 + 1;
           else if (rightPlayerWin()) score2 = _score2 + 1;

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
           */
      }
  GameControlEvent controlEvent2 = controlEvent;
//  getControlEvent(event, &controlEvent2);
  switch (controlEvent2.cursorEvent) {
    case GameControlEvent::kStart:
      /*                        if (gameover)
                                {
                                if (menu_active_is(commander->gameOverMenu, "NO"))
                                menu_next_item(commander->gameOverMenu);
                                quit = 1;
                                }*/
      break;
    case GameControlEvent::kBack:
      if (!gameover) {
        backPressed();
        /*                            if (!paused) {
                                      menu_show(menu_pause);
                                      paused = true;
                                      stopRender();
                                      }
                                      else {
                                      paused = false;
                                      restartRender();
                                      menu_hide(menu_pause);
                                      }*/
      }
      else {
        /*
           if (menu_active_is(commander->gameOverMenu, "NO"))
           menu_next_item(commander->gameOverMenu);
           quit = 1;
           */
      }
      break;
    default:
      break;
  }
  if(event.type == SDL_QUIT) {
    /*                    if (menu_active_is(commander->gameOverMenu, "YES"))
                          menu_next_item(commander->gameOverMenu);
                          quit = 1;*/
    exit(0);
  }
}

void PuyoStarter::run(int _score1, int _score2, int lives, int point1, int point2)
{
    this->lives = lives;
    this->score1 = _score1;
    this->score2 = _score2;
//    SDL_Rect drect;
    quit = 0;
    SDL_EnableUNICODE(1);
    
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
}

void PuyoStarter::cycle()
{
    if (!quit) {
        SDL_Event event;
        bool left_danger = (attachedGameA->getMaxColumnHeight() > PUYODIMY - 5);
        bool right_danger = (attachedGameB->getMaxColumnHeight() > PUYODIMY - 5);
        bool danger = left_danger || right_danger;
        gameover = (!attachedGameA->isGameRunning() || !attachedGameB->isGameRunning());
        
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
        
//            commander->updateAll(this);
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
            requestDraw();
            // printf("cycle()\n");
    }
    else {
      if (randomPlayer) {
        for (int i=0; i<NB_PERSO_STATE; ++i)
          IIM_Free(perso[i]);
      }
      GameUIDefaults::SCREEN_STACK->pop();
      kill();
      SDL_SetClipRect(display,NULL);
    }
}
