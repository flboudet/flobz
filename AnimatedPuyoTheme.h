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

#define NUMBER_OF_PUYOS_IN_SET 6
#define NUMBER_OF_PUYOBANS_IN_LEVEL 2

#define NUMBER_OF_LIVES 4
#define MAX_COMPRESSED 32

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
};

class PuyoThemeImpl : public PuyoTheme {
public:
    PuyoThemeImpl(const PuyoThemeDescription &desc, const std::string &path);
    virtual IosSurface *getPuyoSurfaceForValence(int valence, int compression = 0) const;
    virtual IosSurface *getEyeSurfaceForIndex(int index, int compression = 0) const;
    virtual IosSurface *getCircleSurfaceForIndex(int index, int compression = 0) const;
    virtual IosSurface *getShadowSurface(int compression = 0) const;
    virtual IosSurface *getShrinkingSurfaceForIndex(int index, int compression = 0) const;
    virtual IosSurface *getExplodingSurfaceForIndex(int index, int compression = 0) const;
private:
    const PuyoThemeDescription &m_desc;
    const std::string &m_path;

    mutable IosSurface* m_faces[NUMBER_OF_PUYO_FACES][MAX_COMPRESSED];
    mutable IosSurfaceRef m_baseFaces[NUMBER_OF_PUYO_FACES];

    mutable IosSurface* m_eyes[NUMBER_OF_PUYO_EYES][MAX_COMPRESSED];
    mutable IosSurfaceRef m_baseEyes[NUMBER_OF_PUYO_EYES];

    mutable IosSurface* m_circles[NUMBER_OF_PUYO_CIRCLES][MAX_COMPRESSED];
    mutable IosSurfaceRef m_baseCircle;

    mutable IosSurface *m_shadows[MAX_COMPRESSED];
    mutable IosSurfaceRef m_baseShadow;
};

class NeutralPuyoThemeImpl : public PuyoTheme {
public:
    NeutralPuyoThemeImpl(const PuyoThemeDescription &desc, const std::string &path);
    virtual IosSurface *getPuyoSurfaceForValence(int valence, int compression = 0) const;
    virtual IosSurface *getEyeSurfaceForIndex(int index, int compression = 0) const;
    virtual IosSurface *getCircleSurfaceForIndex(int index, int compression = 0) const;
    virtual IosSurface *getShadowSurface(int compression = 0) const;
    virtual IosSurface *getShrinkingSurfaceForIndex(int index, int compression = 0) const;
    virtual IosSurface *getExplodingSurfaceForIndex(int index, int compression = 0) const;
private:
    const PuyoThemeDescription &m_desc;
    const std::string &m_path;
    mutable IosSurface *m_faces[MAX_COMPRESSED];
    mutable IosSurfaceRef m_baseFace;
};

class PuyoSetThemeImpl : public PuyoSetTheme {
public:
    PuyoSetThemeImpl(const PuyoSetThemeDescription &desc);
    virtual const std::string & getName() const;
    virtual const std::string & getLocalizedName() const;
    virtual const std::string & getAuthor() const;
    virtual const std::string & getComments() const;
    virtual const PuyoTheme & getPuyoTheme(PuyoState state) const;
private:
    const PuyoSetThemeDescription &m_desc;
    std::auto_ptr<PuyoTheme> m_puyoThemes[NUMBER_OF_PUYOS_IN_SET];
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

    std::string m_defaultLevelThemeName;
    std::auto_ptr<LevelThemeImpl> m_defaultLevelTheme;
};

#ifdef DISABLED
class PuyoThemeImpl : public PuyoTheme {
};

class StandardAnimatedPuyoTheme : public AnimatedPuyoTheme {

public:
    StandardAnimatedPuyoTheme(const String path, const char * face, const char * disappear, const char * explosions, const char * eyes, const float color_offset);
    ~StandardAnimatedPuyoTheme(void);

