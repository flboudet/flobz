#ifndef _PLAYERNAME_UTILS_H_
#define _PLAYERNAME_UTILS_H_

#include <string>

class PlayerNameUtils
{
public:
    static std::string getPlayerName(int n);
    static std::string getDefaultPlayerName(int n);
    static std::string getDefaultPlayerKey(int n);
    static void setDefaultPlayerName(int n, const char * playerName);
};

#endif // _PLAYERNAME_UTILS_H_

