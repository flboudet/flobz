#include "PuyoStory.h"
#include "PuyoCommander.h"

extern char *dataFolder;
extern SoFont *storyFont;

SDL_Surface *sstory;

static SDL_Surface * createStorySurface()
{
  SDL_Surface *sstory, *tmpsurf;
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
  tmpsurf = SDL_DisplayFormat(sstory);
  SDL_FreeSurface(sstory);
  return tmpsurf;
}

/* Implementation of the Styrolyse Client */
static void *loadImage (StyrolyseClient *_this, const char *path)
{
  char spath[2048];
  sprintf(spath,"story/%s", path);
  return IIM_Load_DisplayFormatAlpha(spath);
}


static void  drawImage (StyrolyseClient *_this, void *image, int x, int y,
                 int clipx, int clipy, int clipw, int cliph)
{
  IIM_Surface *surf = (IIM_Surface*)image;
  SDL_Rect  rect, cliprect;
  rect.x = x;
  rect.y = y;
  cliprect.x = clipx;
  cliprect.y = clipy;
  cliprect.w = clipw;
  cliprect.h = cliph;
  SDL_SetClipRect(sstory, &cliprect);
  SDL_BlitSurface(surf->surf, NULL, sstory, &rect);
}

static void  freeImage (StyrolyseClient *_this, void *image)
{
  IIM_Free((IIM_Surface*)image);
}

static void putText (StyrolyseClient *_this, int x, int y, const char *text)
{
  SoFont_PutString (storyFont, sstory, x, y, text, NULL);
}

static StyrolyseClient client;
StyrolyseClient *client_new()
{
  client.loadImage = loadImage;
  client.drawImage = drawImage;
  client.freeImage = freeImage;
  client.putText   = putText;
  return &client;
};


PuyoStoryWidget::PuyoStoryWidget(int num, Action *finishedAction) : CycledComponent(0.04), finishedAction(finishedAction), once(false)
{
    char scriptPath[1024];
    sprintf(scriptPath, "%s/story/story%d.gsl",dataFolder, num);
    FILE *test = fopen(scriptPath, "r");
    if (test == NULL) {
        sprintf(scriptPath, "%s/story/storyz.gsl",dataFolder);
    }
    else fclose(test);
    currentStory = styrolyse_new(scriptPath, client_new());
    sstory = createStorySurface();
}

PuyoStoryWidget::PuyoStoryWidget(String screenName, Action *finishedAction) : CycledComponent(0.04), finishedAction(finishedAction), once(false)
{
    char scriptPath[1024];
    String fullPath(dataFolder);
    fullPath += String("/story/") + screenName;
    FILE *test = fopen((const char *)fullPath, "r");
    if (test == NULL) {
        printf("%s not found!!!\n", (const char *)fullPath);
        fullPath = dataFolder;
        fullPath += "/story/storyz.gsl";
    }
    else fclose(test);
    currentStory = styrolyse_new((const char *)fullPath, client_new());
    //sstory = createStorySurface();
}

PuyoStoryWidget::~PuyoStoryWidget()
{
    styrolyse_free(currentStory);
}

void PuyoStoryWidget::cycle()
{
    styrolyse_update(currentStory);
    requestDraw();
    if (styrolyse_finished(currentStory) && !once) {
        once = true;
        if (finishedAction)
            finishedAction->action();
    }
}

void PuyoStoryWidget::draw(SDL_Surface *screen)
{
    if (hidden) return;
    sstory = screen;
    styrolyse_draw(currentStory);
    // SDL_BlitSurface(sstory, NULL, screen, NULL);
    SDL_SetClipRect(screen, NULL);
}

void PuyoStoryWidget::setIntegerValue(String varName, int value)
{
    styrolyse_setint(currentStory, varName, value);
}

PuyoStoryScreen::PuyoStoryScreen(int num, Screen &previousScreen, Action *finishedAction) : Screen(0, 0, 640, 480), storyWidget(num, finishedAction), finishedAction(finishedAction), transitionWidget(previousScreen, NULL)
{
    add(&storyWidget);
    add(&transitionWidget);
}

PuyoStoryScreen::PuyoStoryScreen(String screenName, Screen &previousScreen, Action *finishedAction) : Screen(0, 0, 640, 480), storyWidget(screenName, finishedAction), finishedAction(finishedAction), transitionWidget(previousScreen, NULL)
{
    add(&storyWidget);
    add(&transitionWidget);
}

PuyoStoryScreen::~PuyoStoryScreen()
{
    printf("Destruction de la story!\n");
}

void PuyoStoryScreen::onEvent(GameControlEvent *cevent)
{
    switch (cevent->cursorEvent) {
    //case GameControlEvent::kStart:
    case GameControlEvent::kBack:
        if (finishedAction != NULL)
            finishedAction->action();
        break;
    }
    Screen::onEvent(cevent);
}

