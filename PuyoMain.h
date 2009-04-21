#ifndef _PUYO_MAIN_H_
#define _PUYO_MAIN_H_

#include "PuyoCommander.h"
#include "drawcontext.h"

class PuyoMain
{
public:
    PuyoMain(String dataDir, bool fullscreen, int maxDataPackNumber=-1);
    void run();
    void debug_gsl(String gsl_script);
private:
    void initMenus();
    void initSDL();
    void initDisplay(int w, int h, bool fullscreen, bool useGL);

    DrawContext *m_drawContext;
    GameLoop   *loop;
    MainScreen *mainScreen;
    GameCursor *cursor;
};

#endif // _PUYO_MAIN_H_

