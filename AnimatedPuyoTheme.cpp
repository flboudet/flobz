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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stddef.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include "AnimatedPuyoTheme.h"
#include "PuyoCommander.h"
#include "preferences.h"

#include "goomsl/goomsl.h"
#include "goomsl/goomsl_hash.h"


#define PATH_MAX_LEN 256
#define NAME_MAX_LEN 256


static const char * themeFolderExtension = ".fptheme";
static char * defaultThemeFolder = NULL;

static const char * DEFAULTPATH(void)
{
    if (defaultThemeFolder == NULL)
    {
        String path = theCommander->getDataPathManager().getPath("theme/Classic.fptheme");
        defaultThemeFolder = strdup((const char *)(path));
    }
    return defaultThemeFolder;
}

 

static String defaultPuyosName("Invaders");
static String defaultLevelName("Kaori's farm");
static const char * preferedPuyosKey = "ThemeManager.Puyos";
static const char * preferedLevelKey = "ThemeManager.Level";

static AnimatedPuyoThemeManager * globalManager = NULL;
String GlobalCurrentPath;

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

static bool loadPictureAt(const char * path, IIM_Surface ** dst, const char * fallback)
{
    *dst = IIM_Load_Absolute_DisplayFormatAlpha(path);
    if (*dst == NULL) *dst = IIM_Load_Absolute_DisplayFormatAlpha(fallback);
    return (*dst != NULL);
}

