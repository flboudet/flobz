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

#ifndef _PUYO_GAME_WIDGET_H
#define _PUYO_GAME_WIDGET_H

#include "gameui.h"
#include "PuyoGame.h"
#include "PuyoPlayer.h"
#include "PuyoCheatCodeManager.h"
#include "AnimatedPuyoTheme.h"
#include "PuyoStory.h"

/**
 * This object represents all the game options needed as a parameter
 * to the PuyoGameWidget
 */
struct GameOptions
{
    GameOptions() {
        MIN_SPEED = 2;
        MAX_SPEED = 20;
        CYCLES_BEFORE_SPEED_INCREASES = 240;
    }

    static GameOptions FromLevel(int level);

    int MIN_SPEED;
    int MAX_SPEED;
    int CYCLES_BEFORE_SPEED_INCREASES;
};


/**
 * Represents the full featured game widget, with the two players game views.
 * Handles user input.
 */
class PuyoGameWidget : public GarbageCollectableItem,
                       public gameui::Widget, CycledComponent {
public:
    PuyoGameWidget(GameOptions options = GameOptions());
    void setGameOptions(GameOptions options);
    virtual ~PuyoGameWidget();
    void initialize(PuyoView &areaA, PuyoView &areaB,
                    PuyoPlayer &controllerA, PuyoPlayer &controllerB,
                    PuyoLevelTheme &levelTheme,
                    gameui::Action *gameOverAction = NULL);
    void initialize();
    void cycle();
    void draw(SDL_Surface *screen);
    void pause();
    void resume();
    bool isFocusable() { return !paused; }
    void eventOccured(GameControlEvent *event);
    bool backPressed();
    bool startPressed();
    IdleComponent *getIdleComponent() { return this; }
    virtual void abort() { abortedFlag = true; }
    bool getAborted() const { return abortedFlag; }
    void setLives(int l) { lives = l; }
    bool isGameARunning() const { return attachedGameA->isGameRunning(); }
    bool isGameBRunning() const { return attachedGameB->isGameRunning(); }
    void setPlayerOneName(String newName) { playerOneName = newName; }
    void setPlayerTwoName(String newName) { playerTwoName = newName; }
    String getPlayerOneName() const { return playerOneName; }
    PlayerGameStat getStatPlayerOne() { return attachedGameA->getGameStat(); }
    virtual PuyoStoryWidget *getOpponent() { return NULL; }
    virtual std::vector<PuyoFX*> &getPuyoFX() { return puyoFX; }
    void addGameAHandicap(int handicap) {attachedGameA->increaseNeutralPuyos(handicap * PUYODIMX); attachedGameA->dropNeutrals();}
    void addGameBHandicap(int handicap) {attachedGameB->increaseNeutralPuyos(handicap * PUYODIMX); attachedGameB->dropNeutrals();}
    void addSubWidget(Widget *subWidget);
    // A deplacer
    void setAssociatedScreen(PuyoGameScreen *associatedScreen) { this->associatedScreen = associatedScreen; associatedScreenHasBeenSet(associatedScreen); };
    virtual void setScreenToPaused(bool fromControls);
    virtual void setScreenToResumed(bool fromControls);
    virtual void actionAfterGameOver(bool fromControls);
protected:
    virtual void associatedScreenHasBeenSet(PuyoGameScreen *associatedScreen) {}

    // Styrolyse methods
    static void *styro_loadImage(StyrolyseClient *_this, const char *path);
    static void styro_drawImage(StyrolyseClient *_this,
				void *image, int x, int y,
				int clipx, int clipy, int clipw, int cliph);
    static void styro_freeImage(StyrolyseClient *_this, void *image);

    PuyoGameScreen *associatedScreen;
    SDL_Painter painter;
    PuyoLevelTheme *attachedLevelTheme;
    PuyoView *areaA, *areaB;
    PuyoPlayer *controllerA, *controllerB;
    PuyoGame *attachedGameA, *attachedGameB;
    int cyclesBeforeGameCycle;
    int cyclesBeforeSpeedIncreases; // time between speed increases in units of 20ms
    unsigned int tickCounts;
    unsigned long long cycles;
    bool paused;
    bool displayLives;
    int lives;
    bool once;
    gameui::Action *gameOverAction;
    bool gameover;
    bool abortedFlag;
    int gameSpeed; // from 0 (MinSpeed) to 20 (MaxSpeed)
    int MinSpeed,MaxSpeed; // in units of 20ms
    int blinkingPointsA, blinkingPointsB, savePointsA, savePointsB;
    String playerOneName, playerTwoName;
    std::vector<PuyoFX*> puyoFX;
    std::vector<gameui::Widget *> m_subwidgets;
    bool skipGameCycleA, skipGameCycleB;
    double gameOverDate;
    // Foreground animation
    struct StyrolysePainterClient {
      StyrolyseClient m_styroClient;
      SDL_Painter *m_painter;
      PuyoLevelTheme *m_theme;
    };
    Styrolyse *m_foregroundAnimation;
    StyrolysePainterClient m_styroPainter;
};

// Should be moved elsewhere
extern std::vector<PuyoFX*> *activeFX;

#endif // _PUYO_GAME_WIDGET_H
