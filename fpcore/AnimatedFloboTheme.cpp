/* FloboPop
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
#include "AnimatedFloboTheme.h"
#include "GSLFileAccessWrapper.h"
#include "CompositeDrawContext.h"
#include "PackageDescription.h"

using namespace std;

const char * ThemeManagerImpl::s_themeFolderExtension = ".fptheme";
const char * ThemeManagerImpl::s_key_FloboFace[NUMBER_OF_FLOBOS_IN_SET] = {
    "floboset.P1.face",
    "floboset.P2.face",
    "floboset.P3.face",
    "floboset.P4.face",
    "floboset.P5.face",
    "floboset.Neutral.face"
};
const char * ThemeManagerImpl::s_key_FloboDisappear[NUMBER_OF_FLOBOS_IN_SET] = {
    "floboset.P1.disappear",
    "floboset.P2.disappear",
    "floboset.P3.disappear",
    "floboset.P4.disappear",
    "floboset.P5.disappear",
    "floboset.Neutral.face"
};
const char * ThemeManagerImpl::s_key_FloboExplosion[NUMBER_OF_FLOBOS_IN_SET] = {
    "floboset.P1.explosion",
    "floboset.P2.explosion",
    "floboset.P3.explosion",
    "floboset.P4.explosion",
    "floboset.P5.explosion",
    "floboset.Neutral.face"
};
const char * ThemeManagerImpl::s_key_FloboEye[NUMBER_OF_FLOBOS_IN_SET] = {
    "floboset.P1.eye",
    "floboset.P2.eye",
    "floboset.P3.eye",
    "floboset.P4.eye",
    "floboset.P5.eye",
    "floboset.Neutral.face"
};
const char * ThemeManagerImpl::s_key_FloboColorOffset[NUMBER_OF_FLOBOS_IN_SET] = {
    "floboset.P1.offset",
    "floboset.P2.offset",
    "floboset.P3.offset",
    "floboset.P4.offset",
    "floboset.P5.offset",
    "floboset.Neutral.face"
};

ThemeManagerImpl::ThemeManagerImpl(DataPathManager &dataPathManager)
    : m_dataPathManager(dataPathManager),
      m_defaultFloboSetThemeName("Classic"),
      m_defaultLevelThemeName("Basic level")
{
    // List the themes in the various pack folders
    SelfVector<String> themeFolders = dataPathManager.getEntriesAtPath("theme");
    // Load the themes from the list (only those matching the correct extension)
    for (int i = 0 ; i < themeFolders.size() ; i++) {
        if (themeFolders[i].substring(themeFolders[i].size() - 8)
            == s_themeFolderExtension) {
            //cout << "Theme to be loaded: " << (const char *)(themeFolders[i]) << endl;
            loadThemePack((const char *)themeFolders[i]);
        }
    }
}

FloboSetTheme * ThemeManagerImpl::createFloboSetTheme(const std::string &themeName)
{
    //cout << "Creating floboset theme impl " << themeName << endl;
    std::map<std::string, FloboSetThemeDescription>::iterator iter
        = m_floboSetThemeDescriptions.find(themeName);
    if (iter == m_floboSetThemeDescriptions.end())
        return NULL;
    if (themeName == m_defaultFloboSetThemeName)
        return new FloboSetThemeImpl(iter->second);
    if (m_defaultFloboSetTheme.get() == NULL) {
        m_defaultFloboSetTheme.reset(new FloboSetThemeImpl(m_floboSetThemeDescriptions[m_defaultFloboSetThemeName]));
    }
    return new FloboSetThemeImpl(iter->second, m_defaultFloboSetTheme.get());
}

LevelTheme   * ThemeManagerImpl::createLevelTheme(const std::string &themeName)
{
    //cout << "Creating level theme impl " << themeName << endl;
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

const std::vector<std::string> & ThemeManagerImpl::getFloboSetThemeList()
{
    return m_floboSetThemeList;
}

const std::vector<std::string> & ThemeManagerImpl::getLevelThemeList()
{
    return m_levelThemeList;
}

void ThemeManagerImpl::loadThemePack(const std::string &path)
{
    FilePath themePath(path.c_str());
	String themeDictionaryPath = FilePath::combine(FilePath::combine("theme",themePath.basename()),"locale");
	m_localeDictionary.reset(new LocalizedDictionary(m_dataPathManager, themeDictionaryPath, "theme"));
    m_themePackLoadingPath = path;

    GoomSL * gsl = gsl_new();
    if (!gsl) return;
    //String libPath = m_dataPathManager.getPath("lib/themelib.gsl");
    //String packageLibPath = m_dataPathManager.getPath("lib/packagelib.gsl");
    GSLFA_setupWrapper(gsl, &m_dataPathManager);
    gsl_push_file(gsl, "/lib/themelib.gsl");
    gsl_push_file(gsl, "/lib/packagelib.gsl");
    gsl_push_file(gsl, themePath.combine("Description.gsl"));
    gsl_compile(gsl);
    gsl_bind_function(gsl, "end_floboset",     ThemeManagerImpl::end_floboset);
    gsl_bind_function(gsl, "end_level",       ThemeManagerImpl::end_level);
    gsl_bind_function(gsl, "end_description", ThemeManagerImpl::end_description);
    // ugly bit
    DrawContext *dc = GameUIDefaults::GAME_LOOP->getDrawContext();
    CompositeDrawContext *cDC = dynamic_cast<CompositeDrawContext *>(dc);
    if (cDC != NULL)
        PackageDescription packageDesc(gsl, *cDC);
    // end of ugly bit
    GSL_SET_USERDATA_PTR(gsl, this);
    gsl_execute(gsl);
    gsl_free(gsl);
    m_localeDictionary.reset(NULL);
}

void ThemeManagerImpl::end_floboset(GoomSL *gsl, GoomHash *global,
                                          GoomHash *local)
{
    ThemeManagerImpl *themeMgr = (ThemeManagerImpl *)GSL_GET_USERDATA_PTR(gsl);
	const char * themeName  = (const char *) GSL_GLOBAL_PTR(gsl, "floboset.name");
	const char * localizedThemeName = themeMgr->m_localeDictionary->getLocalizedString(themeName,true);
    FloboSetThemeDescription &newThemeDescription = themeMgr->m_floboSetThemeDescriptions[themeName];
    newThemeDescription.name = themeName;
    newThemeDescription.localizedName = localizedThemeName;
    newThemeDescription.author = (const char *)(GSL_GLOBAL_PTR(gsl, "author"));
    newThemeDescription.description = (const char *)(GSL_GLOBAL_PTR(gsl, "floboset.description"));
    newThemeDescription.localizedDescription = themeMgr->m_localeDictionary->getLocalizedString(newThemeDescription.description.c_str(),true);
    newThemeDescription.path = themeMgr->m_themePackLoadingPath;
    for (int i = 0 ; i < NUMBER_OF_FLOBOS_IN_SET ; i++) {
        FloboThemeDescription &currentFloboThemeDescription =
            newThemeDescription.floboThemeDescriptions[i];
        currentFloboThemeDescription.face =      (const char *)GSL_GLOBAL_PTR(gsl, s_key_FloboFace[i]);
        currentFloboThemeDescription.disappear = (const char *)GSL_GLOBAL_PTR(gsl, s_key_FloboDisappear[i]);
        currentFloboThemeDescription.explosion = (const char *)GSL_GLOBAL_PTR(gsl, s_key_FloboExplosion[i]);
        currentFloboThemeDescription.eye =       (const char *)GSL_GLOBAL_PTR(gsl, s_key_FloboEye[i]);
        currentFloboThemeDescription.colorOffset = GSL_GLOBAL_FLOAT(gsl, s_key_FloboColorOffset[i]);
    }
    themeMgr->m_floboSetThemeList.push_back(themeName);
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

    newThemeDescription.opponentIsBehind = (GSL_GLOBAL_INT(gsl, "level.opponent_is_behind") == 0 ? false : true);

    newThemeDescription.statsHeight = GSL_GLOBAL_INT(gsl, "level.stats.height");
    newThemeDescription.statsLegendWidth = GSL_GLOBAL_INT(gsl, "level.stats.legend_width");
    newThemeDescription.statsComboLineValueWidth = GSL_GLOBAL_INT(gsl, "level.stats.combo_line_value_width");
    newThemeDescription.statsLeftBackgroundOffsetX = GSL_GLOBAL_INT(gsl, "level.stats.left_background_offset.x");
    newThemeDescription.statsLeftBackgroundOffsetY = GSL_GLOBAL_INT(gsl, "level.stats.left_background_offset.y");
    newThemeDescription.statsRightBackgroundOffsetX = GSL_GLOBAL_INT(gsl, "level.stats.right_background_offset.x");
    newThemeDescription.statsRightBackgroundOffsetY = GSL_GLOBAL_INT(gsl, "level.stats.right_background_offset.y");

    loadFontDefinition(gsl, "playerNameFont", newThemeDescription.playerNameFont);
    loadFontDefinition(gsl, "scoreFont", newThemeDescription.scoreFont);

    for (int i = 0 ; i < NUMBER_OF_FLOBOBANS_IN_LEVEL ; i++)
        loadFlobobanDefinition(gsl, i, newThemeDescription.floboban[i]);

    themeMgr->m_levelThemeList.push_back(themeName);
}

void ThemeManagerImpl::end_description(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
	// TODO !!!
}

void ThemeManagerImpl::loadFlobobanDefinition(GoomSL *gsl, int playerId, FlobobanThemeDefinition &floboban)
{
    String variablePrefix = String("level.floboban_p") + (playerId+1);
    floboban.displayX = GSL_GLOBAL_INT(gsl, variablePrefix + ".display.x");
    floboban.displayY = GSL_GLOBAL_INT(gsl, variablePrefix + ".display.y");
    floboban.nextX = GSL_GLOBAL_INT(gsl, variablePrefix + ".next.x");
    floboban.nextY = GSL_GLOBAL_INT(gsl, variablePrefix + ".next.y");
    floboban.neutralDisplayX = GSL_GLOBAL_INT(gsl, variablePrefix + ".neutral_display.x");
    floboban.neutralDisplayY = GSL_GLOBAL_INT(gsl, variablePrefix + ".neutral_display.y");
    floboban.nameDisplayX = GSL_GLOBAL_INT(gsl, variablePrefix + ".name_display.x");
    floboban.nameDisplayY = GSL_GLOBAL_INT(gsl, variablePrefix + ".name_display.y");
    floboban.scoreDisplayX = GSL_GLOBAL_INT(gsl, variablePrefix + ".score_display.x");
    floboban.scoreDisplayY = GSL_GLOBAL_INT(gsl, variablePrefix + ".score_display.y");
    floboban.shouldDisplayNext = GSL_GLOBAL_INT(gsl, variablePrefix + ".should_display_next");
    floboban.shouldDisplayShadow = GSL_GLOBAL_INT(gsl, variablePrefix + ".should_display_shadows");
    floboban.shouldDisplayEyes = GSL_GLOBAL_INT(gsl, variablePrefix + ".should_display_eyes");
    floboban.scale = GSL_GLOBAL_FLOAT(gsl, variablePrefix + ".scale");
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

FloboSetThemeImpl::FloboSetThemeImpl(const FloboSetThemeDescription &desc,
                                   FloboSetTheme *defaultTheme)
    : m_desc(desc), m_defaultTheme(defaultTheme)
{
    for (int i = 0 ; i < NUMBER_OF_FLOBOS_IN_SET-1 ; i++) {
        if (m_defaultTheme == NULL)
            m_floboThemes[i].reset(new FloboThemeImpl(desc.floboThemeDescriptions[i], desc.path));
        else {
            m_floboThemes[i].reset(new FloboThemeImpl(desc.floboThemeDescriptions[i], desc.path, &(m_defaultTheme->getFloboTheme((FloboState)i))));
        }
    }
    // Neutral flobo is a special case
    if (m_defaultTheme == NULL)
        m_floboThemes[NUMBER_OF_FLOBOS_IN_SET-1].reset(new NeutralFloboThemeImpl(desc.floboThemeDescriptions[NUMBER_OF_FLOBOS_IN_SET-1], desc.path));
    else
        m_floboThemes[NUMBER_OF_FLOBOS_IN_SET-1].reset(new NeutralFloboThemeImpl(desc.floboThemeDescriptions[NUMBER_OF_FLOBOS_IN_SET-1], desc.path, &(m_defaultTheme->getFloboTheme(FLOBO_NEUTRAL))));
}

const std::string & FloboSetThemeImpl::getName() const
{
    return m_desc.name;
}

const std::string & FloboSetThemeImpl::getLocalizedName() const
{
    return m_desc.localizedName;
}

const std::string & FloboSetThemeImpl::getAuthor() const
{
    return m_desc.author;
}

const std::string & FloboSetThemeImpl::getComments() const
{
    return m_desc.description;
}

const FloboTheme & FloboSetThemeImpl::getFloboTheme(FloboState state) const
{
    switch (state) {
    case FLOBO_FALLINGBLUE:
    case FLOBO_BLUE:
        return *m_floboThemes[0];
    case FLOBO_FALLINGRED:
    case FLOBO_RED:
        return *m_floboThemes[1];
    case FLOBO_FALLINGGREEN:
    case FLOBO_GREEN:
        return *m_floboThemes[2];
    case FLOBO_FALLINGVIOLET:
    case FLOBO_VIOLET:
        return *m_floboThemes[3];
    case FLOBO_FALLINGYELLOW:
    case FLOBO_YELLOW:
        return *m_floboThemes[4];
    default:
        return *m_floboThemes[5];
    }
}


BaseFloboThemeImpl::BaseFloboThemeImpl(const FloboThemeDescription &desc,
                                     const std::string &path,
                                     const FloboTheme *defaultTheme)
    : m_desc(desc), m_path(path), m_defaultTheme(defaultTheme)
{}

BaseFloboThemeImpl::~BaseFloboThemeImpl()
{
    for (std::vector<IosSurface *>::iterator iter = m_surfaceBin.begin() ;
         iter != m_surfaceBin.end() ; iter++) {
        delete (*iter);
    }
}

FloboThemeImpl::FloboThemeImpl(const FloboThemeDescription &desc,
                             const std::string &path,
                             const FloboTheme *defaultTheme)
    : BaseFloboThemeImpl(desc, path, defaultTheme)
{
    // Initializing the arrays of pointers
    for (int i = 0 ; i < NUMBER_OF_FLOBO_FACES ; ++i)
        for (int j = 0 ; j < MAX_COMPRESSED ; ++j)
            m_faces[i][j] = NULL;
    for (int i = 0 ; i < NUMBER_OF_FLOBO_EYES ; ++i)
        for (int j = 0 ; j < MAX_COMPRESSED ; ++j)
            m_eyes[i][j] = NULL;
    for (int i = 0 ; i < NUMBER_OF_FLOBO_CIRCLES ; ++i)
            m_circles[i] = NULL;
    for (int j = 0 ; j < MAX_COMPRESSED ; ++j)
        m_shadows[j] = NULL;
    for (int i = 0 ; i < NUMBER_OF_FLOBO_DISAPPEAR ; ++i)
        m_shrinking[i] = NULL;
    for (int i = 0 ; i < NUMBER_OF_FLOBO_EXPLOSIONS ; ++i)
        m_explosion[i] = NULL;
}

IosSurface *FloboThemeImpl::getFloboSurfaceForValence(int valence, int compression) const
{
    IosSurface * &ref = m_faces[valence][compression];
    if (ref == NULL) {
        IosSurface * &uncompressed = m_faces[valence][0];
        // Do we need to load the uncompressed image?
        if (uncompressed == NULL) {
            ostringstream osstream;
            osstream << m_path << "/" << m_desc.face << "-flobo-"
                     << ((valence&8)>>3)
                     << ((valence&4)>>2)
                     << ((valence&2)>>1)
                     << (valence&1) << ".png";
            ImageOperationList opList;
            opList.resizeAlpha = true;
            if (m_desc.colorOffset == 0) {
                m_baseFaces[valence] = theCommander->getSurface(IMAGE_RGBA,
                                                                osstream.str().c_str(),
                                                                opList);
                uncompressed = m_baseFaces[valence];
            }
            else {
                opList.shiftHue = true;
                IosSurfaceRef baseFace = theCommander->getSurface(IMAGE_RGBA,
                                                                  osstream.str().c_str(),
                                                                  opList);
                uncompressed = baseFace.get()->shiftHue(m_desc.colorOffset);
                m_surfaceBin.push_back(uncompressed);
            }
        }
        // Create the compressed image
        if (uncompressed != NULL) {
            if (compression != 0) {
                ref = uncompressed->resizeAlpha(uncompressed->w,
                                                uncompressed->h - compression);
                m_surfaceBin.push_back(ref);
            }
        }
    }
    if ((ref == NULL) && (m_defaultTheme != NULL))
        return m_defaultTheme->getFloboSurfaceForValence(valence, compression);
    return ref;
}

IosSurface *FloboThemeImpl::getEyeSurfaceForIndex(int index, int compression) const
{
    IosSurface * &ref = m_eyes[index][compression];
    if (ref == NULL) {
        IosSurface * &uncompressed = m_eyes[index][0];
        // Do we need to load the uncompressed image?
        if (uncompressed == NULL) {
            ostringstream osstream;
            osstream << m_path << "/" << m_desc.eye << "-flobo-eye-" << index << ".png";
            ImageOperationList opList;
            opList.resizeAlpha = true;
            m_baseEyes[index] = theCommander->getSurface(IMAGE_RGBA,
                                                         osstream.str().c_str(),
                                                         opList);
            uncompressed = m_baseEyes[index];
        }
        if (uncompressed != NULL) {
            // Create the compressed image
            if (compression != 0) {
                ref = uncompressed->resizeAlpha(uncompressed->w,
                                                uncompressed->h - compression);
                m_surfaceBin.push_back(ref);
            }
        }
    }
    if ((ref == NULL) && (m_defaultTheme != NULL))
        return m_defaultTheme->getEyeSurfaceForIndex(index, compression);
    return ref;
}

IosSurface *FloboThemeImpl::getCircleSurfaceForIndex(int index) const
{
    IosSurface * &ref = m_circles[index];
    if (ref == NULL) {
        // Do we need to load the reference image?
        if (m_baseCircle.get() == NULL) {
            ostringstream osstream;
            osstream << m_path << "/" << m_desc.face << "-flobo-border.png";
            ImageOperationList opList;
            opList.setValue = true;
            m_baseCircle = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), opList);
        }
        ref = m_baseCircle.get()->setAlpha(sin(3.14f/2.0f+index*3.14f/64.0f)*0.6f+0.2f);
        m_surfaceBin.push_back(ref);
    }
    if ((ref == NULL) && (m_defaultTheme != NULL))
        return m_defaultTheme->getCircleSurfaceForIndex(index);
    return ref;
}

IosSurface *FloboThemeImpl::getShadowSurface(int compression) const
{
    IosSurface * &ref = m_shadows[compression];
    if (ref == NULL) {
        IosSurface * &uncompressed = m_shadows[0];
        // Do we need to load the uncompressed image?
        if (uncompressed == NULL) {
            ostringstream osstream;
            osstream << m_path << "/" << m_desc.face << "-flobo-shadow.png";
            ImageOperationList opList;
            opList.resizeAlpha = true;
            m_baseShadow = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), opList);
            uncompressed = m_baseShadow;
        }
        // Create the compressed image
        if (uncompressed != NULL) {
            if (compression != 0) {
                ref = uncompressed->resizeAlpha(uncompressed->w,
                                                uncompressed->h - compression);
                m_surfaceBin.push_back(ref);
            }
        }
    }
    if ((ref == NULL) && (m_defaultTheme != NULL))
        return m_defaultTheme->getShadowSurface(compression);
    return ref;
}

IosSurface *FloboThemeImpl::getShrinkingSurfaceForIndex(int index) const
{
    IosSurface *ref = m_shrinking[index];
    if (ref == NULL) {
        ostringstream osstream;
        osstream << m_path << "/" << m_desc.face << "-flobo-disappear-" << index << ".png";
        ImageOperationList opList;
        if (m_desc.colorOffset == 0) {
            m_baseShrinking[index] = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), opList);
            ref = m_baseShrinking[index];
        }
        else {
            opList.shiftHue = true;
            IosSurfaceRef base = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), opList);
            ref = base.get()->shiftHue(m_desc.colorOffset);
            m_surfaceBin.push_back(ref);
        }
    }
    if ((ref == NULL) && (m_defaultTheme != NULL))
        return m_defaultTheme->getShrinkingSurfaceForIndex(index);
    return ref;
}

IosSurface *FloboThemeImpl::getExplodingSurfaceForIndex(int index) const
{
    IosSurface *ref = m_explosion[index];
    if (ref == NULL) {
        ostringstream osstream;
        osstream << m_path << "/" << m_desc.face << "-flobo-explosion-" << index << ".png";
        ImageOperationList opList;
        if (m_desc.colorOffset == 0) {
            m_baseExplosion[index] = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), opList);
            ref = m_baseExplosion[index];
        }
        else {
            opList.shiftHue = true;
            IosSurfaceRef base = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), opList);
            ref = base.get()->shiftHue(m_desc.colorOffset);
            m_surfaceBin.push_back(ref);
        }
    }
    if ((ref == NULL) && (m_defaultTheme != NULL))
        return m_defaultTheme->getExplodingSurfaceForIndex(index);
    return ref;
}

NeutralFloboThemeImpl::NeutralFloboThemeImpl(const FloboThemeDescription &desc,
                                           const std::string &path,
                                           const FloboTheme *defaultTheme)
    : BaseFloboThemeImpl(desc, path, defaultTheme)
{
    // Initializing the arrays of pointers
    for (int j = 0 ; j < MAX_COMPRESSED ; j++)
        m_faces[j] = NULL;
}

IosSurface *NeutralFloboThemeImpl::getFloboSurfaceForValence(int valence, int compression) const
{
    IosSurface * &ref = m_faces[compression];
    if (ref == NULL) {
        IosSurface * &uncompressed = m_faces[0];
        // Do we need to load the uncompressed image?
        if (uncompressed == NULL) {
            ostringstream osstream;
            osstream << m_path << "/" << m_desc.face << ".png";
            ImageOperationList opList;
            opList.resizeAlpha = true;
            m_baseFace = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), opList);
            uncompressed = m_baseFace;
        }
        // Create the compressed image
        if (uncompressed != NULL) {
            if (compression != 0) {
                ref = uncompressed->resizeAlpha(uncompressed->w,
                                                uncompressed->h - compression);
                m_surfaceBin.push_back(ref);
            }
        }
    }
    if ((ref == NULL) && (m_defaultTheme != NULL))
        return m_defaultTheme->getFloboSurfaceForValence(valence, compression);
    return ref;
}

IosSurface *NeutralFloboThemeImpl::getEyeSurfaceForIndex(int index, int compression) const
{
    return NULL;
}

IosSurface *NeutralFloboThemeImpl::getCircleSurfaceForIndex(int index) const
{
    return NULL;
}

IosSurface *NeutralFloboThemeImpl::getShadowSurface(int compression) const
{
    return NULL;
}

IosSurface *NeutralFloboThemeImpl::getShrinkingSurfaceForIndex(int index) const
{
    IosSurfaceRef &ref = m_shrinking[index];
    if (ref == NULL) {
        ostringstream osstream;
        osstream << m_path << "/" << m_desc.face << "-neutral-" << index << ".png";
        ImageOperationList opList;
        ref = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), opList);
    }
    if ((ref == NULL) && (m_defaultTheme != NULL))
        return m_defaultTheme->getShrinkingSurfaceForIndex(index);
    return ref;
}

IosSurface *NeutralFloboThemeImpl::getExplodingSurfaceForIndex(int index) const
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
        ImageOperationList opList;
        result = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), opList);
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
        ImageOperationList opList;
        ref = theCommander->getSurface(IMAGE_RGBA, osstream.str().c_str(), opList);
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

int LevelThemeImpl::getFlobobanX(int playerId) const
{ return m_desc.floboban[playerId].displayX; }

int LevelThemeImpl::getFlobobanY(int playerId) const
{ return m_desc.floboban[playerId].displayY; }

int LevelThemeImpl::getNextFlobosX(int playerId) const
{ return m_desc.floboban[playerId].nextX; }

int LevelThemeImpl::getNextFlobosY(int playerId) const
{ return m_desc.floboban[playerId].nextY; }

int LevelThemeImpl::getNeutralDisplayX(int playerId) const
{ return m_desc.floboban[playerId].neutralDisplayX; }

int LevelThemeImpl::getNeutralDisplayY(int playerId) const
{ return m_desc.floboban[playerId].neutralDisplayY; }

int LevelThemeImpl::getNameDisplayX(int playerId) const
{ return m_desc.floboban[playerId].nameDisplayX; }

int LevelThemeImpl::getNameDisplayY(int playerId) const
{ return m_desc.floboban[playerId].nameDisplayY; }

int LevelThemeImpl::getScoreDisplayX(int playerId) const
{ return m_desc.floboban[playerId].scoreDisplayX; }

int LevelThemeImpl::getScoreDisplayY(int playerId) const
{ return m_desc.floboban[playerId].scoreDisplayY; }

float LevelThemeImpl::getFlobobanScale(int playerId) const
{ return m_desc.floboban[playerId].scale; }

bool LevelThemeImpl::getShouldDisplayNext(int playerId) const
{ return m_desc.floboban[playerId].shouldDisplayNext; }

bool LevelThemeImpl::getShouldDisplayShadows(int playerId) const
{ return m_desc.floboban[playerId].shouldDisplayShadow; }

bool LevelThemeImpl::getShouldDisplayEyes(int playerId) const
{ return m_desc.floboban[playerId].shouldDisplayEyes; }

bool LevelThemeImpl::getOpponentIsBehind() const
{ return m_desc.opponentIsBehind; }

int LevelThemeImpl::getStatsHeight() const
{ return m_desc.statsHeight; }

int LevelThemeImpl::getStatsLegendWidth() const
{ return m_desc.statsLegendWidth; }

int LevelThemeImpl::getStatsComboLineValueWidth() const
{ return m_desc.statsComboLineValueWidth; }

int LevelThemeImpl::getStatsLeftBackgroundOffsetX() const
{ return m_desc.statsLeftBackgroundOffsetX; }

int LevelThemeImpl::getStatsLeftBackgroundOffsetY() const
{ return m_desc.statsLeftBackgroundOffsetY; }

int LevelThemeImpl::getStatsRightBackgroundOffsetX() const
{ return m_desc.statsRightBackgroundOffsetX; }

int LevelThemeImpl::getStatsRightBackgroundOffsetY() const
{ return m_desc.statsRightBackgroundOffsetY; }

const std::string LevelThemeImpl::getGameLostLeftAnimation2P() const
{
    return m_desc.gameLostLeft2PAnimation;
}

const std::string LevelThemeImpl::getGameLostRightAnimation2P() const
{
    return m_desc.gameLostRight2PAnimation;
}

const std::string LevelThemeImpl::getCentralAnimation2P() const
{
    return m_desc.animation;
}

const std::string LevelThemeImpl::getForegroundAnimation() const
{
    if (m_desc.fgAnimation == "")
        return "";
    return  m_path + "/" + m_desc.fgAnimation;
}

const std::string LevelThemeImpl::getReadyAnimation2P() const
{
    return m_desc.getReadyAnimation;
}

const std::string LevelThemeImpl::getThemeRootPath() const
{ return m_path; }



