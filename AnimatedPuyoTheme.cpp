/* FloboPuyo
 * Copyright (C) 2004
 *   Florent Boudet        <flobo@ios-software.com>,
 *   Jean-Christophe Hoelt <jeko@ios-software.com>,
 *   Guillaume Borios      <gyom@ios-software.com>
 *
 * iOS Software <http://www.ios-software.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *
 */

#include <iostream>
#include <sstream>
#include "DataPathManager.h"
#include "AnimatedPuyoTheme.h"

using namespace std;

const char * ThemeManagerImpl::s_themeFolderExtension = ".fptheme";
const char * ThemeManagerImpl::s_key_PuyoFace[NUMBER_OF_PUYOS_IN_SET] = {
    "puyoset.P1.face",
    "puyoset.P2.face",
    "puyoset.P3.face",
    "puyoset.P4.face",
    "puyoset.P5.face",
    "puyoset.Neutral.face"
};
const char * ThemeManagerImpl::s_key_PuyoDisappear[NUMBER_OF_PUYOS_IN_SET] = {
    "puyoset.P1.disappear",
    "puyoset.P2.disappear",
    "puyoset.P3.disappear",
    "puyoset.P4.disappear",
    "puyoset.P5.disappear",
    "puyoset.Neutral.face"
};
const char * ThemeManagerImpl::s_key_PuyoExplosion[NUMBER_OF_PUYOS_IN_SET] = {
    "puyoset.P1.explosion",
    "puyoset.P2.explosion",
    "puyoset.P3.explosion",
    "puyoset.P4.explosion",
    "puyoset.P5.explosion",
    "puyoset.Neutral.face"
};
const char * ThemeManagerImpl::s_key_PuyoEye[NUMBER_OF_PUYOS_IN_SET] = {
    "puyoset.P1.eye",
    "puyoset.P2.eye",
    "puyoset.P3.eye",
    "puyoset.P4.eye",
    "puyoset.P5.eye",
    "puyoset.Neutral.face"
};
const char * ThemeManagerImpl::s_key_PuyoColorOffset[NUMBER_OF_PUYOS_IN_SET] = {
    "puyoset.P1.offset",
    "puyoset.P2.offset",
    "puyoset.P3.offset",
    "puyoset.P4.offset",
    "puyoset.P5.offset",
    "puyoset.Neutral.face"
};

ThemeManagerImpl::ThemeManagerImpl(DataPathManager &dataPathManager)
    : m_dataPathManager(dataPathManager),
      m_defaultLevelThemeName("Basic level")
{
    // List the themes in the various pack folders
    SelfVector<String> themeFolders = dataPathManager.getEntriesAtPath("theme");
    // Load the themes from the list (only those matching the correct extension)
    for (int i = 0 ; i < themeFolders.size() ; i++) {
        if (themeFolders[i].substring(themeFolders[i].size() - 8)
            == s_themeFolderExtension) {
            cout << "Theme to be loaded: " << (const char *)(themeFolders[i]) << endl;
            loadThemePack((const char *)themeFolders[i]);
        }
    }
}

PuyoSetTheme * ThemeManagerImpl::createPuyoSetTheme(const std::string &themeName)
{
    cout << "Creating puyoset theme impl " << themeName << endl;
    std::map<std::string, PuyoSetThemeDescription>::iterator iter
        = m_puyoSetThemeDescriptions.find(themeName);
    if (iter == m_puyoSetThemeDescriptions.end())
        return NULL;
    return new PuyoSetThemeImpl(iter->second);
}

