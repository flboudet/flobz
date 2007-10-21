#include "PuyoStory.h"
#include "PuyoCommander.h"
#include "audio.h"

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

static char *pathResolverFunction (StyrolyseClient *_this, const char *path)
{
  try {
    String rsvPath = theCommander->getDataPathManager().getPath(FilePath("story").combine(path));
    return strdup((const char *)rsvPath);
  }
  catch (Exception e) {
    return strdup(path);
  }
}

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
  SoFont_PutString (storyFont, sstory, x, y, text, NULL);
}

static const char *getText(StyrolyseClient *_this, const char *text)
{
    return ((PuyoStoryWidget::PuyoStoryStyrolyseClient *)_this)->widget->getText(text);
}

static void playMusic(StyrolyseClient *_this, const char *fileName)
{
    //AudioManager::playMusic(theCommander->getDataPathManager().getPath(String("/music/") + fileName));
    AudioManager::playMusic(fileName);
}

static void playSound(StyrolyseClient *_this, const char *fileName, int volume)
{
    //AudioManager::playSound(theCommander->getDataPathManager().getPath(String("/sfx/") + fileName), volume);
    AudioManager::playSound(fileName, volume);
}

bool PuyoStoryWidget::classInitialized = false;

PuyoStoryWidget::PuyoStoryWidget(String screenName, Action *finishedAction, bool fxMode)
    : /*Cycled*/IdleComponent(/*0.04*/), localeDictionary(NULL), finishedAction(finishedAction), once(false), last_time(-1.)
{
    try {
        localeDictionary = new PuyoLocalizedDictionary(theCommander->getDataPathManager(), "locale/story", screenName);
    } catch (...) {}

    if (!classInitialized) {
        String path0 = theCommander->getDataPathManager().getPath("lib/styrolyse.gsl");
        String path1 = theCommander->getDataPathManager().getPath("lib/nofx.gsl");
        styrolyse_init(path0.c_str(), path1.c_str());
        classInitialized = true;
    }

    FILE *test = NULL;
    String fullPath;
    try {
        fullPath = theCommander->getDataPathManager().getPath(String("story/") + screenName);
        test = fopen((const char *)fullPath, "r");
    }
    catch (Exception e) {
    }
    if (test == NULL) {
        printf("GSL NOT FOUND: %s (%s)\n", screenName.c_str(), (const char *)fullPath);
        fullPath = theCommander->getDataPathManager().getPath("story/error.gsl");
    }
    else fclose(test);
    String storyLocalePath;
    
    // Initializing the styrolyse client
    client.styroClient.loadImage = loadImage;
    client.styroClient.drawImage = drawImage;
    client.styroClient.freeImage = freeImage;
    client.styroClient.putText   = putText;
    client.styroClient.getText   = ::getText;
    client.styroClient.playMusic = ::playMusic;
    client.styroClient.playSound = ::playSound;
    client.styroClient.resolveFilePath = ::pathResolverFunction;
    client.widget = this;
    
    currentStory = styrolyse_new((const char *)fullPath, (StyrolyseClient *)(&client), fxMode);
    //styrolyse_setuserpointer(currentStory, this);
    //sstory = createStorySurface();
}

PuyoStoryWidget::~PuyoStoryWidget()
{
    styrolyse_free(currentStory);
    if (localeDictionary != NULL)
        delete localeDictionary;
}

// void PuyoStoryWidget::cycle()
void PuyoStoryWidget::idle(double currentTime)
{
    double delta_t;
    if (last_time < 0.)
        delta_t = 0.;
    else
        delta_t = currentTime - last_time;
    last_time = currentTime;
    
    styrolyse_update(currentStory, (float)delta_t);
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

int PuyoStoryWidget::getIntegerValue(String varName) const
{
    return styrolyse_getint(currentStory, varName);
}

const char *PuyoStoryWidget::getText(const char *text) const
{
    if (localeDictionary == NULL)
        return text;
    return localeDictionary->getLocalizedString(text);
}

PuyoStoryScreen::PuyoStoryScreen(String screenName, Screen &previousScreen, Action *finishedAction, bool shouldAddTransition) : Screen(0, 0, 640, 480), storyWidget(screenName, finishedAction), finishedAction(finishedAction), transitionWidget(new PuyoScreenTransitionWidget(previousScreen, NULL))
{
    add(&storyWidget);
    if (shouldAddTransition)
        add(transitionWidget);
}

PuyoStoryScreen::PuyoStoryScreen(String screenName) : Screen(0, 0, 640, 480), storyWidget(screenName, NULL), finishedAction(NULL), transitionWidget(NULL)
{
    add(&storyWidget);
}

void PuyoStoryScreen::transitionFromScreen(Screen &fromScreen)
{
    if (transitionWidget != NULL) {
        remove(transitionWidget);
        delete(transitionWidget);
    }
    transitionWidget = new PuyoScreenTransitionWidget(fromScreen, NULL);
    add(transitionWidget);
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
    case GameControlEvent::kStart:
        if (finishedAction != NULL) {
            finishedAction->action();
	    passEvent = false;
        }
        break;
    }
    if (passEvent)
      Screen::onEvent(cevent);
}

PuyoFX::PuyoFX(String fxName)
    : PuyoStoryWidget(fxName,NULL,true)
{}

void PuyoFX::postEvent(const char *name, float x, float y)
{
    styrolyse_event(currentStory, name, x, y);
}
