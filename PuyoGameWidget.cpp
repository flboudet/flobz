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

#include "PuyoGameWidget.h"
#include "PuyoCommander.h"
#include "PuyoStarter.h"
//#include "PuyoStrings.h"


#define TIME_BETWEEN_GAME_CYCLES 0.02
#define TIME_TO_FINISH_GAME_WITH_BONUS 150.0
const char *p1name = "Player1";
const char *p2name = "Player2";

void PuyoGameWidget::setGameOptions(GameOptions game_options)
{
    cyclesBeforeSpeedIncreases = game_options.CYCLES_BEFORE_SPEED_INCREASES;
    MinSpeed = game_options.MIN_SPEED;
    MaxSpeed = game_options.MAX_SPEED;
}

PuyoGameWidget::PuyoGameWidget(GameOptions game_options)
    : CycledComponent(TIME_BETWEEN_GAME_CYCLES), associatedScreen(NULL), cyclesBeforeGameCycle(0), cyclesBeforeSpeedIncreases(game_options.CYCLES_BEFORE_SPEED_INCREASES),
      tickCounts(0), cycles(0), paused(false), displayLives(true), lives(3), abortedFlag(false), gameSpeed(0),
      MinSpeed(game_options.MIN_SPEED), MaxSpeed(game_options.MAX_SPEED),
      blinkingPointsA(0), blinkingPointsB(0), savePointsA(0), savePointsB(0),
      playerOneName(p1name), playerTwoName(p2name),
      m_foregroundAnimation(NULL)
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
    skipGameCycleA = false;
    skipGameCycleB = false;
    
    IIM_Surface * background = attachedLevelTheme->getBackground();
    if (attachedLevelTheme->getForegroundAnimation() != "") {
      // Initializing the styrolyse client
      m_styroPainter.m_styroClient.loadImage = styro_loadImage;
      m_styroPainter.m_styroClient.drawImage = styro_drawImage;
      m_styroPainter.m_styroClient.freeImage = styro_freeImage;
      m_styroPainter.m_styroClient.putText   = NULL;
      m_styroPainter.m_styroClient.getText   = NULL;
      m_styroPainter.m_styroClient.playMusic = NULL;
      m_styroPainter.m_styroClient.playSound = NULL;
      m_styroPainter.m_styroClient.resolveFilePath = NULL;
      m_styroPainter.m_painter = &painter;
      m_styroPainter.m_theme = attachedLevelTheme;
      m_foregroundAnimation =
	styrolyse_new((const char *)
		      (FilePath(attachedLevelTheme->getThemeRootPath()).combine(
		        attachedLevelTheme->getForegroundAnimation())),
		      (StyrolyseClient *)(&m_styroPainter), false);
    }

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

    // Load and preload a few FX for the game
    for (int i=0; i<15; ++i)
        puyoFX.push_back(new PuyoFX("fx/vanish.gsl"));
    for (int i=0; i<3; ++i)
        puyoFX.push_back(new PuyoFX("fx/combo.gsl"));
    // puyoFX.push_back(new PuyoFX("fx/white_star.gsl"));
}

PuyoGameWidget::~PuyoGameWidget()
{
    IIM_Free(painter.gameScreen);
    for (unsigned int i=0; i<puyoFX.size(); ++i)
        delete puyoFX[i];
    if (m_foregroundAnimation != NULL) {
      styrolyse_free(m_foregroundAnimation);
    }
}

