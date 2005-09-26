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

#ifndef _PUYONETGAMECENTER_H
#define _PUYONETGAMECENTER_H

#include "ios_memory.h"
#include "ios_dirigeable.h"
#include "ios_messagebox.h"

using namespace ios_fc;

class PuyoNetGameCenterListener {
public:
    virtual void onChatMessage(const String &msgAuthor, const String &msg) = 0;
    virtual void onPlayerConnect(String playerName, PeerAddress playerAddress) = 0;
    virtual void gameInvitationAgainst(String playerName, PeerAddress playerAddress) = 0;
    virtual void gameGrantedWithMessagebox(MessageBox *mbox) = 0;
};

class PuyoNetGameCenter {
public:
    virtual void sendMessage(const String msgText) = 0;
    void requestGameWith(PeerAddress addr);
    void acceptInvitationWith(PeerAddress addr);
    virtual void idle() = 0;
    String getPeerNameAtIndex(int i) const;
    int getPeerCount() const;
    void addListener(PuyoNetGameCenterListener *r) { listeners.add(r); }
    void removeListener(PuyoNetGameCenterListener *r) { listeners.remove(r); }
    void connectPeer(PeerAddress addr, const String name);
    void disconnectPeer(PeerAddress addr, const String name);
protected:
    AdvancedBuffer<PuyoNetGameCenterListener *> listeners;
    class GamerPeer;
    AdvancedBuffer<GamerPeer *> peers;
    GamerPeer *getPeerForAddress(PeerAddress addr);
    virtual void requestGameWithPeer(String playerName, PeerAddress addr) = 0;
    virtual void acceptInvitationWithPeer(String playerName, PeerAddress addr) = 0;
};

#endif // _PUYONETGAMECENTER_H
