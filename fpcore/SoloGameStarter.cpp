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

#include "SoloGameStarter.h"
#include "GTLog.h"

using namespace event_manager;

#define TIME_BETWEEN_GAME_CYCLES 0.02

SoloGameWidget::SoloGameWidget(FloboSetTheme &puyoThemeSet, LevelTheme &levelTheme, Action *gameOverAction)
    : CycledComponent(TIME_BETWEEN_GAME_CYCLES),
      attachedFloboThemeSet(puyoThemeSet), attachedLevelTheme(levelTheme), attachedRandom(5), m_cyclesBeforeGameCycle(0), m_cyclesBeforeLevelRaise(0)
{
    m_gameFactory.reset(new LocalGameFactory(&attachedRandom));
    m_areaA.reset(new GameView(m_gameFactory.get(), 0, &puyoThemeSet, &levelTheme));
    m_playerController.reset(new CombinedEventPlayer(*m_areaA));
    //initWithGUI(*m_areaA, NULL, *m_playerController, NULL, levelTheme, gameOverAction);
    //setLives(-1);
    setReceiveUpEvents(true);
    setFocusable(true);
    m_areaA->getAttachedGame()->addGameListener(this);
}

void SoloGameWidget::gameDidEndCycle()
{
    if (m_cyclesBeforeLevelRaise == 0) {
        m_areaA->getAttachedGame()->addNeutralLayer();
        m_cyclesBeforeLevelRaise = 800;
    }
    //m_areaA->getAttachedGame()->increaseNeutralFlobos(6);
}

void SoloGameWidget::floboWillVanish(AdvancedBuffer<Flobo *> &floboGroup, int groupNum, int phase)
{
    GTLogTrace("Combo with phase=%d", phase);
}

void SoloGameWidget::cycle()
{
    // Controls
    m_playerController->cycle();
    // Animations
    m_areaA->cycleAnimation();
    if (m_cyclesBeforeGameCycle == 0) {
        if (! m_areaA->isNewMetaCycleStart())
            m_areaA->cycleGame();
        m_areaA->clearMetaCycleStart();
        m_cyclesBeforeGameCycle = 10;
    }
    if (m_cyclesBeforeLevelRaise == 0) {
    }
    else {
        m_cyclesBeforeLevelRaise--;
    }
    m_cyclesBeforeGameCycle--;
    requestDraw();
}

void SoloGameWidget::draw(DrawTarget *dt)
{
    IosRect dtRect = { 0, 0, dt->w, dt->h };
    dt->draw(attachedLevelTheme.getBackground(), &dtRect, &dtRect);
    m_areaA->render(dt);
}

void SoloGameWidget::eventOccured(GameControlEvent *event)
{
    if (paused)
        lostFocus();
    else {
        m_playerController->eventOccured(event);
        //for (std::vector<gameui::Widget *>::iterator iter = m_subwidgets.begin() ;
        //     iter != m_subwidgets.end() ; iter++) {
        //    (*iter)->eventOccured(event);
        //}
    }
}

void SoloGameWidget::setGameOptions(GameOptions options)
{
    m_options = options;
}
void SoloGameWidget::pause(bool obscureScreen)
{
    paused = true;
}
void SoloGameWidget::resume()
{
    paused = false;
    setFocusable(true);
}
bool SoloGameWidget::backPressed()
{
    return false;
}
bool SoloGameWidget::startPressed()
{
    return false;
}
void SoloGameWidget::abort()
{}
bool SoloGameWidget::getAborted() const
{
    return false;
}
StoryWidget *SoloGameWidget::getOpponent()
{
    return NULL;
}
void SoloGameWidget::setPlayerOneName(String newName)
{}
void SoloGameWidget::setPlayerTwoName(String newName)
{}
PlayerGameStat &SoloGameWidget::getStatPlayerOne()
{
    return m_gameStat;
}
PlayerGameStat &SoloGameWidget::getStatPlayerTwo()
{
    return m_gameStat;
}
void SoloGameWidget::addGameAHandicap(int handicap)
{}
void SoloGameWidget::addGameBHandicap(int handicap)
{}
bool SoloGameWidget::isGameARunning() const
{
    return true;
}


//---------------------------------
// Solo mode game state machine
//---------------------------------
SoloModeStarterAction::SoloModeStarterAction(GameDifficulty difficulty, PlayerNameProvider *nameProvider)
{
    // Creating the different game states
    m_pushGameScreen.reset(new PushScreenState());
    m_setupMatch.reset(new SetupMatchState(&m_gameWidgetFactory, GameOptions::fromDifficulty(difficulty), nameProvider, &m_sharedAssets));
    m_enterPlayersReady.reset(new EnterPlayerReadyState(m_sharedAssets, m_sharedGetReadyAssets));
    m_exitPlayersReady.reset(new ExitPlayerReadyState(m_sharedAssets, m_sharedGetReadyAssets));
    m_matchPlaying.reset(new MatchPlayingState(m_sharedAssets));
    m_matchIsOver.reset(new MatchIsOverState(m_sharedAssets));
    m_leaveGame.reset(new LeaveGameState(m_sharedAssets));
    // Linking the states together
    m_pushGameScreen->setNextState(m_setupMatch.get());
    m_setupMatch->setNextState(m_enterPlayersReady.get());
    m_enterPlayersReady->setNextState(m_exitPlayersReady.get());
    m_exitPlayersReady->setNextState(m_matchPlaying.get());
    m_matchPlaying->setNextState(m_matchIsOver.get());
    m_matchPlaying->setAbortedState(m_leaveGame.get());
    m_matchIsOver->setNextState(m_leaveGame.get());
    // Initializing the state machine
    m_stateMachine.setInitialState(m_pushGameScreen.get());
}

void SoloModeStarterAction::action(Widget *sender, int actionType,
                                   event_manager::GameControlEvent *event)
{
    m_stateMachine.reset();
    m_stateMachine.evaluate();
}

