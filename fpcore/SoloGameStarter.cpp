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
    : m_position(position), m_front(front), m_back(back),
      m_targetValue(0.), m_value(0.)
{
}

void VuMeter::draw(DrawTarget *dt)
{
    IosRect meterRect, drect;
    meterRect.x = 0;
    meterRect.w = m_front->w;
    meterRect.h = m_front->h * m_value;
    meterRect.y = m_front->h - meterRect.h;
    drect.x = m_position.x - meterRect.w / 2;
    drect.y = m_position.y - meterRect.h;
    drect.w = meterRect.w;
    drect.h = meterRect.h;
    IosRect meterBlackRect = meterRect;
    IosRect drectBlack     = drect;
    meterBlackRect.h = m_front->h - meterRect.h;
    meterBlackRect.y = 0;
    drectBlack.y = m_position.y - m_front->h;
    drectBlack.h = meterBlackRect.h;
    dt->draw(m_back,&meterBlackRect,&drectBlack);
    dt->draw(m_front,&meterRect, &drect);
}

void VuMeter::step()
{
    m_value += (m_targetValue - m_value)/10.;
}

#define TIME_BETWEEN_GAME_CYCLES 0.02

SoloGameWidget::SoloGameWidget(SoloGameSettings &gameSettings, FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, Action *gameOverAction)
    : CycledComponent(TIME_BETWEEN_GAME_CYCLES),
      m_cyclesDuration(gameSettings.cyclesDuration),
      m_levelIncrease(gameSettings.levelIncrease),
      m_handicapIncrease(gameSettings.handicapIncrease),
      m_handicapDecreaseOnPhase1(gameSettings.handicapDecreaseOnPhase1),
      m_handicapDecreaseAbovePhase1(gameSettings.handicapDecreaseAbovePhase1),
      attachedFloboThemeSet(floboSetTheme), attachedRandom(5),
      m_cyclesBeforeGameCycle(0), m_cyclesBeforeLevelRaise(1000.),
      m_comboHandicap(0.), m_comboHandicap75(false), m_comboHandicap85(false)
{
    m_gameFactory.reset(new LocalGameFactory(&attachedRandom));
    m_areaA.reset(new GameView(m_gameFactory.get(), 0, &floboSetTheme, &levelTheme));
    m_playerController.reset(new CombinedEventPlayer(*m_areaA));
    //initWithGUI(*m_areaA, NULL, *m_playerController, NULL, levelTheme, gameOverAction);
    //setLives(-1);
    setReceiveUpEvents(true);
    setFocusable(true);
    m_areaA->getAttachedGame()->addGameListener(this);
    setLevelTheme(&levelTheme);
    m_comboMeter.reset(new VuMeter(Vec3(levelTheme.getSpeedMeterX(),
                                        levelTheme.getSpeedMeterY()),
                                   levelTheme.getSpeedMeter(true),
                                   levelTheme.getSpeedMeter(false)));

    // TODO: move elsewhere
    // Load and preload a few FX for the game
    for (int i=0; i<3; ++i)
        m_visualFX.push_back(new VisualFX("fx/vanish.gsl", *(m_areaA->getFloboSetTheme())));
    for (int i=0; i<3; ++i)
        m_visualFX.push_back(new VisualFX("fx/combo.gsl", *(m_areaA->getFloboSetTheme())));
    for (int i=0; i<3; ++i)
        m_visualFX.push_back(new VisualFX("fx/starvedcombo.gsl", *(m_areaA->getFloboSetTheme())));
}

void SoloGameWidget::gameDidEndCycle()
{
    if (m_cyclesBeforeLevelRaise <= 0.) {
        m_areaA->getAttachedGame()->addNeutralLayer();
        m_cyclesBeforeLevelRaise = 1000.;
    }
}

void SoloGameWidget::floboWillVanish(AdvancedBuffer<Flobo *> &floboGroup, int groupNum, int phase)
{
    if (phase == 1)
        m_comboHandicap += m_handicapDecreaseOnPhase1.getValue();
    if (phase >= 2) {
        m_comboHandicap += m_handicapDecreaseAbovePhase1.getValue();
    }
    if (phase == 4)
        m_comboHandicap = 0;
    if (m_comboHandicap < 0.)
        m_comboHandicap = 0.;
}

void SoloGameWidget::cycle()
{
    if (!m_paused) {
        // Game parameters
        m_cyclesDuration.step();
        m_levelIncrease.step();
        m_handicapIncrease.step();
        m_handicapDecreaseOnPhase1.step();
        m_handicapDecreaseAbovePhase1.step();
        // Controls
        m_playerController->cycle();
        // Cycling through the foreground animation
        if (m_styroPainter.get() != NULL)
            m_styroPainter->update();
        // Animations
        m_comboMeter->step();
        m_areaA->cycleAnimation();
        if (m_cyclesBeforeGameCycle == 0) {
            if (! m_areaA->isNewMetaCycleStart())
                m_areaA->cycleGame();
            m_areaA->clearMetaCycleStart();
            m_cyclesBeforeGameCycle = m_cyclesDuration.getValue();
        }
        if (m_cyclesBeforeLevelRaise <= 0.) {
        }
        else {
            m_cyclesBeforeLevelRaise -= m_levelIncrease.getValue();
        }
        m_cyclesBeforeGameCycle--;
        m_comboHandicap += m_handicapIncrease.getValue();
        // Warning events on handicap
        if ((m_comboHandicap > 75.) && (!m_comboHandicap75)) {
            EventFX("starvedcombo", 20, 20, 1);
            m_comboHandicap75 = true;
        }
        if ((m_comboHandicap > 85.) && (!m_comboHandicap85)) {
            EventFX("starvedcombo", 20, 20, 1);
            m_comboHandicap85 = true;
        }
        if (m_comboHandicap >= 100.) {
            m_areaA->getAttachedGame()->increaseNeutralFlobos(6);
            m_comboHandicap = 0.;
            EventFX("starvedcombo", 20, 20, 1);
            m_comboHandicap75 = false;
            m_comboHandicap85 = false;
        }
        requestDraw();
    }
    if (m_areaA->isGameOver() || getAborted()) {
        if (gameOverAction)
            gameOverAction->action(this, GAME_IS_OVER, NULL);
    }
}

