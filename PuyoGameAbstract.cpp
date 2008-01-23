#include "PuyoGameAbstract.h"
#include "PuyoCommander.h"

PlayerGameStat::PlayerGameStat(int p)
{
    for (int i=0; i<24; ++i)
        combo_count[i] = 0;
    ghost_sent_count = 0;
    time_left = 0;
    is_dead = false;
    is_winner = false;
    points = p;
}

PlayerGameStatDisplay::PlayerGameStatDisplay(const ios_fc::String &player_name, PlayerGameStat &stat, int player)
    : player_name(player_name), stat(stat), player(player)
{
}

void PlayerGameStatDisplay::draw() const
{
    char txt[1024];
    sprintf(txt, "%d", stat.points);
    // if (player == 0) {
    SDL_Surface *surface = gameui::GameUIDefaults::GAME_LOOP->getSurface();
    SoFont_CenteredString_XY(theCommander->menuFont, surface, 320, 360 + player * 25, txt, NULL);
    // }
}
