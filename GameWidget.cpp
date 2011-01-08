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

#include "GameWidget.h"
#include "PuyoCommander.h"
#include "PuyoStarter.h"

using namespace event_manager;

#define TIME_BETWEEN_GAME_CYCLES 0.02
#define TIME_TO_FINISH_GAME_WITH_BONUS 150.0
const char *p1name = "Player1";
const char *p2name = "Player2";

GameOptions GameOptions::fromDifficulty(GameDifficulty difficulty) {
    GameOptions go;
    switch(difficulty) {
    case EASY:
        go.MIN_SPEED = 4;
        go.MAX_SPEED = 20;
        break;
    case MEDIUM:
        go.MIN_SPEED = 2;
        go.MAX_SPEED = 15;
        break;
    case HARD:
    default:
        go.MIN_SPEED = 1;
        go.MAX_SPEED = 8;
        break;
    }
    return go;
}

void GameWidget::setGameOptions(GameOptions game_options)
{
    cyclesBeforeSpeedIncreases = game_options.CYCLES_BEFORE_SPEED_INCREASES;
    MinSpeed = game_options.MIN_SPEED;
    MaxSpeed = game_options.MAX_SPEED;
}

void GameWidget::setPlayerOneName(String newName) {
    playerOneName = newName;
    areaA->setPlayerNames(playerOneName, playerTwoName);
    areaB->setPlayerNames(playerOneName, playerTwoName);
}
void GameWidget::setPlayerTwoName(String newName) {
    playerTwoName = newName;
    areaA->setPlayerNames(playerOneName, playerTwoName);
    areaB->setPlayerNames(playerOneName, playerTwoName);
}

GameWidget::GameWidget(GameOptions game_options, bool withGUI)
    : CycledComponent(TIME_BETWEEN_GAME_CYCLES), withGUI(withGUI), associatedScreen(NULL),
      painter(*(GameUIDefaults::GAME_LOOP->getDrawContext())), cyclesBeforeGameCycle(0),
      cyclesBeforeSpeedIncreases(game_options.CYCLES_BEFORE_SPEED_INCREASES),
      tickCounts(0), cycles(0), paused(false), m_obscureScreenOnPause(true),
      displayLives(true), lives(3), abortedFlag(false), gameSpeed(0),
      MinSpeed(game_options.MIN_SPEED), MaxSpeed(game_options.MAX_SPEED),
      blinkingPointsA(0), blinkingPointsB(0), savePointsA(0), savePointsB(0),
      playerOneName(p1name), playerTwoName(p2name),
      m_foregroundAnimation(NULL), m_displayPlayerOneName(true), m_displayPlayerTwoName(true)
{
    if (withGUI) {
        ImageLibrary &iimLib = GameUIDefaults::GAME_LOOP->getDrawContext()->getImageLibrary();
        painterGameScreen = iimLib.createImage(IMAGE_RGB, GameUIDefaults::GAME_LOOP->getDrawContext()->w, GameUIDefaults::GAME_LOOP->getDrawContext()->h);
    }
}

void GameWidget::initWithGUI(PuyoView &areaA, PuyoView &areaB, PuyoPlayer &controllerA, PuyoPlayer &controllerB, LevelTheme &levelTheme, Action *gameOverAction)
{
    this->areaA = &areaA;
    this->areaB = &areaB;
    areaA.setPlayerNames(playerOneName, playerTwoName);
    areaB.setPlayerNames(playerOneName, playerTwoName);
    this->controllerA = &controllerA;
    this->controllerB = &controllerB;
    this->attachedLevelTheme = &levelTheme;
    this->gameOverAction = gameOverAction;
    priv_initialize();
}
void GameWidget::initWithoutGUI(PuyoView &areaA, PuyoView &areaB, PuyoPlayer &controllerA, PuyoPlayer &controllerB, Action *gameOverAction)
{
    this->areaA = &areaA;
    this->areaB = &areaB;
    areaA.setPlayerNames(playerOneName, playerTwoName);
    areaB.setPlayerNames(playerOneName, playerTwoName);
    this->controllerA = &controllerA;
    this->controllerB = &controllerB;
    this->attachedLevelTheme = NULL;
    this->gameOverAction = gameOverAction;
    priv_initialize();
}

