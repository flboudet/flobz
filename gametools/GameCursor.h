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
        CLICK = 1
    };
    struct CursorEventArg {
        CursorEventArg(int x, int y) : x(x), y(y) {}
        int x, y;
    };
    void setVisible(bool visible) { this->visible = visible; }
protected:
    virtual void draw(SDL_Surface *screen);
private:
    void setCursorPosition(int x, int y);
    void click(int x, int y);
    
    IIM_Surface * cursorSurface;
    int blitX, blitY;
    int prevblitX, prevblitY;
    float blitAngle, tgtBlitAngle;
    int idleDx, idleDy;
    bool visible;
};

#endif // _GameCursor_H