LevelTheme   * ThemeManagerImpl::createLevelTheme(const std::string &themeName)
{
    cout << "Creating level theme impl " << themeName << endl;
    std::map<std::string, LevelThemeDescription>::iterator iter
        = m_levelThemeDescriptions.find(themeName);
    if (iter == m_levelThemeDescriptions.end())
        return NULL;
    if (themeName == m_defaultLevelThemeName)
        return new LevelThemeImpl(iter->second, m_dataPathManager);
    if (m_defaultLevelTheme.get() == NULL) {
        m_defaultLevelTheme.reset(new LevelThemeImpl(m_levelThemeDescriptions[m_defaultLevelThemeName], m_dataPathManager));
    }
    return new LevelThemeImpl(iter->second, m_dataPathManager, m_defaultLevelTheme.get());
}

const std::vector<std::string> & ThemeManagerImpl::getPuyoSetThemeList()
{
    return m_puyoSetThemeList;
}

const std::vector<std::string> & ThemeManagerImpl::getLevelThemeList()
{
    return m_levelThemeList;
}

void ThemeManagerImpl::loadThemePack(const std::string &path)
{
    FilePath themePath(path.c_str());
    String scriptPath = m_dataPathManager.getPath(themePath.combine("Description.gsl"));
	String themeDictionaryPath = FilePath::combine(FilePath::combine("theme",themePath.basename()),"locale");
	m_localeDictionary.reset(new LocalizedDictionary(m_dataPathManager, themeDictionaryPath, "theme"));
    m_themePackLoadingPath = path;

#ifdef DISABLED
    // Verify input file
    struct stat s;
    if (stat((const char *)scriptPath,&s) == -1)
    {
        fprintf(stderr,"Couldn't load theme from %s. Ignoring.\n",(const char *)scriptPath);
        return;
    }
#endif
    GoomSL * gsl = gsl_new();
    if (!gsl) return;
    String libPath = m_dataPathManager.getPath("lib/themelib.gsl");
    char * fbuffer = gsl_init_buffer((const char *)libPath);
    gsl_append_file_to_buffer(scriptPath, &fbuffer);
    gsl_compile(gsl,fbuffer);
    gsl_bind_function(gsl, "end_puyoset",     ThemeManagerImpl::end_puyoset);
    gsl_bind_function(gsl, "end_level",       ThemeManagerImpl::end_level);
    gsl_bind_function(gsl, "end_description", ThemeManagerImpl::end_description);
    GSL_SET_USERDATA_PTR(gsl, this);
    gsl_execute(gsl);
    gsl_free(gsl);
    free(fbuffer);
    m_localeDictionary.reset(NULL);
}

void ThemeManagerImpl::end_puyoset(GoomSL *gsl, GoomHash *global,
                                          GoomHash *local)
{
    ThemeManagerImpl *themeMgr = (ThemeManagerImpl *)GSL_GET_USERDATA_PTR(gsl);
	const char * themeName  = (const char *) GSL_GLOBAL_PTR(gsl, "puyoset.name");
	const char * localizedThemeName = themeMgr->m_localeDictionary->getLocalizedString(themeName,true);
    PuyoSetThemeDescription &newThemeDescription = themeMgr->m_puyoSetThemeDescriptions[themeName];
    newThemeDescription.name = themeName;
    newThemeDescription.localizedName = localizedThemeName;
    newThemeDescription.author = (const char *)(GSL_GLOBAL_PTR(gsl, "author"));
    newThemeDescription.description = (const char *)(GSL_GLOBAL_PTR(gsl, "puyoset.description"));
    newThemeDescription.localizedDescription = themeMgr->m_localeDictionary->getLocalizedString(newThemeDescription.description.c_str(),true);
    newThemeDescription.path = themeMgr->m_themePackLoadingPath;
    for (int i = 0 ; i < NUMBER_OF_PUYOS_IN_SET ; i++) {
        PuyoThemeDescription &currentPuyoThemeDescription =
            newThemeDescription.puyoThemeDescriptions[i];
        currentPuyoThemeDescription.face =      (const char *)GSL_GLOBAL_PTR(gsl, s_key_PuyoFace[i]);
        currentPuyoThemeDescription.disappear = (const char *)GSL_GLOBAL_PTR(gsl, s_key_PuyoDisappear[i]);
        currentPuyoThemeDescription.explosion = (const char *)GSL_GLOBAL_PTR(gsl, s_key_PuyoExplosion[i]);
        currentPuyoThemeDescription.eye =       (const char *)GSL_GLOBAL_PTR(gsl, s_key_PuyoEye[i]);
        currentPuyoThemeDescription.colorOffset = GSL_GLOBAL_FLOAT(gsl, s_key_PuyoColorOffset[i]);
    }
    themeMgr->m_puyoSetThemeList.push_back(themeName);
}

