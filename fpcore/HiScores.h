#ifndef _HISCORE
#define _HISCORE

#include <string>
#include <vector>
#include "PreferencesManager.h"

struct HiScoreEntry {
    std::string _name;
    int  _score;

    HiScoreEntry(std::string name, int score)
        : _name(name), _score(score) {}
    HiScoreEntry() : _score(0) {}
    bool operator < (const HiScoreEntry &c) const
    { return _score < c._score; }
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
    PreferencesManager *_prefsMgr;
    std::string _boardId;
    std::vector<HiScoreEntry> _entries;
};

#endif // _HISCORE
