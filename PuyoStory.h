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

#include "SDL.h"
#include "SDL_image.h"
#include "styrolyse.h"

#include "gameui.h"
#include "PuyoScreenTransition.h"

extern int NB_STORIES;

using namespace gameui;

class PuyoCommander;

class PuyoStoryWidget : public Widget, public CycledComponent {
public:
    PuyoStoryWidget(int num, Action *finishedAction = NULL);
    ~PuyoStoryWidget();
    void cycle();
    void draw(SDL_Surface *screen);
    IdleComponent *getIdleComponent() { return this; }
private:
    int num;
    Styrolyse *currentStory;
    Action *finishedAction;
    bool once;
};

class PuyoStoryScreen : public Screen {
public:
    PuyoStoryScreen(int num, Screen &previousScreen, Action *finishedAction = NULL);
    virtual ~PuyoStoryScreen();
    void onEvent(GameControlEvent *cevent);
private:
    PuyoStoryWidget storyWidget;
    Action *finishedAction;
    PuyoScreenTransitionWidget transitionWidget;
};

class PuyoStory : public DrawableComponent {
  public:
    PuyoStory(PuyoCommander *com, int num);
    virtual ~PuyoStory();
    void loop();
    void draw();
  private:
    int num;
    Styrolyse *currentStory;
    PuyoCommander *commander;
};

#endif
