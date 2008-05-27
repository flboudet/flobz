/*
 *  PuyoStatsWidget.h
 *  flobopuyo
 *
 *  Created by Florent Boudet on 26/05/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _PUYO_STATS_WIDGET_H
#define _PUYO_STATS_WIDGET_H

#include "gameui.h"
#include "Frame.h"
#include "PuyoGameAbstract.h"

class ProgressBarWidget : public gameui::Widget, IdleComponent {
public:
    ProgressBarWidget();
    virtual ~ProgressBarWidget() {}
    void draw(SDL_Surface *screen);
    void idle(double currentTime);
    IdleComponent *getIdleComponent() { return this; }
    void setValue(float value, bool progressive = false);
private:
    float m_value, m_fromValue, m_targetValue;
    bool m_progressive;
    double m_targetBaseTime;
    double m_progressiveDuration;
};

class PuyoStatsWidget : public gameui::Frame {
public:
    PuyoStatsWidget(PlayerGameStat &stats, const gameui::FramePicture *framePicture);
    virtual ~PuyoStatsWidget() {}
private:
    PlayerGameStat &m_stats;
    gameui::Text m_statTitle;
    gameui::HBox m_comboLine;
    gameui::Text m_comboLabel;
    ProgressBarWidget m_progressBar;
};

#endif // _PUYO_STATS_WIDGET_H

