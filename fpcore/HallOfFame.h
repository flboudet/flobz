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
	HBox m_titleBox;
	Text m_titleText;
	Text m_titleScore;
    Text *m_names, *m_points;
    VBox m_hiScoreNameBox, m_hiScorePointBox;
    HBox m_hiScoreBox;
    HiScoreBoard *m_scoreBoard;
};

class HiScoreDefaultBoard : public HiScoreBoard {
public:
    HiScoreDefaultBoard();
    virtual const HiScoreEntry &getEntry(int rank) const
    { return m_entries[rank]; }
    virtual int getMaxRank() const
    { return m_entries.size(); }
    virtual int setHiScore(std::string name, int score)
    { return -1; }
private:
    std::vector<HiScoreEntry> m_entries;
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
            : storyScreen(storyScreen)
        {}
        void action() {
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
        PushStoryScreenAction(StoryScreen *storyScreen, Screen *fromScreen)
            : storyScreen(storyScreen), fromScreen(fromScreen)
        {}
        void action() {
            storyScreen->getStoryWidget()->reset();
            GameUIDefaults::SCREEN_STACK->push(storyScreen);
        }
    private:
        StoryScreen *storyScreen;
        Screen *fromScreen;
};

#endif
