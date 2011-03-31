#ifndef _HISCORE
#define _HISCORE

#define kHiScoresNumber 10

#define kHiScoreNameLenght 20

typedef struct hiscore {
  char name[kHiScoreNameLenght+1];
  int  score;
} hiscore;

void initHiScores(const char * const defaultNames[kHiScoresNumber]);

hiscore * getHiScores(void);

int setHiScore(int score, const char * name);

#endif // _HISCORE
