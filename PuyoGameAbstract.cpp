#include "PuyoGameAbstract.h"
#include "PuyoCommander.h"
#include "gameui.h"

PlayerGameStat::PlayerGameStat(int p)
{
    for (int i=0; i<24; ++i)
        combo_count[i] = 0;
    ghost_sent_count = 0;
    time_left = 0;
    is_dead = false;
    is_winner = false;
    points = p;
    total_points = 0;
}

PlayerGameStatDisplay::PlayerGameStatDisplay(const ios_fc::String &player_name, PlayerGameStat &stat, int player)
    : player_name(player_name), stat(stat), player(player)
{
    display_stats = 0;
}

PlayerGameStatDisplay::~PlayerGameStatDisplay()
{
}

void PlayerGameStatDisplay::draw() const
{
    char txt[4096];
    sprintf(txt, "%d", stat.points);
    // if (player == 0) {
    SDL_Surface *surface = gameui::GameUIDefaults::GAME_LOOP->getSurface();
    SoFont_CenteredString_XY(GameUIDefaults::FONT_FUNNY, surface, 300 + player * 40, 360 - player * 40, txt, NULL);
    // }
}

void PlayerGameStatDisplay::gameIsOver()
{
    if (display_stats == 0)
        display_stats_start = ios_fc::getTimeMs();
    display_stats = 1;
}
