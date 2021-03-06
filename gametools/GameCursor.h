#ifndef _GameCursor_H
#define _GameCursor_H

#include "gameloop.h"

namespace Cursor
{

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
}

class AbstractCursor
{
public:
    virtual void setVisible(bool visible) {}
    virtual void setObscured(bool obscured) {}
};

class GameCursor : public AbstractCursor,
                   public DrawableComponent,
                   public IdleComponent
{
public:
    GameCursor(const char *cursorImage);
    virtual ~GameCursor();
    virtual void onEvent(event_manager::GameControlEvent *event);
    void setVisible(bool visible) { this->visible = visible; }
    void setObscured(bool obscured) { this->obscured = obscured; }
protected:
    virtual void draw(DrawTarget *dt);
private:
    void setCursorPosition(int x, int y);

    IosSurface * cursorSurface;
    int blitX, blitY;
    int prevblitX, prevblitY;
    float blitAngle, tgtBlitAngle;
    bool visible;
    bool obscured;
};

#endif // _GameCursor_H

