/**
 * iosfc::IGPDatagram: contains an object for handling IGP messages.
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

#include "ios_igpdatagram.h"
#include "ios_memory.h"

namespace ios_fc {

const char IGPDatagram::MSGIDENT[] = "CMD";
const char IGPDatagram::IGPIDENT[] = "ID";
const char IGPDatagram::IGPORIGIDENT[] = "OID";
const char IGPDatagram::IGPDESTIDENT[] = "DID";
const char IGPDatagram::IGPMSG[] = "MSG";

IGPDatagram::IGPDatagram(Message *data) : message(data)
{
    msgIdent = message->getInt(MSGIDENT);
}

IGPDatagram::IGPDatagram(Message *data, IGPMsgIdent ident) : message(data), msgIdent(ident)
{
    message->addInt(MSGIDENT, ident);
}

};
