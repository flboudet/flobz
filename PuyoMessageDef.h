#ifndef _PUYO_MESSAGE_DEF_H
#define _PUYO_MESSAGE_DEF_H

enum {
    kGameState   = 0,
    kGameOver    = 1,
    kAddNeutral  = 2
};

namespace PuyoMessage {

static const char *TYPE = "TYPE";
static const char *NAME = "NAME";
static const char *PUYOS = "PUYOS";

};

#endif
