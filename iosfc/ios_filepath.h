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

#include <string>
#include <vector>

namespace ios_fc {

class FilePath {
public:
    FilePath(const std::string &path);
    FilePath(const FilePath &origin);
    std::string combine(const std::string &path) const;
    std::string basename(void) const;
    static std::string combine(const std::string &path1, const std::string &path2);
    std::vector<std::string> listFiles();
    bool exists() const;
    const std::string &getPathString() const { return _path; }
private:
    std::string _path;
};

}

#endif // _IOS_FILEPATH_H

