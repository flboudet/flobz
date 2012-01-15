#ifndef _HISCORE
#define _HISCORE

#include <string>
#include <vector>
#include "PreferencesManager.h"

struct HiScoreEntry {
    std::string name;
    int  score;

    HiScoreEntry(std::string name, int score)
        : name(name), score(score) {}
    HiScoreEntry() : score(0) {}
    bool operator < (const HiScoreEntry &c)
    { return score < c.score; }
};

class HiScoreBoard {
public:
    virtual ~HiScoreBoard() {}
    virtual const HiScoreEntry &getEntry(int rank) const = 0;
    virtual int getMaxRank() const = 0;
    virtual int setHiScore(std::string name, int score) = 0;
};

class LocalStorageHiScoreBoard : public HiScoreBoard
{
public:
    LocalStorageHiScoreBoard(const char *boardId, PreferencesManager *prefsMgr, HiScoreBoard &defaultScores);
    virtual const HiScoreEntry &getEntry(int rank) const;
    virtual int getMaxRank() const;
    virtual int setHiScore(std::string name, int score);
private:
    PreferencesManager *m_prefsMgr;
    std::string m_boardId;
    std::vector<HiScoreEntry> m_entries;
};

#endif // _HISCORE
