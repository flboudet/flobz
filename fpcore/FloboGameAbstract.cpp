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
    : stat(stat), m_x(0), m_y(0), m_font(GameUIDefaults::FONT_FUNNY), m_color(GT_WHITE)
{
}

PlayerGameStatDisplay::~PlayerGameStatDisplay()
{
}

void PlayerGameStatDisplay::draw(DrawTarget *dt) const
{
    char txt[4096];
    sprintf(txt, "%d", stat.points);
    dt->setClipRect(NULL);
    dt->putStringCenteredXY(m_font, m_x, m_y, txt, m_color);
}
