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

#include "PuyoOptionMenu.h"
#include "preferences.h"
#include "PuyoStrings.h"

/**
 * Toggle fullscreen
 */
void ToggleFullScreenAction::action()
{
    optMenu.toggleFullScreen();
}

void OptionMenu::toggleFullScreen()
{
    fullscreen = !fullscreen;
    SetBoolPreference(kFullScreen,fullscreen);
    fullScreenButton.setToggle(fullscreen);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    theCommander->initDisplay(fullscreen, useGL);
}

OptionMenu::OptionMenu(PuyoStoryWidget *story) : PuyoMainScreen(story),
    fullscreen(GetBoolPreference(kFullScreen, true)),
    useGL(GetBoolPreference(kOpenGL, false)),
    toggleFullScreenAction(*this),
    audioButton(kAudioFX, "OFF", "ON ", fullscreen, &toggleFullScreenAction),
    musicButton(kMusic, "OFF", "ON ", fullscreen, &toggleFullScreenAction),
    fullScreenButton(kFullScreen, "OFF", "ON ", fullscreen, &toggleFullScreenAction),
    changeControlsButton(kControls, (gameui::Action *)NULL)
{
}

void OptionMenu::build() {
    menu.add(new Text("Options"));
    menu.add(&audioButton);
    menu.add(&musicButton);
    menu.add(&fullScreenButton);
    menu.add(&changeControlsButton);
    menu.add(new Button("Back", new PopScreenAction()));
    PuyoMainScreen::build();
}

