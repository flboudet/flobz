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
    void setValue(double value) { m_targetValue = value; }
private:
    Vec3 m_position;
    IosSurface *m_front;
    IosSurface *m_back;
    double m_targetValue;
    double m_value;
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
        : setting(setting), value(setting.initialValue), m_steps(0), m_noEvo(false)
    {
        if (setting.evolution != 0.)
            m_steps = (setting.finalValue - setting.initialValue) / setting.evolution;
        else
            m_noEvo = true;
    }
    double getValue() const { return value; }
    void step() {
        if (m_noEvo)
            return;
        if (m_steps <= 0) {
            value = setting.finalValue;
            m_noEvo = true;
        }
        else {
            value += setting.evolution;
            --m_steps;
        }
    }
private:
    GameParameterSetting setting;
    double value;
    int m_steps;
    bool m_noEvo;
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
    virtual void setPlayerOneName(String newName);
    virtual void setPlayerTwoName(String newName);
    virtual PlayerGameStat &getStatPlayerOne();
    virtual PlayerGameStat &getStatPlayerTwo();
    virtual void addGameAHandicap(int handicap);
    virtual void addGameBHandicap(int handicap);
    virtual bool isGameARunning() const;
    // GameListener implementation
    virtual void gameDidEndCycle();
    virtual void floboWillVanish(AdvancedBuffer<Flobo *> &floboGroup, int groupNum, int phase);
    // CycledComponent implementation
    virtual void cycle();
    // Widget methods
    void draw(DrawTarget *dt);
    IdleComponent *getIdleComponent() { return this; }
    void eventOccured(event_manager::GameControlEvent *event);
protected:
    GameParameter m_cyclesDuration;
    GameParameter m_levelIncrease;
    GameParameter m_handicapIncrease;
    GameParameter m_handicapDecreaseOnPhase1;
    GameParameter m_handicapDecreaseAbovePhase1;
    FloboSetTheme &attachedFloboThemeSet;
    RandomSystem attachedRandom;
    std::auto_ptr<LocalGameFactory> m_gameFactory;
    std::auto_ptr<GameView>        m_areaA;
    std::auto_ptr<GamePlayer> m_playerController;
    GameOptions m_options;
    int m_cyclesBeforeGameCycle;
    double m_cyclesBeforeLevelRaise;
    std::auto_ptr<VuMeter> m_comboMeter;
    double m_comboHandicap;
    bool m_comboHandicap75, m_comboHandicap85;
    std::string m_playerName;
};

class SoloGameWidgetFactory : public GameWidgetFactory {
public:
    SoloGameWidgetFactory(SoloGameSettings &gameSettings)
        : m_gameSettings(gameSettings) {}
    GameWidget *createGameWidget(FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, String centerFace, Action *gameOverAction)
    {
        return new SoloGameWidget(m_gameSettings, floboSetTheme, levelTheme, gameOverAction);
    }
private:
    SoloGameSettings m_gameSettings;
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
protected:
    SoloGameSettings m_gameSettings;
    std::auto_ptr<SoloGameWidgetFactory>  m_gameWidgetFactory;
    GameStateMachine m_stateMachine;
    SharedMatchAssets m_sharedAssets;
    SharedGetReadyAssets        m_sharedGetReadyAssets;
    std::auto_ptr<PushScreenState>        m_pushGameScreen;
    std::auto_ptr<SetupMatchState>        m_setupMatch;
    std::auto_ptr<EnterPlayerReadyState>  m_enterPlayersReady;
    std::auto_ptr<ExitPlayerReadyState>   m_exitPlayersReady;
    std::auto_ptr<MatchPlayingState>      m_matchPlaying;
    std::auto_ptr<MatchIsOverState>       m_matchIsOver;
    std::auto_ptr<DisplayHallOfFameState> m_hallOfFame;
    std::auto_ptr<LeaveGameState>         m_leaveGame;
};

#endif // _SOLOGAMESTARTER_H_
