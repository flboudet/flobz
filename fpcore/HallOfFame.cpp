#include <iostream>
#include <sstream>
#include "HallOfFame.h"

GameOverScreen::GameOverScreen(const char   *storyName,
                               Action       *finishedAction,
                               bool initialTransition)
        : StoryScreen(storyName, finishedAction, initialTransition),
          m_names(NULL), m_points(NULL), m_scoreBoard(NULL)
{

    m_titleText.setFont(GameUIDefaults::FONT_INACTIVE);
    m_titleScore.setFont(GameUIDefaults::FONT);
    m_titleBox.add(&m_titleText);
    m_titleBox.add(&m_titleScore);

    m_hiScoreBox.add(&m_hiScoreNameBox);
    m_hiScoreBox.add(&m_hiScorePointBox);

	add(&m_titleBox);
    add(&m_hiScoreBox);
    refresh();
}

void GameOverScreen::setScoreBoard(HiScoreBoard *scoreBoard)
{
    m_scoreBoard = scoreBoard;
    if (m_names != NULL)
        delete[] m_names;
    if (m_points != NULL)
        delete[] m_points;
    m_names  = new Text[scoreBoard->getMaxRank()];
    m_points = new Text[scoreBoard->getMaxRank()];
    for (int i = scoreBoard->getMaxRank() - 1 ;
         i >= 0 ; --i) {
        const HiScoreEntry &entry = scoreBoard->getEntry(i);
        ostringstream sPoints;
        sPoints << entry.score;
        m_names[i].setValue(entry.name.c_str());
        m_points[i].setValue(sPoints.str().c_str());
        m_hiScoreNameBox.add(&m_names[i]);
        m_hiScorePointBox.add(&m_points[i]);
    }
    refresh();
}

void GameOverScreen::refresh()
{
    Vec3 titlePos = m_titleBox.getPosition();
    titlePos.x = storyWidget.getIntegerValue("@hiScoreTopBox.x");
    titlePos.y = storyWidget.getIntegerValue("@hiScoreTopBox.y");
    m_titleBox.setPosition(titlePos);
    m_titleBox.setSize(Vec3(storyWidget.getIntegerValue("@hiScoreTopBox.w"),
                            storyWidget.getIntegerValue("@hiScoreTopBox.h"), 0));
    Vec3 hiScorePos = m_hiScoreBox.getPosition();
    hiScorePos.x = storyWidget.getIntegerValue("@hiScoreBox.x");
    hiScorePos.y = storyWidget.getIntegerValue("@hiScoreBox.y");
    m_hiScoreBox.setPosition(hiScorePos);

    m_hiScoreBox.setSize(Vec3(storyWidget.getIntegerValue("@hiScoreBox.w"),
                              storyWidget.getIntegerValue("@hiScoreBox.h"), 0));
}

void GameOverScreen::setFinalScore(const char *playerName, int points)
{
    ostringstream sPoints;
    sPoints << points;
    m_titleText.setValue(theCommander->getLocalizedString("Your Final Score:"));
    m_titleScore.setValue(sPoints.str().c_str());
}

void GameOverScreen::highlightRank(int rank)
{
    if (m_scoreBoard == NULL)
        return;
    if (rank >= 0) {
        m_names[rank].setFont(GameUIDefaults::FONT);
        m_points[rank].setFont(GameUIDefaults::FONT);
    }
}

GameOverScreen::~GameOverScreen()
{
    delete[] m_names;
    delete[] m_points;
}

HiScoreDefaultBoard::HiScoreDefaultBoard()
{
    m_entries.push_back(HiScoreEntry("A. Mc Flurry", 100000));
    m_entries.push_back(HiScoreEntry("Mr Gyom",       90000));
    m_entries.push_back(HiScoreEntry("Gizmo",         80000));
    m_entries.push_back(HiScoreEntry("Satanas",       70000));
    m_entries.push_back(HiScoreEntry("Tania",         60000));
    m_entries.push_back(HiScoreEntry("Jeko",          50000));
    m_entries.push_back(HiScoreEntry("The Dude",      40000));
    m_entries.push_back(HiScoreEntry("Big Rabbit",    30000));
    m_entries.push_back(HiScoreEntry("Herbert",       20000));
    m_entries.push_back(HiScoreEntry("Garou",         10000));
}

