#include "goomsl.h"
#include "styrolyse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


static char scriptPath[1024];

struct _Styrolyse {

  char fname[512];
  GoomSL *gsl;

  /* client app */
  StyrolyseClient *client;

  /* loadImage */
  GoomHash *images;
};

Styrolyse *styrolyse = NULL;

/**
 * GSL->C Sprite management
 */

void styro_sin(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    GSL_GLOBAL_FLOAT(gsl, "sin") = sin(GSL_LOCAL_FLOAT(gsl, local, "value"));
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

void put_text(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
  char txt[1024];
  int         x    = (int)GSL_LOCAL_FLOAT(gsl, local, "pos.x");
  int         y    = (int)GSL_LOCAL_FLOAT(gsl, local, "pos.y");
  const char *text = (const char *)GSL_LOCAL_PTR  (gsl, local, "text");
  int         i    = (int)GSL_LOCAL_FLOAT(gsl, local, "text_index");
  if (i>sizeof(txt)-1) i=sizeof(txt)-1;
  strncpy(txt,text,i);
  txt[i]=0;
  styrolyse->client->putText(styrolyse->client,x,y,txt);
}

typedef struct _Vec2 {
  float x;
  float y;
} Vec2;

Vec2 global_sprite_get_position(GoomSL *gsl, const char *name)
{
  Vec2 v;
  if (strcmp(name, "none") == 0)
  {
    v.x = 0;
    v.y = 0;
    return v;
  }

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
    const char *path = (const char *)GSL_LOCAL_PTR  (gsl, local, "file");
    styrolyse->client->playMusic(styrolyse->client, path);
}

void styro_sound(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    const char *path   = (const char *)GSL_LOCAL_PTR  (gsl, local, "file");
    float       volume = GSL_LOCAL_FLOAT (gsl, local, "volume");
    styrolyse->client->playSound(styrolyse->client, path, volume);
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

void sprite_draw(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
  const char *path = (const char *)GSL_LOCAL_PTR  (gsl, local, "&this.image");
  int         x    = (int)GSL_LOCAL_FLOAT(gsl, local, "&this.pos.x");
  int         y    = (int)GSL_LOCAL_FLOAT(gsl, local, "&this.pos.y");
  int        dx    = (int)GSL_LOCAL_FLOAT(gsl, local, "display.x");
  int        dy    = (int)GSL_LOCAL_FLOAT(gsl, local, "display.y");
  int        dw    = (int)GSL_LOCAL_FLOAT(gsl, local, "display.width");
  int        dh    = (int)GSL_LOCAL_FLOAT(gsl, local, "display.height");

  const char *parent = (const char*)GSL_LOCAL_PTR(gsl, local, "&this.parent");
  Vec2 parentPos = global_sprite_get_position(gsl, parent);
  x += (int)parentPos.x;
  y += (int)parentPos.y;
  
  GHashValue  *img  = goom_hash_get(styrolyse->images, path);
  void       *data = NULL;
  if (img == NULL) {
    data = styrolyse->client->loadImage(styrolyse->client, path);
    goom_hash_put_ptr (styrolyse->images, path, data);
  }
  else {
    data = img->ptr;
  }

  if (data == NULL) {
    printf("'%s' not loaded\n", path);
    return;
  }
  styrolyse->client->drawImage(styrolyse->client, data, x+dx, y+dy, dx, dy, dw, dh);
}

char *pathResolverFunction(GoomSL *gsl, const char *path)
{
    return styrolyse->client->resolveFilePath(styrolyse->client, path);
}

/**/

static void sbind(GoomSL *gsl)
{
  gsl_bind_function(gsl, "put_text",   put_text);
  gsl_bind_function(gsl, "draw",  sprite_draw);
  gsl_bind_function(gsl, "sin",   styro_sin);
//  gsl_bind_function(gsl, "f2i",   styro_f2i);
  gsl_bind_function(gsl, "mod",   styro_mod);
  gsl_bind_function(gsl, "music", styro_music);
  gsl_bind_function(gsl, "sound", styro_sound);
  gsl_bind_function(gsl, "gettext",   styro_gettext);
}

/* Externals */

void styrolyse_init(const char *styrolyse_path)
{
    strcpy(scriptPath, styrolyse_path);
}

Styrolyse *styrolyse_new(const char *fname, StyrolyseClient *client)
{
    Styrolyse *_this;
    _this = (Styrolyse*)malloc(sizeof(Styrolyse));
    _this->client = client;
    _this->gsl =  gsl_new();
    gsl_bind_path_resolver(_this->gsl, pathResolverFunction);
    _this->images = goom_hash_new();
    strncpy(_this->fname, fname, 512);
    styrolyse_reload(_this);
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
    char *fbuffer;
    if (!_this->gsl) return;
    fbuffer = gsl_init_buffer(scriptPath);
    gsl_append_file_to_buffer(_this->fname, &fbuffer);
    styrolyse = _this;
    gsl_compile(_this->gsl,fbuffer);
    sbind(_this->gsl);
    free(fbuffer);
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

int  styrolyse_current_cycle(Styrolyse *_this)
{
  if (!_this->gsl) return 0;
  return GSL_GLOBAL_INT(_this->gsl, "@cycle");
}
