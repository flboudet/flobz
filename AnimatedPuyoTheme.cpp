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
#include "preferences.h"

#include <stdio.h>
#include <strings.h>
#include <math.h>


#define PATH_MAX_LEN 256
#define NAME_MAX_LEN 256


static const char * defaultPuyosName = "Invaders";
static const char * defaultLevelName = "Kaori's farm";
static const char * preferedPuyosKey = "ThemeManager.Puyos";
static const char * preferedLevelKey = "ThemeManager.Level";


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
        return (fallback != NULL);
    }
    return (*dst != NULL);
}

static bool loadPictureWithOffset(const char * path, IIM_Surface ** dst, IIM_Surface * fallback, float offset)
{
    //fprintf(stderr,"Loading %s...\n",path);

    IIM_Surface *tmp = IIM_Load_DisplayFormatAlpha(path);
    if (tmp == NULL)
    {
        *dst = fallback;
        return (fallback != NULL);
    }
    *dst = iim_surface_shift_hue(tmp,offset);
    IIM_Free(tmp);
    return (*dst != NULL);
}

static bool copyPictureWithOffset(IIM_Surface * src, IIM_Surface ** dst, IIM_Surface * fallback, float offset)
{
    *dst = iim_surface_shift_hue(src,offset);
    return (*dst != NULL);
}

static bool copyPictureWithLuminosity(IIM_Surface * src, IIM_Surface ** dst, IIM_Surface * fallback, float lum)
{
    *dst = iim_surface_set_value(src, lum);
    return (*dst != NULL);
}


//*****************************************************************************************
//*********************************** AnimatedPuyoTheme ***********************************
//*****************************************************************************************


