#include "GTLog.h"
#include "goomsl.h"
#include "styrolyse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ios_filepath.h>
#include <ios_memory.h>
#include "preferences.h"

static char *scriptPath0 = NULL;
static char *scriptPath_fx = NULL;
static char *scriptPath_nofx = NULL;


struct _Styrolyse {

  char fname[512];
  GoomSL *gsl;

  /* client app */
  StyrolyseClient *client;

  /* loadImage */
  GoomHash *images;

  int fxMode;
};

Styrolyse *styrolyse = NULL;

/**
 * GSL->C Sprite management
 */

void styro_sin(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    GSL_GLOBAL_FLOAT(gsl, "sin") = sin(GSL_LOCAL_FLOAT(gsl, local, "value"));
}

void styro_random(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    GSL_GLOBAL_FLOAT(gsl, "random") = (float)(1.0 * (rand() / (RAND_MAX + 1.0)));
}

void styro_strcmp(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    char *str1 = (char*)GSL_LOCAL_PTR(gsl, local, "s1");
    char *str2 = (char*)GSL_LOCAL_PTR(gsl, local, "s2");
    GSL_GLOBAL_INT(gsl, "strcmp") = strcmp(str1,str2);
}

/*
void styro_f2i(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    GSL_GLOBAL_INT(gsl, "f2i") = (int)(GSL_LOCAL_FLOAT(gsl, local, "value"));
}
*/

void styro_mod(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    int numerator = GSL_LOCAL_INT(gsl, local, "numerator");
    int denominator = GSL_LOCAL_INT(gsl, local, "denominator");
    GSL_GLOBAL_INT(gsl, "mod") = numerator % denominator;
}

static void styro_get_BoolPreference(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    char *name  = (char*)GSL_LOCAL_PTR(gsl, local, "name");
    int def     = GSL_LOCAL_INT(gsl, local, "default");
    GSL_GLOBAL_INT(gsl, "getBoolPreference")
        = GetBoolPreference(name, def);
}

void put_text(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
  char txt[1024];
  int         x    = (int)GSL_LOCAL_FLOAT(gsl, local, "pos.x");
  int         y    = (int)GSL_LOCAL_FLOAT(gsl, local, "pos.y");
  const char *text = (const char *)GSL_LOCAL_PTR  (gsl, local, "text");
  int         i    = (int)GSL_LOCAL_FLOAT(gsl, local, "text_index");
  if (((size_t)i)>sizeof(txt)-1) i=sizeof(txt)-1;
  int m = 0, n = 0;
  while (true)
  {
    // if end of src or dst string, break
    if ( (text[m] == 0) || ((size_t)m >= sizeof(txt) - 1) ) break;

    // test for new character
    if ((text[m] & 0xC0) != 0x80) n++;

    // if not needed, break
    if (n>i) break;

    // copy byte
    txt[m]=text[m];

    // Next byte
    m++;
  }
  txt[m] = 0;
  styrolyse->client->putText(styrolyse->client,x,y,txt);
}

typedef struct _Vec2 {
  float x;
  float y;
} Vec2;

Vec2 global_sprite_get_position(GoomSL *gsl, const char *name)
{
  Vec2 v;
  v.x = 0.0;
  v.y = 0.0;

  if (strcmp(name, "none") == 0) return v;

  char *vx_s = (char*)malloc(strlen(name)+7);
  char *vy_s = (char*)malloc(strlen(name)+7);
  char *parent_s = (char*)malloc(strlen(name)+8);

  strcpy(vx_s, name); strcpy(vy_s, name);
  strcat(vx_s, ".pos.x"); strcat(vy_s, ".pos.y");

  strcpy(parent_s, name);
  strcat(parent_s, ".parent");

  if (GSL_HAS_GLOBAL(gsl, vx_s))
  {
    v.x = GSL_GLOBAL_FLOAT(gsl, vx_s);
    v.y = GSL_GLOBAL_FLOAT(gsl, vy_s);
    const char *parent = (const char*)GSL_GLOBAL_PTR(gsl, parent_s);

    Vec2 vp = global_sprite_get_position(gsl, parent);
    v.x += vp.x;
    v.y += vp.y;
  }
  else
  {
    fprintf(stderr, "STYROLYSE: INVALID PARENT, '%s'\n", name);
  }

  free(vx_s);
  free(vy_s);
  free(parent_s);
  return v;
}

