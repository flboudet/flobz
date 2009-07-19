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

#include "PuyoTwoPlayerStarter.h"

TwoPlayersGameWidget::TwoPlayersGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, String aiFace, Action *gameOverAction) : attachedPuyoThemeSet(puyoThemeSet),
                                                     attachedRandom(5), attachedGameFactory(&attachedRandom),
                                                     areaA(&attachedGameFactory, &attachedPuyoThemeSet, &levelTheme,
                                                     1 + CSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + FSIZE, BSIZE+ESIZE),
                                                     areaB(&attachedGameFactory, &attachedPuyoThemeSet, &levelTheme,
                                                     1 + CSIZE + PUYODIMX*TSIZE + DSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + DSIZE - FSIZE - TSIZE, BSIZE+ESIZE),
                                                     playercontrollerA(areaA, GameControlEvent::kPlayer1Down, GameControlEvent::kPlayer1Left, GameControlEvent::kPlayer1Right,
                                                     GameControlEvent::kPlayer1TurnLeft, GameControlEvent::kPlayer1TurnRight),
                                                     playercontrollerB(areaB, GameControlEvent::kPlayer2Down, GameControlEvent::kPlayer2Left, GameControlEvent::kPlayer2Right,
                                                     GameControlEvent::kPlayer2TurnLeft, GameControlEvent::kPlayer2TurnRight),
                                                     opponentFace(aiFace)
{
    initWithGUI(areaA, areaB, playercontrollerA, playercontrollerB, levelTheme, gameOverAction);
    setLives(-1);
}

StoryWidget *TwoPlayersGameWidget::getOpponent()
{
    return &opponentFace;
}

void TwoPlayersGameWidget::cycle()
{
    opponentFace.setIntegerValue("@maxHeightLeft", attachedGameA->getColumnHeigth(2));
    opponentFace.setIntegerValue("@maxHeightRight", attachedGameB->getColumnHeigth(2));
    opponentFace.setIntegerValue("@neutralsForLeft", attachedGameA->getNeutralPuyos());
    opponentFace.setIntegerValue("@neutralsForRight", attachedGameB->getNeutralPuyos());
    opponentFace.setIntegerValue("@comboPhaseLeft", attachedGameA->getComboPhase());
    opponentFace.setIntegerValue("@comboPhaseRight", attachedGameB->getComboPhase());
    GameWidget::cycle();
}

TwoPlayersStarterAction::TwoPlayersStarterAction(int difficulty, GameWidgetFactory &gameWidgetFactory, PuyoTwoNameProvider *nameProvider)
  : m_state(kNotRunning), difficulty(difficulty), gameWidgetFactory(gameWidgetFactory), gameScreen(NULL),
    nameProvider(nameProvider), gameLostWidget(NULL), currentLevelTheme(NULL), leftVictories(0), rightVictories(0),
    m_statsWidget(NULL), totalPointsPlayerOne_(0), totalPointsPlayerTwo_(0) {}

void TwoPlayersStarterAction::action(Widget *sender, int actionType,
                                     GameControlEvent *event)
{
  if (m_state == kNotRunning)
    stateMachine();
  else if (sender == gameWidget) {
    if (gameWidget->getAborted())
      endGameSession();
    else
      stateMachine();
  }
  else if (sender == gameLostWidget)
    stateMachine();
}

void TwoPlayersStarterAction::stateMachine()
{
  switch (m_state) {
  case kNotRunning:
    startGame();
    break;
  case kMatchPlaying:
    gameOver();
    break;
  case kMatchWonP1Animation:
    gameScores();
    break;
  case kMatchWonP2Animation:
    gameScores();
    break;
  case kMatchScores:
    restartGame();
    break;
  default:
    break;
  }
}

