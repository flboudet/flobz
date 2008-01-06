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
  void  (*drawImage) (StyrolyseClient *_this, void *image, int x, int y,
                      int clipx, int clipy, int clipw, int cliph);
  void  (*drawImageAdvanced) (StyrolyseClient *_this, void *image, int x, int y,
                      int clipx, int clipy, int clipw, int cliph, int degrees, int alpha);
  void  (*freeImage) (StyrolyseClient *_this, void *image);
  void  (*putText)   (StyrolyseClient *_this, int x, int y, const char *text);
  const char *(*getText)   (StyrolyseClient *_this, const char *text);
  void (*playMusic) (StyrolyseClient *_this, const char *fileName);
  void (*playSound) (StyrolyseClient *_this, const char *fileName, int volume);
  char *(*resolveFilePath)(StyrolyseClient *_this, const char *file_name);
};

void styrolyse_init(const char *styrolyse_path, const char *styrolyse_path_nofx, const char *styrolyse_path_fx);

Styrolyse *styrolyse_new(const char *fname, StyrolyseClient *client, int fxMode);
void styrolyse_reload(Styrolyse *_this);
void styrolyse_free(Styrolyse *_this);
void styrolyse_update(Styrolyse *_this, float delta_t);
int  styrolyse_finished(Styrolyse *_this);
void styrolyse_draw(Styrolyse *_this);

void styrolyse_setint(Styrolyse *_this, const char *varname, int value);
int styrolyse_getint(Styrolyse *_this, const char *varname);
int styrolyse_secured_getint(Styrolyse *_this, const char *varname);
const char *styrolyse_getstr(Styrolyse *_this, const char *varname);

void styrolyse_event(Styrolyse *_this, const char *event, float x, float y, int player);

int  styrolyse_current_cycle(Styrolyse *_this);

#ifdef __cplusplus
};
#endif

#endif

