#ifndef PUYOIA_H
#define PUYOIA_H

#include "PuyoPlayer.h"

enum IA_Type {
  RANDOM,
  FLOBO,
  JEKO,
  TANIA,
  GYOM
};

class PuyoIA : public virtual PuyoPlayer {
public:
    PuyoIA(IA_Type type, int level, PuyoView *targetView);
    virtual void cycle();
 private:
    PuyoState extractColor(PuyoState A) const;
    struct PosEvaluator *evaluator;
    int choosenMove;
    bool firstLine;
    IA_Type type;
    int level;
};

#endif // PUYOIA_H

