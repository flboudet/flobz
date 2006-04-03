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

const char *p1name = "Player1";
const char *p2name = "Player2";

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
    : CycledComponent(0.02), associatedScreen(NULL), attachedLevelTheme(&levelTheme), areaA(&areaA), areaB(&areaB), controllerA(&controllerA), controllerB(&controllerB),
      cyclesBeforeGameCycle(0), cyclesBeforeSpeedIncreases(500), tickCounts(0), paused(false), displayLives(true), lives(3),
      gameOverAction(gameOverAction), abortedFlag(false), gameSpeed(0), blinkingPointsA(0), blinkingPointsB(0), savePointsA(0), savePointsB(0),
      playerOneName(p1name), playerTwoName(p2name), MinSpeed(10), MaxSpeed(50)
{
    initialize();
}

PuyoGameWidget::PuyoGameWidget()
    : CycledComponent(0.02), associatedScreen(NULL), cyclesBeforeGameCycle(0), cyclesBeforeSpeedIncreases(500),
      tickCounts(0), paused(false), displayLives(true), lives(3), abortedFlag(false), gameSpeed(0),
      blinkingPointsA(0), blinkingPointsB(0), savePointsA(0), savePointsB(0),
      playerOneName(p1name), playerTwoName(p2name), MinSpeed(10), MaxSpeed(50)
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
    
    IIM_Surface * background = attachedLevelTheme->getBackground();

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
    //printf("Ennemis positionnes! %x, %x\n", attachedGameA, attachedGameB);
    
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
    if (cyclesBeforeGameCycle == 0) {
      
      cyclesBeforeGameCycle =  MaxSpeed + (((MinSpeed-MaxSpeed)*gameSpeed)/20);
      areaA->cycleGame();
      areaB->cycleGame();
      
      // Blinking point animation
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
      // end of the blinking point animation code
    }
    
    cyclesBeforeGameCycle--;
    
    if (++tickCounts == cyclesBeforeSpeedIncreases)
    {
      tickCounts = 0;
      if (gameSpeed < 20) gameSpeed++;
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
        IIM_Surface * grid = attachedLevelTheme->getGrid();
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
            IIM_Surface * liveImage = attachedLevelTheme->getLifeForIndex(lives);
            drect.x = painter.gameScreen->w / 2 - liveImage->w / 2;
            drect.y = 436;
            drect.w = liveImage->w;
            drect.h = liveImage->h;
            painter.requestDraw(liveImage, &drect);
        }
        
        // Drawing the painter
        painter.draw();
    }
    SDL_BlitSurface(painter.gameScreen->surf, NULL, screen, NULL);
    
    // Rendering the game speed meter
    // Should be moved to the painter    
    SDL_Rect speedRect;
    IIM_Surface * speedFront = attachedLevelTheme->getSpeedMeter(true);
    IIM_Surface * speedBack  = attachedLevelTheme->getSpeedMeter(false);
    speedRect.x = 0;
    speedRect.w = speedFront->w;
    speedRect.h = gameSpeed * 6;
    speedRect.y = speedFront->h - speedRect.h;
    
    SDL_Rect drect;
    drect.x = attachedLevelTheme->getSpeedMeterX() - speedRect.w / 2;
    drect.y = attachedLevelTheme->getSpeedMeterY() - speedRect.h;
    drect.w = speedRect.w;
    drect.h = speedRect.h;
    
    SDL_Rect speedBlackRect = speedRect;
    SDL_Rect drectBlack     = drect;
    
    speedBlackRect.h = speedFront->h - speedRect.h;
    speedBlackRect.y = 0;
    drectBlack.y = attachedLevelTheme->getSpeedMeterY() - speedFront->h;
    drectBlack.h = speedBlackRect.h;

    SDL_BlitSurface(speedBack->surf,&speedBlackRect, screen, &drectBlack);
    if (!paused)
        SDL_BlitSurface(speedFront->surf,&speedRect, screen, &drect);
    else
        SDL_BlitSurface(speedBack->surf,&speedRect, screen, &drect);
    
    // Rendering the scores
    SoFont *fontBl = NULL;
    int blinkingPointsA = 0; int blinkingPointsB = 0;
    char text[1024];
    
    if ((blinkingPointsA % 2) == 0)
        fontBl = theCommander->smallFont;
    else
        fontBl = theCommander->menuFont;
    
    if (!paused) {
        sprintf(text, "<< %d", attachedGameA->getPoints());
        SoFont_CenteredString_XY (fontBl, display,
                                  300, 380,   text, NULL);
        
        if ((blinkingPointsB % 2) == 0)
            fontBl = theCommander->smallFont;
        else
            fontBl = theCommander->menuFont;
        
        sprintf(text, "%d >>", attachedGameB->getPoints());
        SoFont_CenteredString_XY (fontBl, display,
                                  340, 395, text, NULL);
        
        // Rendering the player names
        SoFont *font = (paused?theCommander->darkFont:theCommander->menuFont);
        SoFont_CenteredString_XY (font, screen, 130, 460,  playerOneName, NULL);
        SoFont_CenteredString_XY (font, screen, 510, 460, playerTwoName, NULL);
    }
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

