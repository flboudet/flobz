#ifndef _IOSEXCEPTION_H
#define _IOSEXCEPTION_H

/**
 * iosfc::Exception: contains a management handling class.
 *
 * - Exception:         A simple exception containing a string message.
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
 
#include <exception>

namespace ios_fc {

    class Exception : public std::exception {
        public:
            Exception(const String msg);
            ~Exception() throw();
            void  printMessage()      const;
            const String getMessage() const;
            const char *what()        const throw();

        private:
            String message;
    };

#define IOS_ERROR(msg) { fprintf(stderr,"%s\n",msg); fflush(stderr); throw new ios_fc::Exception(msg); }

};

#endif // _EXCEPTION_H

