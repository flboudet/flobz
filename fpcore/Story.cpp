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
    std::string rsvPath = FilePath("/story").combine(path);
    return strdup(rsvPath.c_str());
  }
  catch (const std::exception &e) {
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
                       const std::string &path, bool removePrefix)
    : _path(path), _surface(NULL)
{
    for (int i = 0 ; i < 10 ; ++i)
        _alphaSurface[i] = NULL;
    if (path[0] == '@') {
        int state = 0;
        int type = 0;
        const FloboSetTheme *attachedTheme = ((StoryWidget::StoryStyrolyseClient *)_this)->attachedTheme;
        if ((attachedTheme != NULL)
            && (extract_state_and_type(path.c_str(),&state,&type)==0)) {
            const FloboTheme &theme = attachedTheme->getFloboTheme((FloboState)state);
            switch (type) {
            case 0: // FLOBO_FACES
                _surface = theme.getFloboSurfaceForValence(0);
                break;
            case 1: // FLOBO_CIRCLES
                _surface = theme.getCircleSurfaceForIndex(0);
                break;
            case 2: // FLOBO_EXPLOSIONS
                _surface = theme.getExplodingSurfaceForIndex(0);
                break;
            case 3: //FLOBO_DISAPPEAR
                _surface = theme.getShrinkingSurfaceForIndex(0);
                break;
            case 4: //FLOBO_EYES
                _surface = theme.getEyeSurfaceForIndex(0);
                break;
            case 5: //FLOBO_SHADOWS
            default:
                _surface = theme.getShadowSurface();
                break;
            }
        }
    }
    else {
        std::string imgPath = (removePrefix ? path
                          : FilePath("gfx").combine(path));
        _surfaceRef = theCommander->getSurface(IMAGE_RGBA, imgPath);
        if (_surfaceRef.empty())
            throw std::runtime_error((std::string("Image ") + imgPath + " not found!").c_str());
        _surface = _surfaceRef;
    }
}

static void *loadImage (StyrolyseClient *_this, const char *path)
{
  try {
      return new StyroImage(_this, path);
  }
  catch (const std::exception &e) {
    return NULL;
  }
}


