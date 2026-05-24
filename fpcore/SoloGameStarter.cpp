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

#include "SoloGameStarter.h"
#include "GTLog.h"

using namespace event_manager;

VuMeter::VuMeter(Vec3 position, IosSurface *front, IosSurface *back)
    : _position(position), _front(front), _back(back),
      _targetValue(0.), _value(0.)
{
}

void VuMeter::draw(DrawTarget *dt)
{
    IosRect meterRect, drect;
    meterRect.x = 0;
    meterRect.w = _front->w;
    meterRect.h = _front->h * _value;
    meterRect.y = _front->h - meterRect.h;
    drect.x = _position.x - meterRect.w / 2;
    drect.y = _position.y - meterRect.h;
    drect.w = meterRect.w;
    drect.h = meterRect.h;
    IosRect meterBlackRect = meterRect;
    IosRect drectBlack     = drect;
    meterBlackRect.h = _front->h - meterRect.h;
    meterBlackRect.y = 0;
    drectBlack.y = _position.y - _front->h;
    drectBlack.h = meterBlackRect.h;
    dt->draw(_back,&meterBlackRect,&drectBlack);
    dt->draw(_front,&meterRect, &drect);
}

void VuMeter::step()
{
    _value += (_targetValue - _value)/10.;
}

#define TIME_BETWEEN_GAME_CYCLES 0.02

SoloGameWidget::SoloGameWidget(SoloGameSettings &gameSettings, FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, Action *_gameOverAction)
    : CycledComponent(TIME_BETWEEN_GAME_CYCLES),
      _cyclesDuration(gameSettings.cyclesDuration),
      _levelIncrease(gameSettings.levelIncrease),
      _handicapIncrease(gameSettings.handicapIncrease),
      _handicapDecreaseOnPhase1(gameSettings.handicapDecreaseOnPhase1),
      _handicapDecreaseAbovePhase1(gameSettings.handicapDecreaseAbovePhase1),
      _attachedFloboThemeSet(floboSetTheme), _attachedRandom(5),
      _cyclesBeforeGameCycle(0), _cyclesBeforeLevelRaise(1000.),
      _comboHandicap(0.), _comboHandicap75(false), _comboHandicap85(false), _comboHandicap100(false)
{
    _gameFactory.reset(new LocalGameFactory(&_attachedRandom));
    _areaA.reset(new GameView(_gameFactory.get(), 0, &floboSetTheme, &levelTheme));
    _playerController.reset(new CombinedEventPlayer(*_areaA));
    //initWithGUI(*_areaA, NULL, *_playerController, NULL, levelTheme, _gameOverAction);
    //setLives(-1);
    setReceiveUpEvents(true);
    setFocusable(true);
    _areaA->getAttachedGame()->addGameListener(this);
    setLevelTheme(&levelTheme);
    _comboMeter.reset(new VuMeter(Vec3(levelTheme.getSpeedMeterX(),
                                        levelTheme.getSpeedMeterY()),
                                   levelTheme.getSpeedMeter(true),
                                   levelTheme.getSpeedMeter(false)));

    // TODO: move elsewhere
    // Load and preload a few FX for the game
    for (int i=0; i<3; ++i)
        _visualFX.push_back(new VisualFX("fx/vanish.gsl", *(_areaA->getFloboSetTheme())));
    for (int i=0; i<3; ++i)
        _visualFX.push_back(new VisualFX("fx/combo.gsl", *(_areaA->getFloboSetTheme())));
    for (int i=0; i<1; ++i)
        _visualFX.push_back(new VisualFX("fx/starvedcombo.gsl", *(_areaA->getFloboSetTheme())));
    for (int i=0; i<1; ++i)
        _visualFX.push_back(new VisualFX("fx/penaltycleared.gsl", *(_areaA->getFloboSetTheme())));
    for (int i=0; i<1; ++i)
        _visualFX.push_back(new VisualFX("fx/combopenalty.gsl", *(_areaA->getFloboSetTheme())));
}

void SoloGameWidget::gameDidEndCycle()
{
    if (_cyclesBeforeLevelRaise <= 0.) {
        _areaA->getAttachedGame()->addNeutralLayer();
        _cyclesBeforeLevelRaise = 1000.;
    }
    if (_comboHandicap100) {
        _comboHandicap75 = false;
        _comboHandicap85 = false;
        _comboHandicap100 = false;
        _comboHandicap = 0.;
        if (_areaA->getAttachedGame()->getNeutralFlobos() > 0)
            EventFX("combopenalty", 20, 20, 1);
    }
}

