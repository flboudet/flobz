#ifndef HALL_OF_FAME_H
#define HALL_OF_FAME_H

#include "PuyoSinglePlayerStarter.h"

class HallOfFameScreen : public PuyoGameOver1PScreen {
    public:
        HallOfFameScreen(Screen &previousScreen, Action *finishedAction)
            : PuyoGameOver1PScreen("gamewon_highscores_1p.gsl",previousScreen,finishedAction,"NONE",PlayerGameStat(-1))
        {}
};

/// Action to close a screen and restore main screen.
///
/// Note: this class was conceived for the HallOfFame but can be used for other screens
class PopToMainScreenAction : public Action
{
    public:
        PopToMainScreenAction(PuyoMainScreen *mainScreen, Screen *fromScreen = NULL)
            : mainScreen(mainScreen), fromScreen(fromScreen)
        {}
        void action() {
            mainScreen->transitionFromScreen(*fromScreen);
            GameUIDefaults::SCREEN_STACK->pop();
        }
        void setFromScreen(Screen *screen) {
            fromScreen = screen;
        }
    private:
        PuyoMainScreen *mainScreen;
        Screen *fromScreen;
};

/// Action to open the Hall of Fame
class PushHallOfFameAction : public Action
{
    public:
        PushHallOfFameAction(HallOfFameScreen *storyScreen, Screen *fromScreen)
            : storyScreen(storyScreen), fromScreen(fromScreen)
        {}
        void action() {
            storyScreen->transitionFromScreen(*fromScreen);
            GameUIDefaults::SCREEN_STACK->push(storyScreen);
            storyScreen->refresh();
        }
    private:
        HallOfFameScreen *storyScreen;
        Screen *fromScreen;
};

/// Action to open a story screen with a transition
class PushStoryScreenAction : public Action
{
    public:
        PushStoryScreenAction(PuyoStoryScreen *storyScreen, Screen *fromScreen)
            : storyScreen(storyScreen), fromScreen(fromScreen)
        {}
        void action() {
            storyScreen->transitionFromScreen(*fromScreen);
            GameUIDefaults::SCREEN_STACK->push(storyScreen);
        }
    private:
        PuyoStoryScreen *storyScreen;
        Screen *fromScreen;
};

#endif
