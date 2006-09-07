#include "PuyoStory.h"
#include "PuyoCommander.h"

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
  try {
    String imgPath = theCommander->getDataPathManager().getPath(FilePath("gfx").combine(path));
    return IIM_Load_Absolute_DisplayFormatAlpha(imgPath);
  }
  catch (Exception e) {
    return NULL;
  }
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
  String output;
  char previousChar = text[0];
  for (int i = 0 ; i < strlen(text) ; i++) {
    char texti = text[i];
    switch (previousChar) {
    case '\\':
        switch (texti) {
        case 'n':
            output += '\n';
            break;
        default:
            output += texti;
        }
        break;
    default:
        switch (texti) {
        case '\\':
            break;
        default:
            output += texti;
        }
    }
    previousChar = texti;
  }
  SoFont_PutString (storyFont, sstory, x, y, output, NULL);
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

bool PuyoStoryWidget::classInitialized = false;

PuyoStoryWidget::PuyoStoryWidget(String screenName, Action *finishedAction) : CycledComponent(0.04), finishedAction(finishedAction), once(false)
{
    if (!classInitialized) {
        styrolyse_init(theCommander->getDataPathManager().getPath("lib/styrolyse.gsl"));
        classInitialized = true;
    }
    FILE *test = NULL;
    String fullPath;
    try {
        fullPath = theCommander->getDataPathManager().getPath(String("/story/") + screenName);
        test = fopen((const char *)fullPath, "r");
    }
    catch (Exception e) {
    }
    if (test == NULL) {
        printf("%s not found!!!\n", (const char *)fullPath);
        fullPath = theCommander->getDataPathManager().getPath("/story/storyz.gsl");
    }
    else fclose(test);
    String storyLocalePath;
    try {
        storyLocalePath = theCommander->getDataPathManager().getPath(String("locale/story/") + screenName);
    } catch (...) {}
    printf("Chargement locale: %s\n", (const char *)storyLocalePath);
    currentStory = styrolyse_new((const char *)fullPath, storyLocalePath, client_new());
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
    bool passEvent = true;
    switch (cevent->cursorEvent) {
    //case GameControlEvent::kStart:
    case GameControlEvent::kBack:
        if (finishedAction != NULL) {
            finishedAction->action();
	    passEvent = false;
        }
        break;
    }
    if (passEvent)
      Screen::onEvent(cevent);
}

