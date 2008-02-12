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
#ifndef _PUYOCHATBOX
#define _PUYOCHATBOX

#include "gameui.h"
#include "ios_memory.h"
#include "ios_messagebox.h"

using namespace ios_fc;
using namespace gameui;

class ChatBoxDelegate
{
public:
    virtual void sendChat(String chatText) = 0;
    virtual ~ChatBoxDelegate() {};
};

class NetCenterChatArea : public VBox {
public:
    NetCenterChatArea(int height);
    ~NetCenterChatArea();
    void addChat(String name, String text);
private:
    int height;
    HBox **lines;
    Text **names;
    Text **texts;
};

class ChatBox : public SliderContainer
{
public:
    ChatBox(ChatBoxDelegate &delegate);
    virtual ~ChatBox();
    void addChat(String name, String message);
    void slideOut();
    void slideIn();
private:
    IIM_Surface   *chatBoxBG;
    class ChatAction : public Action {
    public:
        ChatAction(ChatBox *owner) : owner(owner) {}
        void setEditField(EditField *attachedEditField) { this->attachedEditField = attachedEditField; }
        void action();
    private:
        ChatBox *owner;
        EditField *attachedEditField;
    };
    ChatBoxDelegate &delegate;
    ChatAction chatAction;
    Text chatInputLabel;
    EditField chatInput;
    NetCenterChatArea chatArea;
    Separator topSeparator, leftEditSeparator, betweenSeparator, leftChatSeparator, bottomSeparator;
    HBox chatInputContainer, chatAreaContainer;
    VBox chatBoxContainer;
};

#endif // _PUYOCHATBOX
