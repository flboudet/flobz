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

#include <stdexcept>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include "ios_filepath.h"


namespace ios_fc {

    FilePath::FilePath(const std::string &path) : _path(path)
    {
    }

    std::vector<std::string> FilePath::listFiles()
    {
        struct	dirent	*dptr;
        DIR	*dirp;
        std::vector<std::string> result;
        if((dirp=opendir(_path.c_str()))==NULL) {
            std::string errorMessage("Error opening ");
            perror("dirlist");
            throw std::runtime_error((errorMessage + _path).c_str());
        }
        dptr = readdir(dirp);
        while(dptr != NULL) {
            result.push_back(dptr->d_name);
            dptr = readdir(dirp);
        }
        closedir(dirp);
        return result;
    }

    std::string FilePath::combine(const std::string &path) const
    {
        return combine(this->_path, path);
    }

    std::string FilePath::combine(const std::string &path1, const std::string &path2)
    {
        return std::string(path1 + "/" + path2);
    }

    bool FilePath::exists() const
    {
        struct stat s;
        if (stat(_path.c_str(), &s) == -1) {
            if (errno == ENOENT)
                return false;
        }
        return true;
    }

    std::string FilePath::basename(void) const
    {
      int size = this->_path.size();
      int i;
      for (i=size; i>=0; i--)
      {
        if (this->_path[i] == '/')
        {
          i++;
          break;
        }
      }
      return this->_path.substr(i);
    }
}

