#include "PuyoStory.h"
#include "PuyoCommander.h"

#ifndef DATADIR
extern char *DATADIR;
#endif

PuyoStory *theStory;

int NB_STORIES = 6;

extern void launch_scenar(const char *f);
extern void draw_scenar();

PuyoStory::PuyoStory(PuyoCommander *com, int num) : num(num), commander(com)
{}

PuyoStory::~PuyoStory()
{}

void PuyoStory::loop()
{
    theStory = this;
    if (num==0) {
        char temp[1024];
        sprintf(temp, "%s/story/intro.txt", DATADIR);
        launch_scenar(temp);
    }
    else {
      char stories[1024];
      sprintf(stories, "%s/story%d.txt", DATADIR, num);
      launch_scenar(stories);
    }
}

void PuyoStory::draw()
{
    draw_scenar();
}