void SoloGameWidget::floboWillVanish(std::vector<std::shared_ptr<Flobo>> &floboGroup, int groupNum, int phase)
{
    double prevComboHandicap = _comboHandicap;
    if (phase == 1)
        _comboHandicap += _handicapDecreaseOnPhase1.getValue();
    if (phase >= 2) {
        _comboHandicap += _handicapDecreaseAbovePhase1.getValue();
    }
    if (phase == 4)
        _comboHandicap = 0.;
    if (_comboHandicap <= 0.) {
        _comboHandicap = 0.;
        _comboHandicap75 = false;
        _comboHandicap85 = false;
        _comboHandicap100 = false;
    }
    if ((_comboHandicap <= 0.) && (prevComboHandicap >= 50.)) {
        EventFX("penaltycleared", 20, 20, 1);
    }
}

void SoloGameWidget::cycle()
{
    bool isGameOver = _areaA->isGameOver() || getAborted();
    if ((!_paused) && (!isGameOver)) {
        theCommander->playMusicTrack("herbert");
        // Game parameters
        _cyclesDuration.step();
        _levelIncrease.step();
        _handicapIncrease.step();
        _handicapDecreaseOnPhase1.step();
        _handicapDecreaseAbovePhase1.step();
        // Controls
        _playerController->cycle();
        // Cycling through the foreground animation
        if (_styroPainter.get() != NULL)
            _styroPainter->update();
        // Animations
        _comboMeter->step();
        _areaA->cycleAnimation();
        if (_cyclesBeforeGameCycle == 0) {
            if (! _areaA->isNewMetaCycleStart())
                _areaA->cycleGame();
            _areaA->clearMetaCycleStart();
            _cyclesBeforeGameCycle = _cyclesDuration.getValue();
        }
        if (_cyclesBeforeLevelRaise <= 0.) {
        }
        else {
            _cyclesBeforeLevelRaise -= _levelIncrease.getValue();
        }
        _cyclesBeforeGameCycle--;
        if (_comboHandicap < 100.)
            _comboHandicap += _handicapIncrease.getValue();
        // Warning events on handicap
        if ((_comboHandicap > 75.) && (!_comboHandicap75)) {
            EventFX("starvedcombo", 20, 20, 1);
            _comboHandicap75 = true;
        }
        if ((_comboHandicap > 85.) && (!_comboHandicap85)) {
            // EventFX("starvedcombo", 20, 20, 1);
            _comboHandicap85 = true;
        }
        if ((_comboHandicap >= 100.) && (!_comboHandicap100)) {
            _areaA->getAttachedGame()->increaseNeutralFlobos(6);
            _comboHandicap100 = true;
        }
        requestDraw();
    }
    if (isGameOver) {
        if (_gameOverAction)
            _gameOverAction->action(this, GAME_IS_OVER, NULL);
    }
}

void SoloGameWidget::draw(DrawTarget *dt)
{
    if ((_paused) && (_obscureScreenOnPause)) {
        dt->draw(_painterGameScreen, NULL, NULL);
        return;
    }
    IosRect dtRect = { 0, 0, dt->w, dt->h };
    dt->draw(getLevelTheme()->getBackground(), &dtRect, &dtRect);
    _areaA->render(dt);
    // Rendering the foreground animation
    if (_styroPainter.get() != NULL)
        _styroPainter->draw(dt);
    // Rendering the combo meter
    _comboMeter->setValue(_comboHandicap / 100.);
    _comboMeter->draw(dt);
    // Rendering the scores
    _areaA->renderScore(dt);
    // Rendering the player names
    IosFont *font = getLevelTheme()->getPlayerNameFont();
    const RGBA *color = getLevelTheme()->getPlayerNameColor();
    dt->putStringCenteredXY(font,
                            getLevelTheme()->getNameDisplayX(0),
                            getLevelTheme()->getNameDisplayY(0),
                            _playerName.c_str(), *color);
    // Rendering the neutral puyos
    _areaA->renderNeutral(dt);
}

void SoloGameWidget::eventOccured(GameControlEvent *event)
{
    if (_paused)
        lostFocus();
    else {
        _playerController->eventOccured(event);
        //for (std::vector<gameui::Widget *>::iterator iter = _subwidgets.begin() ;
        //     iter != _subwidgets.end() ; iter++) {
        //    (*iter)->eventOccured(event);
        //}
    }
}

void SoloGameWidget::setGameOptions(GameOptions options)
{
    _options = options;
}
bool SoloGameWidget::backPressed()
{
    if (_areaA->isGameOver() || getAborted()) {
        _gameOverAction->action(this, GAMEOVER_STARTPRESSED, NULL);
        return true;
    }
    return false;
}
bool SoloGameWidget::startPressed()
{
    if (_areaA->isGameOver() || getAborted()) {
        _gameOverAction->action(this, GAMEOVER_STARTPRESSED, NULL);
        return true;
    }
    return false;
}
StoryWidget *SoloGameWidget::getOpponent()
{
    return NULL;
}
void SoloGameWidget::setPlayerOneName(const std::string & newName)
{
    _playerName = newName;
}
void SoloGameWidget::setPlayerTwoName(const std::string & newName)
{}
PlayerGameStat &SoloGameWidget::getStatPlayerOne()
{
    return _areaA->getAttachedGame()->getGameStat();
}
PlayerGameStat &SoloGameWidget::getStatPlayerTwo()
{
    return _areaA->getAttachedGame()->getGameStat();
}
void SoloGameWidget::addGameAHandicap(int handicap)
{}
void SoloGameWidget::addGameBHandicap(int handicap)
{}
bool SoloGameWidget::isGameARunning() const
{
    return ! _areaA->isGameOver();
}


