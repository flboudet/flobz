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

#include "OptionMenu.h"
#include "preferences.h"
#include "PuyoStrings.h"
#include "audio.h"

MusicPrefSwitch::MusicPrefSwitch()
: gameui::SwitchedButton(theCommander->getLocalizedString(kMusic), true,
                         theCommander->getSwitchOnPicture(), theCommander->getSwitchOffPicture(),
                         String(AudioManager::musicOnOffKey()))
{
}

AudioPrefSwitch::AudioPrefSwitch()
: gameui::SwitchedButton(theCommander->getLocalizedString(kAudioFX), true,
                         theCommander->getSwitchOnPicture(), theCommander->getSwitchOffPicture(),
                         String(AudioManager::soundOnOffKey()))
{
}

FSPrefSwitch::FSPrefSwitch()
: gameui::SwitchedButton(theCommander->getLocalizedString(kFullScreen), true,
                         theCommander->getSwitchOnPicture(), theCommander->getSwitchOffPicture(),
                         String(theCommander->getFullScreenKey()))
{
}

OptionMenu::OptionMenu(MainScreen *mainScreen) : MainScreenMenu(mainScreen),
    screenTitleFrame(theCommander->getSeparatorFramePicture()),
    optionTitle(theCommander->getLocalizedString("Options")),
    audioButton(),
    musicButton(),
    fullScreenButton(),
    controlMenu(mainScreen), themeMenu(mainScreen), levelMenu(mainScreen),
    changeThemeAction(&themeMenu, mainScreen), changeLevelAction(&levelMenu, mainScreen), changeControlsAction(&controlMenu, mainScreen, true),
    changePuyoThemeButton(theCommander->getLocalizedString(kChangePuyoTheme), &changeThemeAction),
    changeLevelThemeButton(theCommander->getLocalizedString(kChangePuyoLevel), &changeLevelAction),
    changeControlsButton(theCommander->getLocalizedString(kControls), &changeControlsAction), backAction(mainScreen), backButton(theCommander->getLocalizedString("Back"), &backAction)
{
}

void OptionMenu::build() {
    controlMenu.build();
    themeMenu.build();
    levelMenu.build();
    setPolicy(USE_MIN_SIZE);
    screenTitleFrame.setPreferedSize(Vec3(0, 20));
    screenTitleFrame.add(&optionTitle);
    add(&screenTitleFrame);
    buttonsBox.add(&audioButton);
    buttonsBox.add(&musicButton);
    buttonsBox.add(&fullScreenButton);
    if (getPuyoThemeManger()->getAnimatedPuyoSetThemeList()->size() > 1)
        buttonsBox.add(&changePuyoThemeButton);
    if (getPuyoThemeManger()->getPuyoLevelThemeList()->size() > 1)
        buttonsBox.add(&changeLevelThemeButton);
    buttonsBox.add(&changeControlsButton);
    buttonsBox.add(&backButton);
    add(&buttonsBox);
}

