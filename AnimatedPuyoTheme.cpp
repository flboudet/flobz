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

#include <stdio.h>
#include <strings.h>
#include <math.h>


#define PATH_MAX_LEN 250

//*********************************************************************************
//*********************************** Utilities ***********************************
//*********************************************************************************

#define NOT_IMPLEMENTED { fprintf(stderr,"Not Implemented __FILE__ __LINE__"); }
#define CACHE_IT_OR_DIE { if (_cached == false) { if (cache() == false) exit(0); } }
#define LOG { fprintf(stderr,"Logged __FILE__ __LINE__"); }

#ifdef DEBUG
#define DEBUG_PARAM_MISSING(p,n,o) { if (p==NULL) fprintf(stderr,"Parameter %s missing in %s __FILE__ __LINE__",n,o); exit(0); }
#define ASSERT_RANGE(min,max,value) { if ((value < min) || (value > max)) { fprintf(stderr, "Value out of range __FILE__ __LINE__"); exit(0); } }
#else
#define DEBUG_PARAM_MISSING(p,n,o) { }
#define ASSERT_RANGE(min,max,value) { }
#endif

static bool loadPictureAt(const char * path, IIM_Surface ** dst, IIM_Surface * fallback)
{
    *dst = IIM_Load_DisplayFormatAlpha(path);
    if (*dst == NULL)
    {
        *dst = fallback;
        return false;
    }
    return true;
}

static bool loadPictureWithOffset(const char * path, IIM_Surface ** dst, IIM_Surface * fallback, float offset)
{
    fprintf(stderr,"Loading %s...\n",path);

    IIM_Surface *tmp = IIM_Load_DisplayFormatAlpha(path);
    if (tmp == NULL)
    {
        *dst = fallback;
        return false;
    }
    *dst = iim_surface_shift_hue(tmp,offset);
    IIM_Free(tmp);
    return true;
}

static bool copyPictureWithOffset(IIM_Surface * src, IIM_Surface ** dst, IIM_Surface * fallback, float offset)
{
    *dst = iim_surface_shift_hue(src,offset);
    return true;
}

static bool copyPictureWithLuminosity(IIM_Surface * src, IIM_Surface ** dst, IIM_Surface * fallback, float lum)
{
    *dst = iim_surface_set_value(src, lum);
    return true;
}


//*****************************************************************************************
//*********************************** AnimatedPuyoTheme ***********************************
//*****************************************************************************************


AnimatedPuyoTheme::AnimatedPuyoTheme(const char * path, const char * face, const char * disappear, const char * explosions, const char * eyes, const float color_offset)
{
    DEBUG_PARAM_MISSING(path,"path","AnimatedPuyoTheme")
    DEBUG_PARAM_MISSING(face,"face","AnimatedPuyoTheme")
    DEBUG_PARAM_MISSING(disappear,"disappear","AnimatedPuyoTheme")
    DEBUG_PARAM_MISSING(explosions,"explosions","AnimatedPuyoTheme")
    DEBUG_PARAM_MISSING(eyes,"eyes","AnimatedPuyoTheme")
    
    
    _path = strdup(path);
    
    _face = strdup(face);
    _disappear = strdup(disappear);
    _explosions = strdup(explosions);
    _eyes = strdup(eyes);
    _color_offset = color_offset;
    
    _cached = false;
    
#ifdef DEBUG
    counter = 0;
#endif
}


AnimatedPuyoTheme::~AnimatedPuyoTheme(void)
{
#ifdef DEBUG
    if (counter > 0) fprintf(stderr,"AnimatedPuyoTheme released while in use !!!");
#endif

    free(_path);

    if (_face != NULL) free(_face);
    if (_disappear != NULL) free(_disappear);
    if (_explosions != NULL) free(_explosions);
    if (_eyes != NULL) free(_eyes);
    
    releaseCached();
}

IIM_Surface * AnimatedPuyoTheme::getSurface(PuyoPictureType picture, int index)
{
    
#ifdef DEBUG
    ASSERT_RANGE(PUYO_FACES,PUYO_SHADOWS,picture);
    int max;
    switch (picture)
    {
        case PUYO_FACES:
            max = NUMBER_OF_PUYO_FACES;
            break;
        case PUYO_CIRCLES:
            max = NUMBER_OF_PUYO_CIRCLES;
            break;
        case PUYO_EXPLOSIONS:
            max = NUMBER_OF_PUYO_EXPLOSIONS;
            break;
        case PUYO_DISAPPEAR:
            max = NUMBER_OF_PUYO_DISAPPEAR;
            break;
        case PUYO_EYES:
            max = NUMBER_OF_PUYO_EYES;
            break;
        case PUYO_SHADOWS:
            max = 1;
            break;
        default:
            max = 0;
            break;
    }
    ASSERT_RANGE(0,max-1,index);
#endif
    
    if (_cached == false) cache();
    
    switch (picture)
    {
        case PUYO_FACES:
            return _puyoFaces[index];
            break;
        case PUYO_CIRCLES:
            return _puyoCircles[index];
            break;
        case PUYO_EXPLOSIONS:
            return _puyoExplosion[index];
            break;
        case PUYO_DISAPPEAR:
            return _puyoDisappear[index];
            break;
        case PUYO_EYES:
            return _puyoEyes[index];
            break;
        case PUYO_SHADOWS:
            return _puyoShadow;
            break;
        default:
            break;
    }
    return NULL;
}


