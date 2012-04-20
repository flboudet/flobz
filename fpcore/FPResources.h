#ifndef _FPRESOURCES_H_
#define _FPRESOURCES_H_

#include "ResourceManager.h"
#include "DataPathManager.h"
#include "Theme.h"

/**
 * Resource key for IosSurface management.
 * Makes an unique key for an ImageType, path, Ability association
 */
class IosSurfaceResourceKey {
public:
    IosSurfaceResourceKey(ImageType type, const std::string &path, ImageSpecialAbility specialAbility)
        : path(path), type(type), specialAbility(specialAbility) {}
    bool operator ==(const IosSurfaceResourceKey k) const {
        return  ((this->path == k.path) &&
                 (this->type == k.type) &&
                 (this->specialAbility == k.specialAbility));
    }
    bool operator < (const IosSurfaceResourceKey k) const {
        if (this->path == k.path) {
            if (this->type == k.type)
                return this->specialAbility < k.specialAbility;
            return this->type < k.type;
        }
        return (this->path < k.path);
    }
    std::string path;
    ImageType type;
    ImageSpecialAbility specialAbility;
};

/**
 * Resource key for IosFont management.
 * Makes an unique key for an ImageType, path, Ability association
 */
class IosFontResourceKey {
public:
    IosFontResourceKey(const std::string &path, int size, IosFontFx fx = Font_STD)
    : path(path), size(size), fx(fx) {}
    bool operator ==(const IosFontResourceKey k) const {
        return  ((this->path == k.path) &&
                 (this->size == k.size) &&
                 (this->fx == k.fx));
    }
    bool operator < (const IosFontResourceKey k) const {
        if (this->path == k.path) {
            if (this->size == k.size)
                return this->fx < k.fx;
            return this->size < k.size;
        }
        return (this->path < k.path);
    }
    std::string path;
    int size;
    IosFontFx fx;
};

/**
 * Factory for IosSurface resources
 */
class IosSurfaceFactory : public ResourceFactory<IosSurface, IosSurfaceResourceKey>
{
public:
    IosSurfaceFactory(DataPathManager &dataPathManager)
        : m_dataPathManager(dataPathManager) {}
    virtual IosSurface *create(const IosSurfaceResourceKey &resourceKey);
    virtual void destroy(IosSurface *res);
private:
    DataPathManager &m_dataPathManager;
};

/**
 * Factory for IosFont resources
 */
class IosFontFactory : public ResourceFactory<IosFont, IosFontResourceKey>
{
public:
    IosFontFactory(DataPathManager &dataPathManager)
    : m_dataPathManager(dataPathManager) {}
    virtual IosFont *create(const IosFontResourceKey &resourceKey);
    virtual void destroy(IosFont *res);
private:
    DataPathManager &m_dataPathManager;
};

/**
 * Factory for Sound resources
 */
class SoundFactory : public ResourceFactory<audio_manager::Sound>
{
public:
    SoundFactory(DataPathManager &dataPathManager)
        : m_dataPathManager(dataPathManager) {}
    virtual audio_manager::Sound *create(const std::string &path);
    virtual void destroy(audio_manager::Sound *res);
private:
    DataPathManager &m_dataPathManager;
};

/**
 * Factory for Music resources
 */
class MusicFactory : public ResourceFactory<audio_manager::Music>
{
public:
    MusicFactory(DataPathManager &dataPathManager)
        : m_dataPathManager(dataPathManager) {}
    virtual audio_manager::Music *create(const std::string &path);
    virtual void destroy(audio_manager::Music *res);
private:
    DataPathManager &m_dataPathManager;
};

/**
 * Factory for FloboSetTheme resources
 */
class FloboSetThemeFactory : public ResourceFactory<FloboSetTheme>
{
public:
    FloboSetThemeFactory(ThemeManager &themeManager);
    virtual FloboSetTheme *create(const std::string &name);
    virtual void destroy(FloboSetTheme *res);
private:
    ThemeManager &m_themeManager;
};

/**
 * Factory for LevelTheme resources
 */
class LevelThemeFactory : public ResourceFactory<LevelTheme>
{
public:
    LevelThemeFactory(ThemeManager &themeManager);
    virtual LevelTheme *create(const std::string &name);
    virtual void destroy(LevelTheme *res);
private:
    ThemeManager &m_themeManager;
};

// IosSurface resources
typedef ResourceReference<IosSurface> IosSurfaceRef;
typedef ResourceManager<IosSurface, IosSurfaceResourceKey> IosSurfaceResourceManager;
// Font resources
typedef ResourceReference<IosFont> IosFontRef;
typedef ResourceManager<IosFont, IosFontResourceKey> IosFontResourceManager;
// Sound resources
typedef ResourceReference<audio_manager::Sound> SoundRef;
typedef ResourceManager<audio_manager::Sound> SoundResourceManager;
// Music resources
typedef ResourceReference<audio_manager::Music> MusicRef;
typedef ResourceManager<audio_manager::Music> MusicResourceManager;
// Theme resources
typedef ResourceReference<FloboSetTheme> FloboSetThemeRef;
typedef ResourceManager<FloboSetTheme> FloboSetThemeResourceManager;
typedef ResourceReference<LevelTheme> LevelThemeRef;
typedef ResourceManager<LevelTheme> LevelThemeResourceManager;

#endif // _FPRESOURCES_H_

