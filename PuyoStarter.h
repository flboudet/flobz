#include "PuyoGame.h"
#include "PuyoIA.h"
#include "PuyoCommander.h"

#ifndef _PUYOSTARTER
#define _PUYOSTARTER

class PuyoStarter : public PuyoDrawable {
public:
    PuyoStarter(PuyoCommander *commander, bool aiLeft, int aiLevel, IA_Type aiType, int theme);
    virtual ~PuyoStarter();
    void run(int score1, int score2, int lives, int point1, int point2);
    void draw();
    
    bool leftPlayerWin() const  { return attachedGameA->isGameRunning(); }
    bool rightPlayerWin() const { return attachedGameB->isGameRunning(); }
    
    int leftPlayerPoints() const { return attachedGameA->getPoints(); }
    int rightPlayerPoints() const { return attachedGameB->getPoints(); }
    
private:
        PuyoCommander *commander;
    PuyoView *areaA, *areaB;
    PuyoGame *attachedGameA, *attachedGameB;
    PuyoIA *randomPlayer;
    PuyoRandomSystem attachedRandom;
    int tickCounts;
    int lives;
    int score1;
    int score2;
    bool stopRendering;
    bool paused;
    int gameSpeed;
    
    void stopRender();
    void restartRender();
    
    int blinkingPointsA, blinkingPointsB, savePointsA, savePointsB;
};

#endif // _PUYOSTARTER