    IosSurface *getSurface(PuyoPictureType picture, int index);
    IosSurface *getShrunkSurface(PuyoPictureType picture, int index, int compression);
    IosSurface *getPuyoSurfaceForValence(int valence, int compression = 0);
    IosSurface *getEyeSurfaceForIndex(int index, int compression = 0) { return getShrunkSurface(PUYO_EYES,index,compression); };
    IosSurface *getCircleSurfaceForIndex(int index, int compression = 0) { return getShrunkSurface(PUYO_CIRCLES,index,compression); };
    IosSurface *getShadowSurface(int compression = 0) { return getShrunkSurface(PUYO_SHADOWS,0,compression); };
    IosSurface *getShrinkingSurfaceForIndex(int index, int compression = 0) { return getShrunkSurface(PUYO_DISAPPEAR,index,compression); };
    IosSurface *getExplodingSurfaceForIndex(int index, int compression = 0) { return getShrunkSurface(PUYO_EXPLOSIONS,index,compression); };

    bool validate(void);
    bool cache(void);
    void releaseCached(void);

private:
    PuyoPictureType _type;

    String _path;
    char * _face;
    char * _disappear;
    char * _explosions;
    char * _eyes;
    float _color_offset;

    IosSurfaceRef _puyoOriginalCircle;
    IosSurfaceRef _puyoOriginalShadow;
    IosSurface * _puyoFaces[NUMBER_OF_PUYO_FACES][MAX_COMPRESSED];
    IosSurface * _puyoCircles[NUMBER_OF_PUYO_CIRCLES][MAX_COMPRESSED];
    IosSurface * _puyoShadow[MAX_COMPRESSED];
    IosSurface * _puyoExplosion[NUMBER_OF_PUYO_EXPLOSIONS][MAX_COMPRESSED];
    IosSurface * _puyoDisappear[NUMBER_OF_PUYO_DISAPPEAR][MAX_COMPRESSED];
    IosSurface * _puyoEyes[NUMBER_OF_PUYO_EYES][MAX_COMPRESSED];

    bool _cached;
};

class NeutralAnimatedPuyoTheme : public AnimatedPuyoTheme {
public:
    NeutralAnimatedPuyoTheme(const String path, const char * face);
    ~NeutralAnimatedPuyoTheme(void);
    IosSurface *getSurface(PuyoPictureType picture, int index);
    IosSurface *getShrunkSurface(PuyoPictureType picture, int index, int compression) { return getSurface(picture, index);}
    IosSurface *getPuyoSurfaceForValence(int valence, int compression = 0);
    IosSurface *getEyeSurfaceForIndex(int index, int compression = 0) { return NULL; }
    IosSurface *getCircleSurfaceForIndex(int index, int compression = 0) { return NULL; }
    IosSurface *getShadowSurface(int compression = 0) { return NULL; }
    IosSurface *getShrinkingSurfaceForIndex(int index, int compression = 0) { return NULL; }
    IosSurface *getExplodingSurfaceForIndex(int index, int compression = 0);
    bool cache(void);
    void releaseCached(void);
private:
    String imageFullPath;
    String imageDefaultPath;
    String faceName;
    IosSurfaceRef _puyoNeutral;
    IosSurfaceRef _puyoNeutralPop[3];
    bool _cached;
};

// Class containing the theme for a puyo set

class AnimatedPuyoSetTheme {

public:
    AnimatedPuyoSetTheme(const String path, const String name, const String lname);
    ~AnimatedPuyoSetTheme(void);

    const String getName(void);
    const String getLocalizedName(void);
    const String getAuthor(void);
    const String getComments(void);

    bool addAnimatedPuyoTheme(const String face, const char * disappear, const char * explosions, const char * eyes, const float color_offset);
    bool addNeutralPuyo(const String face, const char * disappear, const char * explosions, const char * eyes, const float color_offset);
    void addInfo(const String author, const String comments);

    AnimatedPuyoTheme * getAnimatedPuyoTheme(PuyoState state);

    bool validate(void);
    bool cache(void);
    void releaseCached(void);

private:
    String _path;
    String _name;
    String _lname;

    String _author;
    String _comments;

    StandardAnimatedPuyoTheme * _puyos[NUMBER_OF_PUYOS];
    NeutralAnimatedPuyoTheme * _neutral;

    int _numberOfPuyos;
};


// Class containing the theme for a level

