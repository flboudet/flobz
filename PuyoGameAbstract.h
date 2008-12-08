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
    int total_points;
    PlayerGameStat(int p = 0);
};


class PlayerGameStatDisplay
{
    public:
        PlayerGameStatDisplay(const ios_fc::String &player_name, PlayerGameStat &stat, int player);
        ~PlayerGameStatDisplay();
        void draw() const;
        void gameIsOver();

    private:
        ios_fc::String player_name;
        PlayerGameStat &stat;
        int player;
        double display_stats_start;
        int display_stats;
};


#endif