void styro_music(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    const char *path = (const char *)GSL_LOCAL_PTR  (gsl, local, "command");
    styrolyse->client->music(styrolyse->client, path);
}

void styro_sound(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    const char *path   = (const char *)GSL_LOCAL_PTR  (gsl, local, "file");
    float       volume = GSL_LOCAL_FLOAT (gsl, local, "volume");
    styrolyse->client->playSound(styrolyse->client, path, (int)volume);
}

void styro_gettext(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    static bool firstTime = true;

    const char *text = styrolyse->client->getText(styrolyse->client, (const char *)GSL_LOCAL_PTR  (gsl, local, "text"));

    int *globalPtrReturn = (int*)goom_hash_get(gsl_globals(gsl), "gettext")->ptr;

    if ((!firstTime) && (gsl_get_ptr(gsl, *globalPtrReturn))) {
        gsl_free_ptr(gsl, *globalPtrReturn);
        firstTime = false;
    }

    int newPtrId = gsl_malloc(gsl, strlen(text)+1); // allocate a new pointer (should we allow realloc?)
    strcpy((char*)gsl_get_ptr(gsl, newPtrId), text);
    *globalPtrReturn = newPtrId;
}

void styro_cache_picture(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    const char *path = (const char *)GSL_LOCAL_PTR(gsl, local, "path");
    int mode = GSL_LOCAL_INT(gsl, local, "mode");
    styrolyse->client->cachePicture(styrolyse->client, mode, path);
}

void styro_cache_sound(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    const char *path = (const char *)GSL_LOCAL_PTR(gsl, local, "path");
    styrolyse->client->cacheSound(styrolyse->client, path);
}

void styro_cache_music(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    const char *path = (const char *)GSL_LOCAL_PTR(gsl, local, "path");
    styrolyse->client->cacheMusic(styrolyse->client, path);
}

void sprite_draw(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    const char *path = (const char *)GSL_LOCAL_PTR  (gsl, local, "&this.image");
    int         x    = (int)GSL_LOCAL_FLOAT(gsl, local, "&this.pos.x");
    int         y    = (int)GSL_LOCAL_FLOAT(gsl, local, "&this.pos.y");
    int         w    = (int)GSL_LOCAL_FLOAT(gsl, local, "&this.size.x");
    int         h    = (int)GSL_LOCAL_FLOAT(gsl, local, "&this.size.y");
    int        fl    = (int)GSL_LOCAL_INT(gsl, local, "&this.flipped");
    int        dx    = (int)GSL_LOCAL_FLOAT(gsl, local, "display.x");
    int        dy    = (int)GSL_LOCAL_FLOAT(gsl, local, "display.y");
    int        dw    = (int)GSL_LOCAL_FLOAT(gsl, local, "display.width");
    int        dh    = (int)GSL_LOCAL_FLOAT(gsl, local, "display.height");
    float  scalex    = (float)GSL_LOCAL_FLOAT(gsl, local, "&this.scale.x");
    float  scaley    = (float)GSL_LOCAL_FLOAT(gsl, local, "&this.scale.y");

    const char *parent = (const char*)GSL_LOCAL_PTR(gsl, local, "&this.parent");
    Vec2 parentPos = global_sprite_get_position(gsl, parent);
    x += (int)parentPos.x;
    y += (int)parentPos.y;

    GHashValue  *img  = goom_hash_get(styrolyse->images, path);
    void        *data = NULL;
    if (img == NULL) {
        data = styrolyse->client->loadImage(styrolyse->client, path);
        goom_hash_put_ptr (styrolyse->images, path, data);
    }
    else {
        data = img->ptr;
    }

    if (data == NULL) {
        char txt[1024];
        char tmp[1024];
        strcpy(tmp,path);
        sprintf(txt, "IMAGE NOT FOUND:\n%s", (const char *)ios_fc::FilePath(tmp).basename());
        styrolyse->client->putText(styrolyse->client,x,y,txt);
    }
    else {
        styrolyse->client->drawImage(styrolyse->client, data, x+dx, y+dy, w, h, dx, dy, dw, dh, fl, scalex, scaley);
    }
}

