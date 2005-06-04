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
void ToggleSoundFxAction::action()
{
    optMenu.toggleSoundFx();
}

void ToggleMusicAction::action()
{
    optMenu.toggleMusic();
}

void ToggleFullScreenAction::action()
{
    optMenu.toggleFullScreen();
}

void OptionMenu::toggleSoundFx()
{
    theCommander->setSoundFx(! theCommander->getSoundFx());
    audioButton.setToggle(theCommander->getSoundFx());
}

void OptionMenu::toggleMusic()
{
    theCommander->setMusic(! theCommander->getMusic());
    musicButton.setToggle(theCommander->getMusic());
}

void OptionMenu::toggleFullScreen()
{
    theCommander->setFullScreen(! theCommander->getFullScreen());
    fullScreenButton.setToggle(theCommander->getFullScreen());
}

OptionMenu::OptionMenu(PuyoRealMainScreen *mainScreen) : PuyoMainScreenMenu(mainScreen),
    toggleSoundFxAction(*this), toggleMusicAction(*this), toggleFullScreenAction(*this),
    audioButton(kAudioFX, "OFF", "ON ", theCommander->getSoundFx(), &toggleSoundFxAction),
    musicButton(kMusic, "OFF", "ON ", theCommander->getMusic(), &toggleMusicAction),
    fullScreenButton(kFullScreen, "OFF", "ON ", theCommander->getFullScreen(), &toggleFullScreenAction),
    controlMenu(mainScreen), 
    changeControlsButton(kControls, new PuyoPushMenuAction(&controlMenu, mainScreen))
{
}

void OptionMenu::build() {
    controlMenu.build();
    add(new Text("Options"));
    add(&audioButton);
    add(&musicButton);
    add(&fullScreenButton);
    add(&changeControlsButton);
    add(new Button("Back", new PuyoPopMenuAction(mainScreen)));
}

