#ifndef PUYO_GAME_ABSTRACT_H
#define PUYO_GAME_ABSTRACT_H

#include "iosfc/ios_fc.h"

struct PlayerGameStat
{
    int combo_count[24];
    int ghost_sent_count;
    double time_left;
    bool is_dead;
    bool is_winner;
    int point;

    PlayerGameStat();
};


class PlayerGameStatDisplay
{
    public:
        PlayerGameStatDisplay(const ios_fc::String &player_name, const PlayerGameStat &stat);

    private:
        ios_fc::String player_name;
        PlayerGameStat stat;
};



#endif
