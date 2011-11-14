#ifndef _FLOBO_MAIN_H_
#define _FLOBO_MAIN_H_

#include "MainScreen.h"
#include "PuyoCommander.h"
#include "PreferencesManager.h"
#include "FPDataPathManager.h"
#include "drawcontext.h"
#include "CompositeDrawContext.h"

class PuyoMain : NotificationResponder
{
public:
    PuyoMain(String dataDir, bool fullscreen, int maxDataPackNumber=-1);
    virtual ~PuyoMain();
    void run();
    void connect_ia(String params);
    void debug_gsl(String gsl_script);
protected:
    void notificationOccured(String identifier, void * context);
private:
    void initWithGUI();
    void initMenus();
    void initSDL();

    String m_dataDir;
    bool m_fullscreen;
    int m_maxDataPackNumber;

    PreferencesManager *m_preferencesManager;
    CompositeDrawContext *m_drawContext;
    DrawContext *m_nativeDrawContext;
    event_manager::EventManager *m_eventManager;
    audio_manager::AudioManager *m_audioManager;

    FPDataPathManager m_dataPathManager;

    GameLoop   *loop;
    MainScreen *mainScreen;
    GameCursor *cursor;
};

#endif // _FLOBO_MAIN_H_