AnimatedPuyoTheme::AnimatedPuyoTheme(const String path, const char * face, const char * disappear, const char * explosions, const char * eyes, const float color_offset):_path(path)
{
    DEBUG_PARAM_MISSING(path,"path","AnimatedPuyoTheme")
    DEBUG_PARAM_MISSING(face,"face","AnimatedPuyoTheme")
    DEBUG_PARAM_MISSING(disappear,"disappear","AnimatedPuyoTheme")
    DEBUG_PARAM_MISSING(explosions,"explosions","AnimatedPuyoTheme")
    DEBUG_PARAM_MISSING(eyes,"eyes","AnimatedPuyoTheme")
    
    
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
    const char * fullPath = (const char *)_path;
    
    if (_cached) releaseCached();
    
    // PUYOS
    for (j=0; j<NUMBER_OF_PUYO_FACES; j++)
    {
        snprintf(path, sizeof(path), "%s/%s-puyo-%d%d%d%d.png",fullPath,_face,(j&8)>>3,(j&4)>>2,(j&2)>>1,j&1);
        OK = OK && loadPictureWithOffset(path,&(_puyoFaces[j]),NULL,_color_offset);
    }
    snprintf(path, sizeof(path), "%s/%s-puyo-border.png",fullPath,_face);
    OK = OK && loadPictureAt(path,&(_puyoCircles[0]),NULL);
    for (int i = 1; i<NUMBER_OF_PUYO_CIRCLES; i++)
    {
        OK = OK && copyPictureWithLuminosity(_puyoCircles[0],&(_puyoCircles[i]),NULL,sin(3.14f/2.0f+i*3.14f/64.0f)*0.6f+0.2f);
    }
    
    snprintf(path, sizeof(path), "%s/%s-puyo-shadow.png",fullPath,_face);
    OK = OK && loadPictureAt(path,&(_puyoShadow),NULL);
    
    for (j=0; j<NUMBER_OF_PUYO_EXPLOSIONS; j++)
    {
        snprintf(path, sizeof(path), "%s/%s-puyo-explosion-%d.png",fullPath,_explosions,j);
        OK = OK && loadPictureWithOffset(path,&(_puyoExplosion[j]),NULL,_color_offset);
    }
    
    for (j=0; j<NUMBER_OF_PUYO_DISAPPEAR; j++)
    {
        snprintf(path, sizeof(path), "%s/%s-puyo-disappear-%d.png",fullPath,_disappear,j);
        OK = OK && loadPictureWithOffset(path,&(_puyoDisappear[j]),NULL,_color_offset);
    }
    
    for (j=0; j<NUMBER_OF_PUYO_EYES; j++)
    {
        snprintf(path, sizeof(path), "%s/%s-puyo-eye-%d.png",fullPath,_eyes,j);
        OK = OK && loadPictureWithOffset(path,&(_puyoEyes[j]),NULL,_color_offset);
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

AnimatedPuyoSetTheme::AnimatedPuyoSetTheme(const String path, const String name):_path(path),_name(name)
{
    DEBUG_PARAM_MISSING(path,"path","AnimatedPuyoTheme")
    DEBUG_PARAM_MISSING(name,"name","AnimatedPuyoTheme")
    
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

    for (int i=0; i<_numberOfPuyos; i++)
    {
        delete _puyos[i];
    }
    if (_neutral != NULL) delete _neutral;
}

const String AnimatedPuyoSetTheme::getName(void)
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


bool AnimatedPuyoSetTheme::addAnimatedPuyoTheme(const String face, const char * disappear, const char * explosions, const char * eyes, const float color_offset)
{ 
    if (_numberOfPuyos >= NUMBER_OF_PUYOS)
    {
        printf("Too many puyos in theme %s... Ignoring puyo.\n",(const char *)_name);
        return false;
    }
    
    // add puyo
    _puyos[_numberOfPuyos] = new AnimatedPuyoTheme(_path, face, disappear, explosions, eyes, color_offset);
    if (_puyos[_numberOfPuyos]!=NULL) _numberOfPuyos++;
    return true;
}

bool AnimatedPuyoSetTheme::addNeutralPuyo(const String face, const char * disappear, const char * explosions, const char * eyes, const float color_offset)
{ 
    if (_neutral != NULL)
    {
        printf("Too many neutral puyos in theme %s... Ignoring puyo.\n",(const char *)_name);
        return false;
    }

    // add puyo
    _neutral=new AnimatedPuyoTheme(_path, face, disappear, explosions, eyes, color_offset);
    return (_neutral!=NULL);
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
        retour = retour && _puyos[i]->cache();
    }
    if (_neutral != NULL) retour = retour && _neutral->cache();
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

PuyoLevelTheme::PuyoLevelTheme(const String path, const String name):_path(path),_name(name)
{ 
    DEBUG_PARAM_MISSING(path,"path","PuyoLevelTheme")
    DEBUG_PARAM_MISSING(name,"name","PuyoLevelTheme")
    
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

    if (_lives != NULL) free(_lives);
    if (_background != NULL) free(_background);
    if (_grid != NULL) free(_grid);
    if (_speed_meter != NULL) free(_speed_meter);
    
    releaseCached();
}

const String PuyoLevelTheme::getName(void)
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
    //NOT_IMPLEMENTED;
    //fprintf(stderr,"Malformed theme : %s", name);
    return true;//false;
}

bool PuyoLevelTheme::cache(void)
{
    bool OK = true;
    int i;
    
    char path[PATH_MAX_LEN];
    const char * fullPath = (const char *)_path;

    if (_cached) releaseCached();
    _cached = false;
    
    if (validate() == false) return false;
    
    // LIVES
    for (i=0; i<NUMBER_OF_LIVES; i++)
    {
        snprintf(path, sizeof(path), "%s/%s-lives-%d.png",fullPath,_lives,i);
        OK = OK && loadPictureAt(path,&(_levelLives[i]),NULL);
    }
    
    // BACKGROUND
    snprintf(path, sizeof(path), "%s/%s-background.jpg",fullPath,_background);
    OK = OK && loadPictureAt(path,&_levelBackground,NULL);
    
    // GRID
    snprintf(path, sizeof(path), "%s/%s-background-grid.png",fullPath,_grid);
    OK = OK && loadPictureAt(path,&_levelGrid,NULL);
    
    // SPEED METER
    snprintf(path, sizeof(path), "%s/%s-background-meter-below.png",fullPath,_speed_meter);
    OK = OK && loadPictureAt(path,&(_levelMeter[0]),NULL);
    snprintf(path, sizeof(path), "%s/%s-background-meter-above.png",fullPath,_speed_meter);
    OK = OK && loadPictureAt(path,&(_levelMeter[1]),NULL);
    
    _cached = OK;
    return OK;
}



//**************************************************************************************
//*********************************** Theme Manager ************************************
//**************************************************************************************

static AnimatedPuyoThemeManager globalManager("../../data/gfx/","../../");

AnimatedPuyoThemeManager * getPuyoThemeManger(void) {return &globalManager; }


AnimatedPuyoThemeManager::AnimatedPuyoThemeManager(const char * path, const char * alternatePath)
{
    String fullPath(path);
    fullPath += "Classic.fptheme";

    _currentPuyoSetTheme = new AnimatedPuyoSetTheme(fullPath, defaultPuyosName);
    _currentPuyoSetTheme->addAnimatedPuyoTheme("stone", "round", "round", "normal", 000.0f);
    _currentPuyoSetTheme->addAnimatedPuyoTheme("stone", "round", "round", "normal", 072.0f);
    _currentPuyoSetTheme->addAnimatedPuyoTheme("stone", "round", "round", "normal", 144.0f);
    _currentPuyoSetTheme->addAnimatedPuyoTheme("stone", "round", "round", "normal", 216.0f);
    _currentPuyoSetTheme->addAnimatedPuyoTheme("stone", "round", "round", "normal", 288.0f);
    _currentPuyoSetTheme->addNeutralPuyo("stone", "round", "round", "normal", 0.0f);
    
    _currentLevel = new PuyoLevelTheme(fullPath, defaultLevelName);
    _currentLevel->setLives("heart");
    _currentLevel->setBackground("dark");
    _currentLevel->setGrid("metal");
    _currentLevel->setSpeedMeter("fire");
    
    puyoSets.add(_currentPuyoSetTheme);
    puyoSetList.add(_currentPuyoSetTheme->getName());
    themes.add(_currentLevel);
    themeList.add(_currentLevel->getName());
}

AnimatedPuyoThemeManager::~AnimatedPuyoThemeManager(void)
{
    delete _currentPuyoSetTheme;
    delete _currentLevel;
}
    
AnimatedPuyoSetTheme * AnimatedPuyoThemeManager::getAnimatedPuyoSetTheme(const String name)
{
    int size = puyoSets.size();
    
    for (int i = 0; i < size; i++)
    {
        if (puyoSets[i]->getName() == name) return puyoSets[i];
    }

    if (name != defaultPuyosName)
    {
        fprintf(stderr, "Puyos theme \"%s\" not found, falling back to default...\n",(const char *)name);
        return getAnimatedPuyoSetTheme(String(defaultPuyosName));
    }
    else
    {
        if (size != 0)
        {
            fprintf(stderr, "Default puyos theme (%s) not found??? Trying another one...\n",defaultPuyosName);
            return puyoSets[0];
        }
        else
        {
            fprintf(stderr, "Default puyos theme (%s) not found??? No possible fallback, Exiting...\n",defaultPuyosName);
            exit(0);
        }
    }
}

AnimatedPuyoSetTheme * AnimatedPuyoThemeManager::getAnimatedPuyoSetTheme(void)
{
    char out[NAME_MAX_LEN];
    GetStrPreference (preferedPuyosKey, out, defaultPuyosName, sizeof(out));
    return getAnimatedPuyoSetTheme(String(out));
}

void AnimatedPuyoThemeManager::setPreferedAnimatedPuyoSetTheme(const String name)
{
    SetStrPreference (preferedPuyosKey, (const char *)name);
}

PuyoLevelTheme * AnimatedPuyoThemeManager::getPuyoLevelTheme(const String name)
{
    int size = themes.size();
    
    for (int i = 0; i < size; i++)
    {
        if (themes[i]->getName() == name) return themes[i];
    }
    
    if (name != defaultLevelName)
    {
        fprintf(stderr, "Level theme \"%s\" not found, falling back to default...\n",(const char *)name);
        return getPuyoLevelTheme(String(defaultLevelName));
    }
    else
    {
        if (size != 0)
        {
            fprintf(stderr, "Default level theme (%s) not found??? Trying another one...\n",defaultLevelName);
            return themes[0];
        }
        else
        {
            fprintf(stderr, "Default level theme (%s) not found??? No possible fallback, Exiting...\n",defaultLevelName);
            exit(0);
        }
    }
}

PuyoLevelTheme * AnimatedPuyoThemeManager::getPuyoLevelTheme(void)
{
    char out[NAME_MAX_LEN];
    GetStrPreference (preferedLevelKey, out, defaultLevelName, sizeof(out));
    return getPuyoLevelTheme(String(out));
}

void AnimatedPuyoThemeManager::setPreferedPuyoLevelTheme(const String name)
{
    SetStrPreference (preferedLevelKey, (const char *)name);
}

AdvancedBuffer<const char *> AnimatedPuyoThemeManager::getAnimatedPuyoSetThemeList(void)
{
    return puyoSetList;
}

AdvancedBuffer<const char *> AnimatedPuyoThemeManager::getPuyoLevelThemeList(void)
{
    return themeList;
}

