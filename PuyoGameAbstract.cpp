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

    if (display_stats) {
        double t = ios_fc::getTimeMs() - display_stats_start;
        int combo[24];
        double date[24];
        int txt_len = 13;
        sprintf(txt, "-- Combos --\n");

        for (int i=0; i<10; ++i)
            combo[i] = stat.combo_count[i] - stat.combo_count[i+1];

        date[0] = 0.0;
        for (int i=1; i<10; ++i) {
            date[i] = date[i-1]
                + 100.0 * (double)combo[i-1]
                + 250.0;
        }

        for (int i=0; i<9; ++i) {
            if (t >= date[i]) {
                int n = combo[i];
                if (t < date[i+1]) {
                    n = (int)((t - date[i]) / 150.0);
                    if (n > combo[i]) n = combo[i];
                }
                if (combo[i] > 0) {
                    sprintf(&txt[txt_len],
                            "%1dx: %d ", i+1, n);
                    txt_len += 6;
                    if (n >= 10) txt_len += 1;
                    for (int j=0; j<n; ++j) {
                        txt[txt_len++] = '%';
                    }
                }
                txt[txt_len++] = '\n';
                // txt[txt_len++] = '\n';
                txt[txt_len] = '\0';
            }
        }

        SoFont_PutString(GameUIDefaults::FONT_FUNNY, surface, 20 + player * 410, 40, txt, NULL);
    }
}

void PlayerGameStatDisplay::gameIsOver()
{
    if (display_stats == 0)
        display_stats_start = ios_fc::getTimeMs();
    display_stats = 1;
}
