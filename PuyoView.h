#ifndef _PUYOVIEW
#define _PUYOVIEW

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "PuyoGame.h"
#include "PuyoIA.h"
#include "PuyoCommander.h"

#define TSIZE 32
#define ASIZE 32
#define BSIZE 32
#define CSIZE 32
#define DSIZE 192
#define ESIZE 32
#define FSIZE 16

extern SDL_Surface *display, *image;

class PuyoAnimation {
  public:
    PuyoAnimation();
    bool isFinished();
    virtual void cycle() = 0;
    virtual void draw(int semiMove) = 0;
  protected:
    bool finishedFlag;
};

class NeutralAnimation : public PuyoAnimation {
  public:
    NeutralAnimation(int X, int Y, int xOffset, int yOffset);
    void cycle();
    void draw(int semiMove);
  private:
    int X, Y, currentY;
};

class PuyoView : public virtual PuyoDelegate {
  public:
    PuyoView(PuyoGame *attachedGame, int xOffset, int yOffset, int nXOffset, int nYOffset);
    void setEnemyGame(PuyoGame *enemyGame);
    void render();
    void renderNeutral();
    void cycleAnimation();
    void gameDidAddNeutral(PuyoPuyo *neutralPuyo);
    void gameDidEndCycle();
    void companionDidTurn(PuyoPuyo *companionPuyo, int companionVector, bool counterclockwise);
    void puyoDidFall(PuyoPuyo *puyo, int originX, int originY);
    void puyoWillVanish(PuyoPuyo *puyo);
    void gameLost();
    void scheduleAnimations(int tickCount);
    static SDL_Surface *getSurfaceForState(PuyoState state);
    SDL_Surface *getSurfaceForPuyo(PuyoPuyo *puyo);
  private:
    void render(PuyoPuyo *puyo);
    void cycleAnimation(PuyoPuyo *puyo);
    bool gameRunning;
    PuyoGame *attachedGame, *enemyGame;
    int xOffset, yOffset, nXOffset, nYOffset;
    PuyoAnimation *animationBoard[PUYODIMX][PUYODIMY];
    int puyoEyeState[PUYODIMX][PUYODIMY];
};

class PuyoStarter : public PuyoDrawable {
  public:
    PuyoStarter(PuyoCommander *commander, bool aiLeft, int aiLevel, IA_Type aiType, int theme);
    virtual ~PuyoStarter();
    void run(int score1, int score2, int lives);
    void draw();

    bool leftPlayerWin() const  { return attachedGameA->isGameRunning(); }
    bool rightPlayerWin() const { return attachedGameB->isGameRunning(); }
    
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

    void stopRender();
    void restartRender();
};

#endif // _PUYOVIEW
