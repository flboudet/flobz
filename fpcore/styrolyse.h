#ifndef _STYROLYSE_H
#define _STYROLYSE_H

#ifdef __cplusplus
extern "C" {
#endif

/* #define USE_SDL */

typedef struct _Styrolyse Styrolyse;
typedef struct _StyrolyseClient StyrolyseClient;

struct _StyrolyseClient {
  void *(*loadImage) (StyrolyseClient *_this, const char *path);
  void  (*drawImage) (StyrolyseClient *_this, void *image, int x, int y, int w, int h,
                      int clipx, int clipy, int clipw, int cliph, int flipped,
					  float scaleX, float scaleY, float alpha);
  void  (*drawImageAdvanced) (StyrolyseClient *_this, void *image, int x, int y,
                      int clipx, int clipy, int clipw, int cliph, int degrees, int alpha);
  void  (*freeImage) (StyrolyseClient *_this, void *image);
  void  (*putText)   (StyrolyseClient *_this, int x, int y, const char *text);
  const char *(*getText)   (StyrolyseClient *_this, const char *text);
  void (*music) (StyrolyseClient *_this, const char *command);
  void (*playSound) (StyrolyseClient *_this, const char *fileName, float volume);
  char *(*resolveFilePath)(StyrolyseClient *_this, const char *file_name);
  void *(*openFile)(StyrolyseClient *_this, const char *file_name);
  void (*closeFile)(StyrolyseClient *_this, void *file);
  int (*readFile)(StyrolyseClient *_this, void *buffer, void *file, int read_size);
  void (*cachePicture)(StyrolyseClient *_this, int mode, const char *path);
  void (*cacheSound)(StyrolyseClient *_this, const char *path);
  void (*cacheMusic)(StyrolyseClient *_this, const char *path);
};

void styrolyse_init(const char *styrolyse_path, const char *styrolyse_path_nofx, const char *styrolyse_path_fx);

Styrolyse *styrolyse_new(const char *fname, StyrolyseClient *client, int fxMode);
void styrolyse_reload(Styrolyse *_this);
void styrolyse_free(Styrolyse *_this);
void styrolyse_reduce_memory(Styrolyse *_this);
void styrolyse_update(Styrolyse *_this, float delta_t);
int  styrolyse_finished(Styrolyse *_this);
void styrolyse_draw(Styrolyse *_this);

void styrolyse_setint(Styrolyse *_this, const char *varname, int value);
int styrolyse_getint(Styrolyse *_this, const char *varname);
int styrolyse_secured_getint(Styrolyse *_this, const char *varname);

void styrolyse_setfloat(Styrolyse *_this, const char *varname, float value);
float styrolyse_getfloat(Styrolyse *_this, const char *varname);
float styrolyse_secured_getfloat(Styrolyse *_this, const char *varname);

void styrolyse_setstr(Styrolyse *_this, const char *varname, const char * value);
const char *styrolyse_getstr(Styrolyse *_this, const char *varname);
const char *styrolyse_secured_getstr(Styrolyse *_this, const char *varname);

void styrolyse_event(Styrolyse *_this, const char *event, float x, float y, int player);

int  styrolyse_current_cycle(Styrolyse *_this);

#ifdef __cplusplus
};
#endif

#endif

