#define kHiScoresNumber 10

#define kHiScoreNameLenght 20

typedef struct hiscore {
  char name[kHiScoreNameLenght+1];
  int  score;
} hiscore;

hiscore * getHiscores(const char * const defaultNames[kHiScoresNumber]);

int setHiScore(int score, const char * name);
