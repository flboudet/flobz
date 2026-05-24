#ifndef HALL_OF_FAME_H
#define HALL_OF_FAME_H

#include "HiScores.h"
#include "gameui.h"
#include "Story.h"

using namespace gameui;

#define SOLO_SCOREBOARD_ID "Solo"
#define STORY_SCOREBOARD_ID "Story"

class GameOverScreen : public StoryScreen {
public:
    GameOverScreen(const char   *storyName,
                   Action       *finishedAction,
                   bool initialTransition=false);
    void setScoreBoard(HiScoreBoard *scoreBoard);
    void refresh();
    void setFinalScore(const char *playerName, int points);
    void highlightRank(int rank);
    virtual ~GameOverScreen();
private:
	HBox _titleBox;
	Text _titleText;
	Text _titleScore;
    Text *_names, *_points;
    VBox _hiScoreNameBox, _hiScorePointBox;
    HBox _hiScoreBox;
    HiScoreBoard *_scoreBoard;
};

class HiScoreDefaultBoard : public HiScoreBoard {
public:
    HiScoreDefaultBoard();
    virtual const HiScoreEntry &getEntry(int rank) const
    { return _entries[rank]; }
    virtual int getMaxRank() const
    { return _entries.size(); }
    virtual int setHiScore(std::string name, int score)
    { return -1; }
private:
    std::vector<HiScoreEntry> _entries;
};

class HallOfFameScreen : public GameOverScreen {
    public:
        HallOfFameScreen(Action *finishedAction)
            : GameOverScreen("gamewon_highscores_1p.gsl", finishedAction)
        {}
};

/// Action to close a screen and restore main screen.
///
/// Note: this class was conceived for the HallOfFame but can be used for other screens
class PopToMainScreenAction : public Action
{
    public:
        PopToMainScreenAction()
        {}
        void action() {
            GameUIDefaults::SCREEN_STACK->pop();
        }
};

/// Action to open the Hall of Fame
class PushHallOfFameAction : public Action
{
    public:
        PushHallOfFameAction(HallOfFameScreen *storyScreen)
            : _storyScreen(storyScreen)
        {}
        void action() {
            GameUIDefaults::SCREEN_STACK->push(_storyScreen);
            _storyScreen->refresh();
        }
    private:
        HallOfFameScreen *_storyScreen;
        Screen *_fromScreen;
};

/// Action to open a story screen with a transition
class PushStoryScreenAction : public Action
{
    public:
        PushStoryScreenAction(StoryScreen *storyScreen, Screen *fromScreen)
            : _storyScreen(storyScreen), _fromScreen(fromScreen)
        {}
        void action() {
            _storyScreen->getStoryWidget()->reset();
            GameUIDefaults::SCREEN_STACK->push(_storyScreen);
        }
    private:
        StoryScreen *_storyScreen;
        Screen *_fromScreen;
};

#endif
