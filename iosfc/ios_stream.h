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
     * @brief Interface for input stream.
     */
    class InputStream {
        public:
            /**
             * @brief Check if the stream is available for input
             * @return number of bytes available int the stream (0 if stream not available)
             */

            virtual int streamAvailable() = 0;

            /**
             * @brief Read datas from stream.
             * @param buffer the buffer to store datas
             * @param size the size to read into buffer
             */
            virtual int streamRead(VoidBuffer buffer, int size) = 0;
            
            /**
             * @brief Read datas from stream.
             * @param buffer the buffer to store datas (buffer.size() octets will be read)
             */
            virtual int streamRead(VoidBuffer buffer);

            /**
             * @brief Read all the datas from the stream till its end.
             * This method returns when the Stream is closed.
             * @param buffer The buffer where datas will be stored.
             */
            virtual void streamReadAll (VoidBuffer buffer);

            /**
             * @brief virtual destructor.
             */
            virtual ~InputStream() {};
    };

    /**
     * @brief Interface for output stream.
     */
    class OutputStream {
        public:

            /**
             * @brief Write datas to stream.
             * @param buffer the buffer to store datas
             * @param size the size to write to the stream
             */
            virtual int streamWrite(VoidBuffer buffer, int size) = 0;
            
            /**
             * @brief Write datas to stream.
             * @param buffer the buffer where are the datas to write (buffer.size() octets will be written)
             */
            virtual int streamWrite(VoidBuffer buffer);
      
            /**
             * @brief virtual destructor.
             */
             virtual ~OutputStream() {};

    };

}

#endif // _IOSSTREAM

