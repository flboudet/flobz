#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include "preferences.h"
#include "HiScores.h"

static hiscore HS[kHiScoresNumber];
static bool loaded = false;

hiscore * getHiscores(void)
{
  if (loaded==true) return HS;

  char HSIDN[6];
  char HSIDS[6];

  for (int i=0; i<kHiScoresNumber; i++)
  {
    sprintf(HSIDS,"HSS%2d",i);
    sprintf(HSIDN,"HSN%2d",i);
    GetStrPreference(HSIDN,HS[i].name,"Nobody",kHiScoreNameLenght);      
    HS[i].score = GetIntPreference(HSIDS,0);
  }
  loaded = true;
  
  return HS;
}




int setHiScore(int score, const char * name)
{
  int retour = -1;

  hiscore * tmp = (hiscore*)malloc(sizeof(hiscore));
  hiscore * tmp2= (hiscore*)malloc(sizeof(hiscore));

  if ((tmp==NULL) || (tmp2 == NULL))
  {
    fprintf(stderr,"Malloc failed, I won't save your score...");
    return retour;
  }

  strncpy(tmp->name,name,kHiScoreNameLenght-1);
  tmp->score = score;

  char HSIDN[6];
  char HSIDS[6];

  for (int i=0; i<kHiScoresNumber; i++)
  {
    if (tmp->score >= HS[i].score)
    {
      if (retour == -1) retour = i;

      sprintf(HSIDS,"HSS%2d",i);
      sprintf(HSIDN,"HSN%2d",i);
      SetStrPreference(HSIDN,tmp->name);
      SetIntPreference(HSIDS,tmp->score);

      memcpy(tmp2,&(HS[i]),sizeof(hiscore));
      memcpy(&(HS[i]),tmp,sizeof(hiscore));
      memcpy(tmp,tmp2,sizeof(hiscore));
    }
  }

  free(tmp);
  free(tmp2);

  return retour;
}
