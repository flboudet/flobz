#include <iostream>
#include <sstream>
#include <list>
#include "HiScores.h"
#include "GTLog.h"

#include "FPCommander.h"

using namespace std;

LocalStorageHiScoreBoard::LocalStorageHiScoreBoard(const char *boardId, PreferencesManager *prefsMgr, HiScoreBoard &defaultScores)
    : m_prefsMgr(prefsMgr)
{
    std::list<HiScoreEntry> entries;
    for (int i=0; i < defaultScores.getMaxRank(); ++i)
    {
        const HiScoreEntry & defaultEntry = defaultScores.getEntry(i);
        HiScoreEntry entry;
        ostringstream hiScoreNameKey, hiScoreValKey;
        hiScoreNameKey << "score." << boardId << ".name." << i;
        hiScoreValKey << "score." << boardId << ".val." << i;
        entry.name = prefsMgr->getStrPreference(hiScoreNameKey.str().c_str(), defaultEntry.name.c_str());
        entry.score = prefsMgr->getIntPreference(hiScoreValKey.str().c_str(), defaultEntry.score);
        entries.push_back(entry);
    }
    entries.sort();
    entries.reverse();
    m_entries.resize(entries.size());
    entries.reverse();
    copy(entries.begin(), entries.end(), m_entries.begin());
}

const HiScoreEntry & LocalStorageHiScoreBoard::getEntry(int rank) const
{
    return m_entries[rank];
}

int LocalStorageHiScoreBoard::getMaxRank() const
{
    return m_entries.size();
}

int LocalStorageHiScoreBoard::setHiScore(std::string name, int score)
{
    HiScoreEntry entry(name, score);
    std::list<HiScoreEntry> entries;
    entries.resize(m_entries.size());
    copy(m_entries.begin(), m_entries.end(), entries.begin());
    entries.push_back(entry);
    std::list<HiScoreEntry>::iterator newScore = entries.end();
    --newScore;
    entries.sort();
    entries.reverse();
    entries.resize(m_entries.size());
    entries.reverse();
    int rank = -1;
    int i = 0;
    for (std::list<HiScoreEntry>::iterator iter = entries.begin();
         iter != entries.end() ; ++i, ++iter) {
        GTLogTrace("%s %d", iter->name.c_str(), iter->score);
        if (iter == newScore)
            rank = i;
        m_entries[i] = *iter;
    }
    GTLogTrace("Rank: %d", rank);
    return rank;
}

