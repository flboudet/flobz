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
}

void PuyoStory::draw()
{
}
