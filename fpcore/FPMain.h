#ifndef _FLOBO_MAIN_H_
#define _FLOBO_MAIN_H_

#include <string>
#include "MainScreen.h"
#include "FPCommander.h"
#include "PreferencesManager.h"
#include "FPDataPathManager.h"
#include "drawcontext.h"
#include "CompositeDrawContext.h"
#include "audio.h"

class FPMain : NotificationResponder
{
public:
    FPMain(const std::string & dataDir, bool fullscreen, int maxDataPackNumber=-1);
    virtual ~FPMain();
    void run();
    void connect_ia(const std::string & params);
    void debug_gsl(const std::string & gsl_script);
protected:
    void notificationOccured(const std::string & identifier, void * context);
private:
    void initWithGUI();
    void initMenus();
    void initSDL();

    std::string _dataDir;
    bool _fullscreen;
    int _maxDataPackNumber;

    PreferencesManager *_preferencesManager;
    CompositeDrawContext *_drawContext;
    DrawContext *_nativeDrawContext;
    event_manager::EventManager *_eventManager;
    audio_manager::AudioManager *_audioManager;

    Jukebox _jukebox;

    FPDataPathManager _dataPathManager;

    GameLoop   *_loop;
    MainScreen *_mainScreen;
    GameCursor *_cursor;
};

#endif // _FLOBO_MAIN_H_

