#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "FPCommander.h"
#include "HiScores.h"

static hiscore HS[kHiScoresNumber];
static bool loaded = false;

void initHiScores(const char * const defaultNames[kHiScoresNumber])
{
    if (loaded) return;
    char HSID[8];

    for (int i=0; i<kHiScoresNumber; i++)
    {
        sprintf(HSID,"HSN%2d",i);
        HS[i].name = theCommander->getPreferencesManager()->getStrPreference(HSID,defaultNames[kHiScoresNumber-i-1]);
        sprintf(HSID,"HSS%2d",i);
        HS[i].score = theCommander->getPreferencesManager()->getIntPreference(HSID,(kHiScoresNumber-i) * 10000);
    }
    loaded = true;
}

hiscore * getHiScores(void)
{
    if (loaded!=true)
    {
        fprintf(stderr,"getHiscores() called before init, app may crash...");
        return NULL;
    }
    else return  HS;
}




int setHiScore(int score, const char * name)
{
  int retour = -1;

  if (loaded!=true)
  {
    fprintf(stderr,"setHiscores() called before init, app may crash...");
    return retour;
  }

  hiscore tmp, tmp2;

  tmp.name = name;
  tmp.score = score;

  char HSID[8];

  for (int i=0; i<kHiScoresNumber; i++)
  {
    if (tmp.score >= HS[i].score)
    {
      if (retour == -1) retour = i;

      sprintf(HSID,"HSN%2d",i);
      theCommander->getPreferencesManager()->setStrPreference(HSID,tmp.name.c_str());
      sprintf(HSID,"HSS%2d",i);
      theCommander->getPreferencesManager()->setIntPreference(HSID,tmp.score);

      tmp2 = HS[i];
      HS[i] = tmp;
      tmp = tmp2;
    }
  }
  return retour;
}
