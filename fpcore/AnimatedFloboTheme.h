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

 #ifndef _ANIMATEDPUYOTHEME_H
 #define _ANIMATEDPUYOTHEME_H

#include <map>
#include <memory>
#include "goomsl.h"
#include "goomsl_hash.h"
#include "LocalizedDictionary.h"
#include "DataPathManager.h"
#include "Theme.h"
#include "FPCommander.h"

class FloboThemeDescription {
public:
    std::string face;
    std::string disappear;
    std::string explosion;
    std::string eye;
    float colorOffset;
    int eyeOffsetX, eyeOffsetY;
};

class FloboSetThemeDescription {
public:
    std::string name;
    std::string localizedName;
    std::string author;
    std::string description;
    std::string localizedDescription;
    std::string path;
    FloboThemeDescription floboThemeDescriptions[NUMBER_OF_FLOBOS_IN_SET];
};

class FlobobanThemeDefinition {
public:
    int displayX, displayY;
    int nextX, nextY;
    int neutralDisplayX, neutralDisplayY;
    int nameDisplayX, nameDisplayY;
    int scoreDisplayX, scoreDisplayY;
    bool shouldDisplayNext;
    bool shouldDisplayShadow;
    bool shouldDisplayEyes;
    float scale;
    int trophyDisplayX, trophyDisplayY;
};

class FontDefinition {
public:
    std::string fontPath;
    int fontSize;
    RGBA fontColor;
};

class LevelThemeDescription {
public:
    int         nbPlayers;
    std::string name;
    std::string localizedName;
    std::string author;
    std::string description;
    std::string localizedDescription;
    std::string path;
    std::string lives;
    std::string trophy;
    std::string background;
    std::string grid;
    std::string speedMeter;
    std::string neutralIndicator;

    std::string gameLostRight2PAnimation;
    std::string gameLostLeft2PAnimation;
    std::string animation, fgAnimation;
    std::string getReadyAnimation;

    int speedMeterX, speedMeterY;
    int lifeDisplayX, lifeDisplayY;

    FlobobanThemeDefinition floboban[NUMBER_OF_FLOBOBANS_IN_LEVEL];
    FontDefinition playerNameFont, scoreFont;

    bool opponentIsBehind;

    int statsHeight, statsLegendWidth, statsComboLineValueWidth;
    int statsLeftBackgroundOffsetX, statsLeftBackgroundOffsetY;
    int statsRightBackgroundOffsetX, statsRightBackgroundOffsetY;
};

class BaseFloboThemeImpl : public FloboTheme {
public:
    BaseFloboThemeImpl(const FloboThemeDescription &desc,
                      const std::string &path,
                      const FloboTheme *defaultTheme = NULL);
    virtual ~BaseFloboThemeImpl();
protected:
    const FloboThemeDescription &_desc;
    const std::string &_path;
    const FloboTheme *_defaultTheme;
    mutable std::vector<IosSurface *> _surfaceBin;
};

class FloboThemeImpl : public BaseFloboThemeImpl {
public:
    FloboThemeImpl(const FloboThemeDescription &desc,
                  const std::string &path,
                  const FloboTheme *defaultTheme = NULL);
    virtual IosSurface *getFloboSurfaceForValence(int valence, int compression = 0) const;
    virtual IosSurface *getEyeSurfaceForIndex(int index, int compression = 0) const;
    virtual IosSurface *getCircleSurfaceForIndex(int index) const;
    virtual IosSurface *getShadowSurface(int compression = 0) const;
    virtual IosSurface *getShrinkingSurfaceForIndex(int index) const;
    virtual IosSurface *getExplodingSurfaceForIndex(int index) const;
    virtual int getEyeSurfaceOffsetX() const;
    virtual int getEyeSurfaceOffsetY() const;
private:
    mutable IosSurface* _faces[NUMBER_OF_FLOBO_FACES][MAX_COMPRESSED];
    mutable IosSurfaceRef _baseFaces[NUMBER_OF_FLOBO_FACES];

    mutable IosSurface* _eyes[NUMBER_OF_FLOBO_EYES][MAX_COMPRESSED];
    mutable IosSurfaceRef _baseEyes[NUMBER_OF_FLOBO_EYES];

    mutable IosSurface* _circles[NUMBER_OF_FLOBO_CIRCLES];
    mutable IosSurfaceRef _baseCircle;

