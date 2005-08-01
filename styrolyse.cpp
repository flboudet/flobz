#include "goomsl.h"
#include "styrolyse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern char *dataFolder;

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

void styro_mod(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    int numerator = GSL_LOCAL_INT(gsl, local, "numerator");
    int denominator = GSL_LOCAL_INT(gsl, local, "denominator");
    GSL_LOCAL_INT(gsl, local, "mod") = numerator % denominator;
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

void sprite_draw(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
  const char *path = (const char *)GSL_LOCAL_PTR  (gsl, local, "&this.image");
  int         x    = (int)GSL_LOCAL_FLOAT(gsl, local, "&this.pos.x");
  int         y    = (int)GSL_LOCAL_FLOAT(gsl, local, "&this.pos.y");
  int        dx    = (int)GSL_LOCAL_FLOAT(gsl, local, "display.x");
  int        dy    = (int)GSL_LOCAL_FLOAT(gsl, local, "display.y");
  int        dw    = (int)GSL_LOCAL_FLOAT(gsl, local, "display.width");
  int        dh    = (int)GSL_LOCAL_FLOAT(gsl, local, "display.height");
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

/**/

static void sbind(GoomSL *gsl)
{
  gsl_bind_function(gsl, "put_text",   put_text);
  gsl_bind_function(gsl, "draw",  sprite_draw);
  gsl_bind_function(gsl, "sin",  styro_sin);
  gsl_bind_function(gsl, "mod",  styro_mod);
}

/* Externals */

Styrolyse *styrolyse_new(const char *fname, StyrolyseClient *client)
{
    Styrolyse *_this;
    _this = (Styrolyse*)malloc(sizeof(Styrolyse));
    _this->client = client;
    _this->gsl =  gsl_new();
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

void styrolyse_execute(Styrolyse *_this, int mode)
{
  /* mutexifier cette fonction si multi-thread */
    GSL_GLOBAL_INT(_this->gsl, "@mode") = mode;
    styrolyse = _this;
    gsl_execute(_this->gsl);
}

void styrolyse_reload(Styrolyse *_this)
{
    char scriptPath[1024];
    char *fbuffer;
    if (!_this->gsl) return;
    sprintf(scriptPath, "%s/story/styrolyse.gsl", dataFolder);
    fbuffer = gsl_init_buffer(scriptPath);
    gsl_append_file_to_buffer(_this->fname, &fbuffer);
    gsl_compile(_this->gsl,fbuffer);
    sbind(_this->gsl);
    free(fbuffer);
    styrolyse_execute(_this, 0);
}

int styrolyse_finished(Styrolyse *_this)
{
    return GSL_GLOBAL_INT(_this->gsl, "@finished");
}

void styrolyse_update(Styrolyse *_this)
{
  if (!_this->gsl) return;
  styrolyse_execute(_this, 1);
}

void styrolyse_draw(Styrolyse *_this)
{
  if (!_this->gsl) return;
  styrolyse_execute(_this, 2);
}

int  styrolyse_current_cycle(Styrolyse *_this)
{
  if (!_this->gsl) return 0;
  return GSL_GLOBAL_INT(_this->gsl, "@cycle");
}