void ThemeManagerImpl::end_level(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    ThemeManagerImpl *themeMgr = (ThemeManagerImpl *)GSL_GET_USERDATA_PTR(gsl);
    const char * themeName  = (const char *) GSL_GLOBAL_PTR(gsl, "level.name");
    const char * localizedThemeName = themeMgr->m_localeDictionary->getLocalizedString(themeName,true);
    LevelThemeDescription &newThemeDescription = themeMgr->m_levelThemeDescriptions[themeName];
    newThemeDescription.name = themeName;
    newThemeDescription.localizedName = localizedThemeName;
    newThemeDescription.author = (const char *)(GSL_GLOBAL_PTR(gsl, "author"));
    newThemeDescription.description = (const char *)(GSL_GLOBAL_PTR(gsl, "level.description"));
    newThemeDescription.localizedDescription = themeMgr->m_localeDictionary->getLocalizedString(newThemeDescription.description.c_str(),true);
    newThemeDescription.path = themeMgr->m_themePackLoadingPath;

    newThemeDescription.lives = (const char *) GSL_GLOBAL_PTR(gsl, "level.lives");
    newThemeDescription.background = (const char *) GSL_GLOBAL_PTR(gsl, "level.background");
    newThemeDescription.grid = (const char *) GSL_GLOBAL_PTR(gsl, "level.grid");
    newThemeDescription.speedMeter = (const char *) GSL_GLOBAL_PTR(gsl, "level.speedmeter");
    newThemeDescription.neutralIndicator = (const char *) GSL_GLOBAL_PTR(gsl, "level.neutralindicator");

    newThemeDescription.gameLostRight2PAnimation = (const char *) GSL_GLOBAL_PTR(gsl, "level.gamelost_right_2p");
    newThemeDescription.gameLostLeft2PAnimation = (const char *) GSL_GLOBAL_PTR(gsl, "level.gamelost_left_2p");
    newThemeDescription.animation = (const char *) GSL_GLOBAL_PTR(gsl, "level.animation_2p");
    newThemeDescription.fgAnimation = (const char *) GSL_GLOBAL_PTR(gsl, "level.foreground_animation");
    newThemeDescription.getReadyAnimation = (const char *) GSL_GLOBAL_PTR(gsl, "level.get_ready_animation");

    newThemeDescription.speedMeterX = GSL_GLOBAL_INT(gsl, "level.speedmeter_display.x");
    newThemeDescription.speedMeterY = GSL_GLOBAL_INT(gsl, "level.speedmeter_display.y");
    newThemeDescription.lifeDisplayX = GSL_GLOBAL_INT(gsl, "level.life_display.x");
    newThemeDescription.lifeDisplayY = GSL_GLOBAL_INT(gsl, "level.life_display.y");

    loadFontDefinition(gsl, "playerNameFont", newThemeDescription.playerNameFont);
    loadFontDefinition(gsl, "scoreFont", newThemeDescription.scoreFont);

    for (int i = 0 ; i < NUMBER_OF_PUYOBANS_IN_LEVEL ; i++)
        loadPuyobanDefinition(gsl, i, newThemeDescription.puyoban[i]);

    themeMgr->m_levelThemeList.push_back(themeName);
}

void ThemeManagerImpl::end_description(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
	// TODO !!!
}

