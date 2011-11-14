#include "PlayerNameUtils.h"
#include "PuyoCommander.h"
#include <string.h>

std::string PlayerNameUtils::getPlayerName(int n)
{
  std::string playerName = theCommander->getPreferencesManager()->getStrPreference(getDefaultPlayerKey(n).c_str(), getDefaultPlayerName(n).c_str());
  return playerName;
}

std::string PlayerNameUtils::getDefaultPlayerName(int n)
{
  char defaultPlayerName[256];
  if (n <= 0)
  {
    char * defaultName = getenv("USER");
    if ((defaultName == NULL) || (defaultName[0]<32))
    {
      defaultName = getenv("USERNAME");
      if ((defaultName == NULL) || (defaultName[0]<32))
      {
        sprintf(defaultName,"Kaori");
      }
    }

    strncpy(defaultPlayerName,defaultName,255);
    defaultPlayerName[255]=0;

    if ((defaultPlayerName[0]>='a') && (defaultPlayerName[0]<='z'))
    {
      defaultPlayerName[0] += 'A' - 'a';
    }
  }
  else
  {
    sprintf(defaultPlayerName,"Player %d",n);
  }
  return defaultPlayerName;
}

std::string PlayerNameUtils::getDefaultPlayerKey(int n)
{
  char playerKey[50];
  sprintf(playerKey,"Game.Player.Name.%d",n);
  return playerKey;
}

void PlayerNameUtils::setDefaultPlayerName(int n, const char * playerName)
{
  static char playerKey[50];
  sprintf(playerKey,"Game.Player.Name.%d",n);
  theCommander->getPreferencesManager()->setStrPreference(playerKey, playerName);
}



