
#include <vector>
#include "ios_filemanager.h"

namespace ios_fc {

static std::vector<FileLoader*> loaders;

class FSLoader : public FileLoader
{
  public:
    bool isSupportedType(const String &type) const
    {
      return type == "file";
    }

    Buffer<char> load(const String &param) const
    {
      printf("Loading %s\n", param.c_str());
      int step   = 128;
      int offset = 0;
      Buffer<char> ret(step);
      FILE *f = fopen(param.c_str(), "r");
      if (!f) {
        ret.realloc(1);
        ret[0] = 0;
        return ret;
      }
      do {
        step = fread(ret + offset, 1, step, f);
        offset += step;
        ret.realloc(offset + step + 1);
      } while (step);
      ret[offset] = 0;
      return ret;
    }
};

static void initHandlers()
{
  static bool firstTime = true;
  if (firstTime) {
    FileLoader::addHandler(new FSLoader);
    firstTime = false;
  }
}

static FileLoader *findHandler(const String &type)
{
  printf("Looking for %s-loader\n", type.c_str());
  for (int i = 0; i < static_cast<int>(loaders.size()); ++i) {
    if (loaders[i]->isSupportedType(type))
      return loaders[i];
  }
  return NULL;
}


void FileLoader::addHandler(FileLoader *loader)
{
  loaders.push_back(loader);
}

Buffer<char> FileLoader::loadFrom(const String &url) 
{
  initHandlers();

  int i = 0;
  while((url[i]!=':') && (i<url.size())) i++;
  String type  = ios_fc::substring(url, 0, i);
  String param = ios_fc::substring(url, i+3);
  FileLoader *loader = findHandler(type);
  if (loader)
    return loader->load(param);
  return Buffer<char>(0);
}

}
