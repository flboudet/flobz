#include "goomsl.h"
#include "styrolyse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "goomsl_hash.h"

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

void sprite_say(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
/*  int id = GSL_LOCAL_INT(gsl,global,"this.id");*/
}

void sprite_draw(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
  const char *path = GSL_LOCAL_PTR  (gsl, local, "&this.image");
  int         x    = GSL_LOCAL_FLOAT(gsl, local, "&this.pos.x");
  int         y    = GSL_LOCAL_FLOAT(gsl, local, "&this.pos.y");
  HashValue  *img  = goom_hash_get(styrolyse->images, path);
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
  styrolyse->client->drawImage(styrolyse->client, data, x, y);
}

/**/

static void bind(GoomSL *gsl)
{
  gsl_bind_function(gsl, "say",   sprite_say);
  gsl_bind_function(gsl, "draw",  sprite_draw);
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

static void images_free_from_hash(GoomHash *caller, const char *key, HashValue *value)
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
    sprintf(scriptPath, "%s/story/styrolyse.gsl",dataFolder);
    
    char *fbuffer;
    if (!_this->gsl) return;
    fbuffer = gsl_init_buffer(scriptPath);
    gsl_append_file_to_buffer(_this->fname, &fbuffer);
    gsl_compile(_this->gsl,fbuffer);
    bind(_this->gsl);
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
