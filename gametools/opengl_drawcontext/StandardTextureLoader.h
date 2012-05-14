#ifndef _STANDARD_TEXTURE_LOADER_H
#define _STANDARD_TEXTURE_LOADER_H

#include "OpenGLDrawContext.h"
#include "DataPathManager.h"

class StandardTextureLoader : public OpenGLBackendUtil
{
public:
    StandardTextureLoader(DataPathManager *dataPathManager);
    virtual unsigned short * utf8ToUnicode(const char *utf8Text, unsigned short *unicodeTextBuffer, size_t unicodeTextBufferSize);
    virtual OpenGLRawImage * loadImage(ImageType type, const char *path);
    virtual DataPathManager * getdataPathManager() const { return m_dataPathManager; }
private:
    OpenGLRawImage * loadImagePNG(ImageType type, const char *path);
    OpenGLRawImage * loadImageJPG(ImageType type, const char *path);
private:
    DataPathManager *m_dataPathManager;
};

#endif // _STANDARD_TEXTURE_LOADER_H

