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
    _client._styroClient.loadImage = styro_loadImage;
    _client._styroClient.drawImage = styro_drawImage;
    _client._styroClient.freeImage = styro_freeImage;
    _client._styroClient.putText   = NULL;
    _client._styroClient.getText   = NULL;
    _client._styroClient.music = NULL;
    _client._styroClient.playSound = NULL;
    _client._styroClient.resolveFilePath = NULL;
    _client._styroClient.openFile = openFileFunction;
    _client._styroClient.closeFile = closeFileFunction;
    _client._styroClient.readFile = readFileFunction;
    _client._painter = NULL;
    _client._theme = theme;
    // Initialize the animation
    _animation = styrolyse_new(theme->getForegroundAnimation().c_str(),
                                &(_client._styroClient), false);
}

StyrolysePainterClient::~StyrolysePainterClient()
{
    styrolyse_free(_animation);
}

void StyrolysePainterClient::update()
{
    styrolyse_update(_animation, 0.);
}

void StyrolysePainterClient::draw(DrawTarget *dt)
{
    _client._painter = dt;
    styrolyse_draw(_animation);
}

void *StyrolysePainterClient::styro_loadImage(StyrolyseClient *_this, const char *path)
{
    StyroImage *image;
    image = new StyroImage(_this,
        FilePath(((ExtendedClient *)_this)->_theme->getThemeRootPath())
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
    rect.h = surf->_surface->h;
    rect.w = surf->_surface->w;
    cliprect.x = clipx;
    cliprect.y = clipy;
    cliprect.w = clipw;
    cliprect.h = cliph;
    ((ExtendedClient *)_this)->_painter->setClipRect(&cliprect);
    if (flipped)
		((ExtendedClient *)_this)->_painter->drawHFlipped(surf->_surface, NULL, &rect);
    else {
        if (fabs(scaleX - 1.0f) > 0.001f) {
            rect.w *= scaleX;
            rect.h *= scaleY;
        }
        ((ExtendedClient *)_this)->_painter->draw(surf->_surface, NULL, &rect);
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
        go._MIN_SPEED = 4;
        go._MAX_SPEED = 20;
        break;
    case MEDIUM:
        go._MIN_SPEED = 2;
        go._MAX_SPEED = 15;
        break;
    case HARD:
    default:
        go._MIN_SPEED = 1;
        go._MAX_SPEED = 8;
        break;
    }
    return go;
}

VictoryDisplay::VictoryDisplay(Vec3 position, IosSurface *trophy, int victories)
  : _position(position), _trophy(trophy), _victories(victories)
{
}

void VictoryDisplay::draw(DrawTarget *dt)
{
    if (_victories == 0)
        return;
    int offx = (_trophy->w * _victories) / 2;
    IosRect dst = { (int)_position.x - offx, (int)_position.y, _trophy->w, _trophy->h };
    for (int i = 0 ; i < _victories ; ++i) {
        dt->draw(_trophy, NULL, &dst);
        dst.x += _trophy->w;
    }
}

GameWidget::GameWidget()
  : _levelTheme(NULL),
    _gameOverAction(NULL), _associatedScreen(NULL),
    _paused(false), _obscureScreenOnPause(true),
    _abortedFlag(false)
{
    ImageLibrary &iimLib = GameUIDefaults::GAME_LOOP->getDrawContext()->getImageLibrary();
    _painterGameScreen = iimLib.createImage(IMAGE_RGB, GameUIDefaults::GAME_LOOP->getDrawContext()->w, GameUIDefaults::GAME_LOOP->getDrawContext()->h);
}

GameWidget::~GameWidget()
{
    for (unsigned int i=0; i<_visualFX.size(); ++i)
        delete _visualFX[i];
}

void GameWidget::setScreenToPaused(bool fromControls)
{
    if (_associatedScreen != NULL)
        _associatedScreen->setPaused(fromControls);
}

void GameWidget::setScreenToResumed(bool fromControls)
{
  if (_associatedScreen != NULL)
    if (!fromControls)
      _associatedScreen->getPauseMenu().backPressed(false);
}

void GameWidget::setLevelTheme(LevelTheme *levelTheme)
{
    _levelTheme = levelTheme;
    if (_levelTheme->getForegroundAnimation() != "")
        _styroPainter.reset(new StyrolysePainterClient(levelTheme));
}

void GameWidget::pause(bool obscureScreen)
{
    // Call draw on offscreen surface before setting the game to paused
    if (obscureScreen) {
        draw(_painterGameScreen);
    }
    _paused = true;
    _obscureScreenOnPause = obscureScreen;
    // Draw the obscured screen on display
    if (_obscureScreenOnPause) {
        _painterGameScreen->convertToGray();
        requestDraw();
    }
}

void GameWidget::resume()
{
    _paused = false;
    setFocusable(true);
}





void GameWidget2P::setGameOptions(GameOptions game_options)
{
    _cyclesBeforeSpeedIncreases = game_options._CYCLES_BEFORE_SPEED_INCREASES;
    _MinSpeed = game_options._MIN_SPEED;
    _MaxSpeed = game_options._MAX_SPEED;
}

void GameWidget2P::setVictories(int left, int right)
{
    _victoryDisplayA->setValue(left);
    _victoryDisplayB->setValue(right);
}

void GameWidget2P::setPlayerOneName(const std::string & newName) {
    _playerOneName = newName;
    _areaA->setPlayerNames(_playerOneName, _playerTwoName);
    _areaB->setPlayerNames(_playerOneName, _playerTwoName);
}
void GameWidget2P::setPlayerTwoName(const std::string &newName) {
    _playerTwoName = newName;
    _areaA->setPlayerNames(_playerOneName, _playerTwoName);
    _areaB->setPlayerNames(_playerOneName, _playerTwoName);
}

GameWidget2P::GameWidget2P(GameOptions game_options)
    : CycledComponent(TIME_BETWEEN_GAME_CYCLES),
      _painter(*(GameUIDefaults::GAME_LOOP->getDrawContext())), _cyclesBeforeGameCycle(0),
      _cyclesBeforeSpeedIncreases(game_options._CYCLES_BEFORE_SPEED_INCREASES),
      _tickCounts(0), _cycles(0),
      _displayLives(true), _lives(3), _gameSpeed(0),
      _MinSpeed(game_options._MIN_SPEED), _MaxSpeed(game_options._MAX_SPEED),
      _blinkingPointsA(0), _blinkingPointsB(0), _savePointsA(0), _savePointsB(0),
      _playerOneName(p1name), _playerTwoName(p2name),
      _displayPlayerOneName(true), _displayPlayerTwoName(true)
{
}

void GameWidget2P::initWithGUI(GameView &areaA, GameView &areaB, LevelTheme &levelTheme, Action *gameOverAction)
{
    this->_areaA = &areaA;
    this->_areaB = &areaB;
    areaA.setPlayerNames(_playerOneName, _playerTwoName);
    areaB.setPlayerNames(_playerOneName, _playerTwoName);
    this->_gameOverAction = gameOverAction;
    priv_initialize();
    setLevelTheme(&levelTheme);
    _victoryDisplayA.reset(new VictoryDisplay(Vec3(levelTheme.getTrophyDisplayX(0), levelTheme.getTrophyDisplayY(0)), levelTheme.getTrophy()));
    _victoryDisplayB.reset(new VictoryDisplay(Vec3(levelTheme.getTrophyDisplayX(1), levelTheme.getTrophyDisplayY(1)), levelTheme.getTrophy()));
}

void GameWidget2P::priv_initialize()
{
    _once = false;
    _gameover = false;
    _skipGameCycleA = false;
    _skipGameCycleB = false;

    // Setting up games
    _attachedGameA = this->_areaA->getAttachedGame();
    _attachedGameB = this->_areaB->getAttachedGame();
    this->_areaA->setEnemyGame(_attachedGameB);
    this->_areaB->setEnemyGame(_attachedGameA);

    setReceiveUpEvents(true);
    setFocusable(true);

    // TODO: move elsewhere
    // Load and preload a few FX for the game
    for (int i=0; i<3; ++i)
        _visualFX.push_back(new VisualFX("fx/vanish.gsl", *(_areaA->getFloboSetTheme())));
    for (int i=0; i<3; ++i)
        _visualFX.push_back(new VisualFX("fx/combo.gsl", *(_areaA->getFloboSetTheme())));
}

GameWidget2P::~GameWidget2P()
{
    dead();
}

void GameWidget2P::cycle()
{
  if (!_paused) {
    _tickCounts++;
    _cycles++;

    int animCyclesBeforeGameCycles = (_MaxSpeed + (((_MinSpeed - _MaxSpeed) * _gameSpeed) / 20));

    // Cycling through the foreground animation
    if (_styroPainter.get() != NULL)
        _styroPainter->update();

    // Controls
    _controllerA->cycle();
    _controllerB->cycle();

    if (!_skipGameCycleA && _areaA->isNewMetaCycleStart() && (_cyclesBeforeGameCycle < animCyclesBeforeGameCycles/2))
      _skipGameCycleA = true;
    if (!_skipGameCycleB && _areaB->isNewMetaCycleStart() && (_cyclesBeforeGameCycle < animCyclesBeforeGameCycles/2))
      _skipGameCycleB = true;

    // Animations
    _areaA->cycleAnimation();
    _areaB->cycleAnimation();

    if (!_skipGameCycleA && _areaA->isNewMetaCycleStart() && (_cyclesBeforeGameCycle < animCyclesBeforeGameCycles/2))
      _skipGameCycleA = true;
    if (!_skipGameCycleB && _areaB->isNewMetaCycleStart() && (_cyclesBeforeGameCycle < animCyclesBeforeGameCycles/2))
      _skipGameCycleB = true;

    // Game cycles
    if (_cyclesBeforeGameCycle == 0) {

      _areaA->clearMetaCycleStart();
      _areaB->clearMetaCycleStart();

      _cyclesBeforeGameCycle = animCyclesBeforeGameCycles;
      if (!_skipGameCycleA)
      {
        _areaA->cycleGame();
      }
      else _skipGameCycleA = false;

      if (!_skipGameCycleB)
      {
        _areaB->cycleGame();
      }
      else _skipGameCycleB = false;

      // Blinking point animation
      // TODO
      /*
      if (attachedGameA->getPoints()/50000 > savePointsA/50000)
        blinkingPointsA = 10;
      if (attachedGameB->getPoints()/50000 > savePointsB/50000)
        blinkingPointsB = 10;
        */

      if (_blinkingPointsA > 0)
        _blinkingPointsA--;
      if (_blinkingPointsB > 0)
        _blinkingPointsB--;

      /*
      savePointsB = attachedGameB->getPoints();
      savePointsA = attachedGameA->getPoints();

      if (savePointsA < 50000) blinkingPointsA=0;
      if (savePointsB < 50000) blinkingPointsB=0;
      */
      // end of the blinking point animation code
    }

    _cyclesBeforeGameCycle--;

    if (_tickCounts == (unsigned int)_cyclesBeforeSpeedIncreases)
    {
      _tickCounts = 0;
      if (_gameSpeed < 20) _gameSpeed++;
      _cyclesBeforeSpeedIncreases = _cyclesBeforeSpeedIncreases * (20 + _gameSpeed) / 20;
      //printf("Changing speed: %d (next in %d)\n", gameSpeed, cyclesBeforeSpeedIncreases);
    }
    requestDraw();
  }
  _gameover = (_areaA->isGameOver() || _areaB->isGameOver());
  if ((_gameover || getAborted()) && !_once) {
    _once = true;
    _gameOverDate = ios_fc::getTimeMs();
    if (_areaA->isGameOver())
      _areaB->gameWin();
    if (_areaB->isGameOver())
      _areaA->gameWin();
    if (_gameOverAction)
      _gameOverAction->action(this, GAME_IS_OVER, NULL);
  }
}

void GameWidget2P::drawBackground(DrawTarget *dt)
{
    IosRect dtRect = { 0, 0, dt->w, dt->h };
    dt->draw(getLevelTheme()->getBackground(), &dtRect, &dtRect);
}

void GameWidget2P::drawGameAreas(DrawTarget *dt)
{
    _areaA->render(dt);
    _areaB->render(dt);
}

void GameWidget2P::drawGameNeutrals(DrawTarget *dt)
{
    _areaA->renderNeutral(dt);
    _areaB->renderNeutral(dt);
}

void GameWidget2P::draw(DrawTarget *dt)
{
    if ((_paused) && (_obscureScreenOnPause)) {
        dt->draw(_painterGameScreen, NULL, NULL);
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
    if (_styroPainter.get() != NULL)
        _styroPainter->draw(dt);
    // Rendering the neutral flobos
    drawGameNeutrals(dt);
    // Rendering the lives
    if (_displayLives && (_lives>=0) && (_lives<=3))
    {
        IosSurface * liveImage = getLevelTheme()->getLifeForIndex(_lives);
        drect.x = getLevelTheme()->getLifeDisplayX();
        drect.y = getLevelTheme()->getLifeDisplayY();
        drect.w = liveImage->w;
        drect.h = liveImage->h;
        dt->draw(liveImage, NULL, &drect);
    }
    // Rendering the victories
    _victoryDisplayA->draw(dt);
    _victoryDisplayB->draw(dt);
    // Rendering the game speed meter
    IosRect speedRect;
    IosSurface * speedFront = getLevelTheme()->getSpeedMeter(true);
    IosSurface * speedBack  = getLevelTheme()->getSpeedMeter(false);
    speedRect.x = 0;
    speedRect.w = speedFront->w;
    speedRect.h = _gameSpeed * 6;
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
    _areaA->renderScore(dt);
    _areaB->renderScore(dt);
    // Rendering the player names
    IosFont *font = getLevelTheme()->getPlayerNameFont();
    const RGBA *color = getLevelTheme()->getPlayerNameColor();
    if (_displayPlayerOneName)
        dt->putStringCenteredXY(font,
                                getLevelTheme()->getNameDisplayX(0),
                                getLevelTheme()->getNameDisplayY(0),
                                _playerOneName.c_str(), *color);
    if (_displayPlayerTwoName)
        dt->putStringCenteredXY(font,
                                getLevelTheme()->getNameDisplayX(1),
                                getLevelTheme()->getNameDisplayY(1),
                                _playerTwoName.c_str(), *color);
    // Rendering the opponent if it is in front
    if (! getLevelTheme()->getOpponentIsBehind()) {
        if (getOpponent() != NULL)
            getOpponent()->render(dt);
    }
}

void GameWidget2P::addSubWidget(Widget *subWidget)
{
  _subwidgets.push_back(subWidget);
}

void GameWidget2P::eventOccured(GameControlEvent *event)
{
    if (_paused)
        lostFocus();
    else {
        _controllerA->eventOccured(event);
        _controllerB->eventOccured(event);
        for (std::vector<gameui::Widget *>::iterator iter = _subwidgets.begin() ;
             iter != _subwidgets.end() ; iter++) {
            (*iter)->eventOccured(event);
        }
    }
}

bool GameWidget2P::startPressed()
{
    if ((_gameover || getAborted()) && _once && (ios_fc::getTimeMs() > _gameOverDate + 500)) {
        actionAfterGameOver(true, GAMEOVER_STARTPRESSED);
        return true;
    }
    else if (_paused) {
        actionAfterGameOver(true, PAUSED_STARTPRESSED);
    }
    return false;
}

bool GameWidget2P::backPressed()
{
    if ((_gameover || getAborted()) && _once) {
        actionAfterGameOver(true, GAMEOVER_STARTPRESSED);
        return true;
    }
    return false;
}

void GameWidget2P::actionAfterGameOver(bool fromControls, int actionType)
{
    if (_gameOverAction)
      _gameOverAction->action(this, actionType, NULL);
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