void GameWidget::priv_initialize()
{
    once = false;
    gameover = false;
    skipGameCycleA = false;
    skipGameCycleB = false;

    if (attachedLevelTheme->getForegroundAnimation() != "") {
      // Initializing the styrolyse client
      m_styroPainter.m_styroClient.loadImage = styro_loadImage;
      m_styroPainter.m_styroClient.drawImage = styro_drawImage;
      m_styroPainter.m_styroClient.freeImage = styro_freeImage;
      m_styroPainter.m_styroClient.putText   = NULL;
      m_styroPainter.m_styroClient.getText   = NULL;
      m_styroPainter.m_styroClient.music = NULL;
      m_styroPainter.m_styroClient.playSound = NULL;
      m_styroPainter.m_styroClient.resolveFilePath = NULL;
      m_styroPainter.m_painter = &painter;
      m_styroPainter.m_theme = attachedLevelTheme;
      m_foregroundAnimation =
	styrolyse_new(attachedLevelTheme->getForegroundAnimation().c_str(),
                  (StyrolyseClient *)(&m_styroPainter), false);
    }

    // Setting up games
    attachedGameA = this->areaA->getAttachedGame();
    attachedGameB = this->areaB->getAttachedGame();
    this->areaA->setEnemyGame(attachedGameB);
    this->areaB->setEnemyGame(attachedGameA);

    setReceiveUpEvents(true);
    setFocusable(true);

    // Load and preload a few FX for the game
    for (int i=0; i<3; ++i)
        puyoFX.push_back(new PuyoFX("fx/vanish.gsl", *(areaA->getPuyoThemeSet())));
    for (int i=0; i<3; ++i)
        puyoFX.push_back(new PuyoFX("fx/combo.gsl", *(areaA->getPuyoThemeSet())));
    // puyoFX.push_back(new PuyoFX("fx/white_star.gsl"));
}

GameWidget::~GameWidget()
{
    dead();
    for (unsigned int i=0; i<puyoFX.size(); ++i)
        delete puyoFX[i];
    if (m_foregroundAnimation != NULL) {
      styrolyse_free(m_foregroundAnimation);
    }
}

void GameWidget::setGameOverAction(gameui::Action *gameOverAction)
{
    this->gameOverAction = gameOverAction;
}

void GameWidget::cycle()
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
      //printf("Changing speed: %d (next in %d)\n", gameSpeed, cyclesBeforeSpeedIncreases);
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
      gameOverAction->action(this, GAME_IS_OVER, NULL);
  }
}

void GameWidget::drawBackground(DrawTarget *dt)
{
    IosRect dtRect = { 0, 0, dt->w, dt->h };
    dt->draw(attachedLevelTheme->getBackground(), &dtRect, &dtRect);
}

void GameWidget::drawGameAreas(DrawTarget *dt)
{
    areaA->render(dt);
    areaB->render(dt);
}

void GameWidget::drawGameNeutrals(DrawTarget *dt)
{
    areaA->renderNeutral(dt);
    areaB->renderNeutral(dt);
}

void GameWidget::draw(DrawTarget *dt)
{
    if ((paused) && (m_obscureScreenOnPause)) {
        dt->draw(painterGameScreen, NULL, NULL);
        return;
    }
    // Render the background
    drawBackground(dt);
    // Rendering the opponent if it is behind the puyos
    if (attachedLevelTheme->getOpponentIsBehind()) {
        if (getOpponent() != NULL)
            getOpponent()->draw(dt);
    }
    // Rendering puyo views
    drawGameAreas(dt);
    // Rendering the grids
    IosRect drect;
    IosSurface * grid = attachedLevelTheme->getGrid();
    if (grid != NULL) {
        drect.x = 21;
        drect.y = -1;
        drect.w = grid->w;
        drect.h = grid->h;
        dt->draw(grid, NULL, &drect);
        drect.x = 407;
        drect.y = -1;
        drect.w = grid->w;
        drect.h = grid->h;
        dt->draw(grid, NULL, &drect);
    }
    // Rendering the foreground animation
    if (m_foregroundAnimation != NULL)
        styrolyse_draw(m_foregroundAnimation);
    // Rendering the neutral puyos
    drawGameNeutrals(dt);
    // Rendering the lives
    if (displayLives && (lives>=0) && (lives<=3))
    {
        IosSurface * liveImage = attachedLevelTheme->getLifeForIndex(lives);
        drect.x = attachedLevelTheme->getLifeDisplayX();
        drect.y = attachedLevelTheme->getLifeDisplayY();
        drect.w = liveImage->w;
        drect.h = liveImage->h;
        dt->draw(liveImage, NULL, &drect);
    }
    // Rendering the game speed meter
    IosRect speedRect;
    IosSurface * speedFront = attachedLevelTheme->getSpeedMeter(true);
    IosSurface * speedBack  = attachedLevelTheme->getSpeedMeter(false);
    speedRect.x = 0;
    speedRect.w = speedFront->w;
    speedRect.h = gameSpeed * 6;
    speedRect.y = speedFront->h - speedRect.h;
    drect.x = attachedLevelTheme->getSpeedMeterX() - speedRect.w / 2;
    drect.y = attachedLevelTheme->getSpeedMeterY() - speedRect.h;
    drect.w = speedRect.w;
    drect.h = speedRect.h;
    IosRect speedBlackRect = speedRect;
    IosRect drectBlack     = drect;
    speedBlackRect.h = speedFront->h - speedRect.h;
    speedBlackRect.y = 0;
    drectBlack.y = attachedLevelTheme->getSpeedMeterY() - speedFront->h;
    drectBlack.h = speedBlackRect.h;
    dt->draw(speedBack,&speedBlackRect,&drectBlack);
    dt->draw(speedFront,&speedRect, &drect);
    // Rendering the scores
    areaA->renderScore(dt);
    areaB->renderScore(dt);
    // Rendering the player names
    IosFont *font = attachedLevelTheme->getPlayerNameFont();
    if (m_displayPlayerOneName)
        dt->putStringCenteredXY(font,
                                attachedLevelTheme->getNameDisplayX(0),
                                attachedLevelTheme->getNameDisplayY(0),
                                playerOneName);
    if (m_displayPlayerTwoName)
        dt->putStringCenteredXY(font,
                                attachedLevelTheme->getNameDisplayX(1),
                                attachedLevelTheme->getNameDisplayY(1),
                                playerTwoName);
    // Rendering the opponent if it is in front
    if (! attachedLevelTheme->getOpponentIsBehind()) {
        if (getOpponent() != NULL)
            getOpponent()->draw(dt);
    }
}

