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


#include "ChatBox.h"
#include "PuyoCommander.h"

using namespace ios_fc;

void ChatBox::addChat(String name, String text)
{
    suspendLayout();
    float nameLen, maxNameLen = 0.0f;
    for (int i = 0 ; i < height-1 ; i++) {
        names[i]->setValue(names[i+1]->getValue());
        texts[i]->setValue(texts[i+1]->getValue());
        nameLen = names[i]->getPreferedSize().x;
        if (nameLen > maxNameLen) maxNameLen = nameLen;
    }
    names[height-1]->setValue(name);
    texts[height-1]->setValue(text);
    nameLen = names[height-1]->getPreferedSize().x;
    if (nameLen > maxNameLen) maxNameLen = nameLen;
    for (int i = 0 ; i < height ; i++) {
        names[i]->setPreferedSize(Vec3(maxNameLen+innerMargin, 0.0f));
        texts[i]->setPreferedSize(Vec3(0.0f, 0.0f));
    }
    resumeLayout();
    arrangeWidgets();
}

ChatBox::ChatBox(ChatBoxDelegate &delegate)
  : Frame(theCommander->getWindowFramePicture()),
    delegate(delegate), chatAction(this), chatInputLabel(theCommander->getLocalizedString("Say:")),
    chatInput(theCommander->getLocalizedString("Hello"), &chatAction,theCommander->getEditFieldFramePicture(),theCommander->getEditFieldFramePicture()),
    chatInputFrameSurface(
        GameUIDefaults::GAME_LOOP->getDrawContext()->getIIMLibrary()
        .load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/chatzone.png"))),
    chatInputFramePicture(chatInputFrameSurface, 31, 10, 25, 9, 6, 15),
    chatInputContainerFrame(&chatInputFramePicture),
    height(8), lines(new (HBox *[height])), names(new (Text *[height])), texts(new (Text *[height]))
{
    Vec3 lineSize(0.0f, GameUIDefaults::FONT_TEXT->getHeight(), 1.0f);

    suspendLayout();

    for (int i = 0 ; i < height ; i++) {
        names[i] = new Text("");
        names[i]->setFont(GameUIDefaults::FONT_SMALL_ACTIVE);
        names[i]->setPreferedSize(Vec3(0.0f, 0.0f));
        texts[i] = new Text("");
        texts[i]->setFont(GameUIDefaults::FONT_SMALL_INFO);
        texts[i]->setPreferedSize(Vec3(0.0f, 0.0f));
        lines[i] = new HBox;
        lines[i]->setPolicy(USE_MIN_SIZE);
        lines[i]->setPreferedSize(lineSize);
        lines[i]->add(names[i]);
        lines[i]->add(texts[i]);
        add(lines[i]);
    }

    setPolicy(USE_MAX_SIZE_NO_MARGIN);

    //chatInputContainerFrame.setInnerMargin(0.0f);
    chatInputContainerFrame.add(&chatInputContainer);
    chatInputContainer.add(&chatInputLabel);
    chatInputContainer.add(&chatInput);
    chatAction.setEditField(&(chatInput.getEditField()));
    Vec3 s = chatInputLabel.getPreferedSize();
    s.x += innerMargin;
    chatInputLabel.setPreferedSize(s);
    chatInput.getEditField().setEditOnFocus(true);
    chatInput.getEditField().setAutoSize(false);
    chatInputContainerFrame.setPreferedSize(Vec3(0.0f, 0.0f));
    //chatInput.setPreferedSize(Vec3(0.0f, 0.0f));
    chatInputContainer.setPreferedSize(lineSize);
    setInnerMargin(6);
    add(&chatInputContainerFrame);

    resumeLayout();
}

ChatBox::~ChatBox()
{
    for (int i = 0 ; i < height ; i++) {
        delete texts[i];
        delete names[i];
        delete lines[i];
    }
    delete[] lines;
    delete[] names;
    delete[] texts;
    delete chatInputFrameSurface;
}

void ChatBox::ChatAction::action()
{
    String chatString = attachedEditField->getValue();
    attachedEditField->setValue("");
    owner->delegate.sendChat(chatString);
}
