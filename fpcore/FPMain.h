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

    std::string m_dataDir;
    bool m_fullscreen;
    int m_maxDataPackNumber;

    PreferencesManager *m_preferencesManager;
    CompositeDrawContext *m_drawContext;
    DrawContext *m_nativeDrawContext;
    event_manager::EventManager *m_eventManager;
    audio_manager::AudioManager *m_audioManager;

    Jukebox m_jukebox;

    FPDataPathManager m_dataPathManager;

    GameLoop   *loop;
    MainScreen *mainScreen;
    GameCursor *cursor;
};

#endif // _FLOBO_MAIN_H_

