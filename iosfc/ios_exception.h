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

#include <cstdio>
#include <exception>
#include <stdexcept>
#include <string>

namespace ios_fc {

static inline void printException(const std::exception &e)
{
    std::fprintf(stderr, "Exception thrown: %s\n", e.what());
}

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
	throw std::runtime_error(msg);
}

#define IOS_ASSERT(expr) if (!(expr)) IOS_ERROR(std::string("Assertion '" #expr "' failed\n  location: " __FILE__ ":") + std::to_string(__LINE__) + ", " + __PRETTY_FUNCTION__); else;

#else // DEBUG

#define IOS_ASSERT(expr) (static_cast<void>(0))
//#define IOS_ERROR(msg) throw std::runtime_error(msg);
static inline void IOS_ERROR(const char *msg)
{
    throw std::runtime_error(msg);
}

#endif // DEBUG

#endif // _EXCEPTION_H