static bool loadPictureWithOffset(const char * path, IIM_Surface ** dst, const char * fallback, float offset)
{
    //fprintf(stderr,"Loading %s...\n",path);

    IIM_Surface *tmp = IIM_Load_Absolute_DisplayFormatAlpha(path);
    if (tmp == NULL)
    {
        tmp = IIM_Load_Absolute_DisplayFormatAlpha(fallback);
    }
    if (tmp == NULL) return false;
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
//************************************** GSL bindings *************************************
//*****************************************************************************************

static void end_puyoset(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    const char * newName = (const char *) GSL_GLOBAL_PTR(gsl, "puyoset.name");
    AnimatedPuyoSetTheme * theme = new AnimatedPuyoSetTheme(GlobalCurrentPath, newName);
    
    AdvancedBuffer<const char *> * list = globalManager->getAnimatedPuyoSetThemeList();
    int size = list->size();
    //fprintf(stderr,"Verif of size : %d\n", size);
    for (int i = 0; i < size; i++)
    {
        //fprintf(stderr,"Comparing %s and %s\n", (*list)[i], newName);
        if (!strcmp((*list)[i], newName))
        {
            fprintf(stderr,"A Puyoset called %s already exists, ignoring...\n", newName);
            delete theme;
            return;
        }
    }
    
    theme->addInfo(String(((const char *) GSL_GLOBAL_PTR(gsl, "author"))),String(((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.description"))));
    
    theme->addAnimatedPuyoTheme(((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P1.face")),
                               ((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P1.disappear")),
                               ((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P1.explosion")),
                               ((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P1.eye")),
                               ((const float) GSL_GLOBAL_FLOAT(gsl, "puyoset.P1.offset")));
    theme->addAnimatedPuyoTheme(((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P2.face")),
                               ((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P2.disappear")),
                               ((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P2.explosion")),
                               ((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P2.eye")),
                               ((const float) GSL_GLOBAL_FLOAT(gsl, "puyoset.P2.offset")));
    theme->addAnimatedPuyoTheme(((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P3.face")),
                               ((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P3.disappear")),
                               ((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P3.explosion")),
                               ((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P3.eye")),
                               ((const float) GSL_GLOBAL_FLOAT(gsl, "puyoset.P3.offset")));
    theme->addAnimatedPuyoTheme(((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P4.face")),
                               ((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P4.disappear")),
                               ((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P4.explosion")),
                               ((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P4.eye")),
                               ((const float) GSL_GLOBAL_FLOAT(gsl, "puyoset.P4.offset")));
    theme->addAnimatedPuyoTheme(((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P5.face")),
                               ((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P5.disappear")),
                               ((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P5.explosion")),
                               ((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.P5.eye")),
                               ((const float) GSL_GLOBAL_FLOAT(gsl, "puyoset.P5.offset")));
    theme->addNeutralPuyo(((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.Neutral.face")),NULL,NULL,NULL,0.0);
    
    globalManager->addPuyoSet(theme);
}

static void end_level(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    const char * newName = (const char *) GSL_GLOBAL_PTR(gsl, "level.name");
    PuyoLevelTheme * theme = new PuyoLevelTheme(GlobalCurrentPath, newName);

    AdvancedBuffer<const char *> * list = globalManager->getPuyoLevelThemeList();
    int size = list->size();
    for (int i = 0; i < size; i++)
    {
        if (!strcmp((*list)[i], newName))
        {
            fprintf(stderr,"A Level Theme called %s already exists, ignoring...\n", newName);
            delete theme;
            return;
        }
    }

    theme->addInfo(String(((const char *) GSL_GLOBAL_PTR(gsl, "author"))),String(((const char *) GSL_GLOBAL_PTR(gsl, "level.description"))));

    theme->setLives((const char *) GSL_GLOBAL_PTR(gsl, "level.lives"));
    theme->setBackground((const char *) GSL_GLOBAL_PTR(gsl, "level.background"));
    theme->setGrid((const char *) GSL_GLOBAL_PTR(gsl, "level.grid"));
    theme->setSpeedMeter((const char *) GSL_GLOBAL_PTR(gsl, "level.speedmeter"));
    theme->setNeutralIndicator((const char *) GSL_GLOBAL_PTR(gsl, "level.neutralindicator"));
    theme->setSpeedMeterXY(GSL_GLOBAL_INT(gsl, "level.speedmeter_x"), GSL_GLOBAL_INT(gsl, "level.speedmeter_y"));
    theme->setAnimations((const char *) GSL_GLOBAL_PTR(gsl, "level.gamelost_left_2p"),
                         (const char *) GSL_GLOBAL_PTR(gsl, "level.gamelost_right_2p"),
                         (const char *) GSL_GLOBAL_PTR(gsl, "level.animation_2p"));
    
    globalManager->addLevel(theme);
}

static void end_description(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
	// TODO !!!
}


static void sbind(GoomSL *gsl)
{
    gsl_bind_function(gsl, "end_puyoset",   end_puyoset);
    gsl_bind_function(gsl, "end_level",  end_level);
    gsl_bind_function(gsl, "end_description",  end_description);
}

//*****************************************************************************************
//*********************************** AnimatedPuyoTheme ***********************************
//*****************************************************************************************


StandardAnimatedPuyoTheme::StandardAnimatedPuyoTheme(const String path, const char * face, const char * disappear, const char * explosions, const char * eyes, const float color_offset):_path(path)
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


StandardAnimatedPuyoTheme::~StandardAnimatedPuyoTheme(void)
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

IIM_Surface * StandardAnimatedPuyoTheme::getSurface(PuyoPictureType picture, int index)
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


void StandardAnimatedPuyoTheme::releaseCached(void)
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



bool StandardAnimatedPuyoTheme::cache(void)
{
    bool OK = true;
    int j;
    
    char path[PATH_MAX_LEN];
    char defpath[PATH_MAX_LEN];
    const char * fullPath = (const char *)_path;
    
    if (_cached) releaseCached();
    
    // PUYOS
    for (j=0; j<NUMBER_OF_PUYO_FACES; j++)
    {
        snprintf(path, sizeof(path), "%s/%s-puyo-%d%d%d%d.png",fullPath,_face,(j&8)>>3,(j&4)>>2,(j&2)>>1,j&1);
        snprintf(defpath, sizeof(defpath), "%s/%s-puyo-%d%d%d%d.png",DEFAULTPATH(),_face,(j&8)>>3,(j&4)>>2,(j&2)>>1,j&1);
        OK = OK && loadPictureWithOffset(path,&(_puyoFaces[j]),defpath,_color_offset);
    }
    snprintf(path, sizeof(path), "%s/%s-puyo-border.png",fullPath,_face);
    snprintf(defpath, sizeof(defpath), "%s/%s-puyo-border.png",DEFAULTPATH(),_face);
    OK = OK && loadPictureAt(path,&(_puyoCircles[0]),defpath);
    for (int i = 1; i<NUMBER_OF_PUYO_CIRCLES; i++)
    {
        OK = OK && copyPictureWithLuminosity(_puyoCircles[0],&(_puyoCircles[i]),NULL,sin(3.14f/2.0f+i*3.14f/64.0f)*0.6f+0.2f);
    }
    
    snprintf(path, sizeof(path), "%s/%s-puyo-shadow.png",fullPath,_face);
    snprintf(defpath, sizeof(defpath), "%s/%s-puyo-shadow.png",DEFAULTPATH(),_face);
    OK = OK && loadPictureAt(path,&(_puyoShadow),defpath);
    
    for (j=0; j<NUMBER_OF_PUYO_EXPLOSIONS; j++)
    {
        snprintf(path, sizeof(path), "%s/%s-puyo-explosion-%d.png",fullPath,_explosions,j);
        snprintf(defpath, sizeof(defpath), "%s/%s-puyo-explosion-%d.png",DEFAULTPATH(),_explosions,j);
        OK = OK && loadPictureWithOffset(path,&(_puyoExplosion[j]),defpath,_color_offset);
    }
    
    for (j=0; j<NUMBER_OF_PUYO_DISAPPEAR; j++)
    {
        snprintf(path, sizeof(path), "%s/%s-puyo-disappear-%d.png",fullPath,_disappear,j);
        snprintf(defpath, sizeof(defpath), "%s/%s-puyo-disappear-%d.png",DEFAULTPATH(),_disappear,j);
        OK = OK && loadPictureWithOffset(path,&(_puyoDisappear[j]),defpath,_color_offset);
    }
    
    for (j=0; j<NUMBER_OF_PUYO_EYES; j++)
    {
        snprintf(path, sizeof(path), "%s/%s-puyo-eye-%d.png",fullPath,_eyes,j);
        snprintf(defpath, sizeof(defpath), "%s/%s-puyo-eye-%d.png",DEFAULTPATH(),_eyes,j);
        OK = OK && loadPictureWithOffset(path,&(_puyoEyes[j]),defpath,_color_offset);
    }
    
    _cached = OK;
    
    return OK;
}

#ifdef DEBUG
void StandardAnimatedPuyoTheme::retain(void) { counter++; }
void StandardAnimatedPuyoTheme::release(void)
{
    if (counter == 0) fprintf(stderr,"AnimatedPuyoTheme released while not retained");
    else counter--;
}
#endif


NeutralAnimatedPuyoTheme::NeutralAnimatedPuyoTheme(const String path, const char * face)
    : imageFullPath(path), imageDefaultPath(DEFAULTPATH()), faceName(face), _cached(false)
{
    /*imageFullPath = imageFullPath + "/" + face + ".png";
    imageDefaultPath = imageDefaultPath + "/" + face + ".png";
    faceName = face;*/
}

NeutralAnimatedPuyoTheme::~NeutralAnimatedPuyoTheme(void)
{
    releaseCached();
}

IIM_Surface * NeutralAnimatedPuyoTheme::getSurface(PuyoPictureType picture, int index)
{
    CACHE_IT_OR_DIE
    return _puyoNeutral;
}

IIM_Surface *NeutralAnimatedPuyoTheme::getPuyoSurfaceForValence(int valence)
{
    CACHE_IT_OR_DIE
    return _puyoNeutral;
}

IIM_Surface *NeutralAnimatedPuyoTheme::getExplodingSurfaceForIndex(int index)
{
    CACHE_IT_OR_DIE
    return _puyoNeutralPop[index];
}

bool NeutralAnimatedPuyoTheme::cache(void)
{
    bool OK = true;
    OK = OK && loadPictureAt(imageFullPath + "/" + faceName + ".png", &(_puyoNeutral), imageDefaultPath + "/" + faceName + ".png");
    OK = OK && loadPictureAt(imageFullPath + "/" + faceName + "-neutral-1.png", &(_puyoNeutralPop[0]), imageDefaultPath + "/" + faceName + "-neutral-1.png");
    OK = OK && loadPictureAt(imageFullPath + "/" + faceName + "-neutral-2.png", &(_puyoNeutralPop[1]), imageDefaultPath + "/" + faceName + "-neutral-2.png");
    OK = OK && loadPictureAt(imageFullPath + "/" + faceName + "-neutral-3.png", &(_puyoNeutralPop[2]), imageDefaultPath + "/" + faceName + "-neutral-3.png");
    _cached = OK;
    return OK;
}

void NeutralAnimatedPuyoTheme::releaseCached(void)
{
    if (_cached) {
        IIM_Free(_puyoNeutral);
        IIM_Free(_puyoNeutralPop[0]);
        IIM_Free(_puyoNeutralPop[1]);
        IIM_Free(_puyoNeutralPop[2]);
        _cached = false;
    }
}

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

void AnimatedPuyoSetTheme::addInfo(const String author, const String comments)
{
    _author = author;
    _comments = comments;
}

const String AnimatedPuyoSetTheme::getAuthor(void)
{
    return _author;
}

const String AnimatedPuyoSetTheme::getComments(void)
{
    return _comments;
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
    _puyos[_numberOfPuyos] = new StandardAnimatedPuyoTheme(_path, face, disappear, explosions, eyes, color_offset);
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
    _neutral=new NeutralAnimatedPuyoTheme(_path, face);
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
    
    _neutralIndicator = NULL;
    _bigNeutralIndicator = NULL;
    _giantNeutralIndicator = NULL;
    
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
    
    if (_neutral_indicator != NULL) free(_neutral_indicator);
    
    releaseCached();
}

void PuyoLevelTheme::addInfo(const String author, const String comments)
{
    _author = author;
    _comments = comments;
}

const String PuyoLevelTheme::getAuthor(void)
{
    return _author;
}

const String PuyoLevelTheme::getComments(void)
{
    return _comments;
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
    
    if (_neutralIndicator != NULL) free(_neutralIndicator);
    if (_bigNeutralIndicator != NULL) free(_bigNeutralIndicator);
    if (_giantNeutralIndicator != NULL) free(_giantNeutralIndicator);
    _neutralIndicator = NULL;
    _bigNeutralIndicator = NULL;
    _giantNeutralIndicator = NULL;
    
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

bool PuyoLevelTheme::setNeutralIndicator(const char * neutralIndicator)
{
    DEBUG_PARAM_MISSING(speedmeter,"neutralindicator","PuyoLevelTheme")
    
    _neutral_indicator = strdup(neutralIndicator);
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

IIM_Surface * PuyoLevelTheme::getNeutralIndicator()
{
    CACHE_IT_OR_DIE
    return _neutralIndicator;
}

IIM_Surface * PuyoLevelTheme::getBigNeutralIndicator()
{
    CACHE_IT_OR_DIE
    return _bigNeutralIndicator;
}

IIM_Surface * PuyoLevelTheme::getGiantNeutralIndicator()
{
    CACHE_IT_OR_DIE
    return _giantNeutralIndicator;
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
    char defpath[PATH_MAX_LEN];
    const char * fullPath = (const char *)_path;

    if (_cached) releaseCached();
    _cached = false;
    
    if (validate() == false) return false;
    
    // LIVES
    for (i=0; i<NUMBER_OF_LIVES; i++)
    {
        snprintf(path, sizeof(path), "%s/%s-lives-%d.png",fullPath,_lives,i);
        snprintf(defpath, sizeof(defpath), "%s/%s-lives-%d.png",DEFAULTPATH(),_lives,i);
        OK = OK && loadPictureAt(path,&(_levelLives[i]),defpath);
    }
    
    // BACKGROUND
    snprintf(path, sizeof(path), "%s/%s-background.jpg",fullPath,_background);
    snprintf(defpath, sizeof(defpath), "%s/%s-background.jpg",DEFAULTPATH(),_background);
    OK = OK && loadPictureAt(path,&_levelBackground,defpath);
    
    // GRID
    snprintf(path, sizeof(path), "%s/%s-background-grid.png",fullPath,_grid);
    snprintf(defpath, sizeof(defpath), "%s/%s-background-grid.png",DEFAULTPATH(),_grid);
    OK = OK && loadPictureAt(path,&_levelGrid,defpath);
    
    // SPEED METER
    snprintf(path, sizeof(path), "%s/%s-background-meter-below.png",fullPath,_speed_meter);
    snprintf(defpath, sizeof(defpath), "%s/%s-background-meter-below.png",DEFAULTPATH(),_speed_meter);
    OK = OK && loadPictureAt(path,&(_levelMeter[0]),defpath);
    snprintf(path, sizeof(path), "%s/%s-background-meter-above.png",fullPath,_speed_meter);
    snprintf(defpath, sizeof(defpath), "%s/%s-background-meter-above.png",DEFAULTPATH(),_speed_meter);
    OK = OK && loadPictureAt(path,&(_levelMeter[1]),defpath);
    
    // NEUTRAL INDICATORS
    snprintf(path, sizeof(path), "%s/%s-small.png",fullPath,_neutral_indicator);
    snprintf(defpath, sizeof(defpath), "%s/%s-small.png",DEFAULTPATH(),_neutral_indicator);
    OK = OK && loadPictureAt(path,&_neutralIndicator,defpath);
    snprintf(path, sizeof(path), "%s/%s.png",fullPath,_neutral_indicator);
    snprintf(defpath, sizeof(defpath), "%s/%s.png",DEFAULTPATH(),_neutral_indicator);
    OK = OK && loadPictureAt(path,&_bigNeutralIndicator,defpath);
    snprintf(path, sizeof(path), "%s/%s-giant.png",fullPath,_neutral_indicator);
    snprintf(defpath, sizeof(defpath), "%s/%s-giant.png",DEFAULTPATH(),_neutral_indicator);
    OK = OK && loadPictureAt(path,&_giantNeutralIndicator,defpath);
    
    _cached = OK;
    return OK;
}



//**************************************************************************************
//*********************************** Theme Manager ************************************
//**************************************************************************************


static void loadTheme(String fullPath)
{
    String scriptPath = FilePath::combine(fullPath, "Description.gsl");
    
    // Verify input file
    struct stat s;
    if (stat((const char *)scriptPath,&s) == -1)
    {
        fprintf(stderr,"Couldn't load theme from %s. Ignoring.\n",(const char *)scriptPath);
        return;
    }
    //else fprintf(stderr,"Trying to load theme from %s.\n",(const char *)scriptPath);
        
    GlobalCurrentPath = fullPath;
    GoomSL * gsl = gsl_new();
    if (!gsl) return;
    String libPath = theCommander->getDataPathManager().getPath("lib/themelib.gsl");
    char * fbuffer = gsl_init_buffer((const char *)libPath);
    gsl_append_file_to_buffer(scriptPath, &fbuffer);
    gsl_compile(gsl,fbuffer);
    sbind(gsl);
    gsl_execute(gsl);
    gsl_free(gsl);
    free(fbuffer);
}

AnimatedPuyoThemeManager * getPuyoThemeManger(void)
{
    if (globalManager == NULL)
    {
        //fprintf(stderr,"Making new theme manager...\n");
        new AnimatedPuyoThemeManager(false);
    }
    return globalManager;
}

AnimatedPuyoThemeManager::AnimatedPuyoThemeManager(bool useAltLocation)
{
    globalManager = this;
    
    // List the themes in the various pack folders
    SelfVector<String> themeFolders;
    for (int i = 0 ; i < theCommander->getDataPathManager().getNumPacks() ; i++) {
        getThemeListInPath(theCommander->getDataPathManager().getPathInPack("theme", i), themeFolders);
    }
    
    // Load the themes from the list
    for (int i = 0 ; i < themeFolders.size() ; i++) {
        printf("Loading theme %s\n", (const char *)themeFolders[i]);
        loadTheme(themeFolders[i]);
    }
}


AnimatedPuyoThemeManager::~AnimatedPuyoThemeManager(void)
{
}
    
void AnimatedPuyoThemeManager::addPuyoSet(AnimatedPuyoSetTheme * PST)
{
    puyoSets.add(PST);
    puyoSetList.add(strdup((const char *)(PST->getName())));
}

void AnimatedPuyoThemeManager::addLevel(PuyoLevelTheme * PLT)
{
    themes.add(PLT);
    themeList.add(strdup((const char *)(PLT->getName())));
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
        return getAnimatedPuyoSetTheme(defaultPuyosName);
    }
    else
    {
        if (size != 0)
        {
            fprintf(stderr, "Default puyos theme (%s) not found??? Trying another one...\n",(const char *)defaultPuyosName);
            return puyoSets[0];
        }
        else
        {
            fprintf(stderr, "Default puyos theme (%s) not found??? No possible fallback, Exiting...\n",(const char *)defaultPuyosName);
            exit(0);
        }
    }
}

AnimatedPuyoSetTheme * AnimatedPuyoThemeManager::getAnimatedPuyoSetTheme(void)
{
    char out[NAME_MAX_LEN];
    GetStrPreference (preferedPuyosKey, out, (const char *)defaultPuyosName, sizeof(out));
    return getAnimatedPuyoSetTheme(String(out));
}

String AnimatedPuyoThemeManager::getPreferedAnimatedPuyoSetThemeName(void)
{
    char out[NAME_MAX_LEN];
    GetStrPreference (preferedPuyosKey, out, (const char *)defaultPuyosName, sizeof(out));
    String name(out);

    int size = puyoSets.size();
    
    for (int i = 0; i < size; i++)
    {
        if (puyoSets[i]->getName() == name) return puyoSets[i]->getName();
    }

    if (name != defaultPuyosName)
    {
        fprintf(stderr, "Prefered Puyos theme \"%s\" not found, falling back to default...\n",(const char *)name);
        return defaultPuyosName;
    }
    else
    {
        if (size != 0)
        {
            fprintf(stderr, "Default puyos theme (%s) not found??? Trying another one...\n",(const char *)defaultPuyosName);
            return puyoSets[0]->getName();
        }
        else
        {
            fprintf(stderr, "Default puyos theme (%s) not found??? No possible fallback, Exiting...\n",(const char *)defaultPuyosName);
            exit(0);
        }
    }
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
        return getPuyoLevelTheme(defaultLevelName);
    }
    else
    {
        if (size != 0)
        {
            fprintf(stderr, "Default level theme (%s) not found??? Trying another one...\n",(const char *)defaultLevelName);
            return themes[0];
        }
        else
        {
            fprintf(stderr, "Default level theme (%s) not found??? No possible fallback, Exiting...\n",(const char *)defaultLevelName);
            exit(0);
        }
    }
}

PuyoLevelTheme * AnimatedPuyoThemeManager::getPuyoLevelTheme(void)
{
    char out[NAME_MAX_LEN];
    GetStrPreference (preferedLevelKey, out, (const char *)defaultLevelName, sizeof(out));
    return getPuyoLevelTheme(String(out));
}

String AnimatedPuyoThemeManager::getPreferedPuyoLevelThemeName(void)
{
    char out[NAME_MAX_LEN];
    GetStrPreference (preferedLevelKey, out, (const char *)defaultLevelName, sizeof(out));
    String name(out);

    int size = themes.size();
    
    for (int i = 0; i < size; i++)
    {
        if (themes[i]->getName() == name) return themes[i]->getName();
    }
    
    if (name != defaultLevelName)
    {
        fprintf(stderr, "Level theme \"%s\" not found, falling back to default...\n",(const char *)name);
        return defaultLevelName;
    }
    else
    {
        if (size != 0)
        {
            fprintf(stderr, "Default level theme (%s) not found??? Trying another one...\n",(const char *)defaultLevelName);
            return themes[0]->getName();
        }
        else
        {
            fprintf(stderr, "Default level theme (%s) not found??? No possible fallback, Exiting...\n",(const char *)defaultLevelName);
            exit(0);
        }
    }
}


void AnimatedPuyoThemeManager::setPreferedPuyoLevelTheme(const String name)
{
    SetStrPreference (preferedLevelKey, (const char *)name);
}

AdvancedBuffer<const char *> * AnimatedPuyoThemeManager::getAnimatedPuyoSetThemeList(void)
{
    return &puyoSetList;
}

AdvancedBuffer<const char *> * AnimatedPuyoThemeManager::getPuyoLevelThemeList(void)
{
    return &themeList;
}

void AnimatedPuyoThemeManager::getThemeListInPath(const char *path, SelfVector<String> &resultVector) const
{
    FilePath filePath(path);
    if (!filePath.exists())
        return;
    SelfVector<String> pathContent = filePath.listFiles();
    for (int i = 0 ; i < pathContent.size() ; i++) {
        String currentPath = pathContent[i];
        if (strstr(currentPath, themeFolderExtension) != NULL) {
            resultVector.add(FilePath::combine(path, currentPath));
        }
    }
}

