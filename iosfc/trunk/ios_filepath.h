/**
 * iosfc::FilePath: Platform-independant path operations
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

#ifndef _IOS_FILEPATH_H
#define _IOS_FILEPATH_H

#include "ios_memory.h"
#include "ios_vector.h"

namespace ios_fc {

class FilePath {
public:
    FilePath(const String &path);
    FilePath(const FilePath &origin);
    String combine(const String &path) const;
    static String combine(const String &path1, const String &path2);
    SelfVector<String> listFiles();
    bool exists() const;
    const String &getPathString() const { return path; }
private:
    String path;
};

}

#endif // _IOS_FILEPATH_H