static char *pathResolverFunction(GoomSL *gsl, const char *path)
{
    return styrolyse->client->resolveFilePath(styrolyse->client, path);
}

static goomsl_file openFileFunction(GoomSL *gsl, const char *file_name)
{
    return (goomsl_file)(styrolyse->client->openFile(styrolyse->client, file_name));
}
static void closeFileFunction(GoomSL *gsl, goomsl_file file)
{
    styrolyse->client->closeFile(styrolyse->client, file);
}
static int readFileFunction(GoomSL *gsl, void *buffer, goomsl_file file, int read_size)
{
    styrolyse->client->readFile(styrolyse->client, buffer, file, read_size);
}

/**/

static void sbind(GoomSL *gsl)
{
  gsl_bind_function(gsl, "draw",     sprite_draw);
  gsl_bind_function(gsl, "gettext",  styro_gettext);
  gsl_bind_function(gsl, "cache_picture", styro_cache_picture);
  gsl_bind_function(gsl, "cache_sound",   styro_cache_sound);
  gsl_bind_function(gsl, "cache_music",   styro_cache_music);
  gsl_bind_function(gsl, "mod",      styro_mod);
  gsl_bind_function(gsl, "music",    styro_music);
  gsl_bind_function(gsl, "put_text", put_text);
  gsl_bind_function(gsl, "random",   styro_random);
  gsl_bind_function(gsl, "sin",      styro_sin);
  gsl_bind_function(gsl, "sound",    styro_sound);
  gsl_bind_function(gsl, "strcmp",   styro_strcmp);
  gsl_bind_function(gsl, "getBoolPreference", styro_get_BoolPreference);
// gsl_bind_function(gsl, "f2i",      styro_f2i);
}

/* Externals */

void styrolyse_init(const char *styrolyse_path0, const char *styrolyse_path_nofx, const char *styrolyse_path_fx)
{
    if (scriptPath0 != NULL) free(scriptPath0);
    if (scriptPath_nofx != NULL) free(scriptPath_nofx);
    if (scriptPath_fx != NULL) free(scriptPath_fx);

    scriptPath0 = scriptPath_nofx = scriptPath_fx = NULL;

    if (styrolyse_path0 != NULL) {
        scriptPath0 = (char*)malloc(strlen(styrolyse_path0)+1);
        strcpy(scriptPath0, styrolyse_path0);
    }

    if (styrolyse_path_nofx != NULL) {
        scriptPath_nofx = (char*)malloc(strlen(styrolyse_path_nofx)+1);
        strcpy(scriptPath_nofx, styrolyse_path_nofx);
    }

    if (styrolyse_path_fx != NULL) {
        scriptPath_fx = (char*)malloc(strlen(styrolyse_path_fx)+1);
        strcpy(scriptPath_fx, styrolyse_path_fx);
    }
}

Styrolyse *styrolyse_new(const char *fname, StyrolyseClient *client, int fxMode)
{
    Styrolyse *_this;
    _this = (Styrolyse*)malloc(sizeof(Styrolyse));
    _this->client = client;
    _this->gsl =  gsl_new();
    _this->fxMode = fxMode;
    styrolyse = _this;
    gsl_bind_path_resolver(_this->gsl, pathResolverFunction);
    gsl_bind_file_functions(_this->gsl, openFileFunction, closeFileFunction, readFileFunction);
    _this->images = goom_hash_new();
    strncpy(_this->fname, fname, 512);
    GTLogTrace("styrolyse_new(%s) styrolyse_reload", fname);
    styrolyse_reload(_this);
    GTLogTrace("styrolyse_new() finished");
    return _this;
}

static void images_free_from_hash(GoomHash *caller, const char *key, GHashValue *value)
{
  styrolyse->client->freeImage(styrolyse->client, value->ptr);
}