class PuyoLevelTheme {

public:
    PuyoLevelTheme(const String path, const String name, const String lname);
    ~PuyoLevelTheme(void);

    const String getName(void);
    const String getLocalizedName(void);
    const String getAuthor(void);
    const String getComments(void);
    const String getThemeRootPath() const { return _path; }
    void addInfo(const String author, const String comments);

    bool setLives(const char * lives);
    bool setBackground(const char * back);
    bool setGrid(const char * grid);
    bool setSpeedMeter(const char * speedmeter);
    bool setNeutralIndicator(const char * neutralIndicator);
    void setSpeedMeterXY(int x, int y) {_speedMeterX = x; _speedMeterY = y;}
    void setLifeDisplayXY(int x, int y) {_lifeDisplayX = x; _lifeDisplayY = y;}
    void setPuyobanXY(int playerId, int x, int y) {_puyobanX[playerId] = x; _puyobanY[playerId] = y;}
    void setNextPuyosXY(int playerId, int x, int y) {_nextPuyosX[playerId] = x; _nextPuyosY[playerId] = y;}
    void setNeutralDisplayXY(int playerId, int x, int y) {_neutralDisplayX[playerId] = x; _neutralDisplayY[playerId] = y;}
    void setNameDisplayXY(int playerId, int x, int y) {_nameDisplayX[playerId] = x; _nameDisplayY[playerId] = y;}
    void setScoreDisplayXY(int playerId, int x, int y) {_scoreDisplayX[playerId] = x; _scoreDisplayY[playerId] = y;}
    void setShouldDisplayNext(int playerId, bool shouldDisplayNext) {_shouldDisplayNext[playerId] = shouldDisplayNext; }
    void setShouldDisplayShadows(int playerId, bool shouldDisplayShadows) {_shouldDisplayShadows[playerId] = shouldDisplayShadows; }
    void setShouldDisplayEyes(int playerId, bool shouldDisplayEyes) {_shouldDisplayEyes[playerId] = shouldDisplayEyes; }
    void setPuyobanScale(int playerId, float puyobanScale) {_puyobanScale[playerId] = puyobanScale;}
    void setPlayerNameFont(const IosFontRef &font) {_playerNameFont = font;}
    void setScoreFont(const IosFontRef &font) {_scoreFont = font; }
    void setAnimations(String gamelost_left_2p, String gamelost_right_2p, String animation_2p) {
        _gamelost_left_2p = gamelost_left_2p;
        _gamelost_right_2p = gamelost_right_2p;
        _animation_2p = animation_2p;
    }
    void setForegroundAnimation(String foregroundAnimation) { _foreground_animation = foregroundAnimation; }
    void setGetReady2PAnimation(String getReadyAnimation) { _getReadyAnimation = getReadyAnimation; }

    IosSurface * getLifeForIndex(int index);
    IosSurface * getBackground(void);
    IosSurface * getGrid(void);
    IosSurface * getSpeedMeter(bool front);

    IosSurface * getNeutralIndicator();
    IosSurface * getBigNeutralIndicator();
    IosSurface * getGiantNeutralIndicator();

    int getSpeedMeterX() const  { return _speedMeterX; }
    int getSpeedMeterY() const  { return _speedMeterY; }
    int getLifeDisplayX() const { return _lifeDisplayX; }
    int getLifeDisplayY() const { return _lifeDisplayY; }
    int getPuyobanX(int playerId) const { return _puyobanX[playerId]; }
    int getPuyobanY(int playerId) const { return _puyobanY[playerId]; }
    int getNextPuyosX(int playerId) const { return _nextPuyosX[playerId]; }
    int getNextPuyosY(int playerId) const { return _nextPuyosY[playerId]; }
    int getNeutralDisplayX(int playerId) const { return _neutralDisplayX[playerId]; }
    int getNeutralDisplayY(int playerId) const { return _neutralDisplayY[playerId]; }
    int getNameDisplayX(int playerId) const { return _nameDisplayX[playerId]; }
    int getNameDisplayY(int playerId) const { return _nameDisplayY[playerId]; }
    int getScoreDisplayX(int playerId) const { return _scoreDisplayX[playerId]; }
    int getScoreDisplayY(int playerId) const { return _scoreDisplayY[playerId]; }
    bool getShouldDisplayNext(int playerId) const { return _shouldDisplayNext[playerId]; }
    bool getShouldDisplayShadows(int playerId) const { return _shouldDisplayShadows[playerId]; }
    bool getShouldDisplayEyes(int playerId) const { return _shouldDisplayEyes[playerId]; }
    float getPuyobanScale(int playerId) const { return _puyobanScale[playerId]; }
    IosFont *getPlayerNameFont() const { return _playerNameFont; }
    IosFont *getScoreFont() const { return _scoreFont; }
    bool getOpponentIsBehind() const { return false; }