void ThemeManagerImpl::loadPuyobanDefinition(GoomSL *gsl, int playerId, PuyobanThemeDefinition &puyoban)
{
    String variablePrefix = String("level.puyoban_p") + (playerId+1);
    puyoban.displayX = GSL_GLOBAL_INT(gsl, variablePrefix + ".display.x");
    puyoban.displayY = GSL_GLOBAL_INT(gsl, variablePrefix + ".display.y");
    puyoban.nextX = GSL_GLOBAL_INT(gsl, variablePrefix + ".next.x");
    puyoban.nextY = GSL_GLOBAL_INT(gsl, variablePrefix + ".next.y");
    puyoban.neutralDisplayX = GSL_GLOBAL_INT(gsl, variablePrefix + ".neutral_display.x");
    puyoban.neutralDisplayY = GSL_GLOBAL_INT(gsl, variablePrefix + ".neutral_display.y");
    puyoban.nameDisplayX = GSL_GLOBAL_INT(gsl, variablePrefix + ".name_display.x");
    puyoban.nameDisplayY = GSL_GLOBAL_INT(gsl, variablePrefix + ".name_display.y");
    puyoban.scoreDisplayX = GSL_GLOBAL_INT(gsl, variablePrefix + ".score_display.x");
    puyoban.scoreDisplayY = GSL_GLOBAL_INT(gsl, variablePrefix + ".score_display.y");
    puyoban.shouldDisplayNext = GSL_GLOBAL_INT(gsl, variablePrefix + ".should_display_next");
    puyoban.shouldDisplayShadow = GSL_GLOBAL_INT(gsl, variablePrefix + ".should_display_shadows");
    puyoban.shouldDisplayEyes = GSL_GLOBAL_INT(gsl, variablePrefix + ".should_display_eyes");
    puyoban.scale = GSL_GLOBAL_FLOAT(gsl, variablePrefix + ".scale");
}

void ThemeManagerImpl::loadFontDefinition(GoomSL *gsl, const char *fontName, FontDefinition &font)
{
    ostringstream fontPathVar, fontSizeVar, fontFxVar;
    fontPathVar << "level." << fontName << ".path";
    fontSizeVar << "level." << fontName << ".size";
    fontFxVar << "level." << fontName << ".fx";
    font.fontPath = (const char *) GSL_GLOBAL_PTR(gsl, fontPathVar.str().c_str());
    font.fontSize     = GSL_GLOBAL_INT(gsl, fontSizeVar.str().c_str());
    font.fontFx = (IosFontFx)(GSL_GLOBAL_INT(gsl, fontFxVar.str().c_str()));
}

PuyoSetThemeImpl::PuyoSetThemeImpl(const PuyoSetThemeDescription &desc)
    : m_desc(desc)
{
    for (int i = 0 ; i < NUMBER_OF_PUYOS_IN_SET-1 ; i++) {
        m_puyoThemes[i].reset(new PuyoThemeImpl(desc.puyoThemeDescriptions[i], desc.path));
    }
    // Neutral puyo is a special case
    m_puyoThemes[NUMBER_OF_PUYOS_IN_SET-1].reset(new NeutralPuyoThemeImpl(desc.puyoThemeDescriptions[NUMBER_OF_PUYOS_IN_SET-1], desc.path));
}

const std::string & PuyoSetThemeImpl::getName() const
{
    return m_desc.name;
}

const std::string & PuyoSetThemeImpl::getLocalizedName() const
{
    return m_desc.localizedName;
}

const std::string & PuyoSetThemeImpl::getAuthor() const
{
    return m_desc.author;
}

const std::string & PuyoSetThemeImpl::getComments() const
{
    return m_desc.description;
}

