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

#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include "ios_filepath.h"


namespace ios_fc {

    FilePath::FilePath(const String &path) : path(path)
    {
    }

    SelfVector<String> FilePath::listFiles()
    {
        struct	dirent	*dptr;
        DIR	*dirp;
        SelfVector<String> result;
        if((dirp=opendir(path))==NULL) {
            std::string errorMessage("Error opening ");
            perror("dirlist");
            throw Exception((errorMessage + (const char *)path).c_str());
        }
        dptr = readdir(dirp);
        while(dptr != NULL) {
            result.add(dptr->d_name);
            dptr = readdir(dirp);
        }
        closedir(dirp);
        return result;
    }

    String FilePath::combine(const String &path) const
    {
        return combine(this->path, path);
    }

    String FilePath::combine(const String &path1, const String &path2)
    {
        return String(path1 + "/" + path2);
    }

    bool FilePath::exists() const
    {
        struct stat s;
        if (stat(path, &s) == -1) {
            if (errno == ENOENT)
                return false;
        }
        return true;
    }

    String FilePath::basename(void) const
    {
      int size = this->path.size();
      int i;
      for (i=size; i>=0; i--)
      {
        if (this->path[i] == '/')
        {
          i++;
          break;
        }
      }
      return this->path.substring(i);
    }
}

