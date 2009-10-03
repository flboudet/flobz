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

#include "PauseMenu.h"
using namespace event_manager;

PauseMenu::PauseMenu(Action *pauseAction)
    : topSeparator(0, 10), pauseVBox(theCommander->getWindowFramePicture()),
      pauseTitleFrame(theCommander->getSeparatorFramePicture()),
      menuTitle(theCommander->getLocalizedString("Pause")),
      continueButton(theCommander->getLocalizedString("Continue game"), this),
      optionsButton(theCommander->getLocalizedString("Options"), this),
      audioButton(),
      musicButton(),
      fullScreenButton(),
      abortButton(theCommander->getLocalizedString("Abort game"), this),
      optionsBox(theCommander->getWindowFramePicture()),
      optionsTitleFrame(theCommander->getSeparatorFramePicture()),
      optionsTitle(theCommander->getLocalizedString("Options")),
      optionsBack(theCommander->getLocalizedString("Back"), this),
      m_pauseAction(pauseAction)
{
    setPolicy(USE_MIN_SIZE);
    pauseTitleFrame.add(&menuTitle);
    pauseTitleFrame.setPreferedSize(Vec3(0, 20));
    pauseVBox.add(&pauseTitleFrame);
    buttonsBox.add(&continueButton);
    buttonsBox.add(&optionsButton);
    buttonsBox.add(&abortButton);
    pauseVBox.add(&buttonsBox);
    pauseContainer.addListener(*this);
    pauseContainer.setPreferedSize(Vec3(350, 250));
    pauseContainer.setPosition(Vec3((640-350)/2., (480-250)/2.));
    topBox.add(&pauseContainer);
    add(&topSeparator);
    add(&topBox);

    // Options menu
    optionsTitleFrame.add(&optionsTitle);
    optionsTitleFrame.setPreferedSize(Vec3(0, 20));
    optionsBox.add(&optionsTitleFrame);
    optionsButtonsBox.add(&audioButton);
    optionsButtonsBox.add(&musicButton);
    optionsButtonsBox.add(&fullScreenButton);
    optionsButtonsBox.add(&optionsBack);
    optionsBox.add(&optionsButtonsBox);
}

PauseMenu::~PauseMenu()
{
}

void PauseMenu::action(Widget *sender, int actionType, GameControlEvent *event)
{
  if (sender == &optionsButton) {
    pauseContainer.transitionToContent(&optionsBox);
  }
  else if (sender == &optionsBack) {
    pauseContainer.transitionToContent(&pauseVBox);
  }
  else if (sender == &continueButton) {
      pauseContainer.transitionToContent(NULL);
      m_pauseAction->action(this, KPauseMenuClosing_Continue, event);
  }
  else if (sender == &abortButton) {
      pauseContainer.transitionToContent(NULL);
      m_pauseAction->action(this, KPauseMenuClosing_Abort, event);
  }
}

void PauseMenu::backPressed(bool fromControls)
{
  if (!fromControls || (pauseContainer.getContentWidget() == &pauseVBox)) {
    pauseContainer.transitionToContent(NULL);
  }
  if (pauseContainer.getContentWidget() == &optionsBox) {
    pauseContainer.transitionToContent(&pauseVBox);
  }
}

void PauseMenu::onSlideInside(SliderContainer &slider)
{
  if (slider.getContentWidget() == NULL) {
      m_pauseAction->action(this, KPauseMenuClosed_Continue, NULL);
  }
}

void PauseMenu::onWidgetAdded(WidgetContainer *parent)
{
  pauseContainer.transitionToContent(&pauseVBox);
}

