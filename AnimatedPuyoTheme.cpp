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

#include "AnimatedPuyoTheme.h"

AnimatedPuyoThemeManager::AnimatedPuyoThemeManager()
    : puyoBlueTheme("p", -65, "eyev"), puyoRedTheme("g", 100, "eyev"),
    puyoGreenTheme("h", -150, "eye"), puyoVioletTheme("v", 0, "eye"),
    puyoYellowTheme("c", 140, "eye")
{
}

const AnimatedPuyoTheme *AnimatedPuyoThemeManager::getThemeForState(PuyoState state) const
{
    switch (state) {
        case PUYO_FALLINGBLUE:
        case PUYO_BLUE:
            return &puyoBlueTheme;
        case PUYO_FALLINGRED:
        case PUYO_RED:
            return &puyoRedTheme;
        case PUYO_FALLINGGREEN:
        case PUYO_GREEN:
            return &puyoGreenTheme;
        case PUYO_FALLINGVIOLET:
        case PUYO_VIOLET:
            return &puyoVioletTheme;
        case PUYO_FALLINGYELLOW:
        case PUYO_YELLOW:
        default:
            return &puyoYellowTheme;
    }
}

IIM_Surface *AnimatedPuyoTheme::loadWithPrefix(const char *prefix, const char *suffix)
{
    char temp[1024];
    sprintf(temp, "%s%s", prefix, suffix);
    return IIM_Load_DisplayFormatAlpha(temp);
}

AnimatedPuyoTheme::AnimatedPuyoTheme(const char *prefix, int hueShift, const char *eyePrefix)
{
    puyoFaces[0] = loadWithPrefix(prefix, "0.png");
    puyoFaces[1] = loadWithPrefix(prefix, "1a.png");
    puyoFaces[2] = loadWithPrefix(prefix, "1b.png");
    puyoFaces[3] = loadWithPrefix(prefix, "1c.png");
    puyoFaces[4] = loadWithPrefix(prefix, "1d.png");
    puyoFaces[5] = loadWithPrefix(prefix, "2ab.png");
    puyoFaces[6] = loadWithPrefix(prefix, "2ac.png");
    puyoFaces[7] = loadWithPrefix(prefix, "2ad.png");
    puyoFaces[8] = loadWithPrefix(prefix, "2bc.png");
    puyoFaces[9] = loadWithPrefix(prefix, "2bd.png");
    puyoFaces[10] = loadWithPrefix(prefix, "2cd.png");
    puyoFaces[11] = loadWithPrefix(prefix, "3abc.png");
    puyoFaces[12] = loadWithPrefix(prefix, "3abd.png");
    puyoFaces[13] = loadWithPrefix(prefix, "3acd.png");
    puyoFaces[14] = loadWithPrefix(prefix, "3bcd.png");
    puyoFaces[15] = loadWithPrefix(prefix, "4abcd.png");
    if (hueShift != 0) {
        for (int i = 0 ; i < 16 ; i++) {
            IIM_Surface *tmp = puyoFaces[i];
            puyoFaces[i] = iim_surface_shift_hue(tmp, hueShift);
            IIM_Free(tmp);
        }
    }
    puyoEyes[0] = loadWithPrefix(eyePrefix, "0.png");
    puyoEyes[1] = loadWithPrefix(eyePrefix, "1.png");
    puyoEyes[2] = loadWithPrefix(eyePrefix, "2.png");
    IIM_Surface *tmpCircle = loadWithPrefix(prefix, "circle.png");
    for (int i = 0 ; i < 32 ; i++) {
        puyoCircles[i] = iim_surface_set_value(tmpCircle, sin(3.14f/2.0f+i*3.14f/64.0f)*0.6f+0.2f);
    }
    IIM_Free(tmpCircle);
}

IIM_Surface *AnimatedPuyoTheme::getPuyoSurfaceForValence(int valence) const
{
    return puyoFaces[valence];
}

IIM_Surface *AnimatedPuyoTheme::getEyeSurfaceForIndex(int index) const
{
    return puyoEyes[index];
}

IIM_Surface *AnimatedPuyoTheme::getCircleSurfaceForIndex(int index) const
{
    return puyoCircles[index];
}