const PuyoTheme & PuyoSetThemeImpl::getPuyoTheme(PuyoState state) const
{
    switch (state) {
    case PUYO_FALLINGBLUE:
    case PUYO_BLUE:
        return *m_puyoThemes[0];
    case PUYO_FALLINGRED:
    case PUYO_RED:
        return *m_puyoThemes[1];
    case PUYO_FALLINGGREEN:
    case PUYO_GREEN:
        return *m_puyoThemes[2];
    case PUYO_FALLINGVIOLET:
    case PUYO_VIOLET:
        return *m_puyoThemes[3];
    case PUYO_FALLINGYELLOW:
    case PUYO_YELLOW:
        return *m_puyoThemes[4];
    default:
        return *m_puyoThemes[5];
    }
}


BasePuyoThemeImpl::BasePuyoThemeImpl(const PuyoThemeDescription &desc, const std::string &path)
    : m_desc(desc), m_path(path)
{}

PuyoThemeImpl::PuyoThemeImpl(const PuyoThemeDescription &desc, const std::string &path)
    : BasePuyoThemeImpl(desc, path)
{
    // Initializing the arrays of pointers
    for (int i = 0 ; i < NUMBER_OF_PUYO_FACES ; i++)
        for (int j = 0 ; j < MAX_COMPRESSED ; j++)
            m_faces[i][j] = NULL;
    for (int i = 0 ; i < NUMBER_OF_PUYO_EYES ; i++)
        for (int j = 0 ; j < MAX_COMPRESSED ; j++)
            m_eyes[i][j] = NULL;
    for (int i = 0 ; i < NUMBER_OF_PUYO_CIRCLES ; i++)
        for (int j = 0 ; j < MAX_COMPRESSED ; j++)
            m_circles[i][j] = NULL;
    for (int j = 0 ; j < MAX_COMPRESSED ; j++)
        m_shadows[j] = NULL;
}

IosSurface *PuyoThemeImpl::getPuyoSurfaceForValence(int valence, int compression) const
{
    IosSurface * &ref = m_faces[valence][compression];
    if (ref == NULL) {
        IosSurface * &uncompressed = m_faces[valence][0];
        // Do we need to load the uncompressed image?
        if (uncompressed == NULL) {
            ostringstream osstream;
            osstream << m_path << "/" << m_desc.face << "-puyo-"
                     << ((valence&8)>>3)
                     << ((valence&4)>>2)
                     << ((valence&2)>>1)
                     << (valence&1) << ".png";
            if (m_desc.colorOffset == 0) {
                m_baseFaces[valence] = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), IMAGE_READ);
                uncompressed = m_baseFaces[valence];
            }
            else {
                IosSurfaceRef baseFace = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), IMAGE_READ);
                uncompressed = baseFace.get()->shiftHue(m_desc.colorOffset);
            }
        }
        // Create the compressed image
        if (compression != 0)
            ref = uncompressed->resizeAlpha(uncompressed->w,
                                            uncompressed->h - compression);
    }
    return ref;
}

IosSurface *PuyoThemeImpl::getEyeSurfaceForIndex(int index, int compression) const
{
    IosSurface * &ref = m_eyes[index][compression];
    if (ref == NULL) {
        IosSurface * &uncompressed = m_eyes[index][0];
        // Do we need to load the uncompressed image?
        if (uncompressed == NULL) {
            ostringstream osstream;
            osstream << m_path << "/" << m_desc.eye << "-puyo-eye-" << index << ".png";
            m_baseEyes[index] = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), IMAGE_READ);
            uncompressed = m_baseEyes[index];
        }
        // Create the compressed image
        if (compression != 0)
            ref = uncompressed->resizeAlpha(uncompressed->w,
                                            uncompressed->h - compression);
    }
    return ref;
}