    const String getGameLostLeftAnimation2P() const;
    const String getGameLostRightAnimation2P() const;
    const String getCentralAnimation2P() const;
    const String getForegroundAnimation() const;
    const String getReadyAnimation2P() const;
private:
    String _path;
    String _name;
    String _lname;

    String _author;
    String _comments;

    char * _lives;
    char * _background;
    char * _grid;
    char * _speed_meter;
    char * _neutral_indicator;

    String _gamelost_left_2p;
    String _gamelost_right_2p;
    String _animation_2p;
    String _foreground_animation;
    String _getReadyAnimation;

    IosSurfaceRef _levelLives[NUMBER_OF_LIVES];
    IosSurfaceRef _levelBackground;
    IosSurfaceRef _levelGrid;
    IosSurfaceRef _levelMeter[2];

    IosSurfaceRef _neutralIndicator;
    IosSurfaceRef _bigNeutralIndicator;
    IosSurfaceRef _giantNeutralIndicator;

    IosFontRef _scoreFont;
    IosFontRef _playerNameFont;

    int _speedMeterX, _speedMeterY;
    int _lifeDisplayX, _lifeDisplayY;
    int _puyobanX[2], _puyobanY[2];
    int _nextPuyosX[2], _nextPuyosY[2];
    int _neutralDisplayX[2], _neutralDisplayY[2];
    int _nameDisplayX[2], _nameDisplayY[2];
    int _scoreDisplayX[2], _scoreDisplayY[2];
    bool _shouldDisplayNext[2], _shouldDisplayShadows[2], _shouldDisplayEyes[2];
    float _puyobanScale[2];

    bool _cached;

    bool validate(void);
    bool cache(void);
    void releaseCached(void);
};


// Class containing the theme manager
// Should be instantiated only once (see below)

class AnimatedPuyoThemeManager {
public:
    AnimatedPuyoThemeManager(bool useAltLocation);
    ~AnimatedPuyoThemeManager(void);

    AnimatedPuyoSetTheme * getAnimatedPuyoSetTheme(const String);
    AnimatedPuyoSetTheme * getAnimatedPuyoSetTheme(void);
    String getPreferedAnimatedPuyoSetThemeName(void);
    void setPreferedAnimatedPuyoSetTheme(const String);

    PuyoLevelTheme * getPuyoLevelTheme(const String);
    PuyoLevelTheme * getPuyoLevelTheme(void);
    String getPreferedPuyoLevelThemeName(void);
    void setPreferedPuyoLevelTheme(const String);

    AdvancedBuffer<const char *> * getAnimatedPuyoSetThemeList(void);
    AdvancedBuffer<const char *> * getPuyoLevelThemeList(void);

    AdvancedBuffer<AnimatedPuyoSetTheme *> * getAnimatedPuyoSetThemeObjectList(void);
    AdvancedBuffer<PuyoLevelTheme *> * getPuyoLevelThemeObjectList(void);

    void addPuyoSet(AnimatedPuyoSetTheme *);
    void addLevel(PuyoLevelTheme *);

private:
    AdvancedBuffer<const char *> themeList;
    AdvancedBuffer<const char *> puyoSetList;

    AdvancedBuffer<PuyoLevelTheme *> themes;
    AdvancedBuffer<AnimatedPuyoSetTheme *> puyoSets;

};

// Call the following accessor to get the global theme manager
AnimatedPuyoThemeManager * getPuyoThemeManger(void);
#endif


#endif // _ANIMATEDPUYOTHEME_H

