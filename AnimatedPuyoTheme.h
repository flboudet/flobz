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

#include "gametools/IosImgProcess.h"
#include "iosfc/ios_memory.h"
#include "PuyoGame.h"

#define NUMBER_OF_PUYOS 5

#define NUMBER_OF_PUYO_FACES 16
#define NUMBER_OF_PUYO_CIRCLES 32
#define NUMBER_OF_PUYO_EXPLOSIONS 4
#define NUMBER_OF_PUYO_DISAPPEAR 4
#define NUMBER_OF_PUYO_EYES 3

#define NUMBER_OF_LIVES 4

typedef enum {
    PUYO_FACES = 0,
    PUYO_CIRCLES = 1,
    PUYO_EXPLOSIONS = 2,
    PUYO_DISAPPEAR = 3,
    PUYO_EYES = 4,
    PUYO_SHADOWS = 5
} PuyoPictureType;




class AnimatedPuyoTheme {

public:
    AnimatedPuyoTheme(const char * path, const char * face, const char * disappear, const char * explosions, const char * eyes, const float color_offset);
    ~AnimatedPuyoTheme(void);
    
    IIM_Surface * getSurface(PuyoPictureType picture, int index);

    IIM_Surface *getPuyoSurfaceForValence(int valence) { return getSurface(PUYO_FACES,valence); };
    IIM_Surface *getEyeSurfaceForIndex(int index) { return getSurface(PUYO_EYES,index); };
    IIM_Surface *getCircleSurfaceForIndex(int index) { return getSurface(PUYO_CIRCLES,index); };
    IIM_Surface *getShadowSurface() { return getSurface(PUYO_SHADOWS,0); };
    IIM_Surface *getShrinkingSurfaceForIndex(int index) { return getSurface(PUYO_DISAPPEAR,index); };
    IIM_Surface *getExplodingSurfaceForIndex(int index) { return getSurface(PUYO_EXPLOSIONS,index); };

    bool validate(void);
    bool cache(void);
    void releaseCached(void);

private:
    PuyoPictureType _type;
        
    char * _path;
    char * _face;
    char * _disappear;
    char * _explosions;
    char * _eyes;
    float _color_offset;
    
    IIM_Surface * _puyoFaces[NUMBER_OF_PUYO_FACES];
    IIM_Surface * _puyoCircles[NUMBER_OF_PUYO_CIRCLES];
    IIM_Surface * _puyoShadow;
    IIM_Surface * _puyoExplosion[NUMBER_OF_PUYO_EXPLOSIONS];
    IIM_Surface * _puyoDisappear[NUMBER_OF_PUYO_DISAPPEAR];
    IIM_Surface * _puyoEyes[NUMBER_OF_PUYO_EYES];

    bool _cached;
};


class AnimatedPuyoSetTheme {
  
public:
    AnimatedPuyoSetTheme(const char * path, const char * name);
    ~AnimatedPuyoSetTheme(void);
    
    const char * getName(void);
    
    bool addAnimatedPuyoTheme(const char * face, const char * disappear, const char * explosions, const char * eyes, const float color_offset);
    bool addNeutralPuyo(const char * face, const char * disappear, const char * explosions, const char * eyes, const float color_offset);
    
    AnimatedPuyoTheme * getAnimatedPuyoTheme(PuyoState state);
    
    bool validate(void);
    bool cache(void);
    void releaseCached(void);

private:
    char * _path;
    char * _name;
    
    AnimatedPuyoTheme * _puyos[NUMBER_OF_PUYOS];
    AnimatedPuyoTheme * _neutral;

    unsigned int _numberOfPuyos;
};


class PuyoLevelTheme {
    
public:
    PuyoLevelTheme(const char * path, const char * name);
    ~PuyoLevelTheme(void);
    
    const char * getName(void);
    
    bool setLives(const char * lives);
    bool setBackground(const char * back);
    bool setGrid(const char * grid);
    bool setSpeedMeter(const char * speedmeter);
    
    IIM_Surface * getLifeForIndex(int index);
    IIM_Surface * getBackground(void);
    IIM_Surface * getGrid(void);
    IIM_Surface * getSpeedMeter(bool front);
    
private:    
        char * _path;
    char * _name;
    
    char * _lives;
    char * _background;
    char * _grid;
    char * _speed_meter;
    
    IIM_Surface * _levelLives[NUMBER_OF_LIVES];
    IIM_Surface * _levelBackground;
    IIM_Surface * _levelGrid;
    IIM_Surface * _levelMeter[2];
    
    bool _cached;
    
    bool validate(void);
    bool cache(void);
    void releaseCached(void);
    
};


#endif // _ANIMATEDPUYOTHEME_H