void TwoPlayersStarterAction::startGame()
{
    AnimatedPuyoThemeManager * themeManager = getPuyoThemeManger();
    currentLevelTheme = themeManager->getPuyoLevelTheme();

    gameWidget = gameWidgetFactory.createGameWidget(*(themeManager->getAnimatedPuyoSetTheme()), *currentLevelTheme, currentLevelTheme->getCentralAnimation2P(), this);
    gameWidget->setGameOptions(GameOptions::FromLevel(difficulty));
    gameScreen = new GameScreen(*gameWidget, *(GameUIDefaults::SCREEN_STACK->top()));
    if (nameProvider != NULL) {
        gameWidget->setPlayerOneName(nameProvider->getPlayer1Name());
        gameWidget->setPlayerTwoName(nameProvider->getPlayer2Name());
    }
    int victoriesDelta = leftVictories - rightVictories;
    if (victoriesDelta > 0) {
        gameWidget->addGameAHandicap(victoriesDelta);
    }
    else if (victoriesDelta != 0) {
        gameWidget->addGameBHandicap(-victoriesDelta);
    }
    GameUIDefaults::SCREEN_STACK->push(gameScreen);
    m_state = kMatchPlaying;
}

void TwoPlayersStarterAction::gameOver()
{
    if (gameWidget->isGameARunning()) {
        gameLostWidget = new StoryWidget(currentLevelTheme->getGameLostRightAnimation2P(), this);
        leftVictories++;
	m_state = kMatchWonP1Animation;
    }
    else {
        gameLostWidget = new StoryWidget(currentLevelTheme->getGameLostLeftAnimation2P(), this);
        rightVictories++;
	m_state = kMatchWonP2Animation;
    }
    gameScreen->setOverlayStory(gameLostWidget);
}

void TwoPlayersStarterAction::gameScores()
{
  m_state = kMatchScores;
  m_statsWidget = new TwoPlayersStatsWidget(this->gameWidget->getStatPlayerOne(), this->gameWidget->getStatPlayerTwo(), true, true, theCommander->getWindowFramePicture());
  gameScreen->add(m_statsWidget);
}

void TwoPlayersStarterAction::restartGame()
{
    AnimatedPuyoThemeManager * themeManager = getPuyoThemeManger();
    currentLevelTheme = themeManager->getPuyoLevelTheme();

    GameWidget *newGameWidget = gameWidgetFactory.createGameWidget(*(themeManager->getAnimatedPuyoSetTheme()), *currentLevelTheme, currentLevelTheme->getCentralAnimation2P(), this);
    GameScreen *newGameScreen = new GameScreen(*newGameWidget, *(GameUIDefaults::SCREEN_STACK->top()));
    if (nameProvider != NULL) {
        newGameWidget->setPlayerOneName(nameProvider->getPlayer1Name());
        newGameWidget->setPlayerTwoName(nameProvider->getPlayer2Name());
    }

    GameUIDefaults::SCREEN_STACK->pop();
    if (gameWidget != NULL) {
        totalPointsPlayerOne_ += gameWidget->getStatPlayerOne().points;
        totalPointsPlayerTwo_ += gameWidget->getStatPlayerTwo().points;
        GameUIDefaults::GAME_LOOP->garbageCollect(gameWidget);
        GameUIDefaults::GAME_LOOP->garbageCollect(gameScreen);
    }
    if (gameLostWidget != NULL)
        delete gameLostWidget;
    gameLostWidget = NULL;
    gameScreen = newGameScreen;
    gameWidget = newGameWidget;
    gameWidget->getStatPlayerOne().total_points = totalPointsPlayerOne_;
    gameWidget->getStatPlayerTwo().total_points = totalPointsPlayerTwo_;
    GameUIDefaults::SCREEN_STACK->push(gameScreen);
    int victoriesDelta = leftVictories - rightVictories;
    if (victoriesDelta > 0) {
        gameWidget->addGameAHandicap(victoriesDelta);
    }
    else if (victoriesDelta != 0) {
        gameWidget->addGameBHandicap(-victoriesDelta);
    }
    if (m_statsWidget != NULL)
      delete m_statsWidget;
    m_statsWidget = NULL;

    m_state = kMatchPlaying;
}

void TwoPlayersStarterAction::endGameSession()
{
    GameUIDefaults::SCREEN_STACK->pop();
    MainScreen *menuScreen = dynamic_cast<MainScreen *>(GameUIDefaults::SCREEN_STACK->top());
    if (menuScreen != NULL)
        menuScreen->transitionFromScreen(*gameScreen);

    delete gameWidget;
    delete gameScreen;

    gameScreen = NULL;
    gameWidget = NULL;

    if (m_statsWidget != NULL)
      delete m_statsWidget;
    m_statsWidget = NULL;

    m_state = kNotRunning;
}