void GameWidget::addSubWidget(Widget *subWidget)
{
  m_subwidgets.push_back(subWidget);
}

void GameWidget::pause(bool obscureScreen)
{
    m_obscureScreenOnPause = obscureScreen;
    // Call draw on offscreen surface before setting the game to paused
    if (m_obscureScreenOnPause)
        draw(painterGameScreen);
    paused = true;
    // Draw the obscured screen on display
    if (m_obscureScreenOnPause) {
        painterGameScreen->convertToGray();
        requestDraw();
    }
}

void GameWidget::resume()
{
    paused = false;
}

void GameWidget::eventOccured(GameControlEvent *event)
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

bool GameWidget::startPressed()
{
    if ((gameover || abortedFlag) && once && (ios_fc::getTimeMs() > gameOverDate + 500)) {
        actionAfterGameOver(true, GAMEOVER_STARTPRESSED);
        return true;
    }
    else if (paused) {
        actionAfterGameOver(true, PAUSED_STARTPRESSED);
    }
    return false;
}

bool GameWidget::backPressed()
{
    if ((gameover || abortedFlag) && once) {
        actionAfterGameOver(true, GAMEOVER_STARTPRESSED);
        return true;
    }
    return false;
}

void GameWidget::actionAfterGameOver(bool fromControls, int actionType)
{
    if (gameOverAction)
      gameOverAction->action(this, actionType, NULL);
}

void GameWidget::setScreenToPaused(bool fromControls)
{
    if (associatedScreen != NULL)
        associatedScreen->setPaused(fromControls);
}

void GameWidget::setScreenToResumed(bool fromControls)
{
  if (associatedScreen != NULL)
    if (!fromControls)
      associatedScreen->getPauseMenu().backPressed(false);
}

void *GameWidget::styro_loadImage(StyrolyseClient *_this, const char *path)
{
    StyroImage *image;
    image = new StyroImage(_this,
        FilePath(((StyrolysePainterClient *)_this)->m_theme->getThemeRootPath().c_str())
      .combine(path), true);
  return image;
}
void GameWidget::styro_drawImage(StyrolyseClient *_this,
			    void *image, int x, int y, int w, int h,
			    int clipx, int clipy, int clipw, int cliph, int flipped, float scaleX, float scaleY)
{
    StyroImage *surf = (StyroImage *)image;
    IosRect  rect, cliprect;
    rect.x = x;
    rect.y = y;
    rect.h = surf->surface->h;
    rect.w = surf->surface->w;
    cliprect.x = clipx;
    cliprect.y = clipy;
    cliprect.w = clipw;
    cliprect.h = cliph;
    ((StyrolysePainterClient *)_this)->m_painter->setClipRect(&cliprect);
    if (flipped)
		((StyrolysePainterClient *)_this)->m_painter->drawHFlipped(surf->surface, NULL, &rect);
    else {
        if (fabs(scaleX - 1.0f) > 0.001f) {
            rect.w *= scaleX;
            rect.h *= scaleY;
        }
        ((StyrolysePainterClient *)_this)->m_painter->draw(surf->surface, NULL, &rect);
    }
}
void GameWidget::styro_freeImage(StyrolyseClient *_this, void *image)
{
  delete ((StyroImage *)image);
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


