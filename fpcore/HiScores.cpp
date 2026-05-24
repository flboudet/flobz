#include <iostream>
#include <sstream>
#include <list>
#include "HiScores.h"
#include "HallOfFame.h"
#include "GTLog.h"

#include "FPCommander.h"

using namespace std;

LocalStorageHiScoreBoard::LocalStorageHiScoreBoard(const char *boardId, PreferencesManager *prefsMgr, HiScoreBoard &defaultScores)
    : _prefsMgr(prefsMgr), _boardId(boardId)
{
    std::list<HiScoreEntry> entries;
    for (int i=0; i < defaultScores.getMaxRank(); ++i)
    {
        const HiScoreEntry & defaultEntry = defaultScores.getEntry(i);
        HiScoreEntry entry;
        ostringstream hiScoreNameKey, hiScoreValKey;
        hiScoreNameKey << "score." << boardId << ".name." << i;
        hiScoreValKey << "score." << boardId << ".val." << i;
        entry._name = prefsMgr->getStrPreference(hiScoreNameKey.str().c_str(), defaultEntry._name.c_str());
        entry._score = prefsMgr->getIntPreference(hiScoreValKey.str().c_str(), defaultEntry._score);
        entries.push_back(entry);
    }
    entries.sort();
    entries.reverse();
    _entries.resize(entries.size());
    entries.reverse();
    copy(entries.begin(), entries.end(), _entries.begin());
}

const HiScoreEntry & LocalStorageHiScoreBoard::getEntry(int rank) const
{
    return _entries[rank];
}

int LocalStorageHiScoreBoard::getMaxRank() const
{
    return _entries.size();
}

int LocalStorageHiScoreBoard::setHiScore(std::string name, int score)
{
    HiScoreEntry entry(name, score);
    std::list<HiScoreEntry> entries;
    entries.resize(_entries.size());
    copy(_entries.begin(), _entries.end(), entries.begin());
    entries.push_back(entry);
    std::list<HiScoreEntry>::iterator newScore = entries.end();
    --newScore;
    entries.sort();
    entries.reverse();
    entries.resize(_entries.size());
    entries.reverse();
    // Find rank and save entries
    int rank = -1;
    int i = 0;
    for (std::list<HiScoreEntry>::iterator iter = entries.begin();
         iter != entries.end() ; ++i, ++iter) {
        GTLogTrace("%s %d", iter->_name.c_str(), iter->_score);
        if (iter == newScore)
            rank = i;
        _entries[i] = *iter;
        ostringstream hiScoreNameKey, hiScoreValKey;
        hiScoreNameKey << "score." << _boardId << ".name." << i;
        hiScoreValKey << "score." << _boardId << ".val." << i;
        _prefsMgr->setStrPreference(hiScoreNameKey.str().c_str(), iter->_name.c_str());
        _prefsMgr->setIntPreference(hiScoreValKey.str().c_str(), iter->_score);
    }
    GTLogTrace("Rank: %d", rank);
    return rank;
}

