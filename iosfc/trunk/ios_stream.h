#ifndef _IOSSTREAM
#define _IOSSTREAM

/**
 * iosfc::Stream: contains stream management interfaces definitions.
 *
 * - OutputStream:      interface for a writable stream.
 * - InputStream:       interface for a readable stream.
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

#include "ios_memory.h"

namespace ios_fc {

    /**
     * Interface for input stream
     */
    class InputStream {
        public:
            /**
             * Check if the stream is available for input
             * @return number of bytes available int the stream (0 if stream not available)
             */

            virtual int streamAvailable() = 0;

            /**
             * Read datas from stream.
             * @param buffer the buffer to store datas (buffer.size() octets will be read)
             */
            virtual int streamRead(VoidBuffer buffer) = 0;

            /**
             * Read all the datas from the stream till its end.
             * This method returns when the Stream is closed.
             * @param buffer The buffer where datas will be stored.
             */
            virtual void streamReadAll (VoidBuffer buffer);
    };

    /**
     * Interface for input stream
     */
    class OutputStream {
        public:

            /**
             * Write datas to stream.
             * @param buffer the buffer to store datas (buffer.size() octets will be read)
             */
            virtual int streamWrite(VoidBuffer buffer) = 0;
    };

};

#endif // _IOSSTREAM

