#include "FloboGameAbstract.h"
#include "FPCommander.h"
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

PlayerGameStatDisplay::PlayerGameStatDisplay(PlayerGameStat &stat)
    : _stat(stat), _x(0), _y(0), _font(GameUIDefaults::FONT_FUNNY), _color(GT_WHITE)
{
}

PlayerGameStatDisplay::~PlayerGameStatDisplay()
{
}

void PlayerGameStatDisplay::draw(DrawTarget *dt) const
{
    char txt[4096];
    sprintf(txt, "%d", _stat.points);
    dt->setClipRect(NULL);
    dt->putStringCenteredXY(_font, _x, _y, txt, _color);
}
