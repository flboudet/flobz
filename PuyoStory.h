#ifndef PUYO_STORY_H
#define PUYO_STORY_H

#include "SDL.h"
#include "SDL_image.h"
#include "styrolyse.h"

#include "PuyoCommander.h"
extern int NB_STORIES;

class PuyoStory : public PuyoDrawable {
  public:
    PuyoStory(PuyoCommander *com, int num);
    virtual ~PuyoStory();
    void loop();
    void draw();
  private:
    int num;
    Styrolyse *currentStory;
    PuyoCommander *commander;
};

#endif
