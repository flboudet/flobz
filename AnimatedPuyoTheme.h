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

#include "IosImgProcess.h"
#include "PuyoGame.h"
#include <math.h>

class AnimatedPuyoTheme {
public:
    AnimatedPuyoTheme(const char *prefix, int hueShift, const char *eyePrefix);
    virtual ~AnimatedPuyoTheme();
    IIM_Surface *getPuyoSurfaceForValence(int valence) const;
    IIM_Surface *getEyeSurfaceForIndex(int index) const;
    IIM_Surface *getCircleSurfaceForIndex(int index) const;
    IIM_Surface *getShadowSurface() const;
    static IIM_Surface *loadWithPrefix(const char *prefix, const char *suffix);
private:
    IIM_Surface *puyoFaces[16];
    IIM_Surface *puyoEyes[3];
    IIM_Surface *puyoCircles[32];
    IIM_Surface *shadowSurface;
};

class AnimatedPuyoThemeManager {
public:
    AnimatedPuyoThemeManager();
    const AnimatedPuyoTheme *getThemeForState(PuyoState state) const;
private:
    AnimatedPuyoTheme puyoBlueTheme, puyoRedTheme, puyoGreenTheme, puyoVioletTheme, puyoYellowTheme;
};

#endif // _ANIMATEDPUYOTHEME_H