void SoloGameWidget::draw(DrawTarget *dt)
{
    if ((m_paused) && (m_obscureScreenOnPause)) {
        dt->draw(m_painterGameScreen, NULL, NULL);
        return;
    }
    IosRect dtRect = { 0, 0, dt->w, dt->h };
    dt->draw(getLevelTheme()->getBackground(), &dtRect, &dtRect);
    m_areaA->render(dt);
    // Rendering the combo meter
    m_comboMeter->setValue(m_comboHandicap / 100.);
    m_comboMeter->draw(dt);
    // Rendering the scores
    m_areaA->renderScore(dt);
    // Rendering the player names
    IosFont *font = getLevelTheme()->getPlayerNameFont();
    dt->putStringCenteredXY(font,
                            getLevelTheme()->getNameDisplayX(0),
                            getLevelTheme()->getNameDisplayY(0),
                            m_playerName.c_str());
    // Rendering the foreground animation
    if (m_styroPainter.get() != NULL)
        m_styroPainter->draw(dt);
    // Rendering the neutral puyos
    m_areaA->renderNeutral(dt);
}

void SoloGameWidget::eventOccured(GameControlEvent *event)
{
    if (m_paused)
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
bool SoloGameWidget::backPressed()
{
    return false;
}
bool SoloGameWidget::startPressed()
{
    return false;
}
StoryWidget *SoloGameWidget::getOpponent()
{
    return NULL;
}
void SoloGameWidget::setPlayerOneName(String newName)
{
    m_playerName = newName;
}
void SoloGameWidget::setPlayerTwoName(String newName)
{}
PlayerGameStat &SoloGameWidget::getStatPlayerOne()
{
    return m_areaA->getAttachedGame()->getGameStat();
}
PlayerGameStat &SoloGameWidget::getStatPlayerTwo()
{
    return m_areaA->getAttachedGame()->getGameStat();
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
    // Set the game parameters depending on the game difficulty
    SoloGameSettings gameSettings;
    switch (difficulty) {
    case EASY:
        gameSettings = {
            { 20.,   -0.0002,  10. }, // cyclesDuration
            {  1.,    0.,      0. }, // levelIncrease
            {  0.03,  0.,      0. }, // handicapIncrease;
            { -7.,    0.,      0. }, // handicapDecreaseOnPhase1;
            {-40.,    0.,      0. }  // handicapDecreaseAbovePhase1;
        };
        break;
    case MEDIUM:
        gameSettings = {
            { 15.,   -0.0002, 5. }, // cyclesDuration
            {  1.,    0.,     0. }, // levelIncrease
            {  0.05,  0.,     0. }, // handicapIncrease;
            { -7.,    0.,     0. }, // handicapDecreaseOnPhase1;
            {-40.,    0.,     0. }  // handicapDecreaseAbovePhase1;
        };
        break;
    case HARD:
    default:
        gameSettings = {
            {  10.,  -0.0002,  0.   }, // cyclesDuration
            {  1.,    0.0001,  2.   }, // levelIncrease
            {  0.05,  0.00001, 0.15 }, // handicapIncrease;
            { -7.,    0.,      0.   }, // handicapDecreaseOnPhase1;
            {-40.,    0.,      0.   }  // handicapDecreaseAbovePhase1;
        };
        break;
    };
    // Creating the game widget factory
    m_gameWidgetFactory.reset(new SoloGameWidgetFactory(gameSettings));
    // Creating the different game states
    m_pushGameScreen.reset(new PushScreenState());
    m_setupMatch.reset(new SetupMatchState(m_gameWidgetFactory.get(), GameOptions::fromDifficulty(difficulty), nameProvider, &m_sharedAssets));
    m_enterPlayersReady.reset(new EnterPlayerReadyState(m_sharedAssets, m_sharedGetReadyAssets));
    m_exitPlayersReady.reset(new ExitPlayerReadyState(m_sharedAssets, m_sharedGetReadyAssets));
    m_matchPlaying.reset(new MatchPlayingState(m_sharedAssets));
    m_matchIsOver.reset(new MatchIsOverState(m_sharedAssets));
    m_hallOfFame.reset(new DisplayHallOfFameState(&m_sharedAssets, nameProvider,
                                                  SOLO_SCOREBOARD_ID,
                                                  "gamewon_highscores_1p.gsl"));
    m_leaveGame.reset(new LeaveGameState(m_sharedAssets));
    // Linking the states together
    m_pushGameScreen->setNextState(m_setupMatch.get());
    m_setupMatch->setNextState(m_enterPlayersReady.get());
    m_enterPlayersReady->setNextState(m_exitPlayersReady.get());
    m_exitPlayersReady->setNextState(m_matchPlaying.get());
    m_matchPlaying->setNextState(m_matchIsOver.get());
    m_matchPlaying->setAbortedState(m_leaveGame.get());
    m_matchIsOver->setNextState(m_hallOfFame.get());
    m_hallOfFame->setNextState(m_leaveGame.get());
    // Initializing the state machine
    m_stateMachine.setInitialState(m_pushGameScreen.get());
}

void SoloModeStarterAction::action(Widget *sender, int actionType,
                                   event_manager::GameControlEvent *event)
{
    m_stateMachine.reset();
    m_stateMachine.evaluate();
}

