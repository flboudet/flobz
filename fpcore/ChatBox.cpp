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


#include "ChatBox.h"
#include "FPCommander.h"

using namespace ios_fc;

void ChatBox::addChat(const std::string & name, const std::string & text)
{
    suspendLayout();
    float nameLen, maxNameLen = 0.0f;
    for (int i = 0 ; i < _height-1 ; i++) {
        _names[i]->setValue(_names[i+1]->getValue());
        _texts[i]->setValue(_texts[i+1]->getValue());
        nameLen = _names[i]->getPreferedSize().x;
        if (nameLen > maxNameLen) maxNameLen = nameLen;
    }
    _names[_height-1]->setValue(name);
    _texts[_height-1]->setValue(text);
    nameLen = _names[_height-1]->getPreferedSize().x;
    if (nameLen > maxNameLen) maxNameLen = nameLen;
    for (int i = 0 ; i < _height ; i++) {
        _names[i]->setPreferedSize(Vec3(maxNameLen+innerMargin, 0.0f));
        _texts[i]->setPreferedSize(Vec3(0.0f, 0.0f));
    }
    resumeLayout();
    arrangeWidgets();
}

ChatBox::ChatBox(ChatBoxDelegate &delegate)
  : Frame(theCommander->getWindowFramePicture()),
    _delegate(delegate), _chatAction(this), _chatInputLabel(theCommander->getLocalizedString("Say:")),
    _chatInput(theCommander->getLocalizedString("Hello"), &_chatAction,theCommander->getEditFieldFramePicture(),theCommander->getEditFieldFramePicture()),
    _chatInputFrameSurface(theCommander->getSurface(IMAGE_RGBA, "gfx/chatzone.png")),
    _chatInputFramePicture(_chatInputFrameSurface, 31, 10, 25, 9, 6, 15),
    _chatInputContainerFrame(&_chatInputFramePicture),
    _height(8), _lines(new HBox *[_height]), _names(new Text *[_height]), _texts(new Text *[_height])
{
    Vec3 lineSize(0.0f, GameUIDefaults::FONT_TEXT->getHeight(), 1.0f);

    suspendLayout();

    for (int i = 0 ; i < _height ; i++) {
        _names[i] = new Text("");
        _names[i]->setFont(GameUIDefaults::FONT_SMALL_ACTIVE);
        _names[i]->setPreferedSize(Vec3(0.0f, 0.0f));
        _texts[i] = new Text("");
        _texts[i]->setFont(GameUIDefaults::FONT_SMALL_INFO);
        _texts[i]->setPreferedSize(Vec3(0.0f, 0.0f));
        _lines[i] = new HBox;
        _lines[i]->setPolicy(USE_MIN_SIZE);
        _lines[i]->setPreferedSize(lineSize);
        _lines[i]->add(_names[i]);
        _lines[i]->add(_texts[i]);
        add(_lines[i]);
    }

    setPolicy(USE_MAX_SIZE_NO_MARGIN);

    //chatInputContainerFrame.setInnerMargin(0.0f);
    _chatInputContainerFrame.add(&_chatInputContainer);
    _chatInputContainer.add(&_chatInputLabel);
    _chatInputContainer.add(&_chatInput);
    _chatAction.setEditField(&(_chatInput.getEditField()));
    Vec3 s = _chatInputLabel.getPreferedSize();
    s.x += innerMargin;
    _chatInputLabel.setPreferedSize(s);
    _chatInput.getEditField().setEditOnFocus(true);
    _chatInput.getEditField().setAutoSize(false);
    _chatInputContainerFrame.setPreferedSize(Vec3(0.0f, 0.0f));
    //chatInput.setPreferedSize(Vec3(0.0f, 0.0f));
    _chatInputContainer.setPreferedSize(lineSize);
    setInnerMargin(6);
    add(&_chatInputContainerFrame);

    resumeLayout();
}

ChatBox::~ChatBox()
{
    for (int i = 0 ; i < _height ; i++) {
        delete _texts[i];
        delete _names[i];
        delete _lines[i];
    }
    delete[] _lines;
    delete[] _names;
    delete[] _texts;
}

void ChatBox::ChatAction::action()
{
    std::string chatString = _attachedEditField->getValue();
    _attachedEditField->setValue("");
    _owner->_delegate.sendChat(chatString);
}