    mutable IosSurface *_shadows[MAX_COMPRESSED];
    mutable IosSurfaceRef _baseShadow;

    mutable IosSurface *_shrinking[NUMBER_OF_FLOBO_DISAPPEAR];
    mutable IosSurfaceRef _baseShrinking[NUMBER_OF_FLOBO_DISAPPEAR];

    mutable IosSurface *_explosion[NUMBER_OF_FLOBO_EXPLOSIONS];
    mutable IosSurfaceRef _baseExplosion[NUMBER_OF_FLOBO_EXPLOSIONS];
};

class NeutralFloboThemeImpl : public BaseFloboThemeImpl {
public:
    NeutralFloboThemeImpl(const FloboThemeDescription &desc,
                         const std::string &path,
                         const FloboTheme *defaultTheme = NULL);
    virtual IosSurface *getFloboSurfaceForValence(int valence, int compression = 0) const;
    virtual IosSurface *getEyeSurfaceForIndex(int index, int compression = 0) const;
    virtual IosSurface *getCircleSurfaceForIndex(int index) const;
    virtual IosSurface *getShadowSurface(int compression = 0) const;
    virtual IosSurface *getShrinkingSurfaceForIndex(int index) const;
    virtual IosSurface *getExplodingSurfaceForIndex(int index) const;
    virtual int getEyeSurfaceOffsetX() const;
    virtual int getEyeSurfaceOffsetY() const;
private:
    mutable IosSurface *_faces[MAX_COMPRESSED];
    mutable IosSurfaceRef _baseFace;
    mutable IosSurfaceRef _shrinking[NUMBER_OF_FLOBO_DISAPPEAR];
};

class FloboSetThemeImpl : public FloboSetTheme {
public:
    FloboSetThemeImpl(const FloboSetThemeDescription &desc,
                     FloboSetTheme *defaultTheme = NULL);
    virtual const std::string & getName() const;
    virtual const std::string & getLocalizedName() const;
    virtual const std::string & getAuthor() const;
    virtual const std::string & getComments() const;
    virtual const FloboTheme & getFloboTheme(FloboState state) const;
private:
    const FloboSetThemeDescription &_desc;
    std::unique_ptr<FloboTheme> _floboThemes[NUMBER_OF_FLOBOS_IN_SET];
    FloboSetTheme *_defaultTheme;
};

