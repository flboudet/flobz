#ifndef _FILESTREAM
#define _FILESTREAM

/**
 * iosfc::FileStream: contains file management implementation of streams.
 *
 * - FileOutputStream:  To write into a file.
 * - FileInputStream:   To read from a file.
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

#include "ios_stream.h"

namespace ios_fc {

    class FileInputStream : public InputStream {
        public:
            FileInputStream (const String path, const bool istext);
            virtual ~FileInputStream();
            int streamAvailable();
            int streamRead(VoidBuffer buffer);
        private:
            int fd;
            void *file;
    };

    class FileOutputStream : public OutputStream {
        public:
            FileOutputStream (const String path, const bool istext);
            virtual ~FileOutputStream ();
            int streamWrite(VoidBuffer buffer);
        private:
            int fd;
            void *file;
    };

};

#endif
