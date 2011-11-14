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

 #ifndef _THEME_H
 #define _THEME_H

#include <string>
#include <vector>
#include "drawcontext.h"
#include "FloboGame.h"

// Theme Interfaces

#define NUMBER_OF_FLOBO_FACES 16
#define NUMBER_OF_FLOBO_CIRCLES 32
#define NUMBER_OF_FLOBO_EXPLOSIONS 4
#define NUMBER_OF_FLOBO_DISAPPEAR 4
#define NUMBER_OF_FLOBO_EYES 3

#define NUMBER_OF_PUYOS 5

#define NUMBER_OF_PUYOS_IN_SET 6
#define NUMBER_OF_PUYOBANS_IN_LEVEL 2

#define NUMBER_OF_LIVES 4
#define MAX_COMPRESSED 32

/**
 * The ThemeDescriptionInterface provides informations
 * about a theme.
 */
class ThemeDescriptionInterface {
public:
    virtual const std::string & getName() const = 0;
    virtual const std::string & getLocalizedName() const = 0;
    virtual const std::string & getAuthor() const = 0;
    virtual const std::string & getComments() const = 0;
};

/**
 * The PuyoTheme interface represents the theme for a single puyo style
 */
class PuyoTheme {
public:
    virtual ~PuyoTheme() {}
    virtual IosSurface *getPuyoSurfaceForValence(int valence, int compression = 0) const = 0;
    virtual IosSurface *getEyeSurfaceForIndex(int index, int compression = 0) const = 0;
    virtual IosSurface *getCircleSurfaceForIndex(int index) const = 0;
    virtual IosSurface *getShadowSurface(int compression = 0) const = 0;
    virtual IosSurface *getShrinkingSurfaceForIndex(int index) const = 0;
    virtual IosSurface *getExplodingSurfaceForIndex(int index) const = 0;
protected:
    PuyoTheme() {}
private:
    PuyoTheme(const PuyoTheme &theme) {}
};

/**
 * The PuyoSetTheme interface represents a full-featured collection
 * of PuyoThemes
 */
class PuyoSetTheme : public ThemeDescriptionInterface {
public:
    virtual ~PuyoSetTheme(void) {}
    // Accessor to the PuyoThemes
    virtual const PuyoTheme & getPuyoTheme(FloboState state) const = 0;
};

/**
 * The LevelTheme interface represents the theme for a Level (aka Puyoban)
 */
class LevelTheme : public ThemeDescriptionInterface {
public:
    virtual ~LevelTheme() {}
    // Surfaces
    virtual IosSurface * getLifeForIndex(int index) const = 0;
    virtual IosSurface * getBackground(void) const = 0;
    virtual IosSurface * getGrid(void) const = 0;
    virtual IosSurface * getSpeedMeter(bool front) const = 0;
    virtual IosSurface * getNeutralIndicator() const = 0;
    virtual IosSurface * getBigNeutralIndicator() const = 0;
    virtual IosSurface * getGiantNeutralIndicator() const = 0;
    // Fonts
    virtual IosFont *getPlayerNameFont() const = 0;
    virtual IosFont *getScoreFont() const      = 0;
    // Positions and dimensions of the Puyoban
    virtual int getSpeedMeterX() const  = 0;
    virtual int getSpeedMeterY() const  = 0;
    virtual int getLifeDisplayX() const = 0;
    virtual int getLifeDisplayY() const = 0;
    virtual int getPuyobanX(int playerId) const = 0;
    virtual int getPuyobanY(int playerId) const = 0;
    virtual int getNextFlobosX(int playerId) const = 0;
    virtual int getNextFlobosY(int playerId) const = 0;
    virtual int getNeutralDisplayX(int playerId) const = 0;
    virtual int getNeutralDisplayY(int playerId) const = 0;
    virtual int getNameDisplayX(int playerId) const    = 0;
    virtual int getNameDisplayY(int playerId) const    = 0;
    virtual int getScoreDisplayX(int playerId) const   = 0;
    virtual int getScoreDisplayY(int playerId) const   = 0;
    virtual float getPuyobanScale(int playerId) const  = 0;
    // Behaviour of the Puyoban
    virtual bool getShouldDisplayNext(int playerId) const    = 0;
    virtual bool getShouldDisplayShadows(int playerId) const = 0;
    virtual bool getShouldDisplayEyes(int playerId) const    = 0;
    virtual bool getOpponentIsBehind() const   = 0;
    // Stats widget dimensions
    virtual int getStatsHeight() const = 0;
    virtual int getStatsLegendWidth() const = 0;
    virtual int getStatsComboLineValueWidth() const = 0;
    virtual int getStatsLeftBackgroundOffsetX() const = 0;
    virtual int getStatsLeftBackgroundOffsetY() const = 0;
    virtual int getStatsRightBackgroundOffsetX() const = 0;
    virtual int getStatsRightBackgroundOffsetY() const = 0;
    // Animation names
    virtual const std::string getGameLostLeftAnimation2P() const  = 0;
    virtual const std::string getGameLostRightAnimation2P() const = 0;
    virtual const std::string getCentralAnimation2P() const  = 0;
    virtual const std::string getForegroundAnimation() const = 0;
    virtual const std::string getReadyAnimation2P() const    = 0;
    // Misc
    virtual const std::string getThemeRootPath() const = 0;
};

class ThemeManager {
public:
    virtual ~ ThemeManager() {}
    virtual PuyoSetTheme * createFloboSetTheme(const std::string &themeName) = 0;
    virtual LevelTheme   * createLevelTheme(const std::string &themeName) = 0;

    virtual const std::vector<std::string> & getPuyoSetThemeList() = 0;
    virtual const std::vector<std::string> & getLevelThemeList() = 0;
};

#endif // _THEME_H

