/* FloboPop
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
#include "GameScreen.h"
#include "GTLog.h"

using namespace event_manager;

#define TIME_BETWEEN_GAME_CYCLES 0.02
#define TIME_TO_FINISH_GAME_WITH_BONUS 150.0
const char *p1name = "Player1";
const char *p2name = "Player2";

static void *openFileFunction(StyrolyseClient *_this, const char *file_name)
{
    if (theCommander->getDataPathManager().hasDataInputStream(file_name))
        return (void *)(theCommander->getDataPathManager().openDataInputStream(file_name));
    else
        return NULL;
}

static void closeFileFunction(StyrolyseClient *_this, void *file)
{
    DataInputStream *s = (DataInputStream *)file;
    if (s)
        delete s;
}

static int readFileFunction(StyrolyseClient *_this, void *buffer, void *file, int read_size)
{
    DataInputStream *s = (DataInputStream *)file;
    if (s)
        return s->streamRead(buffer, read_size);
    else
        return 0;
}



StyrolysePainterClient::StyrolysePainterClient(LevelTheme *theme)
{
    // Initializing the styrolyse client
    m_client.m_styroClient.loadImage = styro_loadImage;
    m_client.m_styroClient.drawImage = styro_drawImage;
    m_client.m_styroClient.freeImage = styro_freeImage;
    m_client.m_styroClient.putText   = NULL;
    m_client.m_styroClient.getText   = NULL;
    m_client.m_styroClient.music = NULL;
    m_client.m_styroClient.playSound = NULL;
    m_client.m_styroClient.resolveFilePath = NULL;
    m_client.m_styroClient.openFile = openFileFunction;
    m_client.m_styroClient.closeFile = closeFileFunction;
    m_client.m_styroClient.readFile = readFileFunction;
    m_client.m_painter = NULL;
    m_client.m_theme = theme;
    // Initialize the animation
    m_animation = styrolyse_new(theme->getForegroundAnimation().c_str(),
                                &(m_client.m_styroClient), false);
}

StyrolysePainterClient::~StyrolysePainterClient()
{
    styrolyse_free(m_animation);
}

void StyrolysePainterClient::update()
{
    styrolyse_update(m_animation, 0.);
}

void StyrolysePainterClient::draw(DrawTarget *dt)
{
    m_client.m_painter = dt;
    styrolyse_draw(m_animation);
}

void *StyrolysePainterClient::styro_loadImage(StyrolyseClient *_this, const char *path)
{
    StyroImage *image;
    image = new StyroImage(_this,
        FilePath(((ExtendedClient *)_this)->m_theme->getThemeRootPath().c_str())
      .combine(path), true);
    return image;
}

void StyrolysePainterClient::styro_drawImage(StyrolyseClient *_this,
			    void *image, int x, int y, int w, int h,
			    int clipx, int clipy, int clipw, int cliph, int flipped, float scaleX, float scaleY, float alpha)
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
    ((ExtendedClient *)_this)->m_painter->setClipRect(&cliprect);
    if (flipped)
		((ExtendedClient *)_this)->m_painter->drawHFlipped(surf->surface, NULL, &rect);
    else {
        if (fabs(scaleX - 1.0f) > 0.001f) {
            rect.w *= scaleX;
            rect.h *= scaleY;
        }
        ((ExtendedClient *)_this)->m_painter->draw(surf->surface, NULL, &rect);
    }
}

void StyrolysePainterClient::styro_freeImage(StyrolyseClient *_this, void *image)
{
  delete ((StyroImage *)image);
}





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

VictoryDisplay::VictoryDisplay(Vec3 position, IosSurface *trophy, int victories)
  : m_position(position), m_trophy(trophy), m_victories(victories)
{
}

void VictoryDisplay::draw(DrawTarget *dt)
{
    if (m_victories == 0)
        return;
    int offx = (m_trophy->w * m_victories) / 2;
    IosRect dst = { m_position.x - offx, m_position.y, m_trophy->w, m_trophy->h };
    for (int i = 0 ; i < m_victories ; ++i) {
        dt->draw(m_trophy, NULL, &dst);
        dst.x += m_trophy->w;
    }
}

GameWidget::GameWidget()
  : m_levelTheme(NULL),
    gameOverAction(NULL), associatedScreen(NULL),
    m_paused(false), m_obscureScreenOnPause(true),
    m_abortedFlag(false)
{
    ImageLibrary &iimLib = GameUIDefaults::GAME_LOOP->getDrawContext()->getImageLibrary();
    m_painterGameScreen = iimLib.createImage(IMAGE_RGB, GameUIDefaults::GAME_LOOP->getDrawContext()->w, GameUIDefaults::GAME_LOOP->getDrawContext()->h);
}

GameWidget::~GameWidget()
{
    for (unsigned int i=0; i<m_visualFX.size(); ++i)
        delete m_visualFX[i];
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

void GameWidget::setLevelTheme(LevelTheme *levelTheme)
{
    m_levelTheme = levelTheme;
    if (m_levelTheme->getForegroundAnimation() != "")
        m_styroPainter.reset(new StyrolysePainterClient(levelTheme));
}

void GameWidget::pause(bool obscureScreen)
{
    // Call draw on offscreen surface before setting the game to paused
    if (obscureScreen) {
        draw(m_painterGameScreen);
    }
    m_paused = true;
    m_obscureScreenOnPause = obscureScreen;
    // Draw the obscured screen on display
    if (m_obscureScreenOnPause) {
        m_painterGameScreen->convertToGray();
        requestDraw();
    }
}

void GameWidget::resume()
{
    m_paused = false;
    setFocusable(true);
}





void GameWidget2P::setGameOptions(GameOptions game_options)
{
    cyclesBeforeSpeedIncreases = game_options.CYCLES_BEFORE_SPEED_INCREASES;
    MinSpeed = game_options.MIN_SPEED;
    MaxSpeed = game_options.MAX_SPEED;
}

void GameWidget2P::setVictories(int left, int right)
{
    m_victoryDisplayA->setValue(left);
    m_victoryDisplayB->setValue(right);
}

void GameWidget2P::setPlayerOneName(String newName) {
    playerOneName = newName;
    areaA->setPlayerNames(playerOneName, playerTwoName);
    areaB->setPlayerNames(playerOneName, playerTwoName);
}
void GameWidget2P::setPlayerTwoName(String newName) {
    playerTwoName = newName;
    areaA->setPlayerNames(playerOneName, playerTwoName);
    areaB->setPlayerNames(playerOneName, playerTwoName);
}

GameWidget2P::GameWidget2P(GameOptions game_options)
    : CycledComponent(TIME_BETWEEN_GAME_CYCLES),
      painter(*(GameUIDefaults::GAME_LOOP->getDrawContext())), cyclesBeforeGameCycle(0),
      cyclesBeforeSpeedIncreases(game_options.CYCLES_BEFORE_SPEED_INCREASES),
      tickCounts(0), cycles(0),
      displayLives(true), lives(3), gameSpeed(0),
      MinSpeed(game_options.MIN_SPEED), MaxSpeed(game_options.MAX_SPEED),
      blinkingPointsA(0), blinkingPointsB(0), savePointsA(0), savePointsB(0),
      playerOneName(p1name), playerTwoName(p2name),
      m_displayPlayerOneName(true), m_displayPlayerTwoName(true)
{
}

void GameWidget2P::initWithGUI(GameView &areaA, GameView &areaB, LevelTheme &levelTheme, Action *gameOverAction)
{
    this->areaA = &areaA;
    this->areaB = &areaB;
    areaA.setPlayerNames(playerOneName, playerTwoName);
    areaB.setPlayerNames(playerOneName, playerTwoName);
    this->gameOverAction = gameOverAction;
    priv_initialize();
    setLevelTheme(&levelTheme);
    m_victoryDisplayA.reset(new VictoryDisplay(Vec3(levelTheme.getTrophyDisplayX(0), levelTheme.getTrophyDisplayY(0)), levelTheme.getTrophy()));
    m_victoryDisplayB.reset(new VictoryDisplay(Vec3(levelTheme.getTrophyDisplayX(1), levelTheme.getTrophyDisplayY(1)), levelTheme.getTrophy()));
}

void GameWidget2P::priv_initialize()
{
    once = false;
    gameover = false;
    skipGameCycleA = false;
    skipGameCycleB = false;

    // Setting up games
    attachedGameA = this->areaA->getAttachedGame();
    attachedGameB = this->areaB->getAttachedGame();
    this->areaA->setEnemyGame(attachedGameB);
    this->areaB->setEnemyGame(attachedGameA);

    setReceiveUpEvents(true);
    setFocusable(true);

    // TODO: move elsewhere
    // Load and preload a few FX for the game
    for (int i=0; i<3; ++i)
        m_visualFX.push_back(new VisualFX("fx/vanish.gsl", *(areaA->getFloboSetTheme())));
    for (int i=0; i<3; ++i)
        m_visualFX.push_back(new VisualFX("fx/combo.gsl", *(areaA->getFloboSetTheme())));
}

GameWidget2P::~GameWidget2P()
{
    dead();
}

void GameWidget2P::cycle()
{
  if (!m_paused) {
    tickCounts++;
    cycles++;

    int animCyclesBeforeGameCycles = (MaxSpeed + (((MinSpeed - MaxSpeed) * gameSpeed) / 20));

    // Cycling through the foreground animation
    if (m_styroPainter.get() != NULL)
        m_styroPainter->update();

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
  if ((gameover || getAborted()) && !once) {
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

void GameWidget2P::drawBackground(DrawTarget *dt)
{
    IosRect dtRect = { 0, 0, dt->w, dt->h };
    dt->draw(getLevelTheme()->getBackground(), &dtRect, &dtRect);
}

void GameWidget2P::drawGameAreas(DrawTarget *dt)
{
    areaA->render(dt);
    areaB->render(dt);
}

void GameWidget2P::drawGameNeutrals(DrawTarget *dt)
{
    areaA->renderNeutral(dt);
    areaB->renderNeutral(dt);
}

void GameWidget2P::draw(DrawTarget *dt)
{
    if ((m_paused) && (m_obscureScreenOnPause)) {
        dt->draw(m_painterGameScreen, NULL, NULL);
        return;
    }
    // Render the background
    drawBackground(dt);
    // Rendering the opponent if it is behind the flobos
    if (getLevelTheme()->getOpponentIsBehind()) {
        if (getOpponent() != NULL)
            getOpponent()->render(dt);
    }
    // Rendering flobo views
    drawGameAreas(dt);
    // Rendering the grids
    IosRect drect;
    IosSurface * grid = getLevelTheme()->getGrid();
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
    if (m_styroPainter.get() != NULL)
        m_styroPainter->draw(dt);
    // Rendering the neutral flobos
    drawGameNeutrals(dt);
    // Rendering the lives
    if (displayLives && (lives>=0) && (lives<=3))
    {
        IosSurface * liveImage = getLevelTheme()->getLifeForIndex(lives);
        drect.x = getLevelTheme()->getLifeDisplayX();
        drect.y = getLevelTheme()->getLifeDisplayY();
        drect.w = liveImage->w;
        drect.h = liveImage->h;
        dt->draw(liveImage, NULL, &drect);
    }
    // Rendering the victories
    m_victoryDisplayA->draw(dt);
    m_victoryDisplayB->draw(dt);
    // Rendering the game speed meter
    IosRect speedRect;
    IosSurface * speedFront = getLevelTheme()->getSpeedMeter(true);
    IosSurface * speedBack  = getLevelTheme()->getSpeedMeter(false);
    speedRect.x = 0;
    speedRect.w = speedFront->w;
    speedRect.h = gameSpeed * 6;
    speedRect.y = speedFront->h - speedRect.h;
    drect.x = getLevelTheme()->getSpeedMeterX() - speedRect.w / 2;
    drect.y = getLevelTheme()->getSpeedMeterY() - speedRect.h;
    drect.w = speedRect.w;
    drect.h = speedRect.h;
    IosRect speedBlackRect = speedRect;
    IosRect drectBlack     = drect;
    speedBlackRect.h = speedFront->h - speedRect.h;
    speedBlackRect.y = 0;
    drectBlack.y = getLevelTheme()->getSpeedMeterY() - speedFront->h;
    drectBlack.h = speedBlackRect.h;
    dt->draw(speedBack,&speedBlackRect,&drectBlack);
    dt->draw(speedFront,&speedRect, &drect);
    // Rendering the scores
    areaA->renderScore(dt);
    areaB->renderScore(dt);
    // Rendering the player names
    IosFont *font = getLevelTheme()->getPlayerNameFont();
    const RGBA *color = getLevelTheme()->getPlayerNameColor();
    if (m_displayPlayerOneName)
        dt->putStringCenteredXY(font,
                                getLevelTheme()->getNameDisplayX(0),
                                getLevelTheme()->getNameDisplayY(0),
                                playerOneName, *color);
    if (m_displayPlayerTwoName)
        dt->putStringCenteredXY(font,
                                getLevelTheme()->getNameDisplayX(1),
                                getLevelTheme()->getNameDisplayY(1),
                                playerTwoName, *color);
    // Rendering the opponent if it is in front
    if (! getLevelTheme()->getOpponentIsBehind()) {
        if (getOpponent() != NULL)
            getOpponent()->render(dt);
    }
}

void GameWidget2P::addSubWidget(Widget *subWidget)
{
  m_subwidgets.push_back(subWidget);
}

void GameWidget2P::eventOccured(GameControlEvent *event)
{
    if (m_paused)
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

bool GameWidget2P::startPressed()
{
    if ((gameover || getAborted()) && once && (ios_fc::getTimeMs() > gameOverDate + 500)) {
        actionAfterGameOver(true, GAMEOVER_STARTPRESSED);
        return true;
    }
    else if (m_paused) {
        actionAfterGameOver(true, PAUSED_STARTPRESSED);
    }
    return false;
}

bool GameWidget2P::backPressed()
{
    if ((gameover || getAborted()) && once) {
        actionAfterGameOver(true, GAMEOVER_STARTPRESSED);
        return true;
    }
    return false;
}

void GameWidget2P::actionAfterGameOver(bool fromControls, int actionType)
{
    if (gameOverAction)
      gameOverAction->action(this, actionType, NULL);
}


std::vector<VisualFX*> *activeFX = NULL;

void EventFX(const char *name, float x, float y, int player)
{
    if (activeFX == NULL) return;
    GTLogTrace("EventFX trigged with name=%s\n", name);

    VisualFX *supporting_fx = NULL;
    for (unsigned int i=0; i<activeFX->size(); ++i) {

        // Find an FX supporting this event
        VisualFX *fx = (*activeFX)[i];
        if (fx->supportFX(name)) {
            supporting_fx = fx;
            // FX not busy, report it the event
            if (!fx->busy()) {
                GTLogTrace("EventFX %s posted\n", name);
                fx->postEvent(name, x, y, player);
                return;
            }
        }
    }

    // All FX are busy, if we find an FX supporting this event
    // we clone it an report the FX to the newly created one.
    if (supporting_fx != NULL) {
        VisualFX *fx = supporting_fx->clone();
        activeFX->push_back(fx);
        fx->getGameScreen()->add(fx);
        fx->postEvent(name, x, y, player);
    }
}


