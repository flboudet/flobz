#ifndef _PUYO_MESSAGE_DEF_H
#define _PUYO_MESSAGE_DEF_H

/* y'a des conflits de nommage... */
namespace PuyoMessage {

enum {
    kGameState   = 0,
    kGameOver    = 1,
    kGameStart   = 2,
    kGamePause   = 3,
    kGameResume  = 4,
    kGameNext    = 5,
    kGameAbort   = 6,
    kGameChat    = 7,
    kGameAlive   = 8
};

static const char *TYPE          = "TYPE";
static const char *GAMEID        = "ID";
static const char *NAME          = "NAME";
static const char *PUYOS         = "PUYOS";
static const char *PAUSED        = "PAUSED";
static const char *SCORE         = "SCORE";
static const char *NEXT_F        = "NEXT_F";
static const char *NEXT_C        = "NEXT_C";
static const char *ADD_NEUTRALS  = "ADD_NEUTRALS";
static const char *DID_END_CYCLE = "DID_END_CYCLE";
static const char *SEMI_MOVE     = "SEMI_MOVE";
static const char *COMPANION_TURN = "COMPANION_TURN";
static const char *DID_FALL       = "DID_FALL";
static const char *WILL_VANISH    = "WILL_VANISH";
static const char *CURRENT_NEUTRALS = "CURRENT_NEUTRALS";
static const char *NUMBER_BAD_PUYOS = "NUMBER_BAD_PUYOS";

};

#endif
