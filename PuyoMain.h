#ifndef _PUYO_MAIN_H_
#define _PUYO_MAIN_H_

#include "PuyoCommander.h"
#include "drawcontext.h"

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
    void initWithoutGUI();
    void initMenus();
    void initSDL();

    String m_dataDir;
    bool m_fullscreen;
    int m_maxDataPackNumber;

    DrawContext *m_drawContext;
    GameLoop   *loop;
    MainScreen *mainScreen;
    GameCursor *cursor;
};

#endif // _PUYO_MAIN_H_

