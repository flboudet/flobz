#ifndef _FLOBO_MESSAGE_DEF_H
#define _FLOBO_MESSAGE_DEF_H

/* y'a des conflits de nommage... */
namespace PuyoMessage {

enum {
    kGameState    = 0,
    kGameOverLost = 1,
    kGameStart    = 2,
    kGamePause    = 3,
    kGameResume   = 4,
    kGameNext     = 5,
    kGameAbort    = 6,
    kGameChat     = 7,
    kGameAlive    = 8,
    kGameOverWon  = 9,
    kGameSync     = 10,
    kGameAck      = 11,
};

  extern const char *TYPE;
  extern const char *GAMEID;
  extern const char *NAME;
  extern const char *NAME1;
  extern const char *NAME2;
  extern const char *PUYOS;
  extern const char *PAUSED;
  extern const char *SCORE;
  extern const char *TOTAL_SCORE;

  extern const char *COMBO_COUNT;
  extern const char *EXPLODE_COUNT;
  extern const char *DROP_COUNT;
  extern const char *GHOST_SENT_COUNT;
  extern const char *TIME_LEFT;
  extern const char *IS_DEAD;
  extern const char *IS_WINNER;

  extern const char *NEXT_F;
  extern const char *NEXT_C;
  extern const char *ADD_NEUTRALS;
  extern const char *DID_END_CYCLE;
  extern const char *SEMI_MOVE;
  extern const char *MV_L;
  extern const char *MV_R;
  extern const char *MV_D;
  extern const char *COMPANION_TURN;
  extern const char *DID_FALL;
  extern const char *WILL_VANISH;
  extern const char *CURRENT_NEUTRALS;
  extern const char *NUMBER_BAD_PUYOS;

};

#endif
