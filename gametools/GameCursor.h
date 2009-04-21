#ifndef _GameCursor_H
#define _GameCursor_H

#include "gameloop.h"
#include "IosImgProcess.h"

class GameCursor : public DrawableComponent, public CycledComponent
{
public:
    GameCursor(const char *cursorImage);
    virtual ~GameCursor();
    virtual void onEvent(GameControlEvent *event);
    virtual void cycle();
    // SDL events
    enum {
        MOVE = 0,
        MOUSE_DOWN = 1,
        MOUSE_UP = 2
    };
    struct CursorEventArg {
        CursorEventArg(int x, int y) : x(x), y(y) {}
        int x, y;
    };
    void setVisible(bool visible) { this->visible = visible; }
    void setObscured(bool obscured) { this->obscured = obscured; }
protected:
    virtual void draw(DrawTarget *dt);
private:
    void setCursorPosition(int x, int y);
    void pushMouseEvent(int x, int y, int eventType);

    IosSurface * cursorSurface;
    int blitX, blitY;
    int prevblitX, prevblitY;
    float blitAngle, tgtBlitAngle;
    int idleDx, idleDy;
    bool visible;
    bool obscured;
};

#endif // _GameCursor_H

