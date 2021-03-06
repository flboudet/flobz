#include <string.h>
#include "GTLog.h"
#include "Story.h"
#include "FPCommander.h"
#include "audio.h"
#include "Theme.h"

using namespace event_manager;

extern IosFont *storyFont;

DrawTarget *sstory;

/* Implementation of the Styrolyse Client */

void cachePicture(StyrolyseClient *_this, int mode, const char *path)
{
    ImageType type = (mode == 1 ? IMAGE_RGBA : IMAGE_RGB);
    theCommander->cacheSurface(type, path);
}

void cacheSound(StyrolyseClient *_this, const char *path)
{
    theCommander->cacheSound(path);
}

void cacheMusic(StyrolyseClient *_this, const char *path)
{
    theCommander->cacheMusic(path);
}

static char *pathResolverFunction (StyrolyseClient *_this, const char *path)
{
  try {
    String rsvPath = FilePath("/story").combine(path);
    return strdup((const char *)rsvPath);
  }
  catch (Exception e) {
    return strdup(path);
  }
}

static void *openFileFunction(StyrolyseClient *_this, const char *file_name)
{
    if (theCommander->getDataPathManager().hasDataInputStream(file_name))
        return (void *)(theCommander->getDataPathManager().openDataInputStream(file_name));
    else
        return NULL;
}

static void closeFileFunction(StyrolyseClient *_this, void *file)
{
    DataInputStream *s = (DataInputStream *)file;
    if (s)
        delete s;
}

static int readFileFunction(StyrolyseClient *_this, void *buffer, void *file, int read_size)
{
    DataInputStream *s = (DataInputStream *)file;
    if (s)
        return s->streamRead(buffer, read_size);
    else
        return 0;
}

