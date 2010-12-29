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

 #ifndef _ANIMATEDPUYOTHEME_H
 #define _ANIMATEDPUYOTHEME_H

#include <map>
#include <memory>
#include "goomsl/goomsl.h"
#include "goomsl/goomsl_hash.h"
#include "LocalizedDictionary.h"
#include "DataPathManager.h"
#include "Theme.h"
#include "PuyoCommander.h"

class PuyoThemeDescription {
public:
    std::string face;
    std::string disappear;
    std::string explosion;
    std::string eye;
    float colorOffset;
};

class PuyoSetThemeDescription {
public:
    std::string name;
    std::string localizedName;
    std::string author;
    std::string description;
    std::string localizedDescription;
    std::string path;
    PuyoThemeDescription puyoThemeDescriptions[NUMBER_OF_PUYOS_IN_SET];
};

class PuyobanThemeDefinition {
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
};

class FontDefinition {
public:
    std::string fontPath;
    int fontSize;
    IosFontFx fontFx;
};

class LevelThemeDescription {
public:
    std::string name;
    std::string localizedName;
    std::string author;
    std::string description;
    std::string localizedDescription;
    std::string path;
    std::string lives;
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

    PuyobanThemeDefinition puyoban[NUMBER_OF_PUYOBANS_IN_LEVEL];
    FontDefinition playerNameFont, scoreFont;
    
    bool opponentIsBehind;
};

class BasePuyoThemeImpl : public PuyoTheme {
public:
    BasePuyoThemeImpl(const PuyoThemeDescription &desc,
                      const std::string &path,
                      const PuyoTheme *defaultTheme = NULL);
    virtual ~BasePuyoThemeImpl();
protected:
    const PuyoThemeDescription &m_desc;
    const std::string &m_path;
    const PuyoTheme *m_defaultTheme;
    mutable std::vector<IosSurface *> m_surfaceBin;
};

class PuyoThemeImpl : public BasePuyoThemeImpl {
public:
    PuyoThemeImpl(const PuyoThemeDescription &desc,
                  const std::string &path,
                  const PuyoTheme *defaultTheme = NULL);
    virtual IosSurface *getPuyoSurfaceForValence(int valence, int compression = 0) const;
    virtual IosSurface *getEyeSurfaceForIndex(int index, int compression = 0) const;
    virtual IosSurface *getCircleSurfaceForIndex(int index) const;
    virtual IosSurface *getShadowSurface(int compression = 0) const;
    virtual IosSurface *getShrinkingSurfaceForIndex(int index) const;
    virtual IosSurface *getExplodingSurfaceForIndex(int index) const;
private:
    mutable IosSurface* m_faces[NUMBER_OF_PUYO_FACES][MAX_COMPRESSED];
    mutable IosSurfaceRef m_baseFaces[NUMBER_OF_PUYO_FACES];

    mutable IosSurface* m_eyes[NUMBER_OF_PUYO_EYES][MAX_COMPRESSED];
    mutable IosSurfaceRef m_baseEyes[NUMBER_OF_PUYO_EYES];

    mutable IosSurface* m_circles[NUMBER_OF_PUYO_CIRCLES];
    mutable IosSurfaceRef m_baseCircle;

    mutable IosSurface *m_shadows[MAX_COMPRESSED];
    mutable IosSurfaceRef m_baseShadow;

    mutable IosSurface *m_shrinking[NUMBER_OF_PUYO_DISAPPEAR];
    mutable IosSurfaceRef m_baseShrinking[NUMBER_OF_PUYO_DISAPPEAR];

    mutable IosSurface *m_explosion[NUMBER_OF_PUYO_EXPLOSIONS];
    mutable IosSurfaceRef m_baseExplosion[NUMBER_OF_PUYO_EXPLOSIONS];
};

class NeutralPuyoThemeImpl : public BasePuyoThemeImpl {
public:
    NeutralPuyoThemeImpl(const PuyoThemeDescription &desc,
                         const std::string &path,
                         const PuyoTheme *defaultTheme = NULL);
    virtual IosSurface *getPuyoSurfaceForValence(int valence, int compression = 0) const;
    virtual IosSurface *getEyeSurfaceForIndex(int index, int compression = 0) const;
    virtual IosSurface *getCircleSurfaceForIndex(int index) const;
    virtual IosSurface *getShadowSurface(int compression = 0) const;
    virtual IosSurface *getShrinkingSurfaceForIndex(int index) const;
    virtual IosSurface *getExplodingSurfaceForIndex(int index) const;
private:
    mutable IosSurface *m_faces[MAX_COMPRESSED];
    mutable IosSurfaceRef m_baseFace;
    mutable IosSurfaceRef m_shrinking[NUMBER_OF_PUYO_DISAPPEAR];
};