class LevelThemeImpl : public LevelTheme {
public:
    LevelThemeImpl(const LevelThemeDescription &desc,
                   DataPathManager &dataPathManager,
                   LevelThemeImpl *defaultTheme = NULL);
    virtual const std::string & getName() const;
    virtual const std::string & getLocalizedName() const;
    virtual const std::string & getAuthor() const;
    virtual const std::string & getComments() const;
    virtual int getNbPlayers() const;
    virtual IosSurface * getLifeForIndex(int index) const;
    virtual IosSurface * getBackground() const;
    virtual IosSurface * getGrid() const;
    virtual IosSurface * getSpeedMeter(bool front) const;
    virtual IosSurface * getNeutralIndicator() const;
    virtual IosSurface * getBigNeutralIndicator() const;
    virtual IosSurface * getGiantNeutralIndicator() const;
    virtual IosSurface * getTrophy() const;
    // Fonts
    virtual IosFont *getPlayerNameFont() const ;
    virtual const RGBA *getPlayerNameColor() const { return &_desc.playerNameFont.fontColor; }
    virtual IosFont *getScoreFont() const;
    virtual const RGBA *getScoreColor() const { return &_desc.scoreFont.fontColor; }
    // Positions and dimensions of the Floboban
    virtual int getSpeedMeterX() const;
    virtual int getSpeedMeterY() const;
    virtual int getLifeDisplayX() const;
    virtual int getLifeDisplayY() const;
    virtual int getFlobobanX(int playerId) const;
    virtual int getFlobobanY(int playerId) const;
    virtual int getNextFlobosX(int playerId) const;
    virtual int getNextFlobosY(int playerId) const;
    virtual int getNeutralDisplayX(int playerId) const;
    virtual int getNeutralDisplayY(int playerId) const;
    virtual int getNameDisplayX(int playerId) const;
    virtual int getNameDisplayY(int playerId) const;
    virtual int getScoreDisplayX(int playerId) const;
    virtual int getScoreDisplayY(int playerId) const;
    virtual float getFlobobanScale(int playerId) const;
    virtual int getTrophyDisplayX(int playerId) const;
    virtual int getTrophyDisplayY(int playerId) const;
    // Behaviour of the Floboban
    virtual bool getShouldDisplayNext(int playerId) const;
    virtual bool getShouldDisplayShadows(int playerId) const;
    virtual bool getShouldDisplayEyes(int playerId) const;
    virtual bool getOpponentIsBehind() const;
    // Stats widget dimensions
    virtual int getStatsHeight() const;
    virtual int getStatsLegendWidth() const;
    virtual int getStatsComboLineValueWidth() const;
    virtual int getStatsLeftBackgroundOffsetX() const;
    virtual int getStatsLeftBackgroundOffsetY() const;
    virtual int getStatsRightBackgroundOffsetX() const;
    virtual int getStatsRightBackgroundOffsetY() const;
    // Animation names
    virtual const std::string getGameLostLeftAnimation2P() const;
    virtual const std::string getGameLostRightAnimation2P() const;
    virtual const std::string getCentralAnimation2P() const;
    virtual const std::string getForegroundAnimation() const;
    virtual const std::string getReadyAnimation2P() const;
    // Misc
    virtual const std::string getThemeRootPath() const;
private:
    inline IosSurfaceRef & getResource(IosSurfaceRef &ref,
                                    const std::string &resName,
                                    const char *resSuffix) const;
    const LevelThemeDescription &_desc;
    const std::string &_path;
    DataPathManager &_dataPathManager;
    LevelThemeImpl *_defaultTheme;
    mutable IosSurfaceRef _lifes[NUMBER_OF_LIVES];
    mutable IosSurfaceRef _background;
    mutable IosSurfaceRef _grid;
    mutable IosSurfaceRef _speedMeterFront, _speedMeterBack;
    mutable IosSurfaceRef _neutralIndicator, _bigNeutralIndicator, _giantNeutralIndicator;
    mutable IosSurfaceRef _trophy;
    mutable IosFontRef    _playerNameFont, _scoreFont;
};

class ThemeManagerImpl : public ThemeManager {
public:
    ThemeManagerImpl(DataPathManager &dataPathManager);
    virtual FloboSetTheme * createFloboSetTheme(const std::string &themeName);
    virtual LevelTheme   * createLevelTheme(const std::string &themeName);

    virtual const std::vector<std::string> & getFloboSetThemeList();
    virtual std::vector<std::string> getLevelThemeList(int nbPlayers);
private:
    void loadThemePack(const std::string &path);
    static void end_floboset(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static void end_level(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static void end_description(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static void loadFlobobanDefinition(GoomSL *gsl, int playerId, FlobobanThemeDefinition &floboban);
    static void loadFontDefinition(GoomSL *gsl, const char * fontName, FontDefinition &font);

    static const char * s_themeFolderExtension;

    static const char *s_key_FloboFace[NUMBER_OF_FLOBOS_IN_SET];
    static const char *s_key_FloboDisappear[NUMBER_OF_FLOBOS_IN_SET];
    static const char *s_key_FloboExplosion[NUMBER_OF_FLOBOS_IN_SET];
    static const char *s_key_FloboEye[NUMBER_OF_FLOBOS_IN_SET];
    static const char *s_key_FloboEyeOffsetX[NUMBER_OF_FLOBOS_IN_SET];
    static const char *s_key_FloboEyeOffsetY[NUMBER_OF_FLOBOS_IN_SET];
    static const char *s_key_FloboColorOffset[NUMBER_OF_FLOBOS_IN_SET];

    DataPathManager &_dataPathManager;
    std::string _themePackLoadingPath;

    std::vector<std::string> _floboSetThemeList;
    std::vector<std::string> _levelThemeList;
    std::map<std::string, FloboSetThemeDescription> _floboSetThemeDescriptions;
    std::map<std::string, LevelThemeDescription> _levelThemeDescriptions;
    std::unique_ptr<LocalizedDictionary> _localeDictionary;

    std::string _defaultFloboSetThemeName;
    std::unique_ptr<FloboSetTheme> _defaultFloboSetTheme;

    std::string _defaultLevelThemeName;
    std::unique_ptr<LevelThemeImpl> _defaultLevelTheme;
};


#endif // _ANIMATEDPUYOTHEME_H