IosSurface *PuyoThemeImpl::getCircleSurfaceForIndex(int index, int compression) const
{
    IosSurface * &ref = m_circles[index][compression];
    if (ref == NULL) {
        IosSurface * &uncompressed = m_circles[index][0];
        // Do we need to load the uncompressed image?
        if (uncompressed == NULL) {
            // Do we need to load the reference image?
            if (m_baseCircle.get() == NULL) {
                ostringstream osstream;
                osstream << m_path << "/" << m_desc.face << "-puyo-border.png";
                m_baseCircle = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), IMAGE_READ);
            }
            uncompressed = m_baseCircle.get()->setValue(sin(3.14f/2.0f+index*3.14f/64.0f)*0.6f+0.2f);
        }
        // Create the compressed image
        if (compression != 0)
            ref = uncompressed->resizeAlpha(uncompressed->w,
                                            uncompressed->h - compression);
    }
    return ref;
}

IosSurface *PuyoThemeImpl::getShadowSurface(int compression) const
{
    IosSurface * &ref = m_shadows[compression];
    if (ref == NULL) {
        IosSurface * &uncompressed = m_shadows[0];
        // Do we need to load the uncompressed image?
        if (uncompressed == NULL) {
            ostringstream osstream;
            osstream << m_path << "/" << m_desc.face << "-puyo-shadow.png";
            m_baseShadow = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), IMAGE_READ);
            uncompressed = m_baseShadow;
        }
        // Create the compressed image
        if (compression != 0)
            ref = uncompressed->resizeAlpha(uncompressed->w,
                                            uncompressed->h - compression);
    }
    return ref;
}

IosSurface *PuyoThemeImpl::getShrinkingSurfaceForIndex(int index) const
{
    IosSurfaceRef &ref = m_shrinking[index];
    if (ref == NULL) {
        ostringstream osstream;
        osstream << m_path << "/" << m_desc.face << "-puyo-disappear-" << index << ".png";
        ref = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), IMAGE_READ);
    }
    return ref;
}

IosSurface *PuyoThemeImpl::getExplodingSurfaceForIndex(int index) const
{
    IosSurfaceRef &ref = m_explosion[index];
    if (ref == NULL) {
        ostringstream osstream;
        osstream << m_path << "/" << m_desc.face << "-puyo-explosion-" << index << ".png";
        ref = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), IMAGE_READ);
    }
    return ref;
}

NeutralPuyoThemeImpl::NeutralPuyoThemeImpl(const PuyoThemeDescription &desc, const std::string &path)
    : BasePuyoThemeImpl(desc, path)
{
    // Initializing the arrays of pointers
    for (int j = 0 ; j < MAX_COMPRESSED ; j++)
        m_faces[j] = NULL;
}

IosSurface *NeutralPuyoThemeImpl::getPuyoSurfaceForValence(int valence, int compression) const
{
    IosSurface * &ref = m_faces[compression];
    if (ref == NULL) {
        IosSurface * &uncompressed = m_faces[0];
        // Do we need to load the uncompressed image?
        if (uncompressed == NULL) {
            ostringstream osstream;
            osstream << m_path << "/" << m_desc.face << ".png";
            m_baseFace = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), IMAGE_READ);
            uncompressed = m_baseFace;
        }
        // Create the compressed image
        if (compression != 0)
            ref = uncompressed->resizeAlpha(uncompressed->w,
                                            uncompressed->h - compression);
    }
    return ref;
}

IosSurface *NeutralPuyoThemeImpl::getEyeSurfaceForIndex(int index, int compression) const
{
    return NULL;
}

IosSurface *NeutralPuyoThemeImpl::getCircleSurfaceForIndex(int index, int compression) const
{
    return NULL;
}

IosSurface *NeutralPuyoThemeImpl::getShadowSurface(int compression) const
{
}

IosSurface *NeutralPuyoThemeImpl::getShrinkingSurfaceForIndex(int index) const
{
    IosSurfaceRef &ref = m_shrinking[index];
    if (ref == NULL) {
        ostringstream osstream;
        osstream << m_path << "/" << m_desc.face << "-neutral-" << index << ".png";
        ref = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), IMAGE_READ);
    }
    return ref;
}

