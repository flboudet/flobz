#ifndef _FILE_MANAGER_H
#define _FILE_MANAGER_H

#include "ios_fc.h"

namespace ios_fc {

class FileLoader {
  public:
    virtual Buffer<char> load(const std::string &param)   const = 0;
    virtual bool isSupportedType(const std::string &type) const = 0;

    static void addHandler(FileLoader *loader);
    static Buffer<char> loadFrom(const std::string &url);
  
    virtual ~FileLoader() {};
};

}

#endif
