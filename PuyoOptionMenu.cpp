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
    theCommander->setSoundFx(! theCommander->getSoundFx());
    if (toggleButton) toggleButton->setToggle(theCommander->getSoundFx());
}

void ToggleMusicAction::action()
{
    theCommander->setMusic(! theCommander->getMusic());
    if (toggleButton) toggleButton->setToggle(theCommander->getMusic());
}

void ToggleFullScreenAction::action()
{
    theCommander->setFullScreen(! theCommander->getFullScreen());
    if (toggleButton) toggleButton->setToggle(theCommander->getFullScreen());
}

OptionMenu::OptionMenu(PuyoMainScreen *mainScreen) : PuyoMainScreenMenu(mainScreen),
    optionTitle(theCommander->getLocalizedString("Options")),
    audioButton(theCommander->getLocalizedString(kAudioFX), theCommander->getLocalizedString("OFF"), theCommander->getLocalizedString("ON "), theCommander->getSoundFx(), &toggleSoundFxAction),
    musicButton(theCommander->getLocalizedString(kMusic), theCommander->getLocalizedString("OFF"), theCommander->getLocalizedString("ON "), theCommander->getMusic(), &toggleMusicAction),
    fullScreenButton(theCommander->getLocalizedString(kFullScreen), theCommander->getLocalizedString("OFF"), theCommander->getLocalizedString("ON "), theCommander->getFullScreen(), &toggleFullScreenAction),
    controlMenu(mainScreen), themeMenu(mainScreen), levelMenu(mainScreen),
    changeThemeAction(&themeMenu, mainScreen), changeLevelAction(&levelMenu, mainScreen), changeControlsAction(&controlMenu, mainScreen, true),
    changePuyoThemeButton(theCommander->getLocalizedString(kChangePuyoTheme), &changeThemeAction),
    changeLevelThemeButton(theCommander->getLocalizedString(kChangePuyoLevel), &changeLevelAction),
    changeControlsButton(theCommander->getLocalizedString(kControls), &changeControlsAction), backAction(mainScreen), backButton(theCommander->getLocalizedString("Back"), &backAction)
{
    toggleSoundFxAction.setButton(&audioButton);
    toggleMusicAction.setButton(&musicButton);
    toggleFullScreenAction.setButton(&fullScreenButton);
}

void OptionMenu::build() {
    controlMenu.build();
    themeMenu.build();
    levelMenu.build();
    add(&optionTitle);
    add(&audioButton);
    add(&musicButton);
    add(&fullScreenButton);
    if (getPuyoThemeManger()->getAnimatedPuyoSetThemeList()->size() > 1)
        add(&changePuyoThemeButton);
    if (getPuyoThemeManger()->getPuyoLevelThemeList()->size() > 1)
        add(&changeLevelThemeButton);
    add(&changeControlsButton);
    add(&backButton);
}