IosSurface *NeutralPuyoThemeImpl::getExplodingSurfaceForIndex(int index) const
{
    return getShrinkingSurfaceForIndex(index);
}






LevelThemeImpl::LevelThemeImpl(const LevelThemeDescription &desc,
                               DataPathManager &dataPathManager,
                               LevelThemeImpl *defaultTheme)
    : m_desc(desc), m_path(desc.path), m_dataPathManager(dataPathManager), m_defaultTheme(defaultTheme)
{
}

const std::string & LevelThemeImpl::getName() const
{
    return m_desc.name;
}

const std::string & LevelThemeImpl::getLocalizedName() const
{
    return m_desc.localizedName;
}

const std::string & LevelThemeImpl::getAuthor() const
{
    return m_desc.author;
}

const std::string & LevelThemeImpl::getComments() const
{
    return m_desc.description;
}

IosSurface * LevelThemeImpl::getLifeForIndex(int index) const
{
    IosSurfaceRef &result = m_lifes[index];
    if (result.empty()) {
        ostringstream osstream;
        osstream << m_path << "/" << m_desc.lives << "-lives-" << index << ".png";
        result = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str());
        if ((result.empty()) && (m_defaultTheme != NULL)) {
            return m_defaultTheme->getLifeForIndex(index);
        }
    }
    return result;
}

IosSurfaceRef &LevelThemeImpl::getResource(IosSurfaceRef &ref, const std::string &resName, const char *resSuffix) const
{
    if (ref.empty()) {
        ostringstream osstream;
        osstream << m_path << "/" << resName << resSuffix;
        ref = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str());
        if ((ref.empty()) && (m_defaultTheme != NULL)) {
            ref = m_defaultTheme->getResource(ref, resName, resSuffix);
        }
    }
    return ref;
}

IosSurface * LevelThemeImpl::getBackground() const
{
    IosSurfaceRef &result = getResource(m_background, m_desc.background, "");
    return result;
}

IosSurface * LevelThemeImpl::getGrid() const
{
    if (m_desc.grid == "")
        return NULL;
    IosSurfaceRef &result = getResource(m_grid, m_desc.grid, "-background-grid.png");
    return result;
}

IosSurface * LevelThemeImpl::getSpeedMeter(bool front) const
{
    if (front)
        return getResource(m_speedMeterFront, m_desc.speedMeter, "-background-meter-above.png");
    return getResource(m_speedMeterBack, m_desc.speedMeter, "-background-meter-below.png");
}

IosSurface * LevelThemeImpl::getNeutralIndicator() const
{
    return getResource(m_neutralIndicator, m_desc.neutralIndicator, "-small.png");
}

IosSurface * LevelThemeImpl::getBigNeutralIndicator() const
{
    return getResource(m_bigNeutralIndicator, m_desc.neutralIndicator, ".png");
}

IosSurface * LevelThemeImpl::getGiantNeutralIndicator() const
{
    return getResource(m_giantNeutralIndicator, m_desc.neutralIndicator, "-giant.png");
}

IosFont *LevelThemeImpl::getPlayerNameFont() const
{
    if (m_playerNameFont.empty()) {
        string fontPath = m_desc.playerNameFont.fontPath;
        if (fontPath == "__FONT__") {
            fontPath = theCommander->getLocalizedFontName();
        }
        m_playerNameFont = theCommander->getFont(fontPath.c_str(), m_desc.playerNameFont.fontSize, m_desc.playerNameFont.fontFx);
    }
    return m_playerNameFont;
}

IosFont *LevelThemeImpl::getScoreFont() const
{
    if (m_scoreFont.empty()) {
        string fontPath = m_desc.scoreFont.fontPath;
        if (fontPath == "__FONT__") {
            fontPath = theCommander->getLocalizedFontName();
        }
        m_scoreFont = theCommander->getFont(fontPath.c_str(), m_desc.scoreFont.fontSize, m_desc.scoreFont.fontFx);
    }
    return m_scoreFont;
}

