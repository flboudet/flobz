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

#include "OptionMenu.h"
#include "FPStrings.h"

MusicPrefSwitch::MusicPrefSwitch()
: gameui::SwitchedButton(theCommander->getLocalizedString(kMusic), true,
                         theCommander->getSwitchOnPicture(), theCommander->getSwitchOffPicture(),
                         AudioHelper::musicOnOffKey(), theCommander->getPreferencesManager())
{
}

AudioPrefSwitch::AudioPrefSwitch()
: gameui::SwitchedButton(theCommander->getLocalizedString(kAudioFX), true,
                         theCommander->getSwitchOnPicture(), theCommander->getSwitchOffPicture(),
                         AudioHelper::soundOnOffKey(), theCommander->getPreferencesManager())
{
}

FSPrefSwitch::FSPrefSwitch()
: gameui::SwitchedButton(theCommander->getLocalizedString(kFullScreen), false,
                         theCommander->getSwitchOnPicture(), theCommander->getSwitchOffPicture(),
                         theCommander->getFullScreenKey(), theCommander->getPreferencesManager())
{
}

OptionMenu::OptionMenu(MainScreen *mainScreen) : MainScreenMenu(mainScreen),
    _screenTitleFrame(theCommander->getSeparatorFramePicture()),
    _optionTitle(theCommander->getLocalizedString("Options")),
    _audioButton(),
    _musicButton(),
    _fullScreenButton(),
    _controlMenu(mainScreen), _themeMenu(mainScreen),
    _soloLevelMenu(mainScreen, 1), _duoLevelMenu(mainScreen, 2),
    _changeThemeAction(&_themeMenu, mainScreen),
    _changeSoloLevelAction(&_soloLevelMenu, mainScreen),
    _changeDuoLevelAction(&_duoLevelMenu, mainScreen),
    _changeControlsAction(&_controlMenu, mainScreen, true),
    _changeFloboThemeButton(theCommander->getLocalizedString(kChangeFloboTheme), &_changeThemeAction),
    _changeSoloLevelThemeButton(theCommander->getLocalizedString(kChangeLevelTheme), &_changeSoloLevelAction),
    _changeDuoLevelThemeButton(theCommander->getLocalizedString(kChangeLevelTheme), &_changeDuoLevelAction),
    _changeControlsButton(theCommander->getLocalizedString(kControls), &_changeControlsAction), _backAction(mainScreen), _backButton(theCommander->getLocalizedString("Back"), &_backAction)
{
}

void OptionMenu::build() {
    _controlMenu.build();
    _themeMenu.build();
    _soloLevelMenu.build();
    _duoLevelMenu.build();
    setPolicy(USE_MIN_SIZE);
    _screenTitleFrame.setPreferedSize(Vec3(0, 20));
    _screenTitleFrame.add(&_optionTitle);
    add(&_screenTitleFrame);
    _buttonsBox.add(&_audioButton);
    _buttonsBox.add(&_musicButton);
    _buttonsBox.add(&_fullScreenButton);
    if (theCommander->getFloboSetThemeList().size() > 1)
        _buttonsBox.add(&_changeFloboThemeButton);
    if (theCommander->getLevelThemeList(1).size() > 1)
        _buttonsBox.add(&_changeSoloLevelThemeButton);
    if (theCommander->getLevelThemeList(2).size() > 1)
        _buttonsBox.add(&_changeDuoLevelThemeButton);
    _buttonsBox.add(&_changeControlsButton);
    _buttonsBox.add(&_backButton);
    add(&_buttonsBox);
}
