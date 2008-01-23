#ifndef PUYO_GAME_ABSTRACT_H
#define PUYO_GAME_ABSTRACT_H

#include "iosfc/ios_fc.h"

struct PlayerGameStat
{
    int combo_count[24]; //
    int explode_count; //
    int drop_count; //
    int ghost_sent_count; //
    double time_left;
    bool is_dead;
    bool is_winner;
    int points; //

    PlayerGameStat(int p = 0);
};


class PlayerGameStatDisplay
{
    public:
        PlayerGameStatDisplay(const ios_fc::String &player_name, PlayerGameStat &stat, int player);
        void draw() const;

    private:
        ios_fc::String player_name;
        PlayerGameStat &stat;
        int player;
};


#endif