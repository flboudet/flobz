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

IGPDatagram::IGPDatagram(VoidBuffer data)
{
    Buffer<char> chardata(data);
    msgIdent = (chardata[0] << 24) | (chardata[1] << 16) | (chardata[2] << 8) | chardata[3];
    msgSize = (chardata[4] << 24) | (chardata[5] << 16) | (chardata[6] << 8) | chardata[3];
    if (msgSize + 8 > chardata.size()) {
        printf("Message incomplet!");
    }
}

IGPDatagram::IGPDatagram(IGPMsgIdent ident, VoidBuffer message)
{
    msgSize = message.size();
    msgIdent = ident;
    printf("Creation message");
}

VoidBuffer IGPDatagram::serialize()
{
    Buffer<char> result(8);
    result[0] = (msgIdent & 0xFF000000) >> 24;
    result[1] = (msgIdent & 0x00FF0000) >> 16;
    result[2] = (msgIdent & 0x0000FF00) >> 8;
    result[3] = (msgIdent & 0x000000FF);
    
    result[4] = (msgSize & 0xFF000000) >> 24;
    result[5] = (msgSize & 0x00FF0000) >> 16;
    result[6] = (msgSize & 0x0000FF00) >> 8;
    result[7] = (msgSize & 0x000000FF);
    return result;
}

};
