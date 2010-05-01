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

#include "gametools/drawcontext.h"
#include "iosfc/ios_memory.h"
#include "iosfc/ios_vector.h"
#include "PuyoCommander.h"
#include "PuyoGame.h"

#define NUMBER_OF_PUYOS 5

#define NUMBER_OF_PUYO_FACES 16
#define NUMBER_OF_PUYO_CIRCLES 32
#define NUMBER_OF_PUYO_EXPLOSIONS 4
#define NUMBER_OF_PUYO_DISAPPEAR 4
#define NUMBER_OF_PUYO_EYES 3

#define NUMBER_OF_LIVES 4

#define MAX_COMPRESSED 32

typedef enum {
    PUYO_FACES = 0,
    PUYO_CIRCLES = 1,
    PUYO_EXPLOSIONS = 2,
    PUYO_DISAPPEAR = 3,
    PUYO_EYES = 4,
    PUYO_SHADOWS = 5
} PuyoPictureType;



// Class containing the theme for a puyo
class AnimatedPuyoTheme {
public:
    virtual ~AnimatedPuyoTheme() {}
    virtual IosSurface *getSurface(PuyoPictureType picture, int index) = 0;
    virtual IosSurface *getShrunkSurface(PuyoPictureType picture, int index, int compression) = 0;
    virtual IosSurface *getPuyoSurfaceForValence(int valence, int compression = 0) = 0;
    virtual IosSurface *getEyeSurfaceForIndex(int index, int compression = 0) = 0;
    virtual IosSurface *getCircleSurfaceForIndex(int index, int compression = 0) = 0;
    virtual IosSurface *getShadowSurface(int compression = 0) = 0;
    virtual IosSurface *getShrinkingSurfaceForIndex(int index, int compression = 0) = 0;
    virtual IosSurface *getExplodingSurfaceForIndex(int index, int compression = 0) = 0;
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

#endif // _ANIMATEDPUYOTHEME_H

