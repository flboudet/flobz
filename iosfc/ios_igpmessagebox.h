/* Ultimate Othello 1678
 * Copyright (C) 2002  Florent Boudet <flobo@ifrance.com>
 * iOS Software <http://ios.free.fr>
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


#ifndef _IOS_IGPMESSAGEBOX_H
#define _IOS_IGPMESSAGEBOX_H

#include "ios_messagebox.h"
#include "ios_igpclient.h"
#include "ios_dirigeable.h"
#include "ios_igpmessage.h"

namespace ios_fc {

// IGP stands for Ios Gateway Protocol

class IgpMessageBoxBase : public MessageBox, IGPClientMessageListener {
public:
    IgpMessageBoxBase(MessageBox *mbox);
    IgpMessageBoxBase(MessageBox *mbox, int igpIdent);
    virtual ~IgpMessageBoxBase();
    virtual void idle();
    void sendBuffer(VoidBuffer out, bool reliable, int igpDestIdent);
    void bind(int igpIdent) { destIdent = igpIdent; }
    void bind(PeerAddress addr); // a revoir
    int getBound() const { return destIdent; }
    PeerAddress getSelfAddress() const; // a revoir
    bool isConnected() const { return igpClient->isEnabled(); }
 protected:
    MessageBox *mbox;
    bool ownMessageBox;
    IGPClient *igpClient;
    int sendSerialID;
    int destIdent;
};

template <typename T>
class IgpMessageBox : public IgpMessageBoxBase {
public:
    IgpMessageBox(MessageBox *mbox);
    IgpMessageBox(MessageBox *mbox, int igpIdent);
    virtual Message * createMessage();
    virtual void onMessage(VoidBuffer message, int origIdent, int destIdent);
};

template <typename T>
class IgpMessageBoxMessage : public IgpMessage<T> {
public:
    IgpMessageBoxMessage(int serialID, IgpMessageBox<T> &owner, int igpPeerIdent);
    IgpMessageBoxMessage(const Buffer<char> serialized, IgpMessageBox<T> &owner, int igpPeerIdent);
    virtual void send();
    bool isReliable() const {
        if (T::hasBoolProperty("RELIABLE"))
            return T::getBoolProperty("RELIABLE");
        return false;
    }
private:
    IgpMessageBox<T> &owner;
};

template <typename T>
IgpMessageBoxMessage<T>::IgpMessageBoxMessage(int serialID, IgpMessageBox<T> &owner, int igpPeerIdent) : IgpMessage<T>(serialID, igpPeerIdent), owner(owner)
{}

template <typename T>
    IgpMessageBoxMessage<T>::IgpMessageBoxMessage(const Buffer<char> serialized, IgpMessageBox<T> &owner, int igpPeerIdent)
    : IgpMessage<T>(serialized, igpPeerIdent), owner(owner)
{}

/*template <typename T>
void IgpMessageBoxMessage<T>::sendBuffer(Buffer<char> out) const
{
    owner.sendBuffer(out, T::isReliable(), IgpMessageBoxMessage<T>::igpPeerIdent);
}*/

template <typename T>
void IgpMessageBoxMessage<T>::send()
{
    VoidBuffer out = T::serialize();
    owner.sendBuffer(out, isReliable(), IgpMessageBoxMessage<T>::igpPeerIdent);
}

template <typename T>
IgpMessageBox<T>::IgpMessageBox(MessageBox *mbox)
    : IgpMessageBoxBase(mbox)
{}

template <typename T>
IgpMessageBox<T>::IgpMessageBox(MessageBox *mbox, int igpIdent)
    : IgpMessageBoxBase(mbox, igpIdent)
{}

template <typename T>
Message * IgpMessageBox<T>::createMessage()
{
    return new IgpMessageBoxMessage<T>(++(IgpMessageBoxBase::sendSerialID), *this, IgpMessageBoxBase::destIdent);
}

template <typename T>
void IgpMessageBox<T>::onMessage(VoidBuffer message, int origIdent, int destIdent)
    {
        try {
            IgpMessageBoxMessage<T> incomingMessage(message, *this, origIdent);
            propagateMessageToListeners(incomingMessage);
        }
        catch (Exception e) {
            e.printMessage();
        }
    }
}

#endif // _IOS_IGPMESSAGE_BOX_H