// "@state.type"
int extract_state_and_type(const char *s, int *state, int *type)
{
    const char *first;
    first = strchr(s,'@');
    if(first != NULL){
        *state = atoi(first+1);
    }
    const char *second;
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


StyroImage::StyroImage(StyrolyseClient *_this,
                       const char *path, bool removePrefix)
    : path(path), surface(NULL)
{
    for (int i = 0 ; i < 10 ; ++i)
        alphaSurface[i] = NULL;
    if (path[0] == '@') {
        int state = 0;
        int type = 0;
        const FloboSetTheme *attachedTheme = ((StoryWidget::StoryStyrolyseClient *)_this)->attachedTheme;
        if ((attachedTheme != NULL)
            && (extract_state_and_type(path,&state,&type)==0)) {
            const FloboTheme &theme = attachedTheme->getFloboTheme((FloboState)state);
            switch (type) {
            case 0: // FLOBO_FACES
                surface = theme.getFloboSurfaceForValence(0);
                break;
            case 1: // FLOBO_CIRCLES
                surface = theme.getCircleSurfaceForIndex(0);
                break;
            case 2: // FLOBO_EXPLOSIONS
                surface = theme.getExplodingSurfaceForIndex(0);
                break;
            case 3: //FLOBO_DISAPPEAR
                surface = theme.getShrinkingSurfaceForIndex(0);
                break;
            case 4: //FLOBO_EYES
                surface = theme.getEyeSurfaceForIndex(0);
                break;
            case 5: //FLOBO_SHADOWS
            default:
                surface = theme.getShadowSurface();
                break;
            }
        }
    }
    else {
        String imgPath = (removePrefix ? String(path)
                          : FilePath("gfx").combine(path));
        surfaceRef = theCommander->getSurface(IMAGE_RGBA, imgPath);
        if (surfaceRef.empty())
            throw Exception(String("Image ") + imgPath + " not found!");
        surface = surfaceRef;
    }
}

static void *loadImage (StyrolyseClient *_this, const char *path)
{
  try {
      return new StyroImage(_this, path);
  }
  catch (Exception e) {
    return NULL;
  }
}


static void  drawImage (StyrolyseClient *_this, void *image, int x, int y, int w, int h,
                 int clipx, int clipy, int clipw, int cliph, int flipped, float scalex, float scaley, float alpha)
{
  StyroImage  *simg = (StyroImage*)image;
  IosSurface *surf = simg->surface;
  if (alpha != 1.) {
      int sindex = alpha * 10;
      if (simg->alphaSurface[sindex] == NULL)
          simg->alphaSurface[sindex] = surf->setAlpha(sindex / 10.);
      surf = simg->alphaSurface[sindex];
  }
  IosRect  rect, cliprect;
  rect.x = x;
  rect.y = y;
  if (w < 0) {
    rect.h = surf->h;
    rect.w = surf->w;
  }
  else {
      rect.h = h;
      rect.w = w;
  }

  cliprect.x = clipx;
  cliprect.y = clipy;
  cliprect.w = clipw;
  cliprect.h = cliph;
  sstory->setClipRect(&cliprect);
  if (fabs(scalex - 1.0f) > 0.001f) {
      rect.w *= scalex;
      rect.h *= scaley;
  }
  if (flipped) {
	  sstory->drawHFlipped(surf, NULL, &rect);
  }
  else {
	  sstory->draw(surf, NULL, &rect);
  }
}

static void  freeImage (StyrolyseClient *_this, void *image)
{
    delete (StyroImage*)image;
}

static void putText (StyrolyseClient *_this, int x, int y, const char *text)
{
    sstory->setClipRect(NULL);
    sstory->putString(storyFont, x, y, text, GT_WHITE);
}

static const char *getText(StyrolyseClient *_this, const char *text)
{
    return ((StoryWidget::StoryStyrolyseClient *)_this)->widget->getText(text);
}

static void music(StyrolyseClient *_this, const char *name)
{
    theCommander->playMusicTrack(name);
}

static void playSound(StyrolyseClient *_this, const char *fileName, float volume)
{
    theCommander->playSound(fileName, volume);
}

bool StoryWidget::classInitialized = false;

StoryWidget::StoryWidget(String screenName, Action *finishedAction, bool fxMode)
    : IdleComponent(), localeDictionary(NULL), finishedAction(finishedAction), once(false), last_time(-1.), fxMode(fxMode), m_renderEnabled(true)
{
    // GTLogTrace("StoryWidget::StoryWidget(%s)", (const char *)screenName);
    try {
        localeDictionary = new LocalizedDictionary(theCommander->getDataPathManager(), "locale/story", screenName);
    } catch (...) {
        GTLogTrace("StoryWidget::StoryWidget() locale error");
    }
    if (!classInitialized) {
        String path0 = "/lib/styrolyse.gsl";
        String path1 = "/lib/nofx.gsl";
        String path2 = "/lib/fx.gsl";
        styrolyse_init(path0.c_str(), path1.c_str(), path2.c_str());
        classInitialized = true;
    }
    fullPath = String("/story/") + screenName;
    if (!theCommander->getDataPathManager().hasFile(fullPath)) {
        printf("GSL NOT FOUND: %s (%s)\n", screenName.c_str(), (const char *)fullPath);
        fullPath = "/story/error.gsl";
    }
    String storyLocalePath;

    // Initializing the styrolyse client
    client.styroClient.loadImage = loadImage;
    client.styroClient.drawImage = drawImage;
    client.styroClient.freeImage = freeImage;
    client.styroClient.putText   = putText;
    client.styroClient.getText   = ::getText;
    client.styroClient.music     = ::music;
    client.styroClient.playSound = ::playSound;
    client.styroClient.resolveFilePath = ::pathResolverFunction;
    client.styroClient.openFile = ::openFileFunction;
    client.styroClient.closeFile = ::closeFileFunction;
    client.styroClient.readFile = ::readFileFunction;
    client.styroClient.cachePicture = ::cachePicture;
    client.styroClient.cacheSound   = ::cacheSound;
    client.styroClient.cacheMusic   = ::cacheMusic;
    client.widget = this;
    // GTLogTrace("StoryWidget::StoryWidget() styrolyse_new(%s)", (const char *)fullPath);
    client.attachedTheme = NULL;
    currentStory = styrolyse_new((const char *)fullPath, (StyrolyseClient *)(&client), fxMode);
    // GTLogTrace("StoryWidget::StoryWidget() styrolyse_new finished");
}

StoryWidget::~StoryWidget()
{
    styrolyse_free(currentStory);
    if (localeDictionary != NULL)
        delete localeDictionary;
}

void StoryWidget::reset()
{
    styrolyse_free(currentStory);
    currentStory = styrolyse_new((const char *)fullPath, (StyrolyseClient *)(&client), fxMode);
}

void StoryWidget::idle(double currentTime)
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

void StoryWidget::draw(DrawTarget *dt)
{
    if (hidden) return;
    if (!m_renderEnabled) return;
    render(dt);
}
void StoryWidget::render(DrawTarget *dt)
{
    sstory = dt;
    styrolyse_draw(currentStory);
    dt->setClipRect(NULL);
}

void StoryWidget::setIntegerValue(String varName, int value)
{
    styrolyse_setint(currentStory, varName, value);
}

int StoryWidget::getIntegerValue(String varName) const
{
    return styrolyse_secured_getint(currentStory, varName);
}

void StoryWidget::setFloatValue(String varName, float value)
{
    styrolyse_setfloat(currentStory, varName, value);
}

float StoryWidget::getFloatValue(String varName) const
{
    return styrolyse_secured_getfloat(currentStory, varName);
}

void StoryWidget::setStringValue(String varName, const char * value)
{
    styrolyse_setstr(currentStory, varName, value);
}

std::string StoryWidget::getStringValue(String varName) const
{
    return styrolyse_secured_getstr(currentStory, varName);
}

const char *StoryWidget::getText(const char *text) const
{
    if (localeDictionary == NULL)
        return text;
    return localeDictionary->getLocalizedString(text);
}

void StoryWidget::freeMemory()
{
    styrolyse_reduce_memory(currentStory);
}

StoryScreen::StoryScreen(String screenName, Action *finishedAction, bool shouldAddTransition)
    : Screen(), storyWidget(screenName, finishedAction),
      transitionWidget(NULL), finishedAction(finishedAction)
{
    add(&storyWidget);
}

void StoryScreen::onTransitionFromScreen(Screen &fromScreen)
{
    if (transitionWidget != NULL) {
        remove(transitionWidget);
        delete(transitionWidget);
    }
    transitionWidget = theCommander->createScreenTransition(fromScreen);
    add(transitionWidget);
}

StoryScreen::~StoryScreen()
{
    if (transitionWidget != NULL) {
        remove(transitionWidget);
        delete(transitionWidget);
    }
}

void StoryScreen::onEvent(GameControlEvent *cevent)
{
    bool passEvent = true;
    if ((!cevent->isUp) ||(cevent->cursorEvent==kGameMouseUp))
    switch (cevent->cursorEvent) {
    case kBack:
    case kStart:
    case kGameMouseUp:
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

void StoryScreen::onScreenVisibleChanged(bool visible)
{
    theCommander->setCursorVisible(!visible);
    Screen::onScreenVisibleChanged(visible);
}


VisualFX::VisualFX(String fxName, const FloboSetTheme &floboSetTheme)
    : StoryWidget(fxName,NULL,true), fxName(fxName)
{
    client.attachedTheme = &floboSetTheme;
}

bool VisualFX::busy() const
{
    return getIntegerValue("@busy") != 0;
}

bool VisualFX::supportFX(const char *fx) const
{
    String haystack(styrolyse_getstr(currentStory, "@supported_fx"));
    String needle(fx);
    haystack = String(",") + haystack + ",";
    needle = String(",") + needle + ",";
    return (strstr(haystack.c_str(), needle.c_str()) != NULL);
}

VisualFX *VisualFX::clone() const
{
    VisualFX *fx = new VisualFX(fxName, *client.attachedTheme);
    fx->setGameScreen(screen);
    return fx;
}

void VisualFX::postEvent(const char *name, float x, float y, int player)
{
    styrolyse_event(currentStory, name, x, y, player);
}

