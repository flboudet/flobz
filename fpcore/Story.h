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

#ifndef FLOBO_STORY_H
#define FLOBO_STORY_H

class GameScreen;

#include "styrolyse.h"
#include "FPCommander.h"
#include "gameui.h"
#include "ScreenTransition.h"
#include "LocalizedDictionary.h"

extern int NB_STORIES;

using namespace gameui;

class StyroImage
{
public:
    String path;
    IosSurfaceRef surfaceRef;
    IosSurface   *surface;
    IosSurface   *alphaSurface[10];
    StyroImage(StyrolyseClient *_this,
               const char *path, bool removePrefix=false);
};

class StoryWidget : public Widget, public IdleComponent {
public:
    StoryWidget(String screenName, Action *finishedAction = NULL, bool fxMode = false);
    ~StoryWidget();
    void reset();
    void idle(double currentTime);
    void draw(DrawTarget *dt);
    IdleComponent *getIdleComponent() { return this; }
    void setIntegerValue(String varName, int value);
    int getIntegerValue(String varName) const;
    void setFloatValue(String varName, float value);
    float getFloatValue(String varName) const;
    void setStringValue(String varName, const char * value);
    std::string getStringValue(String varName) const;
    const char *getText(const char *text) const;
    void freeMemory();

    void render(DrawTarget *dt);
    void setRenderEnabled(bool enabled=true) { m_renderEnabled = enabled; }

    struct StoryStyrolyseClient {
        StyrolyseClient styroClient;
        StoryWidget *widget;
        const FloboSetTheme *attachedTheme;
    };
protected:

    /* dictionnary for locale translations */
    LocalizedDictionary *localeDictionary;
    Styrolyse *currentStory;
    Action *finishedAction;
    bool once;
    static bool classInitialized;
    StoryStyrolyseClient client;
    double last_time;
    bool fxMode;
    String fullPath;
    bool m_renderEnabled;
};

class VisualFX : public StoryWidget {
public:
    VisualFX(String fxName, const FloboSetTheme &floboSetTheme);
    void postEvent(const char *name, float x, float y, int player);
    bool busy() const;
    bool supportFX(const char *str) const;
    VisualFX *clone() const;

    void setGameScreen(GameScreen *screen) { this->screen = screen; }
    GameScreen *getGameScreen() const { return screen; }
private:
    String fxName;
    GameScreen *screen;
};

class StoryScreen : public Screen {
public:
    StoryScreen(String screenName, Action *finishedAction = NULL, bool shouldAddTransition = true);
    virtual ~StoryScreen();
    // Screen implementation
    virtual void onEvent(event_manager::GameControlEvent *cevent);
    virtual void onTransitionFromScreen(Screen &fromScreen);
    /* Notification on screen visibility change
     * @param visible  true if the scren is visible, otherwise false
     */
    virtual void onScreenVisibleChanged(bool visible);
    const StoryWidget * getStoryWidget() const { return &storyWidget; }
    StoryWidget * getStoryWidget() { return &storyWidget; }
protected:
    StoryWidget storyWidget;
    ScreenTransitionWidget *transitionWidget;
private:
    Action *finishedAction;
};

#endif
