#include <iostream>
#include <sstream>
#include "HallOfFame.h"

GameOverScreen::GameOverScreen(const char   *storyName,
                               Action       *finishedAction,
                               bool initialTransition)
        : StoryScreen(storyName, finishedAction, initialTransition),
          _names(NULL), _points(NULL), _scoreBoard(NULL)
{

    _titleText.setFont(GameUIDefaults::FONT_INACTIVE);
    _titleScore.setFont(GameUIDefaults::FONT);
    _titleBox.add(&_titleText);
    _titleBox.add(&_titleScore);

    _hiScoreBox.add(&_hiScoreNameBox);
    _hiScoreBox.add(&_hiScorePointBox);

	add(&_titleBox);
    add(&_hiScoreBox);
    refresh();
}

void GameOverScreen::setScoreBoard(HiScoreBoard *scoreBoard)
{
    _scoreBoard = scoreBoard;
    if (_names != NULL)
        delete[] _names;
    if (_points != NULL)
        delete[] _points;
    _names  = new Text[scoreBoard->getMaxRank()];
    _points = new Text[scoreBoard->getMaxRank()];
    for (int i = scoreBoard->getMaxRank() - 1 ;
         i >= 0 ; --i) {
        const HiScoreEntry &entry = scoreBoard->getEntry(i);
        ostringstream sPoints;
        sPoints << entry._score;
        _names[i].setValue(entry._name.c_str());
        _points[i].setValue(sPoints.str().c_str());
        _hiScoreNameBox.add(&_names[i]);
        _hiScorePointBox.add(&_points[i]);
    }
    refresh();
}

void GameOverScreen::refresh()
{
    Vec3 titlePos = _titleBox.getPosition();
    titlePos.x = _storyWidget.getIntegerValue("@hiScoreTopBox.x");
    titlePos.y = _storyWidget.getIntegerValue("@hiScoreTopBox.y");
    _titleBox.setPosition(titlePos);
    _titleBox.setSize(Vec3(_storyWidget.getIntegerValue("@hiScoreTopBox.w"),
                            _storyWidget.getIntegerValue("@hiScoreTopBox.h"), 0));
    Vec3 hiScorePos = _hiScoreBox.getPosition();
    hiScorePos.x = _storyWidget.getIntegerValue("@hiScoreBox.x");
    hiScorePos.y = _storyWidget.getIntegerValue("@hiScoreBox.y");
    _hiScoreBox.setPosition(hiScorePos);

    _hiScoreBox.setSize(Vec3(_storyWidget.getIntegerValue("@hiScoreBox.w"),
                              _storyWidget.getIntegerValue("@hiScoreBox.h"), 0));
}

void GameOverScreen::setFinalScore(const char *playerName, int points)
{
    ostringstream sPoints;
    sPoints << points;
    _titleText.setValue(theCommander->getLocalizedString("Your Final Score:"));
    _titleScore.setValue(sPoints.str().c_str());
}

void GameOverScreen::highlightRank(int rank)
{
    if (_scoreBoard == NULL)
        return;
    if (rank >= 0) {
        _names[rank].setFont(GameUIDefaults::FONT);
        _points[rank].setFont(GameUIDefaults::FONT);
    }
}

GameOverScreen::~GameOverScreen()
{
    delete[] _names;
    delete[] _points;
}

HiScoreDefaultBoard::HiScoreDefaultBoard()
{
    _entries.push_back(HiScoreEntry("A. Mc Flurry", 100000));
    _entries.push_back(HiScoreEntry("Mr Gyom",       90000));
    _entries.push_back(HiScoreEntry("Gizmo",         80000));
    _entries.push_back(HiScoreEntry("Satanas",       70000));
    _entries.push_back(HiScoreEntry("Tania",         60000));
    _entries.push_back(HiScoreEntry("Jeko",          50000));
    _entries.push_back(HiScoreEntry("The Dude",      40000));
    _entries.push_back(HiScoreEntry("Big Rabbit",    30000));
    _entries.push_back(HiScoreEntry("Herbert",       20000));
    _entries.push_back(HiScoreEntry("Garou",         10000));
}