void AnimatedPuyoTheme::releaseCached(void)
{
    unsigned int j;
    
        for (j=0; j<NUMBER_OF_PUYO_FACES; j++)
        {
            if (_puyoFaces[j] != NULL) IIM_Free(_puyoFaces[j]);
            _puyoFaces[j] = NULL;
        }
        
        for (j=0; j<NUMBER_OF_PUYO_CIRCLES; j++)
        {
            if (_puyoCircles[j] != NULL) IIM_Free(_puyoCircles[j]);
            _puyoCircles[j] = NULL;
        }
        
        if (_puyoShadow != NULL) IIM_Free(_puyoShadow);
        _puyoShadow = NULL;
        
        for (j=0; j<NUMBER_OF_PUYO_EXPLOSIONS; j++)
        {
            if (_puyoExplosion[j] != NULL) IIM_Free(_puyoExplosion[j]);
            _puyoExplosion[j] = NULL;
        }
        
        for (j=0; j<NUMBER_OF_PUYO_DISAPPEAR; j++)
        {
            if (_puyoDisappear[j] != NULL) IIM_Free(_puyoDisappear[j]);
            _puyoDisappear[j] = NULL;
        }
        
        for (j=0; j<NUMBER_OF_PUYO_EYES; j++)
        {
            if (_puyoEyes[j] != NULL) IIM_Free(_puyoEyes[j]);
            _puyoEyes[j] = NULL;
        }
        _cached = false;
}



bool AnimatedPuyoTheme::cache(void)
{
    bool OK = true;
    int j;
    char path[PATH_MAX_LEN];
    
    if (_cached) releaseCached();
    
    // PUYOS
    for (j=0; j<NUMBER_OF_PUYO_FACES; j++)
    {
        snprintf(path, sizeof(path), "%s/%s-puyo-%d%d%d%d.png",_path,_face,(j&8)>>3,(j&4)>>2,(j&2)>>1,j&1);
        OK |= loadPictureWithOffset(path,&(_puyoFaces[j]),NULL,_color_offset);
    }
    snprintf(path, sizeof(path), "%s/%s-puyo-border.png",_path,_face);
    OK |= loadPictureAt(path,&(_puyoCircles[0]),NULL);
    for (int i = 1; i<NUMBER_OF_PUYO_CIRCLES; i++)
    {
        OK |= copyPictureWithLuminosity(_puyoCircles[0],&(_puyoCircles[i]),NULL,sin(3.14f/2.0f+i*3.14f/64.0f)*0.6f+0.2f);
    }
    
    snprintf(path, sizeof(path), "%s/%s-puyo-shadow.png",_path,_face);
    OK |= loadPictureAt(path,&(_puyoShadow),NULL);
    
    for (j=0; j<NUMBER_OF_PUYO_EXPLOSIONS; j++)
    {
        snprintf(path, sizeof(path), "%s/%s-puyo-explosion-%d.png",_path,_explosions,j);
        OK |= loadPictureWithOffset(path,&(_puyoExplosion[j]),NULL,_color_offset);
    }
    
    for (j=0; j<NUMBER_OF_PUYO_DISAPPEAR; j++)
    {
        snprintf(path, sizeof(path), "%s/%s-puyo-disappear-%d.png",_path,_disappear,j);
        OK |= loadPictureWithOffset(path,&(_puyoDisappear[j]),NULL,_color_offset);
    }
    
    for (j=0; j<NUMBER_OF_PUYO_EYES; j++)
    {
        snprintf(path, sizeof(path), "%s/%s-puyo-eye-%d.png",_path,_eyes,j);
        OK |= loadPictureWithOffset(path,&(_puyoEyes[j]),NULL,_color_offset);
    }
    
    _cached = OK;
    
    return OK;
}

#ifdef DEBUG
void AnimatedPuyoTheme::retain(void) { counter++; }
void AnimatedPuyoTheme::release(void)
{
    if (counter == 0) fprintf(stderr,"AnimatedPuyoTheme released while not retained");
    else counter--;
}
#endif




