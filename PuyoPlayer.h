#ifndef PUYOPLAYER_H
#define PUYOPLAYER_H

#include "PuyoGame.h"

class PuyoPlayer {
public:
    PuyoPlayer(PuyoGame *targetGame) : attachedGame(targetGame) {}
    virtual void cycle() = 0;
protected:
        PuyoGame *attachedGame;
};

#endif