static void  drawImage (StyrolyseClient *_this, void *image, int x, int y, int w, int h,
                 int clipx, int clipy, int clipw, int cliph, int flipped, float scalex, float scaley, float alpha)
{
  StyroImage  *simg = (StyroImage*)image;
  IosSurface *surf = simg->_surface;
  if (alpha != 1.) {
      int sindex = alpha * 10;
      if (simg->_alphaSurface[sindex] == NULL)
          simg->_alphaSurface[sindex] = surf->setAlpha(sindex / 10.);
      surf = simg->_alphaSurface[sindex];
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
    return ((StoryWidget::StoryStyrolyseClient *)_this)->widget->getText(text).c_str();
}

static void music(StyrolyseClient *_this, const char *name)
{
    theCommander->playMusicTrack(name);
}

static void playSound(StyrolyseClient *_this, const char *fileName, float volume)
{
    theCommander->playSound(fileName, volume);
}

bool StoryWidget::_classInitialized = false;

StoryWidget::StoryWidget(const std::string & screenName, Action *finishedAction, bool fxMode)
    : IdleComponent(), _localeDictionary(NULL), _finishedAction(finishedAction), _once(false), _last_time(-1.), _fxMode(fxMode), _renderEnabled(true)
{
    // GTLogTrace("StoryWidget::StoryWidget(%s)", (const char *)screenName);
    try {
        _localeDictionary = new LocalizedDictionary(theCommander->getDataPathManager(), "locale/story", screenName);
    } catch (...) {
        GTLogTrace("StoryWidget::StoryWidget() locale error");
    }
    if (!_classInitialized) {
        std::string path0 = "/lib/styrolyse.gsl";
        std::string path1 = "/lib/nofx.gsl";
        std::string path2 = "/lib/fx.gsl";
        styrolyse_init(path0.c_str(), path1.c_str(), path2.c_str());
        _classInitialized = true;
    }
    _fullPath = std::string("/story/") + screenName;
    if (!theCommander->getDataPathManager().hasFile(_fullPath)) {
        printf("GSL NOT FOUND: %s (%s)\n", screenName.c_str(), _fullPath.c_str());
        _fullPath = "/story/error.gsl";
    }
    std::string storyLocalePath;

    // Initializing the styrolyse client
    _client.styroClient.loadImage = loadImage;
    _client.styroClient.drawImage = drawImage;
    _client.styroClient.freeImage = freeImage;
    _client.styroClient.putText   = putText;
    _client.styroClient.getText   = ::getText;
    _client.styroClient.music     = ::music;
    _client.styroClient.playSound = ::playSound;
    _client.styroClient.resolveFilePath = ::pathResolverFunction;
    _client.styroClient.openFile = ::openFileFunction;
    _client.styroClient.closeFile = ::closeFileFunction;
    _client.styroClient.readFile = ::readFileFunction;
    _client.styroClient.cachePicture = ::cachePicture;
    _client.styroClient.cacheSound   = ::cacheSound;
    _client.styroClient.cacheMusic   = ::cacheMusic;
    _client.widget = this;
    // GTLogTrace("StoryWidget::StoryWidget() styrolyse_new(%s)", (const char *)_fullPath);
    _client.attachedTheme = NULL;
    _currentStory = styrolyse_new(_fullPath.c_str(), (StyrolyseClient *)(&_client), _fxMode);
    // GTLogTrace("StoryWidget::StoryWidget() styrolyse_new finished");
}

StoryWidget::~StoryWidget()
{
    styrolyse_free(_currentStory);
    if (_localeDictionary != NULL)
        delete _localeDictionary;
}

void StoryWidget::reset()
{
    styrolyse_free(_currentStory);
    _currentStory = styrolyse_new(_fullPath.c_str(), (StyrolyseClient *)(&_client), _fxMode);
}

void StoryWidget::idle(double currentTime)
{
    double delta_t;
    if (_last_time < 0.)
        delta_t = 0.;
    else
        delta_t = currentTime - _last_time;
    _last_time = currentTime;

    styrolyse_update(_currentStory, (float)delta_t);
    requestDraw();
    if (styrolyse_finished(_currentStory) && !_once) {
        _once = true;
        if (_finishedAction)
	  _finishedAction->action(this, 0, NULL);
    }
}

void StoryWidget::draw(DrawTarget *dt)
{
    if (hidden) return;
    if (!_renderEnabled) return;
    render(dt);
}
void StoryWidget::render(DrawTarget *dt)
{
    sstory = dt;
    styrolyse_draw(_currentStory);
    dt->setClipRect(NULL);
}

void StoryWidget::setIntegerValue(const std::string & varName, int value)
{
    styrolyse_setint(_currentStory, varName.c_str(), value);
}

int StoryWidget::getIntegerValue(const std::string & varName) const
{
    return styrolyse_secured_getint(_currentStory, varName.c_str());
}

void StoryWidget::setFloatValue(const std::string & varName, float value)
{
    styrolyse_setfloat(_currentStory, varName.c_str(), value);
}

float StoryWidget::getFloatValue(const std::string & varName) const
{
    return styrolyse_secured_getfloat(_currentStory, varName.c_str());
}

void StoryWidget::setStringValue(const std::string & varName, const char * value)
{
    styrolyse_setstr(_currentStory, varName.c_str(), value);
}

std::string StoryWidget::getStringValue(const std::string & varName) const
{
    return styrolyse_secured_getstr(_currentStory, varName.c_str());
}

const std::string &StoryWidget::getText(const std::string &text) const
{
    if (_localeDictionary == NULL)
        return text;
    return _localeDictionary->getLocalizedString(text);
}

void StoryWidget::freeMemory()
{
    styrolyse_reduce_memory(_currentStory);
}

StoryScreen::StoryScreen(const std::string & screenName, Action *finishedAction, bool shouldAddTransition)
    : Screen(), _storyWidget(screenName, finishedAction),
      _transitionWidget(NULL), _finishedAction(finishedAction)
{
    add(&_storyWidget);
}

void StoryScreen::onTransitionFromScreen(Screen &fromScreen)
{
    if (_transitionWidget != NULL) {
        remove(_transitionWidget);
        delete(_transitionWidget);
    }
    _transitionWidget = theCommander->createScreenTransition(fromScreen);
    add(_transitionWidget);
}

StoryScreen::~StoryScreen()
{
    if (_transitionWidget != NULL) {
        remove(_transitionWidget);
        delete(_transitionWidget);
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
        if (_finishedAction != NULL) {
            _finishedAction->action(&_storyWidget, 0, cevent);
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


VisualFX::VisualFX(const std::string & fxName, const FloboSetTheme &floboSetTheme)
    : StoryWidget(fxName,NULL,true), _fxName(fxName)
{
    _client.attachedTheme = &floboSetTheme;
}

bool VisualFX::busy() const
{
    return getIntegerValue("@busy") != 0;
}

bool VisualFX::supportFX(const char *fx) const
{
    std::string haystack(styrolyse_getstr(_currentStory, "@supported_fx"));
    std::string needle(fx);
    haystack = std::string(",") + haystack + ",";
    needle = std::string(",") + needle + ",";
    return (strstr(haystack.c_str(), needle.c_str()) != NULL);
}

VisualFX *VisualFX::clone() const
{
    VisualFX *fx = new VisualFX(_fxName, *_client.attachedTheme);
    fx->setGameScreen(_screen);
    return fx;
}

void VisualFX::postEvent(const char *name, float x, float y, int player)
{
    styrolyse_event(_currentStory, name, x, y, player);
}

