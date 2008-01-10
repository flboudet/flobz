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


#include "PuyoChatBox.h"
#include "PuyoCommander.h"

using namespace ios_fc;

NetCenterChatArea::NetCenterChatArea(int height)
    : height(height), lines(new (HBox *[height])), names(new (Text *[height])), texts(new (Text *[height]))
{
    setPolicy(USE_MIN_SIZE);
    setFocusable(false);
    for (int i = 0 ; i < height ; i++) {
        lines[i] = new HBox;
        lines[i]->setPolicy(USE_MIN_SIZE);
        lines[i]->setPreferedSize(Vec3(0, 0));
        names[i] = new Text("");
        names[i]->setFont(GameUIDefaults::FONT_SMALL_ACTIVE);
        names[i]->setPreferedSize(Vec3(120, 1));
        texts[i] = new Text("");
        texts[i]->setFont(GameUIDefaults::FONT_SMALL_INFO);
        texts[i]->setPreferedSize(Vec3(0, 0));
        lines[i]->add(names[i]);
        lines[i]->add(texts[i]);
        add(lines[i]);
    }
}

NetCenterChatArea::~NetCenterChatArea()
{
    for (int i = 0 ; i < height ; i++) {
        delete texts[i];
        delete names[i];
        delete lines[i];
    }
    delete[] lines;
    delete[] names;
    delete[] texts;
}

void NetCenterChatArea::addChat(String name, String text)
{
    suspendLayout();
    while (text.length() < 150)
        text += " ";
    for (int i = 0 ; i < height-1 ; i++) {
        names[i]->setValue(names[i+1]->getValue());
        texts[i]->setValue(texts[i+1]->getValue());
        names[i]->setPreferedSize(Vec3(120, 12));
        texts[i]->setPreferedSize(Vec3(0, 0));
    }
    names[height-1]->setValue(name);
    texts[height-1]->setValue(text);
    names[height-1]->setPreferedSize(Vec3(120, 12));
    texts[height-1]->setPreferedSize(Vec3(0, 0));
    resumeLayout();
    arrangeWidgets();
}


ChatBox::ChatBox(ChatBoxDelegate &delegate)
  : chatBoxBG(IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/chatzonebg.png"))),
    delegate(delegate), chatAction(this), chatInputLabel("Say:"), chatInput("Hello", &chatAction), chatArea(10),
    topSeparator(1, 8), leftEditSeparator(15, 1), betweenSeparator(1, 8), leftChatSeparator(15, 1), bottomSeparator(1, 4)
{
    suspendLayout();
    setPolicy(USE_MIN_SIZE);
    chatBoxContainer.setPolicy(USE_MIN_SIZE);
    chatInputContainer.setPolicy(USE_MIN_SIZE);
    chatAreaContainer.setPolicy(USE_MIN_SIZE);
    
    chatInputContainer.add(&leftEditSeparator);
    chatInputContainer.add(&chatInputLabel);
    chatInputContainer.add(&chatInput);
    chatAction.setEditField(&chatInput);
    chatInputLabel.setAutoSize(false);
    chatInputLabel.setPreferedSize(Vec3(120, 1));
	chatInput.setEditOnFocus(true);
    chatInput.setAutoSize(false);
    chatInput.setPreferedSize(Vec3(0, 0));
    chatInputContainer.setPreferedSize(Vec3(1, 28));
    
    chatAreaContainer.add(&leftChatSeparator);
    chatAreaContainer.add(&chatArea);
    
    setBackground(chatBoxBG);
    chatBoxContainer.add(&topSeparator);
    chatBoxContainer.add(&chatAreaContainer);
    chatBoxContainer.add(&betweenSeparator);
    chatBoxContainer.add(&chatInputContainer);
    chatBoxContainer.add(&bottomSeparator);
    add(&chatBoxContainer);
    
    //chatArea.setPreferedSize(Vec3(640, 180, 0));
    chatBoxContainer.setPreferedSize(Vec3(640, 216, 0));
    
    resumeLayout();
}

ChatBox::~ChatBox()
{
    IIM_Free(chatBoxBG);
}

void ChatBox::addChat(String name, String message)
{
    chatArea.addChat(name, message);
}

void ChatBox::ChatAction::action()
{
    String chatString = attachedEditField->getValue();
    attachedEditField->setValue("");
    owner->delegate.sendChat(chatString);
}




