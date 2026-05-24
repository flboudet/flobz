#ifndef FLOBO_GAME_ABSTRACT_H
#define FLOBO_GAME_ABSTRACT_H

#include "ios_fc.h"
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
    void setPosition(int x, int y) { _x = x; _y = y; }
    void setFont(IosFont *font) { _font = font; }
private:
    PlayerGameStat &_stat;
    int _x, _y;
    IosFont *_font;
    RGBA _color;
    int _player;
};


#endif
