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
    std::string _path;
    IosSurfaceRef _surfaceRef;
    IosSurface   *_surface;
    IosSurface   *_alphaSurface[10];
    StyroImage(StyrolyseClient *_this,
               const std::string &path, bool removePrefix=false);
};

class StoryWidget : public Widget, public IdleComponent {
public:
    StoryWidget(const std::string &screenName, Action *finishedAction = NULL, bool fxMode = false);
    ~StoryWidget();
    void reset();
    void idle(double currentTime);
    void draw(DrawTarget *dt);
    IdleComponent *getIdleComponent() { return this; }
    void setIntegerValue(const std::string & varName, int value);
    int getIntegerValue(const std::string & varName) const;
    void setFloatValue(const std::string & varName, float value);
    float getFloatValue(const std::string & varName) const;
    void setStringValue(const std::string & varName, const char * value);
    std::string getStringValue(const std::string & varName) const;
    const std::string &getText(const std::string &text) const;
    void freeMemory();

    void render(DrawTarget *dt);
    void setRenderEnabled(bool enabled=true) { _renderEnabled = enabled; }

    struct StoryStyrolyseClient {
        StyrolyseClient styroClient;
        StoryWidget *widget;
        const FloboSetTheme *attachedTheme;
    };
protected:

    /* dictionnary for locale translations */
    LocalizedDictionary *_localeDictionary;
    Styrolyse *_currentStory;
    Action *_finishedAction;
    bool _once;
    static bool _classInitialized;
    StoryStyrolyseClient _client;
    double _last_time;
    bool _fxMode;
    std::string _fullPath;
    bool _renderEnabled;
};

class VisualFX : public StoryWidget {
public:
    VisualFX(const std::string & fxName, const FloboSetTheme &floboSetTheme);
    void postEvent(const char *name, float x, float y, int player);
    bool busy() const;
    bool supportFX(const char *str) const;
    VisualFX *clone() const;

    void setGameScreen(GameScreen *screen) { this->_screen = screen; }
    GameScreen *getGameScreen() const { return _screen; }
private:
    std::string _fxName;
    GameScreen *_screen;
};

class StoryScreen : public Screen {
public:
    StoryScreen(const std::string & screenName, Action *finishedAction = NULL, bool shouldAddTransition = true);
    virtual ~StoryScreen();
    // Screen implementation
    virtual void onEvent(event_manager::GameControlEvent *cevent);
    virtual void onTransitionFromScreen(Screen &fromScreen);
    /* Notification on screen visibility change
     * @param visible  true if the scren is visible, otherwise false
     */
    virtual void onScreenVisibleChanged(bool visible);
    const StoryWidget * getStoryWidget() const { return &_storyWidget; }
    StoryWidget * getStoryWidget() { return &_storyWidget; }
protected:
    StoryWidget _storyWidget;
    ScreenTransitionWidget *_transitionWidget;
private:
    Action *_finishedAction;
};

#endif
