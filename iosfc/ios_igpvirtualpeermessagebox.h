/**
 * iosfc::IgpVirtualPeerMessageBox: a messagebox designed as a virtual peer
 * for the IgpMessageListener. Handy to implement a custom service on an IGP
 * server.
 * 
 * This file is part of the iOS Foundation Classes project.
 *
 * authors:
 *  Jean-Christophe Hoelt <jeko@ios-software.com>
 *  Guillaume Borios      <gyom@ios-software.com>
 *  Florent Boudet       <flobo@ios-software.com>
 *  Michel Metzger -- participate during the developement of Shubunkin!
 *
 * http://www.ios-software.com/
 *
 * Released under the terms of the GNU General Public Licence.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#ifndef _IGPVIRTUALPEERMESSAGEBOX_H
#define _IGPVIRTUALPEERMESSAGEBOX_H

#include "ios_messagebox.h"
#include "ios_igpmessagelistener.h"

namespace ios_fc {

class IgpVirtualPeerMessageBox : public MessageBox, private IgpVirtualPeer {
public:
    IgpVirtualPeerMessageBox(IgpMessageListener &igpListener, int igpIdent);
    IgpVirtualPeerMessageBox(IgpMessageListener &igpListener);
    // Implement MessageBox
    // The idle() function is useless, since the messagebox is virtual and
    // uses the IOs from the IgpMessageListener
    void idle() {}
    Message * createMessage();
    // Implement IgpPeer
    void messageReceived(VoidBuffer message, int origIgpIdent, bool reliable);
    // Own member functions
    void sendBuffer(VoidBuffer out, bool reliable, int igpDestIdent);
    void bind(int igpIdent) { destIdent = igpIdent; }
    void bind(PeerAddress addr); // a revoir
    int getBound() const { return destIdent; }
private:
    int sendSerialID;
    int destIdent;
    bool m_isCorrupted;
};

}

#endif // _IGPVIRTUALPEERMESSAGEBOX_H

