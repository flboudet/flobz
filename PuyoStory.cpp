#include "PuyoStory.h"
#include "PuyoCommander.h"

#ifndef DATADIR
extern char *DATADIR;
#endif

SDL_Surface *sstory;

static SDL_Surface * createStorySurface()
{
  SDL_Surface *sstory;
  Uint32 rmask, gmask, bmask, amask;

  /* SDL interprets each pixel as a 32-bit number, so our masks must depend
   *               on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  rmask = 0xff000000;
  gmask = 0x00ff0000;
  bmask = 0x0000ff00;
  amask = 0x00000000;
#else
  rmask = 0x000000ff;
  gmask = 0x0000ff00;
  bmask = 0x00ff0000;
  amask = 0x00000000;
#endif

  sstory = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, 32,
                               rmask, gmask, bmask, amask);
  if(sstory == NULL) {
    fprintf(stderr, "CreateRGBSurface failed: %s", SDL_GetError());
    exit(1);
  }
  return sstory;
}

/* Implementation of the Styrolyse Client */
static void *loadImage (StyrolyseClient *_this, const char *path)
{
  SDL_Surface *surf = IMG_Load(path);
  return surf;
}


static void  drawImage (StyrolyseClient *_this, void *image, int x, int y)
{
  SDL_Surface *surf = (SDL_Surface*)image;
  SDL_Rect  rect;
  rect.x = x;
  rect.y = y;
  SDL_BlitSurface(surf, NULL, sstory, &rect);
}

static void  freeImage (StyrolyseClient *_this, void *image)
{
  SDL_FreeSurface((SDL_Surface*)image);
}

static StyrolyseClient client;
StyrolyseClient *client_new()
{
  client.loadImage = loadImage;
  client.drawImage = drawImage;
  client.freeImage = freeImage;
  return &client;
};

PuyoStory::PuyoStory(PuyoCommander *com, int num) : num(num), commander(com)
{
    currentStory = styrolyse_new("demo.gsl", client_new());
    sstory = createStorySurface();
}

PuyoStory::~PuyoStory()
{
    styrolyse_free(currentStory);
}

void PuyoStory::loop()
{
    while (!styrolyse_finished(currentStory)) {
        styrolyse_update(currentStory);
        SDL_FillRect(sstory,  NULL, 0);
        styrolyse_draw(currentStory);
        commander->updateAll(this);
    }
}

void PuyoStory::draw()
{
    SDL_BlitSurface(sstory, NULL, display, NULL);
}
