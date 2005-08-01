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
  void  (*freeImage) (StyrolyseClient *_this, void *image);
  void  (*putText)   (StyrolyseClient *_this, int x, int y, const char *text);
};

Styrolyse *styrolyse_new(const char *fname, StyrolyseClient *client);
void styrolyse_reload(Styrolyse *_this);
void styrolyse_free(Styrolyse *_this);
void styrolyse_update(Styrolyse *_this);
int  styrolyse_finished(Styrolyse *_this);
void styrolyse_draw(Styrolyse *_this);

int  styrolyse_current_cycle(Styrolyse *_this);

#ifdef __cplusplus
};
#endif

#endif
