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

IGPDatagram::IGPDatagram(VoidBuffer data) : msgSize(0), message(data)
{
    msgIdent = readBigEndianIntFromMessage(0);
    msgSize = readBigEndianIntFromMessage(4);
    if (msgSize + 8 > message.size()) {
        printf("Message incomplet!");
    }
}

IGPDatagram::IGPDatagram(IGPMsgIdent ident, int msgSize) : msgSize(msgSize), message(msgSize + 8)
{
    msgIdent = ident;
}

IGPDatagram::IGPDatagram(InputStream *stream) : msgSize(0), message(8)
{
    int bytesRead = 0;
    //while (bytesRead < 8) {
        int readResult = stream->streamRead(message, 8 - bytesRead);
        bytesRead += readResult;
        printf("Lu: %d bytes\n", readResult);
    //}
    msgIdent = readBigEndianIntFromMessage(0);
    msgSize = readBigEndianIntFromMessage(4);
    printf("Taille message:%d ident:%d\n", msgSize, msgIdent);
    //printf("Addr msg:%x ; addr msg+1:%x\n", message.ptr(), (message+1).ptr());
    if (msgSize > 0) {
        message.realloc(msgSize + 8);
        int bytesRead = 0;
        while (bytesRead < msgSize) {
            int readResult = stream->streamRead(message + 8 + bytesRead, msgSize-bytesRead);
            bytesRead += readResult;
            printf("Lu: %d bytes\n", readResult);
        }
    }
}

VoidBuffer IGPDatagram::serialize()
{
    writeBigEndianIntToMessage(msgIdent, 0);
    writeBigEndianIntToMessage(msgSize, 4);
    return message;
}


};
