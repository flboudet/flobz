#ifndef PUYO_STORY_H
#define PUYO_STORY_H

#include "SDL.h"
#include "SDL_image.h"
#include "styrolyse.h"

#include "PuyoCommander.h"
extern int NB_STORIES;

class PuyoStoryWidget : public Widget, CycledComponent {
public:
    PuyoStoryWidget(int num);
    ~PuyoStoryWidget();
    void cycle();
    void draw(SDL_Surface *screen) const;
    IdleComponent *getIdleComponent() const { return this; }
private:
    int num;
    Styrolyse *currentStory;
};

class PuyoStory : public DrawableComponent {
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
