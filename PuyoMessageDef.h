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
static const char *PAUSED = "PAUSED";
static const char *SCORE = "SCORE";
static const char *NEXT_F = "NEXT_F";
static const char *NEXT_C = "NEXT_C";
static const char *ADD_NEUTRALS = "ADD_NEUTRALS";
static const char *DID_END_CYCLE = "DID_END_CYCLE";

};

#endif