void styrolyse_free(Styrolyse *_this)
{
  styrolyse = _this;
  if (_this->gsl) gsl_free(_this->gsl);
  goom_hash_for_each(_this->images, images_free_from_hash);
  goom_hash_free(_this->images);
  free(_this);
}

void styrolyse_reduce_memory(Styrolyse *_this)
{
    styrolyse = _this;
    goom_hash_for_each(_this->images, images_free_from_hash);
    goom_hash_clear(_this->images);
}

void styrolyse_event(Styrolyse *_this, const char *event, float x, float y, int player)
{
  if (!_this->gsl) return;
  /* mutexifier cette fonction si multi-thread */
    char *str = (char*)GSL_GLOBAL_PTR(_this->gsl, "@event_type");
    strcpy(str, event);
    GSL_GLOBAL_INT(_this->gsl, "@mode") = 3;
    GSL_GLOBAL_FLOAT(_this->gsl, "@delta_t") = 0.0;
    GSL_GLOBAL_FLOAT(_this->gsl, "@event_pos.x") = x;
    GSL_GLOBAL_FLOAT(_this->gsl, "@event_pos.y") = y;
    GSL_GLOBAL_INT(_this->gsl, "@event_player") = player;
    styrolyse = _this;
    gsl_execute(_this->gsl);
}

void styrolyse_execute(Styrolyse *_this, int mode, float delta_t)
{
  /* mutexifier cette fonction si multi-thread */
    if (delta_t > 0.04) delta_t = 0.04;
    GSL_GLOBAL_INT(_this->gsl, "@mode") = mode;
    GSL_GLOBAL_FLOAT(_this->gsl, "@delta_t") = delta_t;
    styrolyse = _this;
    gsl_execute(_this->gsl);
}

void styrolyse_reload(Styrolyse *_this)
{
    if (!_this->gsl) return;
    gsl_push_file(_this->gsl, scriptPath0);
    if (scriptPath_nofx && !_this->fxMode)
        gsl_push_file(_this->gsl, scriptPath_nofx);
    if (scriptPath_fx && _this->fxMode)
        gsl_push_file(_this->gsl, scriptPath_fx);
    gsl_push_file(_this->gsl, _this->fname);
    styrolyse = _this;
    GTLogTrace("styrolyse_new() gsl_compile");
    gsl_compile(_this->gsl);
    GTLogTrace("styrolyse_new() gsl_compile finished");
    sbind(_this->gsl);
    styrolyse_execute(_this, 0, 0.0);
}

int styrolyse_finished(Styrolyse *_this)
{
    return GSL_GLOBAL_INT(_this->gsl, "@finished");
}

void styrolyse_update(Styrolyse *_this, float delta_t)
{
  if (!_this->gsl) return;
  styrolyse_execute(_this, 1, delta_t);
}

void styrolyse_draw(Styrolyse *_this)
{
  if (!_this->gsl) return;
  styrolyse_execute(_this, 2, 0.0);
}

void styrolyse_setint(Styrolyse *_this, const char *varname, int value)
{
    GSL_GLOBAL_INT(_this->gsl, varname) = value;
}

int styrolyse_getint(Styrolyse *_this, const char *varname)
{
    return GSL_GLOBAL_INT(_this->gsl, varname);
}

int styrolyse_secured_getint(Styrolyse *_this, const char *varname)
{
  if (GSL_HAS_GLOBAL(_this->gsl, varname))
  return GSL_GLOBAL_INT(_this->gsl, varname);
  else throw ios_fc::String("GSL Error, variable ")+ios_fc::String(varname)+ios_fc::String(" in script ")+ios_fc::String(_this->fname)+ios_fc::String(" does not exist\n");
}

const char *styrolyse_getstr(Styrolyse *_this, const char *varname)
{
    return (const char*)GSL_GLOBAL_PTR(_this->gsl, varname);
}

int  styrolyse_current_cycle(Styrolyse *_this)
{
  if (!_this->gsl) return 0;
  return GSL_GLOBAL_INT(_this->gsl, "@cycle");
}