//********************************************************************************************
//*********************************** AnimatedPuyoSetTheme ***********************************
//********************************************************************************************

AnimatedPuyoSetTheme::AnimatedPuyoSetTheme(const char * path, const char * name)
{
    DEBUG_PARAM_MISSING(path,"path","AnimatedPuyoTheme")
    DEBUG_PARAM_MISSING(name,"name","AnimatedPuyoTheme")
    
    _path = strdup(path);
    _name = strdup(name);
    
    _numberOfPuyos = 0;
    _neutral = NULL;
#ifdef DEBUG
    counter = 0;
#endif
}


AnimatedPuyoSetTheme::~AnimatedPuyoSetTheme(void)
{
#ifdef DEBUG
    if (counter > 0) fprintf(stderr,"AnimatedPuyoSetTheme released while in use !!!");
#endif

    free(_path);
    free(_name);
    
    for (int i=0; i<_numberOfPuyos; i++)
    {
        delete _puyos[i];
    }
    if (_neutral != NULL) delete _neutral;
}

const char * AnimatedPuyoSetTheme::getName(void)
{
    return _name;
}

AnimatedPuyoTheme * AnimatedPuyoSetTheme::getAnimatedPuyoTheme(PuyoState state)
{
    //ASSERT_RANGE(PUYO_BLUE,PUYO_NEUTRAL,state);
    
    switch (state) {
        case PUYO_FALLINGBLUE:
        case PUYO_BLUE:
            return _puyos[0];
        case PUYO_FALLINGRED:
        case PUYO_RED:
            return _puyos[1];
        case PUYO_FALLINGGREEN:
        case PUYO_GREEN:
            return _puyos[2];
        case PUYO_FALLINGVIOLET:
        case PUYO_VIOLET:
            return _puyos[3];
        case PUYO_FALLINGYELLOW:
        case PUYO_YELLOW:
            return _puyos[4];
        default:
            return _neutral;
    }
    
    
    if (state == PUYO_NEUTRAL) return _neutral;
    if ((state >= PUYO_BLUE) && (state < PUYO_NEUTRAL) && (state-PUYO_BLUE < _numberOfPuyos))
        return _puyos[state-PUYO_BLUE];
    else return _puyos[0];
}

void AnimatedPuyoSetTheme::releaseCached(void)
{
    for (int i=0; i<_numberOfPuyos; i++)
    {
        _puyos[i]->releaseCached();
    }
    if (_neutral != NULL) _neutral->releaseCached();
}


bool AnimatedPuyoSetTheme::addAnimatedPuyoTheme(const char * face, const char * disappear, const char * explosions, const char * eyes, const float color_offset)
{ 
    if (_numberOfPuyos >= NUMBER_OF_PUYOS)
    {
        printf("Too many puyos in theme %s... Ignoring puyo.",_name);
        return false;
    }
    
    // add puyo
    String fullName(_path);
    fullName += _name;
    _puyos[_numberOfPuyos] = new AnimatedPuyoTheme(fullName, face, disappear, explosions, eyes, color_offset);
    if (_puyos[_numberOfPuyos]!=NULL) _numberOfPuyos++;
    return true;
}

bool AnimatedPuyoSetTheme::addNeutralPuyo(const char * face, const char * disappear, const char * explosions, const char * eyes, const float color_offset)
{ 
    if (_neutral != NULL)
    {
        printf("Too many neutral puyos in theme %s... Ignoring puyo.",_name);
        return false;
    }

    // add puyo
    _neutral=new AnimatedPuyoTheme(_path, face, disappear, explosions, eyes, color_offset);
    return true;
}

bool AnimatedPuyoSetTheme::validate(void)
{
    NOT_IMPLEMENTED;
    return false;
}

bool AnimatedPuyoSetTheme::cache(void)
{
    bool retour = false;
    for (int i=0; i<_numberOfPuyos; i++)
    {
        retour |= _puyos[i]->cache();
    }
    if (_neutral != NULL) retour |= _neutral->cache();
    return retour;
}

#ifdef DEBUG
void AnimatedPuyoSetTheme::retain(void) { counter++; }
void AnimatedPuyoSetTheme::release(void)
{
    if (counter == 0) fprintf(stderr,"AnimatedPuyoSetTheme released while not retained");
    else counter--;
}
#endif

//**************************************************************************************
//*********************************** PuyoLevelTheme ***********************************
//**************************************************************************************

PuyoLevelTheme::PuyoLevelTheme(const char * path, const char * name)
{ 
    DEBUG_PARAM_MISSING(path,"path","PuyoLevelTheme")
    DEBUG_PARAM_MISSING(name,"name","PuyoLevelTheme")
    
    _path = strdup(path);
    _name = strdup(name);
    
    _lives = NULL;
    _background = NULL;
    _grid = NULL;
    _speed_meter = NULL;
    
    _cached = false;
    
#ifdef DEBUG
    counter = 0;
#endif
}

