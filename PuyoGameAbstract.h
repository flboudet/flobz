#ifndef PUYO_GAME_ABSTRACT_H
#define PUYO_GAME_ABSTRACT_H

#include "iosfc/ios_fc.h"
#include "drawcontext.h"

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
    PlayerGameStatDisplay(PlayerGameStat &stat);
    ~PlayerGameStatDisplay();
    void draw(DrawTarget *dt) const;
    // Accessors
    void setPosition(int x, int y) { m_x = x; m_y = y; }
private:
    PlayerGameStat &stat;
    int m_x, m_y;
    int player;
};


#endif
