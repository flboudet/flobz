#ifndef _HISCORE
#define _HISCORE

#include <string>

#define kHiScoresNumber 10

typedef struct hiscore {
  std::string name;
  int  score;
} hiscore;

void initHiScores(const char * const defaultNames[kHiScoresNumber]);

hiscore * getHiScores(void);

int setHiScore(int score, const char * name);

#endif // _HISCORE
