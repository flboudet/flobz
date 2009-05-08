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
    DrawTarget *dt = GameUIDefaults::GAME_LOOP->getDrawContext();
    dt->setClipRect(NULL);
    dt->putStringCenteredXY(GameUIDefaults::FONT_FUNNY, 300 + player * 40, 360 - player * 40, txt);
}

void PlayerGameStatDisplay::gameIsOver()
{
    if (display_stats == 0)
        display_stats_start = ios_fc::getTimeMs();
    display_stats = 1;
}
