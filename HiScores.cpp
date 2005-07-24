#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "preferences.h"
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
        GetStrPreference(HSID,HS[i].name,defaultNames[kHiScoresNumber-i-1],kHiScoreNameLenght+1); 
        sprintf(HSID,"HSS%2d",i);
        HS[i].score = GetIntPreference(HSID,(kHiScoresNumber-i) * 10000);
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

  hiscore * tmp = (hiscore*)malloc(sizeof(hiscore));
  hiscore * tmp2= (hiscore*)malloc(sizeof(hiscore));

  if ((tmp==NULL) || (tmp2 == NULL))
  {
    fprintf(stderr,"Malloc failed, I won't save your score...");
    return retour;
  }

  strncpy(tmp->name,name,kHiScoreNameLenght);
  tmp->name[kHiScoreNameLenght]=0;
  tmp->score = score;

  char HSID[8];

  for (int i=0; i<kHiScoresNumber; i++)
  {
    if (tmp->score >= HS[i].score)
    {
      if (retour == -1) retour = i;

      sprintf(HSID,"HSN%2d",i);
      SetStrPreference(HSID,tmp->name);
      sprintf(HSID,"HSS%2d",i);
      SetIntPreference(HSID,tmp->score);
      
      memcpy(tmp2,&(HS[i]),sizeof(hiscore));
      memcpy(&(HS[i]),tmp,sizeof(hiscore));
      memcpy(tmp,tmp2,sizeof(hiscore));
    }
  }

  free(tmp);
  free(tmp2);

  return retour;
}
