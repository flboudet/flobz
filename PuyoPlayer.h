#ifndef PUYOPLAYER_H
#define PUYOPLAYER_H

#include "PuyoView.h"
#include "PuyoGame.h"

class PuyoPlayer {

protected:
    PuyoView *attachedView;
    PuyoGame *attachedGame;

public:
    PuyoPlayer(PuyoView *targetView) : attachedView(targetView)
    {
        attachedGame = targetView->getAttachedGame();
    }
    virtual void cycle() = 0;

};

#endif
