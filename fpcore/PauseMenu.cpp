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

#include "PauseMenu.h"
#include "FPCommander.h"

using namespace event_manager;
using namespace gameui;

PauseMenu::PauseMenu(Action *pauseAction)
    : _topSeparator(0, 10), _pauseVBox(theCommander->getWindowFramePicture()),
      _pauseTitleFrame(theCommander->getSeparatorFramePicture()),
      _menuTitle(theCommander->getLocalizedString("Pause")),
      _continueButton(theCommander->getLocalizedString("Continue game"), this,
                     theCommander->getButtonFramePicture(),
                     theCommander->getButtonOverFramePicture()),
      //optionsButton(theCommander->getLocalizedString("Options"), this),
      //audioButton(),
      //musicButton(),
      //fullScreenButton(),
      _abortButton(theCommander->getLocalizedString("Abort game"), this,
                  theCommander->getButtonFramePicture(),
                  theCommander->getButtonOverFramePicture()),
      _optionsBox(theCommander->getWindowFramePicture()),
      _optionsTitleFrame(theCommander->getSeparatorFramePicture()),
      _optionsTitle(theCommander->getLocalizedString("Options")),
      _optionsBack(theCommander->getLocalizedString("Back"), this),
      _pauseAction(pauseAction)
{
    setPolicy(USE_MIN_SIZE);
    _pauseTitleFrame.add(&_menuTitle);
    _pauseTitleFrame.setPreferedSize(Vec3(0, 20));
    _pauseVBox.add(&_pauseTitleFrame);
    _buttonsBox.add(&_continueButton);
    //buttonsBox.add(&optionsButton);
    _buttonsBox.add(&_abortButton);
    _pauseVBox.add(&_buttonsBox);
    _pauseContainer.addListener(*this);
    _pauseContainer.setPreferedSize(Vec3(350, 250));
    _pauseContainer.setPosition(Vec3((640-350)/2., (480-250)/2.));
    _topBox.add(&_pauseContainer);
    add(&_topSeparator);
    add(&_topBox);
    
    // Options menu
    /*optionsTitleFrame.add(&optionsTitle);
    optionsTitleFrame.setPreferedSize(Vec3(0, 20));
    optionsBox.add(&optionsTitleFrame);
    optionsButtonsBox.add(&audioButton);
    optionsButtonsBox.add(&musicButton);
    optionsButtonsBox.add(&fullScreenButton);
    optionsButtonsBox.add(&optionsBack);
    optionsBox.add(&optionsButtonsBox);*/
}

PauseMenu::~PauseMenu()
{
}

void PauseMenu::action(Widget *sender, int actionType, GameControlEvent *event)
{
  /*if (sender == &optionsButton) {
    _pauseContainer.transitionToContent(&_optionsBox);
  }
  else if (sender == &_optionsBack) {
    _pauseContainer.transitionToContent(&_pauseVBox);
  }*/
  if (sender == _continueButton.getButton()) {
      _pauseContainer.transitionToContent(NULL);
      _pauseAction->action(this, KPauseMenuClosing_Continue, event);
  }
  else if (sender == _abortButton.getButton()) {
      _pauseContainer.transitionToContent(NULL);
      _pauseAction->action(this, KPauseMenuClosing_Abort, event);
  }
}

void PauseMenu::backPressed(bool fromControls)
{
  if (!fromControls || (_pauseContainer.getContentWidget() == &_pauseVBox)) {
    _pauseContainer.transitionToContent(NULL);
  }
  if (_pauseContainer.getContentWidget() == &_optionsBox) {
    _pauseContainer.transitionToContent(&_pauseVBox);
  }
}

void PauseMenu::onSlideInside(SliderContainer &slider)
{
  if (slider.getContentWidget() == NULL) {
      _pauseAction->action(this, KPauseMenuClosed_Continue, NULL);
  }
}

void PauseMenu::onWidgetAdded(WidgetContainer *parent)
{
  _pauseContainer.transitionToContent(&_pauseVBox);
}

