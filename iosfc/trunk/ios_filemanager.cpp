
#include "ios_filemanager.h"

namespace ios_fc {

class FSLoader : public FileLoader
{
  public:
    bool isSupportedType(const String &type) const
    {
      return type == "file";
    }

    Buffer<char> load(const String &param) const
    {
      printf("Loading %s\n", (const char *)param);
      int step   = 128;
      int offset = 0;
      Buffer<char> ret(step);
      FILE *f = fopen(param, "r");
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


Vector<FileLoader> loaders;

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
  printf("Looking for %s-loader\n", (const char *)type);
  for (int i = 0; i < loaders.size(); ++i) {
    if (loaders[i]->isSupportedType(type))
      return loaders[i];
  }
  return NULL;
}


void FileLoader::addHandler(FileLoader *loader)
{
  loaders.add(loader);
}

Buffer<char> FileLoader::loadFrom(const String &url) 
{
  initHandlers();

  int i = 0;
  while((url[i]!=':') && (i<url.size())) i++;
  String type  = url.substring(0,i);
  String param = url.substring(i+3);
  FileLoader *loader = findHandler(type);
  if (loader)
    return loader->load(param);
  return Buffer<char>(0);
}

};