int LevelThemeImpl::getSpeedMeterX() const
{ return m_desc.speedMeterX; }

int LevelThemeImpl::getSpeedMeterY() const
{ return m_desc.speedMeterY; }

int LevelThemeImpl::getLifeDisplayX() const
{ return m_desc.lifeDisplayX; }

int LevelThemeImpl::getLifeDisplayY() const
{ return m_desc.lifeDisplayY; }

int LevelThemeImpl::getPuyobanX(int playerId) const
{ return m_desc.puyoban[playerId].displayX; }

int LevelThemeImpl::getPuyobanY(int playerId) const
{ return m_desc.puyoban[playerId].displayY; }

int LevelThemeImpl::getNextPuyosX(int playerId) const
{ return m_desc.puyoban[playerId].nextX; }

int LevelThemeImpl::getNextPuyosY(int playerId) const
{ return m_desc.puyoban[playerId].nextY; }

int LevelThemeImpl::getNeutralDisplayX(int playerId) const
{ return m_desc.puyoban[playerId].neutralDisplayX; }

int LevelThemeImpl::getNeutralDisplayY(int playerId) const
{ return m_desc.puyoban[playerId].neutralDisplayY; }

int LevelThemeImpl::getNameDisplayX(int playerId) const
{ return m_desc.puyoban[playerId].nameDisplayX; }

int LevelThemeImpl::getNameDisplayY(int playerId) const
{ return m_desc.puyoban[playerId].nameDisplayY; }

int LevelThemeImpl::getScoreDisplayX(int playerId) const
{ return m_desc.puyoban[playerId].scoreDisplayX; }

int LevelThemeImpl::getScoreDisplayY(int playerId) const
{ return m_desc.puyoban[playerId].scoreDisplayY; }

float LevelThemeImpl::getPuyobanScale(int playerId) const
{ return m_desc.puyoban[playerId].scale; }

bool LevelThemeImpl::getShouldDisplayNext(int playerId) const
{ return m_desc.puyoban[playerId].shouldDisplayNext; }

bool LevelThemeImpl::getShouldDisplayShadows(int playerId) const
{ return m_desc.puyoban[playerId].shouldDisplayShadow; }

bool LevelThemeImpl::getShouldDisplayEyes(int playerId) const
{ return m_desc.puyoban[playerId].shouldDisplayEyes; }

bool LevelThemeImpl::getOpponentIsBehind() const
{ return false; }

const std::string LevelThemeImpl::getGameLostLeftAnimation2P() const
{
    std::string animPath = m_path + "/" + m_desc.gameLostLeft2PAnimation;
    return std::string(m_dataPathManager.getPath(animPath.c_str()));
}

const std::string LevelThemeImpl::getGameLostRightAnimation2P() const
{
    std::string animPath = m_path + "/" + m_desc.gameLostRight2PAnimation;
    return std::string(m_dataPathManager.getPath(animPath.c_str()));
}

const std::string LevelThemeImpl::getCentralAnimation2P() const
{
    std::string animPath = m_path + "/" + m_desc.animation;
    return std::string(m_dataPathManager.getPath(animPath.c_str()));
}

const std::string LevelThemeImpl::getForegroundAnimation() const
{
    std::string animPath = m_path + "/" + m_desc.fgAnimation;
    return std::string(m_dataPathManager.getPath(animPath.c_str()));
}

const std::string LevelThemeImpl::getReadyAnimation2P() const
{
    return m_desc.getReadyAnimation;
    //std::string animPath = m_path + "/" + m_desc.getReadyAnimation;
    //return std::string(m_dataPathManager.getPath(animPath.c_str()));
}

const std::string LevelThemeImpl::getThemeRootPath() const
{ return m_path; }