//---------------------------------
// Solo mode game state machine
//---------------------------------
SoloModeStarterAction::SoloModeStarterAction(GameDifficulty difficulty, PlayerNameProvider *nameProvider)
{
    // Set the game parameters depending on the game difficulty
    SoloGameSettings easySettings = {
      { 20.,   -0.0002,  10. }, // cyclesDuration
      {  1.,    0.,      0. }, // levelIncrease
      {  0.03,  0.,      0. }, // handicapIncrease;
      { -7.,    0.,      0. }, // handicapDecreaseOnPhase1;
      {-40.,    0.,      0. }  // handicapDecreaseAbovePhase1;
    };
    SoloGameSettings mediumSettings = {
      { 15.,   -0.0002, 5. }, // cyclesDuration
      {  1.,    0.,     0. }, // levelIncrease
      {  0.05,  0.,     0. }, // handicapIncrease;
      { -7.,    0.,     0. }, // handicapDecreaseOnPhase1;
      {-40.,    0.,     0. }  // handicapDecreaseAbovePhase1;
    };
    SoloGameSettings hardSettings = {
      {  10.,  -0.0002,  0.   }, // cyclesDuration
      {  1.,    0.0001,  2.   }, // levelIncrease
      {  0.05,  0.00001, 0.15 }, // handicapIncrease;
      { -7.,    0.,      0.   }, // handicapDecreaseOnPhase1;
      {-40.,    0.,      0.   }  // handicapDecreaseAbovePhase1;
    };
    std::string scoreBoardSuffix;
    switch (difficulty) {
    case EASY:
        _gameSettings = easySettings;
        scoreBoardSuffix = "easy";
        break;
    case MEDIUM:
        _gameSettings = mediumSettings;
        scoreBoardSuffix = "medium";
        break;
    case HARD:
    default:
        _gameSettings = hardSettings;
        scoreBoardSuffix = "hard";
        break;
    };
    // Creating the game widget factory
    _gameWidgetFactory.reset(new SoloGameWidgetFactory(_gameSettings));
    // Creating the different game states
    _pushGameScreen.reset(new PushScreenState());
    _setupMatch.reset(new SetupMatchState(_gameWidgetFactory.get(), GameOptions::fromDifficulty(difficulty), nameProvider, &_sharedAssets, 1));
    _enterPlayersReady.reset(new EnterPlayerReadyState(_sharedAssets, _sharedGetReadyAssets));
    _exitPlayersReady.reset(new ExitPlayerReadyState(_sharedAssets, _sharedGetReadyAssets));
    _matchPlaying.reset(new MatchPlayingState(_sharedAssets));
    _matchIsOver.reset(new MatchIsOverState(_sharedAssets));
    _matchIsOver->setStyrolyse("gamelost1p.gsl");
    _hallOfFame.reset(new ManageHiScoresState(&_sharedAssets, nameProvider,
                                               (std::string(SOLO_SCOREBOARD_ID) + "." + scoreBoardSuffix).c_str(),
                                                  "gamewon_highscores_1p.gsl"));
    _leaveGame.reset(new LeaveGameState(_sharedAssets));
    // Linking the states together
    _pushGameScreen->setNextState(_setupMatch.get());
    _setupMatch->setNextState(_enterPlayersReady.get());
    _enterPlayersReady->setNextState(_exitPlayersReady.get());
    _exitPlayersReady->setNextState(_matchPlaying.get());
    _matchPlaying->setNextState(_matchIsOver.get());
    _matchPlaying->setAbortedState(_leaveGame.get());
    _matchIsOver->setNextState(_hallOfFame.get());
    _hallOfFame->setNextState(_leaveGame.get());
    // Initializing the state machine
    _stateMachine.setInitialState(_pushGameScreen.get());
}

void SoloModeStarterAction::action(Widget *sender, int actionType,
                                   event_manager::GameControlEvent *event)
{
    _stateMachine.reset();
    _stateMachine.evaluate();
}

void SoloModeStarterAction::pauseGameIfPossible()
{
    if (_sharedAssets._gameScreen.get() != NULL)
        _sharedAssets._gameScreen->setPaused(true);
}