class PuyoSetThemeImpl : public PuyoSetTheme {
public:
    PuyoSetThemeImpl(const PuyoSetThemeDescription &desc,
                     PuyoSetTheme *defaultTheme = NULL);
    virtual const std::string & getName() const;
    virtual const std::string & getLocalizedName() const;
    virtual const std::string & getAuthor() const;
    virtual const std::string & getComments() const;
    virtual const PuyoTheme & getPuyoTheme(PuyoState state) const;
private:
    const PuyoSetThemeDescription &m_desc;
    std::auto_ptr<PuyoTheme> m_puyoThemes[NUMBER_OF_PUYOS_IN_SET];
    PuyoSetTheme *m_defaultTheme;
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
    virtual IosSurface * getLifeForIndex(int index) const;
    virtual IosSurface * getBackground() const;
    virtual IosSurface * getGrid() const;
    virtual IosSurface * getSpeedMeter(bool front) const;
    virtual IosSurface * getNeutralIndicator() const;
    virtual IosSurface * getBigNeutralIndicator() const;
    virtual IosSurface * getGiantNeutralIndicator() const;
    // Fonts
    virtual IosFont *getPlayerNameFont() const ;
    virtual IosFont *getScoreFont() const;
    // Positions and dimensions of the Puyoban
    virtual int getSpeedMeterX() const;
    virtual int getSpeedMeterY() const;
    virtual int getLifeDisplayX() const;
    virtual int getLifeDisplayY() const;
    virtual int getPuyobanX(int playerId) const;
    virtual int getPuyobanY(int playerId) const;
    virtual int getNextPuyosX(int playerId) const;
    virtual int getNextPuyosY(int playerId) const;
    virtual int getNeutralDisplayX(int playerId) const;
    virtual int getNeutralDisplayY(int playerId) const;
    virtual int getNameDisplayX(int playerId) const;
    virtual int getNameDisplayY(int playerId) const;
    virtual int getScoreDisplayX(int playerId) const;
    virtual int getScoreDisplayY(int playerId) const;
    virtual float getPuyobanScale(int playerId) const;
    // Behaviour of the Puyoban
    virtual bool getShouldDisplayNext(int playerId) const;
    virtual bool getShouldDisplayShadows(int playerId) const;
    virtual bool getShouldDisplayEyes(int playerId) const;
    virtual bool getOpponentIsBehind() const;
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
    const LevelThemeDescription &m_desc;
    const std::string &m_path;
    DataPathManager &m_dataPathManager;
    LevelThemeImpl *m_defaultTheme;
    mutable IosSurfaceRef m_lifes[NUMBER_OF_LIVES];
    mutable IosSurfaceRef m_background;
    mutable IosSurfaceRef m_grid;
    mutable IosSurfaceRef m_speedMeterFront, m_speedMeterBack;
    mutable IosSurfaceRef m_neutralIndicator, m_bigNeutralIndicator, m_giantNeutralIndicator;
    mutable IosFontRef    m_playerNameFont, m_scoreFont;
};

class ThemeManagerImpl : public ThemeManager {
public:
    ThemeManagerImpl(DataPathManager &dataPathManager);
    virtual PuyoSetTheme * createPuyoSetTheme(const std::string &themeName);
    virtual LevelTheme   * createLevelTheme(const std::string &themeName);

    virtual const std::vector<std::string> & getPuyoSetThemeList();
    virtual const std::vector<std::string> & getLevelThemeList();
private:
    void loadThemePack(const std::string &path);
    static void end_puyoset(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static void end_level(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static void end_description(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static void loadPuyobanDefinition(GoomSL *gsl, int playerId, PuyobanThemeDefinition &puyoban);
    static void loadFontDefinition(GoomSL *gsl, const char * fontName, FontDefinition &font);

    static const char * s_themeFolderExtension;

    static const char *s_key_PuyoFace[NUMBER_OF_PUYOS_IN_SET];
    static const char *s_key_PuyoDisappear[NUMBER_OF_PUYOS_IN_SET];
    static const char *s_key_PuyoExplosion[NUMBER_OF_PUYOS_IN_SET];
    static const char *s_key_PuyoEye[NUMBER_OF_PUYOS_IN_SET];
    static const char *s_key_PuyoColorOffset[NUMBER_OF_PUYOS_IN_SET];

    DataPathManager &m_dataPathManager;
    std::string m_themePackLoadingPath;

    std::vector<std::string> m_puyoSetThemeList;
    std::vector<std::string> m_levelThemeList;
    std::map<std::string, PuyoSetThemeDescription> m_puyoSetThemeDescriptions;
    std::map<std::string, LevelThemeDescription> m_levelThemeDescriptions;
    std::auto_ptr<LocalizedDictionary> m_localeDictionary;

    std::string m_defaultPuyoSetThemeName;
    std::auto_ptr<PuyoSetTheme> m_defaultPuyoSetTheme;

    std::string m_defaultLevelThemeName;
    std::auto_ptr<LevelThemeImpl> m_defaultLevelTheme;
};


#endif // _ANIMATEDPUYOTHEME_H

