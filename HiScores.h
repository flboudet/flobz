
#define kHiScoresNumber 5

#define kHiScoreNameLenght 10

typedef struct hiscore {
  char name[kHiScoreNameLenght];
  int  score;
} hiscore;

hiscore * getHiscores(void);

int setHiScore(int score, const char * name);
