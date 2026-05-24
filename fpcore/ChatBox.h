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
#ifndef _PUYOCHATBOX
#define _PUYOCHATBOX

#include <string>
#include "Frame.h"
#include "FramedEditField.h"
#include "ios_memory.h"
#include "ios_messagebox.h"
#include "FPCommander.h"

using namespace ios_fc;
using namespace gameui;

class ChatBoxDelegate
{
public:
    virtual void sendChat(const std::string &chatText) = 0;
    virtual ~ChatBoxDelegate() {};
};

class ChatBox : public Frame
{
public:
    ChatBox(ChatBoxDelegate &delegate);
    virtual ~ChatBox();
    void addChat(const std::string & name, const std::string & message);
private:
    class ChatAction : public Action {
    public:
        ChatAction(ChatBox *owner) : _owner(owner) {}
        void setEditField(EditField *attachedEditField) { this->_attachedEditField = attachedEditField; }
        void action();
        /// @brief 
        /// @param name 
        /// @param text 
        void addChat(const std::string &name, const std::string &text);
    private:
        ChatBox *_owner;
        EditField *_attachedEditField;
    };
    ChatBoxDelegate &_delegate;
    ChatAction _chatAction;
    Text _chatInputLabel;
    FramedEditField _chatInput;
    IosSurfaceRef _chatInputFrameSurface;
    FramePicture _chatInputFramePicture;
    Frame _chatInputContainerFrame;
    HBox _chatInputContainer;
    int _height;
    HBox **_lines;
    Text **_names;
    Text **_texts;
};

#endif // _PUYOCHATBOX
