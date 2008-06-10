#include "PuyoStory.h"
#include "PuyoCommander.h"
#include "audio.h"
#include "AnimatedPuyoTheme.h"

extern SoFont *storyFont;

SDL_Surface *sstory;

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

// "@state.type"
int extract_state_and_type(const char *s, int *state, int *type)
{
    char *first;
    first = strchr(s,'@');
    if(first != NULL){
        *state = atoi(first+1);
    }
    char *second;
    second = strchr(s,'.');
    if(second != NULL){
        *type = atoi(second+1);    
    }
   
    if (first!=NULL && second != NULL){
        return 0;
    }
    else{
        return 1;
    }
}


class StyroImage
{
    public:
        String path;
        IIM_Surface *surface;

        StyroImage(const char *path) : path(path)
        {
            if (path[0] == '@') {
                int state = 0;
                int type = 0;
                if (extract_state_and_type(path,&state,&type)==0){
                    surface = getPuyoThemeManger()->getAnimatedPuyoSetTheme()->getAnimatedPuyoTheme((PuyoState)state)->getSurface((PuyoPictureType)state, 0);
                }
                else{
                    surface = NULL;
                }
            }
            else {
                String imgPath = theCommander->getDataPathManager().getPath(FilePath("gfx").combine(path));
                surface = IIM_Load_Absolute_DisplayFormatAlpha(imgPath);
            }
        }

        ~StyroImage() {
            if (path[0] != '@')
                IIM_Free(surface);
        }
};

static void *loadImage (StyrolyseClient *_this, const char *path)
{
  try {
      return new StyroImage(path);
  }
  catch (Exception e) {
    return NULL;
  }
}


static void  drawImage (StyrolyseClient *_this, void *image, int x, int y,
                 int clipx, int clipy, int clipw, int cliph)
{
  StyroImage  *simg = (StyroImage*)image;
  IIM_Surface *surf = simg->surface;
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
    delete (StyroImage*)image;
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
        String path2 = theCommander->getDataPathManager().getPath("lib/fx.gsl");
        styrolyse_init(path0.c_str(), path1.c_str(), path2.c_str());
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
	  finishedAction->action(this, 0, NULL);
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
    return styrolyse_secured_getint(currentStory, varName);
}

const char *PuyoStoryWidget::getText(const char *text) const
{
    if (localeDictionary == NULL)
        return text;
    return localeDictionary->getLocalizedString(text);
}

PuyoStoryScreen::PuyoStoryScreen(String screenName, Screen &previousScreen, Action *finishedAction, bool shouldAddTransition) : Screen(0, 0, 640, 480), storyWidget(screenName, finishedAction), transitionWidget(new PuyoScreenTransitionWidget(previousScreen, NULL)), finishedAction(finishedAction)
{
    add(&storyWidget);
    if (shouldAddTransition)
        add(transitionWidget);
}

PuyoStoryScreen::PuyoStoryScreen(String screenName) : Screen(0, 0, 640, 480), storyWidget(screenName, NULL), transitionWidget(NULL), finishedAction(NULL)
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
    if (transitionWidget != NULL) {
        remove(transitionWidget);
        delete(transitionWidget);
    }
}

void PuyoStoryScreen::onEvent(GameControlEvent *cevent)
{
    bool passEvent = true;
    switch (cevent->cursorEvent) {
    case GameControlEvent::kBack:
    case GameControlEvent::kStart:
    case GameControlEvent::kGameMouseClicked:
        if (cevent->isUp)
            break;
        if (finishedAction != NULL) {
	  finishedAction->action(&storyWidget, 0, cevent);
	    passEvent = false;
        }
        break;
    default:
        break;
    }
    if (passEvent)
      Screen::onEvent(cevent);
}

void PuyoStoryScreen::onScreenVisibleChanged(bool visible)
{
    theCommander->setCursorVisible(!visible);
    Screen::onScreenVisibleChanged(visible);
}


PuyoFX::PuyoFX(String fxName)
    : PuyoStoryWidget(fxName,NULL,true), fxName(fxName)
{}

bool PuyoFX::busy() const
{
    return getIntegerValue("@busy") != 0;
}

bool PuyoFX::supportFX(const char *fx) const
{
    String haystack(styrolyse_getstr(currentStory, "@supported_fx"));
    String needle(fx);
    haystack = String(",") + haystack + ",";
    needle = String(",") + needle + ",";
    return (strstr(haystack.c_str(), needle.c_str()) != NULL);
}

PuyoFX *PuyoFX::clone() const
{
    PuyoFX *fx = new PuyoFX(fxName);
    fx->setGameScreen(screen);
    return fx;
}

void PuyoFX::postEvent(const char *name, float x, float y, int player)
{
    styrolyse_event(currentStory, name, x, y, player);
}

