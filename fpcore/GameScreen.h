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

#ifndef _GAMESCREEN_H_
#define _GAMESCREEN_H_

#include <memory>
#include "GameWidget.h"
#include "PauseMenu.h"

class GameScreen : public Screen, public Action {
public:
    GameScreen(GameWidget &gameWidget);
    ~GameScreen();
    // Screen implementation
    virtual void onEvent(event_manager::GameControlEvent *cevent);
    virtual void onTransitionFromScreen(Screen &fromScreen);
    // Own methods
    virtual bool backPressed();
    virtual bool startPressed();
    virtual void abort();
    void setOverlayStory(StoryWidget *story);
    /**
     * Suspends the game (no pause menu)
     */
    void setSuspended(bool suspended);
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
    PauseMenu & getPauseMenu() { return pauseMenu; }
    /** Notification on screen visibility change
     * @param visible  true if the scren is visible, otherwise false
     */
    virtual void onScreenVisibleChanged(bool visible);
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType, event_manager::GameControlEvent *event);
private:
    bool paused;
    PauseMenu pauseMenu;
    GameWidget &gameWidget;
    std::auto_ptr<ScreenTransitionWidget> transitionWidget;
    StoryWidget *overlayStory;
};

#endif // _GAMESCREEN_H_

