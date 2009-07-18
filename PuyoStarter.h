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

#ifndef _PUYOSTARTER
#define _PUYOSTARTER

#include <vector>

#include "PuyoGame.h"
#include "PuyoEventPlayer.h"
#include "PuyoIA.h"
#include "PuyoCheatCodeManager.h"
#include "PuyoPauseMenu.h"
#include "GameWidget.h"
#include "PuyoCommander.h"
#include "ios_messagebox.h"
#include "AnimatedPuyoTheme.h"
#include "PuyoScreenTransition.h"
#include "PuyoStrings.h"

class PuyoLocalGameFactory : public PuyoGameFactory {
public:
    PuyoLocalGameFactory(PuyoRandomSystem *attachedRandom): attachedRandom(attachedRandom) {}
    PuyoGame *createPuyoGame(PuyoFactory *attachedPuyoFactory) {
        return new PuyoLocalGame(attachedRandom, attachedPuyoFactory);
    }
private:
    PuyoRandomSystem *attachedRandom;
};

class PuyoGameScreen;
struct GameOptions;

class PuyoGameScreen : public Screen, public Action {
public:
    PuyoGameScreen(GameWidget &gameWidget, Screen &previousScreen);
    ~PuyoGameScreen();
    void onEvent(GameControlEvent *cevent);
    virtual bool backPressed();
    virtual bool startPressed();
    virtual void abort();
    void setOverlayStory(StoryWidget *story);
    /**
     * Sets the game to paused
     * @param fromControls true if the action leading to the pause comes
     *                     in response to a local input control. In this case,
     *                     the action will get forwarded to the opponent in
     *                     a network game. Otherwise it won't.
     */
    virtual void setPaused(bool fromControls);
    /**
     * Sets the game to resumed
     * @param fromControls true if the action leading to the pause comes
     *                     in response to a local input control. In this case,
     *                     the action will get forwarded to the opponent in
     *                     a network game. Otherwise it won't.
     */
    virtual void setResumed(bool fromControls);
    /** Returns the ingame pause menu widget
     */
    PuyoPauseMenu & getPauseMenu() { return pauseMenu; }
    /** Notification on screen visibility change
     * @param visible  true if the scren is visible, otherwise false
     */
    virtual void onScreenVisibleChanged(bool visible);
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType, GameControlEvent *event);
private:
    bool paused;
    PuyoPauseMenu pauseMenu;
    GameWidget &gameWidget;
    PuyoScreenTransitionWidget transitionWidget;
    StoryWidget *overlayStory;
};

class PuyoTwoPlayerGameWidget : public GameWidget {
public:
    PuyoTwoPlayerGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, Action *gameOverAction = NULL);
private:
    AnimatedPuyoSetTheme &attachedPuyoThemeSet;
    PuyoRandomSystem attachedRandom;
    PuyoLocalGameFactory attachedGameFactory;
    PuyoView areaA, areaB;
    PuyoEventPlayer controllerA, controllerB;
};

#endif // _PUYOSTARTER

