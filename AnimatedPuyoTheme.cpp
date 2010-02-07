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

const char *defaultThemeName = "theme/Classic.fptheme";
static const char * themeFolderExtension = ".fptheme";
static char * defaultThemeFolder = strdup(defaultThemeName);//NULL;

static const char * DEFAULTPATH(void)
{
    if (defaultThemeFolder == NULL)
    {
        String path = theCommander->getDataPathManager().getPath(defaultThemeName);
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
#define ADD_PICTURE(A) { bool tmp = loadPictureAt(path,A,defpath); OK = OK && tmp; if (tmp == false) fprintf(stderr,"Unable to load %s\n",(char *)path); }
#define ADD_MODIFIABLE_PICTURE(A) { bool tmp = loadPictureAt(path,A,defpath, IMAGE_READ); OK = OK && tmp; if (tmp == false) fprintf(stderr,"Unable to load %s\n",(char *)path); }
#define LOG { fprintf(stderr,"Logged __FILE__ __LINE__"); }

#ifdef DEBUG_THEMES
#define DEBUG_PARAM_MISSING(p,n,o) { if (p==NULL) fprintf(stderr,"Parameter %s missing in %s __FILE__ __LINE__",n,o); exit(0); }
#define ASSERT_RANGE(min,max,value) { if ((value < min) || (value > max)) { fprintf(stderr, "Value out of range __FILE__ __LINE__"); exit(0); } }
#else
#define DEBUG_PARAM_MISSING(p,n,o) { }
#define ASSERT_RANGE(min,max,value) { }
#endif

static bool loadPictureAt(const char * path, IosSurfaceRef &dst, const char * fallback, int specialAbility = 0)
{
    IosSurfaceRef tmp = theCommander->getSurface(IMAGE_RGBA, path, specialAbility);
    if (tmp.empty())
        tmp = theCommander->getSurface(IMAGE_RGBA, fallback, specialAbility);
    dst = tmp;
    return (! tmp.empty());
}

static bool loadPictureWithOffset(const char * path, IosSurface ** dst, const char * fallback, float offset)
{
    IosSurfaceRef tmp = theCommander->getSurface(IMAGE_RGBA, path, IMAGE_READ); // TODO Drop CPU buffer when fini avec lui
    // TODO 2 pour que ca soit propre, creer l'ability: IMAGE_HSV_OPERATIONS
    if (tmp.empty())
        tmp = theCommander->getSurface(IMAGE_RGBA, fallback, IMAGE_READ);
    if (tmp.empty())
        return false;
    *dst = tmp.get()->shiftHue(offset);
    return (*dst != NULL);
}
/*
static bool copyPictureWithOffset(IIM_Surface * src, IIM_Surface ** dst, IIM_Surface * fallback, float offset)
{
    *dst = iim_surface_shift_hue(src,offset);
    return (*dst != NULL);
}
*/
static bool copyPictureWithLuminosity(IosSurface * src, IosSurface ** dst, IosSurface * fallback, float lum)
{
    *dst = src->setValue(lum);
    return (*dst != NULL);
}


//*****************************************************************************************
//************************************** GSL bindings *************************************
//*****************************************************************************************

static void end_puyoset(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
	FilePath tmp0(GlobalCurrentPath);
	String tmp1 = FilePath::combine(FilePath::combine("theme",tmp0.basename()),"locale");
	LocalizedDictionary localeDictionary(theCommander->getDataPathManager(), tmp1, "theme");

	const char * newName  = (const char *) GSL_GLOBAL_PTR(gsl, "puyoset.name");
	const char * newLName = localeDictionary.getLocalizedString(String(newName),true);
    AnimatedPuyoSetTheme * theme = new AnimatedPuyoSetTheme(GlobalCurrentPath, newName, newLName);

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

    theme->addInfo(
		String(((const char *) GSL_GLOBAL_PTR(gsl, "author"))),
		localeDictionary.getLocalizedString(String(((const char *) GSL_GLOBAL_PTR(gsl, "puyoset.description"))),true));

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

static void loadPuyobanDefinition(GoomSL *gsl, int playerId, PuyoLevelTheme *theme)
{
    String variablePrefix = String("level.puyoban_p") + (playerId+1);
    theme->setPuyobanXY(playerId,
                        GSL_GLOBAL_INT(gsl, variablePrefix + ".display.x"),
                        GSL_GLOBAL_INT(gsl, variablePrefix + ".display.y"));
    theme->setNextPuyosXY(playerId,
                          GSL_GLOBAL_INT(gsl, variablePrefix + ".next.x"),
                          GSL_GLOBAL_INT(gsl, variablePrefix + ".next.y"));
    theme->setNeutralDisplayXY(playerId,
                               GSL_GLOBAL_INT(gsl, variablePrefix + ".neutral_display.x"),
                               GSL_GLOBAL_INT(gsl, variablePrefix + ".neutral_display.y"));
    theme->setNameDisplayXY(playerId,
                            GSL_GLOBAL_INT(gsl, variablePrefix + ".name_display.x"),
                            GSL_GLOBAL_INT(gsl, variablePrefix + ".name_display.y"));
    theme->setScoreDisplayXY(playerId,
                             GSL_GLOBAL_INT(gsl, variablePrefix + ".score_display.x"),
                             GSL_GLOBAL_INT(gsl, variablePrefix + ".score_display.y"));
    theme->setShouldDisplayNext(playerId,
                                GSL_GLOBAL_INT(gsl, variablePrefix + ".should_display_next"));
    theme->setShouldDisplayShadows(playerId,
                                   GSL_GLOBAL_INT(gsl, variablePrefix + ".should_display_shadows"));
    theme->setShouldDisplayEyes(playerId,
                                GSL_GLOBAL_INT(gsl, variablePrefix + ".should_display_eyes"));
    theme->setPuyobanScale(playerId,
                           GSL_GLOBAL_FLOAT(gsl, variablePrefix + ".scale"));
}

static IosFontRef loadFontDefinition(GoomSL *gsl, const String &fontName)
{
    String variablePrefix = String("level.") + fontName;
    String fontPath = (const char *) GSL_GLOBAL_PTR(gsl, variablePrefix + ".path");
    if (fontPath == "__FONT__") {
        fontPath = theCommander->getLocalizedFontName();
    }
    int fontSize     = GSL_GLOBAL_INT(gsl, variablePrefix + ".size");
    IosFontFx fontFx = (IosFontFx)(GSL_GLOBAL_INT(gsl, variablePrefix + ".fx"));
    return theCommander->getFont(fontPath, fontSize, fontFx);
}

static void end_level(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
	FilePath tmp0(GlobalCurrentPath);
	String tmp1 = FilePath::combine(FilePath::combine("theme",tmp0.basename()),"locale");
	LocalizedDictionary localeDictionary(theCommander->getDataPathManager(), tmp1, "theme");

	  const char * newName  = (const char *) GSL_GLOBAL_PTR(gsl, "level.name");
	  const char * newLName = localeDictionary.getLocalizedString(String(newName),true);
    PuyoLevelTheme * theme = new PuyoLevelTheme(GlobalCurrentPath, newName, newLName);

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

    theme->addInfo(
		String(((const char *) GSL_GLOBAL_PTR(gsl, "author"))),
		localeDictionary.getLocalizedString(String(((const char *) GSL_GLOBAL_PTR(gsl, "level.description"))),true));

    theme->setLives((const char *) GSL_GLOBAL_PTR(gsl, "level.lives"));
    theme->setBackground((const char *) GSL_GLOBAL_PTR(gsl, "level.background"));
    theme->setGrid((const char *) GSL_GLOBAL_PTR(gsl, "level.grid"));
    theme->setSpeedMeter((const char *) GSL_GLOBAL_PTR(gsl, "level.speedmeter"));
    theme->setNeutralIndicator((const char *) GSL_GLOBAL_PTR(gsl, "level.neutralindicator"));
    theme->setSpeedMeterXY(GSL_GLOBAL_INT(gsl, "level.speedmeter_display.x"), GSL_GLOBAL_INT(gsl, "level.speedmeter_display.y"));
    theme->setLifeDisplayXY(GSL_GLOBAL_INT(gsl, "level.life_display.x"), GSL_GLOBAL_INT(gsl, "level.life_display.y"));
    loadPuyobanDefinition(gsl, 0, theme);
    loadPuyobanDefinition(gsl, 1, theme);
    theme->setPlayerNameFont(loadFontDefinition(gsl, "playerNameFont"));
    theme->setScoreFont(loadFontDefinition(gsl, "scoreFont"));
    theme->setAnimations((const char *) GSL_GLOBAL_PTR(gsl, "level.gamelost_left_2p"),
                         (const char *) GSL_GLOBAL_PTR(gsl, "level.gamelost_right_2p"),
                         (const char *) GSL_GLOBAL_PTR(gsl, "level.animation_2p"));
    theme->setForegroundAnimation((const char *) GSL_GLOBAL_PTR(gsl, "level.foreground_animation"));
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

#ifdef DEBUG_THEMES
    counter = 0;
#endif

    unsigned int j;
    unsigned int i;

    for (j=0; j<NUMBER_OF_PUYO_FACES; j++)
    {
        for (i=0; i<MAX_COMPRESSED; i++) {
            _puyoFaces[j][i] = NULL;
        }
    }

    for (j=0; j<NUMBER_OF_PUYO_CIRCLES; j++)
    {
        for (i=0; i<MAX_COMPRESSED; i++) {
            _puyoCircles[j][i] = NULL;
        }
    }

    for (i=0; i<MAX_COMPRESSED; i++) {
        _puyoShadow[i] = NULL;
    }

    for (j=0; j<NUMBER_OF_PUYO_EXPLOSIONS; j++)
    {
        for (i=0; i<MAX_COMPRESSED; i++) {
            _puyoExplosion[j][i] = NULL;
        }
    }

    for (j=0; j<NUMBER_OF_PUYO_DISAPPEAR; j++)
    {
        for (i=0; i<MAX_COMPRESSED; i++) {
            _puyoDisappear[j][i] = NULL;
        }
    }

    for (j=0; j<NUMBER_OF_PUYO_EYES; j++)
    {
        for (i=0; i<MAX_COMPRESSED; i++) {
            _puyoEyes[j][i] = NULL;
        }
    }
}


StandardAnimatedPuyoTheme::~StandardAnimatedPuyoTheme(void)
{
#ifdef DEBUG_THEMES
    if (counter > 0) fprintf(stderr,"AnimatedPuyoTheme released while in use !!!");
#endif

    if (_face != NULL) free(_face);
    _face = NULL;
    if (_disappear != NULL) free(_disappear);
    _disappear = NULL;
    if (_explosions != NULL) free(_explosions);
    _explosions = NULL;
    if (_eyes != NULL) free(_eyes);
    _eyes = NULL;

    releaseCached();
}

IosSurface * StandardAnimatedPuyoTheme::getShrunkSurface(PuyoPictureType picture, int index, int compression)
{

#ifdef DEBUG_THEMES
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

    if (compression<1) return getSurface(picture, index);
    if (compression>(MAX_COMPRESSED-1)) compression = MAX_COMPRESSED-1;

    IosSurface * * myImage = NULL;

    switch (picture)
    {
        case PUYO_FACES:
            myImage = &(_puyoFaces[index][0]);
            break;
        case PUYO_CIRCLES:
            myImage = &(_puyoCircles[index][0]);
            break;
        case PUYO_EXPLOSIONS:
            myImage = &(_puyoExplosion[index][0]);
            break;
        case PUYO_DISAPPEAR:
            myImage = &(_puyoDisappear[index][0]);
            break;
        case PUYO_EYES:
            myImage = &(_puyoEyes[index][0]);
            break;
        case PUYO_SHADOWS:
            myImage = &(_puyoShadow[0]);
            break;
        default:
            break;
    }
    if (myImage && myImage[0]) {
        if (myImage[compression] == NULL) {
            myImage[compression] = myImage[0]->resizeAlpha(myImage[0]->w, (myImage[0]->h - compression));
        }
        return myImage[compression];
    }
    return NULL;
}

IosSurface * StandardAnimatedPuyoTheme::getSurface(PuyoPictureType picture, int index)
{

#ifdef DEBUG_THEMES
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
            return _puyoFaces[index][0];
            break;
        case PUYO_CIRCLES:
            return _puyoCircles[index][0];
            break;
        case PUYO_EXPLOSIONS:
            return _puyoExplosion[index][0];
            break;
        case PUYO_DISAPPEAR:
            return _puyoDisappear[index][0];
            break;
        case PUYO_EYES:
            return _puyoEyes[index][0];
            break;
        case PUYO_SHADOWS:
            return _puyoShadow[0];
            break;
        default:
            break;
    }
    return NULL;
}

IosSurface *StandardAnimatedPuyoTheme::getPuyoSurfaceForValence(int valence, int compression)
{
    //if (compression < 1) return getSurface(PUYO_FACES, valence);
    return getShrunkSurface(PUYO_FACES, valence, compression);
}



void StandardAnimatedPuyoTheme::releaseCached(void)
{
    unsigned int j;
    unsigned int i;

    for (j=0; j<NUMBER_OF_PUYO_FACES; j++)
    {
        for (i=0; i<MAX_COMPRESSED; i++) {
            if (_puyoFaces[j][i] != NULL) delete _puyoFaces[j][i];
            _puyoFaces[j][i] = NULL;
        }
    }

    for (j=0; j<NUMBER_OF_PUYO_CIRCLES; j++)
    {
        for (i=0; i<MAX_COMPRESSED; i++) {
            if (j == 0 && i == 0) continue;
            if (_puyoCircles[j][i] != NULL) delete _puyoCircles[j][i];
            _puyoCircles[j][i] = NULL;
        }
    }

    for (i=1; i<MAX_COMPRESSED; i++) {
        if (_puyoShadow[i] != NULL) delete _puyoShadow[i];
        _puyoShadow[i] = NULL;
    }

    for (j=0; j<NUMBER_OF_PUYO_EXPLOSIONS; j++)
    {
        for (i=0; i<MAX_COMPRESSED; i++) {
            if (_puyoExplosion[j][i] != NULL) delete _puyoExplosion[j][i];
            _puyoExplosion[j][i] = NULL;
        }
    }

    for (j=0; j<NUMBER_OF_PUYO_DISAPPEAR; j++)
    {
        for (i=0; i<MAX_COMPRESSED; i++) {
            if (_puyoDisappear[j][i] != NULL) delete _puyoDisappear[j][i];
            _puyoDisappear[j][i] = NULL;
        }
    }

    for (j=0; j<NUMBER_OF_PUYO_EYES; j++)
    {
        for (i=0; i<MAX_COMPRESSED; i++) {
            if (_puyoEyes[j][i] != NULL) delete _puyoEyes[j][i];
            _puyoEyes[j][i] = NULL;
        }
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

    if (_cached) return true;
//    if (_cached) releaseCached();

    // PUYOS
    for (j=0; j<NUMBER_OF_PUYO_FACES; j++)
    {
        snprintf(path, sizeof(path), "%s/%s-puyo-%d%d%d%d.png",fullPath,_face,(j&8)>>3,(j&4)>>2,(j&2)>>1,j&1);
        snprintf(defpath, sizeof(defpath), "%s/%s-puyo-%d%d%d%d.png",DEFAULTPATH(),_face,(j&8)>>3,(j&4)>>2,(j&2)>>1,j&1);
        OK = OK && loadPictureWithOffset(path,&(_puyoFaces[j][0]),defpath,_color_offset);
    }
    snprintf(path, sizeof(path), "%s/%s-puyo-border.png",fullPath,_face);
    snprintf(defpath, sizeof(defpath), "%s/%s-puyo-border.png",DEFAULTPATH(),_face);
    ADD_MODIFIABLE_PICTURE(_puyoOriginalCircle);
    _puyoCircles[0][0] = _puyoOriginalCircle;
    for (int i = 1; i<NUMBER_OF_PUYO_CIRCLES; i++)
    {
        OK = OK && copyPictureWithLuminosity(_puyoCircles[0][0],&(_puyoCircles[i][0]),NULL,sin(3.14f/2.0f+i*3.14f/64.0f)*0.6f+0.2f);
    }

    snprintf(path, sizeof(path), "%s/%s-puyo-shadow.png",fullPath,_face);
    snprintf(defpath, sizeof(defpath), "%s/%s-puyo-shadow.png",DEFAULTPATH(),_face);
    ADD_PICTURE(_puyoOriginalShadow);
    _puyoShadow[0] = _puyoOriginalShadow;
    for (j=0; j<NUMBER_OF_PUYO_EXPLOSIONS; j++)
    {
        snprintf(path, sizeof(path), "%s/%s-puyo-explosion-%d.png",fullPath,_explosions,j);
        snprintf(defpath, sizeof(defpath), "%s/%s-puyo-explosion-%d.png",DEFAULTPATH(),_explosions,j);
        OK = OK && loadPictureWithOffset(path,&(_puyoExplosion[j][0]),defpath,_color_offset);
    }

    for (j=0; j<NUMBER_OF_PUYO_DISAPPEAR; j++)
    {
        snprintf(path, sizeof(path), "%s/%s-puyo-disappear-%d.png",fullPath,_disappear,j);
        snprintf(defpath, sizeof(defpath), "%s/%s-puyo-disappear-%d.png",DEFAULTPATH(),_disappear,j);
        OK = OK && loadPictureWithOffset(path,&(_puyoDisappear[j][0]),defpath,_color_offset);
    }

    for (j=0; j<NUMBER_OF_PUYO_EYES; j++)
    {
        snprintf(path, sizeof(path), "%s/%s-puyo-eye-%d.png",fullPath,_eyes,j);
        snprintf(defpath, sizeof(defpath), "%s/%s-puyo-eye-%d.png",DEFAULTPATH(),_eyes,j);
        OK = OK && loadPictureWithOffset(path,&(_puyoEyes[j][0]),defpath,_color_offset);
    }

    _cached = true;

    return OK;
}

#ifdef DEBUG_THEMES
void StandardAnimatedPuyoTheme::retain(void) { counter++; }
void StandardAnimatedPuyoTheme::release(void)
{
    if (counter == 0) fprintf(stderr,"AnimatedPuyoTheme released while not retained\n");
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

IosSurface * NeutralAnimatedPuyoTheme::getSurface(PuyoPictureType picture, int index)
{
    CACHE_IT_OR_DIE
    return _puyoNeutral;
}

IosSurface *NeutralAnimatedPuyoTheme::getPuyoSurfaceForValence(int valence, int compression)
{
    CACHE_IT_OR_DIE
    return _puyoNeutral;
}

IosSurface *NeutralAnimatedPuyoTheme::getExplodingSurfaceForIndex(int index, int compression)
{
    CACHE_IT_OR_DIE
    return _puyoNeutralPop[index];
}

bool NeutralAnimatedPuyoTheme::cache(void)
{
    bool OK = true;
    OK = OK && loadPictureAt(imageFullPath + "/" + faceName + ".png", _puyoNeutral, imageDefaultPath + "/" + faceName + ".png");
    OK = OK && loadPictureAt(imageFullPath + "/" + faceName + "-neutral-1.png", _puyoNeutralPop[0], imageDefaultPath + "/" + faceName + "-neutral-1.png");
    OK = OK && loadPictureAt(imageFullPath + "/" + faceName + "-neutral-2.png", _puyoNeutralPop[1], imageDefaultPath + "/" + faceName + "-neutral-2.png");
    OK = OK && loadPictureAt(imageFullPath + "/" + faceName + "-neutral-3.png", _puyoNeutralPop[2], imageDefaultPath + "/" + faceName + "-neutral-3.png");
    _cached = true;
    return OK;
}

void NeutralAnimatedPuyoTheme::releaseCached(void)
{
    if (_cached) {
        _cached = false;
    }
}

//********************************************************************************************
//*********************************** AnimatedPuyoSetTheme ***********************************
//********************************************************************************************

AnimatedPuyoSetTheme::AnimatedPuyoSetTheme(const String path, const String name, const String lname):_path(path),_name(name),_lname(lname)
{
    DEBUG_PARAM_MISSING(path,"path","AnimatedPuyoTheme")
    DEBUG_PARAM_MISSING(name,"name","AnimatedPuyoTheme")

    _numberOfPuyos = 0;
    _neutral = NULL;
#ifdef DEBUG_THEMES
    counter = 0;
#endif
}


AnimatedPuyoSetTheme::~AnimatedPuyoSetTheme(void)
{
#ifdef DEBUG_THEMES
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

const String AnimatedPuyoSetTheme::getLocalizedName(void)
{
    return _lname;
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
        printf("Too many puyos in theme %s... Ignoring puyo.\n",(const char *)_lname);
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
        printf("Too many neutral puyos in theme %s... Ignoring puyo.\n",(const char *)_lname);
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

#ifdef DEBUG_THEMES
void AnimatedPuyoSetTheme::retain(void) { counter++; }
void AnimatedPuyoSetTheme::release(void)
{
    if (counter == 0) fprintf(stderr,"AnimatedPuyoSetTheme released while not retained\n");
    else counter--;
}
#endif

//**************************************************************************************
//*********************************** PuyoLevelTheme ***********************************
//**************************************************************************************

PuyoLevelTheme::PuyoLevelTheme(const String path, const String name, const String lname):_path(path),_name(name),_lname(lname)
{
    DEBUG_PARAM_MISSING(path,"path","PuyoLevelTheme")
    DEBUG_PARAM_MISSING(name,"name","PuyoLevelTheme")

    _cached = false;

#ifdef DEBUG_THEMES
    counter = 0;
#endif
}

PuyoLevelTheme::~PuyoLevelTheme(void)
{
#ifdef DEBUG_THEMES
    if (counter > 0) fprintf(stderr,"PuyoLevelTheme released while used !!!\n");
#endif

    if (_lives != NULL) free(_lives);
    _lives = NULL;
    if (_background != NULL) free(_background);
    _background = NULL;
    if (_grid != NULL) free(_grid);
    _grid = NULL;
    if (_speed_meter != NULL) free(_speed_meter);
    _speed_meter = NULL;
    if (_neutral_indicator != NULL) free(_neutral_indicator);
    _neutral_indicator = NULL;

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

const String PuyoLevelTheme::getLocalizedName(void)
{
  return _lname;
}

void PuyoLevelTheme::releaseCached(void)
{
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
    DEBUG_PARAM_MISSING(neutralIndicator,"neutralindicator","PuyoLevelTheme")

    _neutral_indicator = strdup(neutralIndicator);
    return true;
}

IosSurface * PuyoLevelTheme::getLifeForIndex(int index)
{
    ASSERT_RANGE(0,NUMBER_OF_LIVES-1,index);
    CACHE_IT_OR_DIE

        return _levelLives[index];
}

IosSurface * PuyoLevelTheme::getBackground(void)
{
    CACHE_IT_OR_DIE

    return _levelBackground;
}

IosSurface * PuyoLevelTheme::getGrid(void)
{
    CACHE_IT_OR_DIE

    return _levelGrid;
}

IosSurface * PuyoLevelTheme::getSpeedMeter(bool front)
{
    CACHE_IT_OR_DIE
    return _levelMeter[front?1:0];
}

IosSurface * PuyoLevelTheme::getNeutralIndicator()
{
    CACHE_IT_OR_DIE
    return _neutralIndicator;
}

IosSurface * PuyoLevelTheme::getBigNeutralIndicator()
{
    CACHE_IT_OR_DIE
    return _bigNeutralIndicator;
}

IosSurface * PuyoLevelTheme::getGiantNeutralIndicator()
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

    if (validate() == false) return false;

    // LIVES
    for (i=0; i<NUMBER_OF_LIVES; i++)
    {
        snprintf(path, sizeof(path), "%s/%s-lives-%d.png",fullPath,_lives,i);
        snprintf(defpath, sizeof(defpath), "%s/%s-lives-%d.png",DEFAULTPATH(),_lives,i);
        ADD_PICTURE(_levelLives[i])
    }

    // BACKGROUND
    snprintf(path, sizeof(path), "%s/%s",fullPath,_background);
    snprintf(defpath, sizeof(defpath), "%s/%s",DEFAULTPATH(),_background);
    ADD_PICTURE(_levelBackground)

    // GRID
    if (strcmp(_grid, "") != 0) {
      snprintf(path, sizeof(path), "%s/%s-background-grid.png",fullPath,_grid);
      snprintf(defpath, sizeof(defpath), "%s/%s-background-grid.png",DEFAULTPATH(),_grid);
      ADD_PICTURE(_levelGrid)
    }
    else _levelGrid = NULL;

    // SPEED METER
    snprintf(path, sizeof(path), "%s/%s-background-meter-below.png",fullPath,_speed_meter);
    snprintf(defpath, sizeof(defpath), "%s/%s-background-meter-below.png",DEFAULTPATH(),_speed_meter);
    ADD_PICTURE(_levelMeter[0])
    snprintf(path, sizeof(path), "%s/%s-background-meter-above.png",fullPath,_speed_meter);
    snprintf(defpath, sizeof(defpath), "%s/%s-background-meter-above.png",DEFAULTPATH(),_speed_meter);
    ADD_PICTURE(_levelMeter[1])

    // NEUTRAL INDICATORS
    snprintf(path, sizeof(path), "%s/%s-small.png",fullPath,_neutral_indicator);
    snprintf(defpath, sizeof(defpath), "%s/%s-small.png",DEFAULTPATH(),_neutral_indicator);
    ADD_PICTURE(_neutralIndicator)
    snprintf(path, sizeof(path), "%s/%s.png",fullPath,_neutral_indicator);
    snprintf(defpath, sizeof(defpath), "%s/%s.png",DEFAULTPATH(),_neutral_indicator);
    ADD_PICTURE(_bigNeutralIndicator)
    snprintf(path, sizeof(path), "%s/%s-giant.png",fullPath,_neutral_indicator);
    snprintf(defpath, sizeof(defpath), "%s/%s-giant.png",DEFAULTPATH(),_neutral_indicator);
    ADD_PICTURE(_giantNeutralIndicator)

    _cached = true;
    return OK;
}

const String PuyoLevelTheme::getGameLostLeftAnimation2P() const
{
    return _gamelost_left_2p;
}

const String PuyoLevelTheme::getGameLostRightAnimation2P() const
{
    return _gamelost_right_2p;
}

const String PuyoLevelTheme::getCentralAnimation2P() const
{
    return _animation_2p;
}

const String PuyoLevelTheme::getForegroundAnimation() const
{
    if (_foreground_animation == "")
        return "";
    else
        return theCommander->getDataPathManager().getPath(
            FilePath(getThemeRootPath()).combine(
            _foreground_animation));
}


//**************************************************************************************
//*********************************** Theme Manager ************************************
//**************************************************************************************


static void loadTheme(String fullPath)
{
    String scriptPath = theCommander->getDataPathManager().getPath(FilePath::combine(fullPath, "Description.gsl"));

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
    SelfVector<String> themeFolders = theCommander->getDataPathManager().getEntriesAtPath("theme");
    // Load the themes from the list (only those matching the correct extension)
    for (int i = 0 ; i < themeFolders.size() ; i++) {
        if (themeFolders[i].substring(themeFolders[i].size() - 8)
            == themeFolderExtension) {
            loadTheme(themeFolders[i]);
        }
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

AdvancedBuffer<AnimatedPuyoSetTheme *> * AnimatedPuyoThemeManager::getAnimatedPuyoSetThemeObjectList(void)
{
  return &puyoSets;
}

AdvancedBuffer<PuyoLevelTheme *> * AnimatedPuyoThemeManager::getPuyoLevelThemeObjectList(void)
{
  return &themes;
}
