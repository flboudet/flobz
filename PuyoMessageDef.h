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

  extern const char *TYPE;
  extern const char *GAMEID;
  extern const char *NAME;
  extern const char *PUYOS;
  extern const char *PAUSED;
  extern const char *SCORE;
  extern const char *NEXT_F;
  extern const char *NEXT_C;
  extern const char *ADD_NEUTRALS;
  extern const char *DID_END_CYCLE;
  extern const char *SEMI_MOVE;
  extern const char *COMPANION_TURN;
  extern const char *DID_FALL;
  extern const char *WILL_VANISH;
  extern const char *CURRENT_NEUTRALS;
  extern const char *NUMBER_BAD_PUYOS;

};

#endif
