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
#ifndef _SOLOGAMESTARTER_H_
#define _SOLOGAMESTARTER_H_

#include "GameStateMachine.h"
#include "LocalGameStates.h"
#include "EventPlayer.h"

class VuMeter {
public:
    VuMeter(Vec3 position, IosSurface *front, IosSurface *back);
    virtual ~VuMeter() {}
    void draw(DrawTarget *dt);
    void step();
    void setValue(double value) { _targetValue = value; }
private:
    Vec3 _position;
    IosSurface *_front;
    IosSurface *_back;
    double _targetValue;
    double _value;
};

struct GameParameterSetting {
    double initialValue;
    double evolution;
    double finalValue;
};

struct SoloGameSettings {
    GameParameterSetting cyclesDuration;
    GameParameterSetting levelIncrease;
    GameParameterSetting handicapIncrease;
    GameParameterSetting handicapDecreaseOnPhase1;
    GameParameterSetting handicapDecreaseAbovePhase1;
};

class GameParameter {
public:
    GameParameter(GameParameterSetting &setting)
        : _setting(setting), _value(setting.initialValue), _steps(0), _noEvo(false)
    {
        if (setting.evolution != 0.)
            _steps = (setting.finalValue - setting.initialValue) / setting.evolution;
        else
            _noEvo = true;
    }
    double getValue() const { return _value; }
    void step() {
        if (_noEvo)
            return;
        if (_steps <= 0) {
            _value = _setting.finalValue;
            _noEvo = true;
        }
        else {
            _value += _setting.evolution;
            --_steps;
        }
    }
private:
    GameParameterSetting _setting;
    double _value;
    int _steps;
    bool _noEvo;
};

class SoloGameWidget : public GameWidget, GameListener, CycledComponent {
public:
    SoloGameWidget(SoloGameSettings &gameSettings, FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, Action *gameOverAction = NULL);
    // GameWidget implementation
    virtual void setGameOptions(GameOptions options);
    // Callbacks
    virtual bool backPressed();
    virtual bool startPressed();
    //
    virtual StoryWidget *getOpponent();
    // TODO: Make this N-players generic (N from 1 to +inf)
    virtual void setPlayerOneName(const std::string & newName);
    virtual void setPlayerTwoName(const std::string & newName);
    virtual PlayerGameStat &getStatPlayerOne();
    virtual PlayerGameStat &getStatPlayerTwo();
    virtual void addGameAHandicap(int handicap);
    virtual void addGameBHandicap(int handicap);
    virtual bool isGameARunning() const;
    // GameListener implementation
    virtual void gameDidEndCycle();
    virtual void floboWillVanish(std::vector<std::shared_ptr<Flobo>> &floboGroup, int groupNum, int phase);
    // CycledComponent implementation
    virtual void cycle();
    // Widget methods
    void draw(DrawTarget *dt);
    IdleComponent *getIdleComponent() { return this; }
    void eventOccured(event_manager::GameControlEvent *event);
protected:
    GameParameter _cyclesDuration;
    GameParameter _levelIncrease;
    GameParameter _handicapIncrease;
    GameParameter _handicapDecreaseOnPhase1;
    GameParameter _handicapDecreaseAbovePhase1;
    FloboSetTheme &_attachedFloboThemeSet;
    RandomSystem _attachedRandom;
    std::unique_ptr<LocalGameFactory> _gameFactory;
    std::unique_ptr<GameView>        _areaA;
    std::unique_ptr<GamePlayer> _playerController;
    GameOptions _options;
    int _cyclesBeforeGameCycle;
    double _cyclesBeforeLevelRaise;
    std::unique_ptr<VuMeter> _comboMeter;
    double _comboHandicap;
    bool _comboHandicap75, _comboHandicap85, _comboHandicap100;
    std::string _playerName;
};

class SoloGameWidgetFactory : public GameWidgetFactory {
public:
    SoloGameWidgetFactory(SoloGameSettings &gameSettings)
        : _gameSettings(gameSettings) {}
    GameWidget *createGameWidget(FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, const std::string & centerFace, Action *gameOverAction)
    {
        return new SoloGameWidget(_gameSettings, floboSetTheme, levelTheme, gameOverAction);
    }
private:
    SoloGameSettings _gameSettings;
};

class SoloModeStarterAction : public Action {
public:
    SoloModeStarterAction(GameDifficulty difficulty,
                          PlayerNameProvider *nameProvider = NULL);
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType,
			event_manager::GameControlEvent *event);
    // Own methods
    void pauseGameIfPossible();
protected:
    SoloGameSettings _gameSettings;
    std::unique_ptr<SoloGameWidgetFactory>  _gameWidgetFactory;
    GameStateMachine _stateMachine;
    SharedMatchAssets _sharedAssets;
    SharedGetReadyAssets        _sharedGetReadyAssets;
    std::unique_ptr<PushScreenState>        _pushGameScreen;
    std::unique_ptr<SetupMatchState>        _setupMatch;
    std::unique_ptr<EnterPlayerReadyState>  _enterPlayersReady;
    std::unique_ptr<ExitPlayerReadyState>   _exitPlayersReady;
    std::unique_ptr<MatchPlayingState>      _matchPlaying;
    std::unique_ptr<MatchIsOverState>       _matchIsOver;
    std::unique_ptr<ManageHiScoresState>    _hallOfFame;
    std::unique_ptr<LeaveGameState>         _leaveGame;
};

#endif // _SOLOGAMESTARTER_H_
