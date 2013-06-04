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
#include <string>

namespace ios_fc {
    class Exception : public std::exception {
        public:
            Exception(const char *format, ...);
            Exception(const Exception &e);
            void operator=(const Exception &e);
            ~Exception() throw();
            void  printMessage()      const;
            const char *what()        const throw();
            const char *getStackTrace() const;

        private:
            char *message;
            std::string stack;
    };

std::string get_stack_trace();
void catch_signals();
void reportCrash(const std::string &error);
}


#ifdef DEBUG

#include <cstdio>
#include <cstdlib>

static inline void IOS_ERROR(const char *msg)
{
	std::fprintf(stderr,"[DEBUG] ios_error: %s\n", msg);
	std::fflush(stderr);
	throw ios_fc::Exception(msg);
}

#define IOS_ASSERT(expr) if (!(expr)) IOS_ERROR(ios_fc::String("Assertion '" #expr "' failed\n  location: " __FILE__ ":") + __LINE__ + ", " + __PRETTY_FUNCTION__); else;

#else // DEBUG

#define IOS_ASSERT(expr) (static_cast<void>(0))
//#define IOS_ERROR(msg) throw ios_fc::Exception(msg);
static inline void IOS_ERROR(const char *msg)
{
    throw ios_fc::Exception(msg);
}

#endif // DEBUG

#endif // _EXCEPTION_H