PuyoLevelTheme::~PuyoLevelTheme(void)
{
#ifdef DEBUG
    if (counter > 0) fprintf(stderr,"PuyoLevelTheme released while used !!!");
#endif
    
    free(_path);
    free(_name);
    
    if (_lives != NULL) free(_lives);
    if (_background != NULL) free(_background);
    if (_grid != NULL) free(_grid);
    if (_speed_meter != NULL) free(_speed_meter);
    
    releaseCached();
}

const char * PuyoLevelTheme::getName(void)
{
    return _name;
}

void PuyoLevelTheme::releaseCached(void)
{    
    unsigned int i;
    
    for (i=0; i<NUMBER_OF_LIVES; i++)
    {
        if (_levelLives[i] != NULL) IIM_Free(_levelLives[i]);
        _levelLives[i] = NULL;
    }
    if (_levelBackground != NULL) IIM_Free(_levelBackground);
    _levelBackground = NULL;
    if (_levelGrid != NULL) IIM_Free(_levelGrid);
    _levelGrid = NULL;
    if (_levelMeter[0] != NULL) IIM_Free(_levelMeter[0]);
    _levelMeter[0] = NULL;
    if (_levelMeter[1] != NULL) IIM_Free(_levelMeter[1]);
    _levelMeter[1] = NULL;
    
    _cached = false;
}


bool PuyoLevelTheme::setLives(const char * lives)
{ 
    DEBUG_PARAM_MISSING(lives,"lives","PuyoLevelTheme")
    
    _lives = strdup(lives);
    return true;
}

bool PuyoLevelTheme::setBackground(const char * back)
{ 
    DEBUG_PARAM_MISSING(back,"back","PuyoLevelTheme")
    
    _background = strdup(back);
    return true;
}

bool PuyoLevelTheme::setGrid(const char * grid)
{ 
    DEBUG_PARAM_MISSING(grid,"grid","PuyoLevelTheme")
    
    _grid = strdup(grid);
    return true;
}

bool PuyoLevelTheme::setSpeedMeter(const char * speedmeter)
{ 
    DEBUG_PARAM_MISSING(speedmeter,"speedmeter","PuyoLevelTheme")
    
    _speed_meter = strdup(speedmeter);
    return true;
}

IIM_Surface * PuyoLevelTheme::getLifeForIndex(int index)
{ 
    ASSERT_RANGE(0,NUMBER_OF_LIVES-1,index);
    CACHE_IT_OR_DIE
        
        return _levelLives[index];
}

IIM_Surface * PuyoLevelTheme::getBackground(void)
{ 
    CACHE_IT_OR_DIE
    
    return _levelBackground;
}

IIM_Surface * PuyoLevelTheme::getGrid(void)
{ 
    CACHE_IT_OR_DIE
    
    return _levelGrid;
}

IIM_Surface * PuyoLevelTheme::getSpeedMeter(bool front)
{
    CACHE_IT_OR_DIE
    return _levelMeter[front?1:0];
}

bool PuyoLevelTheme::validate(void)
{
    NOT_IMPLEMENTED;
    //fprintf(stderr,"Malformed theme : %s", name);
    return false;
}

bool PuyoLevelTheme::cache(void)
{
    bool OK = true;
    int i;
    char path[PATH_MAX_LEN];
    
    if (_cached) releaseCached();
    _cached = false;
    
    if (validate() == false) return false;
    
    // LIVES
    for (i=0; i<NUMBER_OF_LIVES; i++)
    {
        snprintf(path, sizeof(path), "%s/%s-lives-%d.png",_path,_lives,i);
        OK |= loadPictureAt(path,&(_levelLives[i]),NULL);
    }
    
    // BACKGROUND
    snprintf(path, sizeof(path), "%s/%s-background.png",_path,_background);
    OK |= loadPictureAt(path,&_levelBackground,NULL);
    
    // GRID
    snprintf(path, sizeof(path), "%s/%s-background-grid.png",_path,_grid);
    OK |= loadPictureAt(path,&_levelGrid,NULL);
    
    // SPEED METER
    snprintf(path, sizeof(path), "%s/%s-background-meter-below.png",_path,_speed_meter);
    OK |= loadPictureAt(path,&(_levelMeter[0]),NULL);
    snprintf(path, sizeof(path), "%s/%s-background-meter-above.png",_path,_speed_meter);
    OK |= loadPictureAt(path,&(_levelMeter[1]),NULL);
    
    _cached = OK;
    
    return OK;
}