void PuyoGameWidget::cycle()
{
  if (!paused) {
    tickCounts++;
    cycles++;
    
    int animCyclesBeforeGameCycles = (MaxSpeed + (((MinSpeed - MaxSpeed) * gameSpeed) / 20));
    
    // Cycling through the foreground animation
    if (m_foregroundAnimation != NULL)
      styrolyse_update(m_foregroundAnimation, 0.);

    // Controls
    controllerA->cycle();
    controllerB->cycle();
    
    if (!skipGameCycleA && areaA->isNewMetaCycleStart() && (cyclesBeforeGameCycle < animCyclesBeforeGameCycles/2))
      skipGameCycleA = true;  
    if (!skipGameCycleB && areaB->isNewMetaCycleStart() && (cyclesBeforeGameCycle < animCyclesBeforeGameCycles/2))
      skipGameCycleB = true;  

    // Animations
    areaA->cycleAnimation();
    areaB->cycleAnimation();
    
    if (!skipGameCycleA && areaA->isNewMetaCycleStart() && (cyclesBeforeGameCycle < animCyclesBeforeGameCycles/2))
      skipGameCycleA = true;  
    if (!skipGameCycleB && areaB->isNewMetaCycleStart() && (cyclesBeforeGameCycle < animCyclesBeforeGameCycles/2))
      skipGameCycleB = true;  

    // Game cycles
    if (cyclesBeforeGameCycle == 0) {
      
      areaA->clearMetaCycleStart();
      areaB->clearMetaCycleStart();

      cyclesBeforeGameCycle = animCyclesBeforeGameCycles;
      if (!skipGameCycleA) 
      {
        areaA->cycleGame();
      }
      else skipGameCycleA = false;
      
      if (!skipGameCycleB)
      {
        areaB->cycleGame();
      }
      else skipGameCycleB = false;
      
      // Blinking point animation
      // TODO
      /*
      if (attachedGameA->getPoints()/50000 > savePointsA/50000)
        blinkingPointsA = 10;
      if (attachedGameB->getPoints()/50000 > savePointsB/50000)
        blinkingPointsB = 10;
        */
      
      if (blinkingPointsA > 0)
        blinkingPointsA--;
      if (blinkingPointsB > 0)
        blinkingPointsB--;
      
      /*
      savePointsB = attachedGameB->getPoints();
      savePointsA = attachedGameA->getPoints();
      
      if (savePointsA < 50000) blinkingPointsA=0;
      if (savePointsB < 50000) blinkingPointsB=0;
      */
      // end of the blinking point animation code
    }
    
    cyclesBeforeGameCycle--;
    
    if (tickCounts == (unsigned int)cyclesBeforeSpeedIncreases)
    {
      tickCounts = 0;
      if (gameSpeed < 20) gameSpeed++;
      cyclesBeforeSpeedIncreases = cyclesBeforeSpeedIncreases * (20 + gameSpeed) / 20;
      printf("Changing speed: %d (next in %d)\n", gameSpeed, cyclesBeforeSpeedIncreases);
    }
    requestDraw();
  }
  gameover = (areaA->isGameOver() || areaB->isGameOver());
  if ((gameover || abortedFlag) && !once) {
    once = true;
    gameOverDate = ios_fc::getTimeMs();
    if (areaA->isGameOver())
      areaB->gameWin();
    if (areaB->isGameOver())
      areaA->gameWin();
    if (gameOverAction)
      gameOverAction->action(this, 0, NULL);
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
	if (grid != NULL) {
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
	}

	// Rendering the foreground animation
	if (m_foregroundAnimation != NULL)
	  styrolyse_draw(m_foregroundAnimation);
        
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
    areaA->renderOverlay();
    areaB->renderOverlay();
    /*
    SoFont *fontBl = NULL;
    int blinkingPointsA = 0; int blinkingPointsB = 0;
    char text[1024];
    
    if (!paused) {
        double time = TIME_TO_FINISH_GAME_WITH_BONUS - (double)cycles * TIME_BETWEEN_GAME_CYCLES;
        if (time < 0.0) time = 0.0;
        double min  = floor(time / 60.0);
        double sec  = floor(time - min * 60.0);
        sprintf(text, "%01.0f:%02.0f", min, sec);
        fontBl = theCommander->menuFont;
        SoFont_CenteredString_XY (fontBl, display,
                                  320, 380,   text, NULL);
    
//    if ((blinkingPointsA % 2) == 0)
//        fontBl = theCommander->smallFont;
//    else
//        fontBl = theCommander->menuFont;
//        sprintf(text, "<< %d", attachedGameA->getPoints());
//        SoFont_CenteredString_XY (fontBl, display,
//                                  300, 380,   text, NULL);
//        
//        if ((blinkingPointsB % 2) == 0)
//            fontBl = theCommander->smallFont;
//        else
//            fontBl = theCommander->menuFont;
//        
//        sprintf(text, "%d >>", attachedGameB->getPoints());
//        SoFont_CenteredString_XY (fontBl, display,
//                                  340, 395, text, NULL);
        
        // Rendering the player names
        SoFont *font = (paused?theCommander->darkFont:theCommander->menuFont);
        SoFont_CenteredString_XY (font, screen, 130, 460, playerOneName, NULL);
        SoFont_CenteredString_XY (font, screen, 510, 460, playerTwoName, NULL);
    }
    */
}

void PuyoGameWidget::addSubWidget(Widget *subWidget)
{
  m_subwidgets.push_back(subWidget);
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
	for (std::vector<gameui::Widget *>::iterator iter = m_subwidgets.begin() ;
	     iter != m_subwidgets.end() ; iter++) {
	  (*iter)->eventOccured(event);
	}
    }
}

