#include "gameui.h"
#include "FPResources.h"

using namespace gameui;

#ifdef PRODUCE_CACHE_FILE
FILE *cacheOutputGsl;
#endif

IosSurface *IosSurfaceFactory::create(const IosSurfaceResourceKey &resourceKey)
{
    try {
#ifdef PRODUCE_CACHE_FILE
        fprintf(cacheOutputGsl, "  [cache_picture: path=\"%s\" mode=%d]\n",
                resourceKey.path.c_str(), resourceKey.type);
#endif
        ImageLibrary &iimLib = GameUIDefaults::GAME_LOOP->getDrawContext()->getImageLibrary();
        IosSurface *newSurface = iimLib.loadImage(resourceKey.type, resourceKey.path.c_str(), resourceKey.specialAbility);
        if (newSurface != NULL)
            newSurface->enableExceptionOnDeletion(true);
        return newSurface;
    }
    catch (Exception e) {
        return NULL;
    }
}

void IosSurfaceFactory::destroy(IosSurface *res)
{
    res->enableExceptionOnDeletion(false);
    delete res;
}

IosFont *IosFontFactory::create(const IosFontResourceKey &resourceKey)
{
    try {
#ifdef PRODUCE_CACHE_FILE
        fprintf(cacheOutputGsl, "  [cache_picture: path=\"%s\" mode=%d]\n",
                resourceKey.path.c_str(), resourceKey.type);
#endif
        ImageLibrary &iimLib = GameUIDefaults::GAME_LOOP->getDrawContext()->getImageLibrary();
        IosFont *newFont = iimLib.createFont(resourceKey.path.c_str(), resourceKey.size);
        return newFont;
    }
    catch (Exception e) {
        return NULL;
    }
}

void IosFontFactory::destroy(IosFont *res)
{
    delete res;
}

audio_manager::Sound * SoundFactory::create(const std::string &path)
{
    try {
#ifdef PRODUCE_CACHE_FILE
        fprintf(cacheOutputGsl, "  [cache_sound: path=\"%s\"]\n", path.c_str());
#endif
        if (m_dataPathManager.hasFile(path.c_str())) {
            String fullPath = m_dataPathManager.getPath(path.c_str());
            audio_manager::Sound *newSound = GameUIDefaults::GAME_LOOP->getAudioManager()->loadSound(fullPath);
            return newSound;
        }
    }
    catch (Exception e) {
    }
    return NULL;
}

void SoundFactory::destroy(audio_manager::Sound *res)
{
    delete res;
}

audio_manager::Music * MusicFactory::create(const std::string &path)
{
    try {
#ifdef PRODUCE_CACHE_FILE
        fprintf(cacheOutputGsl, "  [cache_music: path=\"%s\"]\n", path.c_str());
#endif
        if (m_dataPathManager.hasFile(path.c_str())) {
            String fullPath = m_dataPathManager.getPath(path.c_str());
            audio_manager::Music *newMusic = GameUIDefaults::GAME_LOOP->getAudioManager()->loadMusic(fullPath);
            return newMusic;
        }
    }
    catch (Exception e) {
    }
    return NULL;
}

void MusicFactory::destroy(audio_manager::Music *res)
{
    delete res;
}

FloboSetThemeFactory::FloboSetThemeFactory(ThemeManager &themeManager)
    : m_themeManager(themeManager)
{
}

FloboSetTheme *FloboSetThemeFactory::create(const std::string &name)
{
    return m_themeManager.createFloboSetTheme(name);
}

void FloboSetThemeFactory::destroy(FloboSetTheme *res)
{
    delete res;
}

LevelThemeFactory::LevelThemeFactory(ThemeManager &themeManager)
    : m_themeManager(themeManager)
{
}

LevelTheme *LevelThemeFactory::create(const std::string &name)
{
    return m_themeManager.createLevelTheme(name);
}

void LevelThemeFactory::destroy(LevelTheme *res)
{
    delete res;
}