bool PuyoGameWidget::backPressed()
{
    if ((gameover || abortedFlag) && once) {
        actionAfterGameOver(true);
        return true;
    }
    return false;
}

void PuyoGameWidget::actionAfterGameOver(bool fromControls)
{
    if (gameOverAction)
            gameOverAction->action();
}

void PuyoGameWidget::setScreenToPaused(bool fromControls)
{
    if (associatedScreen != NULL)
        associatedScreen->setPaused(fromControls);
}

void PuyoGameWidget::setScreenToResumed(bool fromControls)
{
    if (associatedScreen != NULL)
        associatedScreen->setResumed(fromControls);
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
      pauseMenu(&continueAction, &abortAction), gameWidget(gameWidget), transitionWidget(previousScreen, NULL),
      overlayStory(NULL)
{
#ifdef DEBUG_GAMELOOP
  printf("GameScreen %x created\n", this);
  printf("   Idle: %x, Drawable:%x\n",
	 dynamic_cast<IdleComponent *>(this),
	 dynamic_cast<DrawableComponent *>(this));
#endif
    add(&gameWidget);
    if (gameWidget.getOpponentFace() != NULL)
        add(gameWidget.getOpponentFace());
    add(&transitionWidget);
    gameWidget.setAssociatedScreen(this);
}

PuyoGameScreen::~PuyoGameScreen()
{
#ifdef DEBUG_GAMELOOP
  printf("GameScreen %x deleted\n", this);
  printf("   Idle: %x, Drawable:%x\n",
	 dynamic_cast<IdleComponent *>(this),
	 dynamic_cast<DrawableComponent *>(this));
  fflush(stdout);
#endif
}

void PuyoGameScreen::onEvent(GameControlEvent *cevent)
{
    bool backPressedFromGameWidget = false;
    switch (cevent->cursorEvent) {
    case GameControlEvent::kStart:
        break;
    case GameControlEvent::kBack:
        backPressedFromGameWidget = backPressed();
        break;
    }
    if (!backPressedFromGameWidget)
        Screen::onEvent(cevent);
}

bool PuyoGameScreen::backPressed()
{
    if (gameWidget.backPressed())
        return true;
    if (!paused) {
        // Seems complicated.
        // The pause method is called from the game widget
        // because this method is virtual and can be overloaded
        // (see PuyoNetworkGameWidget)
        gameWidget.setScreenToPaused(true);
    }
    else {
        // Same as for pause
        gameWidget.setScreenToResumed(true);
    }
    return false;
}

void PuyoGameScreen::setPaused(bool fromControls)
{
    if (!paused) {
        if (gameWidget.getOpponentFace() != NULL)
            gameWidget.getOpponentFace()->hide();
        this->add(&pauseMenu);
        this->focus(&pauseMenu);
        paused = true;
        gameWidget.pause();
    }
}

void PuyoGameScreen::setResumed(bool fromControls)
{
    if (paused) {
        paused = false;
        if (gameWidget.getOpponentFace() != NULL)
            gameWidget.getOpponentFace()->show();
        this->remove(&pauseMenu);
        this->focus(&gameWidget);
        gameWidget.resume();
    }
}

void PuyoGameScreen::abort()
{
    gameWidget.abort();
}

void PuyoGameScreen::setOverlayStory(PuyoStoryWidget *story)
{
  overlayStory = story;
  this->add(story);
}

PuyoTwoPlayerGameWidget::PuyoTwoPlayerGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, Action *gameOverAction) : attachedPuyoThemeSet(puyoThemeSet),
                                                     attachedGameFactory(&attachedRandom),
                                                     areaA(&attachedGameFactory, &attachedPuyoThemeSet, &levelTheme,
                                                     1 + CSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + FSIZE, BSIZE+ESIZE, painter),
                                                     areaB(&attachedGameFactory, &attachedPuyoThemeSet, &levelTheme,
                                                     1 + CSIZE + PUYODIMX*TSIZE + DSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + DSIZE - FSIZE - TSIZE, BSIZE+ESIZE, painter),
                                                     controllerA(areaA, GameControlEvent::kPlayer1Down, GameControlEvent::kPlayer1Left, GameControlEvent::kPlayer1Right,
                                                     GameControlEvent::kPlayer1TurnLeft, GameControlEvent::kPlayer1TurnRight),
                                                     controllerB(areaB, GameControlEvent::kPlayer2Down, GameControlEvent::kPlayer2Left, GameControlEvent::kPlayer2Right,
                                                     GameControlEvent::kPlayer2TurnLeft, GameControlEvent::kPlayer2TurnRight)
{
    initialize(areaA, areaB, controllerA, controllerB, levelTheme, gameOverAction);
}