bool PuyoGameWidget::startPressed()
{
    if ((gameover || abortedFlag) && once && (ios_fc::getTimeMs() > gameOverDate + 2000)) {
        actionAfterGameOver(true);
        return true;
    }
    return false;
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
      gameOverAction->action(this, 0, NULL);
}

void PuyoGameWidget::setScreenToPaused(bool fromControls)
{
    if (associatedScreen != NULL)
        associatedScreen->setPaused(fromControls);
}

void PuyoGameWidget::setScreenToResumed(bool fromControls)
{
  if (associatedScreen != NULL)
    if (!fromControls)
      associatedScreen->getPauseMenu().backPressed(false);
}

void *PuyoGameWidget::styro_loadImage(StyrolyseClient *_this, const char *path)
{
  IIM_Surface *surface;
  surface = IIM_Load_Absolute_DisplayFormatAlpha
    ((FilePath(((StyrolysePainterClient *)_this)->m_theme->getThemeRootPath())
      .combine(path)));
  return surface;
}
void PuyoGameWidget::styro_drawImage(StyrolyseClient *_this,
			    void *image, int x, int y,
			    int clipx, int clipy, int clipw, int cliph)
{
  IIM_Surface *surf = (IIM_Surface *)image;
  SDL_Rect  rect, cliprect;
  rect.x = x;
  rect.y = y;
  cliprect.x = clipx;
  cliprect.y = clipy;
  cliprect.w = clipw;
  cliprect.h = cliph;
  ((StyrolysePainterClient *)_this)->m_painter->requestDraw(surf, &rect);
}
void PuyoGameWidget::styro_freeImage(StyrolyseClient *_this, void *image)
{
  IIM_Free((IIM_Surface *)image);
}

std::vector<PuyoFX*> *activeFX = NULL;

void EventFX(const char *name, float x, float y, int player)
{
    if (activeFX == NULL) return;

    /* printf("Simultaneous FX: %d\n", activeFX->size()); */

    PuyoFX *supporting_fx = NULL;
    for (unsigned int i=0; i<activeFX->size(); ++i) {

        // Find an FX supporting this event
        PuyoFX *fx = (*activeFX)[i];
        if (fx->supportFX(name)) {
            supporting_fx = fx;
            // FX not busy, report it the event
            if (!fx->busy()) {
                fx->postEvent(name, x, y, player);
                return;
            }
        }
    }

    // All FX are busy, if we find an FX supporting this event
    // we clone it an report the FX to the newly created one.
    if (supporting_fx != NULL) {
        PuyoFX *fx = supporting_fx->clone();
        activeFX->push_back(fx);
        fx->getGameScreen()->add(fx);
        fx->postEvent(name, x, y, player);
    }
}

