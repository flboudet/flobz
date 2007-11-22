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

#ifndef PUYO_STORY_H
#define PUYO_STORY_H

class PuyoGameScreen;

#include "SDL.h"
#include "SDL_image.h"
#include "styrolyse.h"

#include "gameui.h"
#include "PuyoScreenTransition.h"
#include "PuyoLocalizedDictionary.h"

extern int NB_STORIES;

using namespace gameui;

class PuyoCommander;

class PuyoStoryWidget : public Widget, public /*Cycled*/IdleComponent {
public:
    PuyoStoryWidget(String screenName, Action *finishedAction = NULL, bool fxMode = false);
    ~PuyoStoryWidget();
    // void cycle();
    void idle(double currentTime);
    void draw(SDL_Surface *screen);
    IdleComponent *getIdleComponent() { return this; }
    void setIntegerValue(String varName, int value);
    int getIntegerValue(String varName) const;
    const char *getText(const char *text) const;

    struct PuyoStoryStyrolyseClient {
        StyrolyseClient styroClient;
        PuyoStoryWidget *widget;
    };
protected:

    /* dictionnary for locale translations */
    PuyoLocalizedDictionary *localeDictionary;
    Styrolyse *currentStory;
    Action *finishedAction;
    bool once;
    static bool classInitialized;
    PuyoStoryStyrolyseClient client;
    double last_time;
};

class PuyoFX : public PuyoStoryWidget {
public:
    PuyoFX(String fxName);
    void postEvent(const char *name, float x, float y, int player);
    bool busy() const;
    bool supportFX(const char *str) const;
    PuyoFX *clone() const;

    void setGameScreen(PuyoGameScreen *screen) { this->screen = screen; }
    PuyoGameScreen *getGameScreen() const { return screen; }
private:
    String fxName;
    PuyoGameScreen *screen;
};

class PuyoStoryScreen : public Screen {
public:
    PuyoStoryScreen(String screenName, Screen &previousScreen, Action *finishedAction = NULL, bool shouldAddTransition = true);
    PuyoStoryScreen(String screenName);
    virtual ~PuyoStoryScreen();
    void onEvent(GameControlEvent *cevent);
    void transitionFromScreen(Screen &fromScreen);
    /* Notification on screen visibility change
     * @param visible  true if the scren is visible, otherwise false
     */
    virtual void onScreenVisibleChanged(bool visible);
protected:
    PuyoStoryWidget storyWidget;
    PuyoScreenTransitionWidget *transitionWidget;
private:
    Action *finishedAction;
};

#endif
