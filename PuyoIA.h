#ifndef PUYOIA_H
#define PUYOIA_H

#include "PuyoPlayer.h"

enum IA_Type {
  RANDOM,
  CASTOR
};

class PuyoIA : public virtual PuyoPlayer {
public:
    PuyoIA(IA_Type type, int level, PuyoGame *targetGame);
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

