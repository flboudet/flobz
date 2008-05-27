/*
 *  PuyoStatsWidget.cpp
 *  flobopuyo
 *
 *  Created by Florent Boudet on 26/05/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "PuyoStatsWidget.h"
//#include "PuyoCommander.h"

using namespace gameui;

ProgressBarWidget::ProgressBarWidget()
  : m_value(0.), m_targetValue(0.), m_progressive(false), m_progressiveDuration(3.)
{
    setPreferedSize(Vec3(0, 10));
}

void ProgressBarWidget::draw(SDL_Surface *screen)
{
    Vec3 bsize = getSize();
    Vec3 bpos = getPosition();
    SDL_Rect dstrect;
    dstrect.x = bpos.x;
    dstrect.y = bpos.y;
    dstrect.w = bsize.x;
    dstrect.h = bsize.y;
    SDL_FillRect(screen, &dstrect, 0x11666666);
    dstrect.w = bsize.x * m_value;
    SDL_FillRect(screen, &dstrect, 0x11661166);
}

void ProgressBarWidget::idle(double currentTime)
{
    if (m_progressive) {
        double deltaT = currentTime - m_targetBaseTime;
        if (deltaT < m_progressiveDuration) {
            m_value = m_fromValue + (m_targetValue - m_fromValue) * (deltaT / m_progressiveDuration);
        }
        else {
            m_value = m_targetValue;
            m_progressive = false;
        }
        requestDraw(false);
    }
}

void ProgressBarWidget::setValue(float value, bool progressive)
{
    if (m_value == value)
        return;
    m_progressive = progressive;
    if (!progressive) {
        m_value = value;
        requestDraw(false);
    }
    else {
        m_fromValue = m_value;
        m_targetValue = value;
        m_targetBaseTime = GameLoop::getCurrentTime();
    }
}

PuyoStatsWidget::PuyoStatsWidget(PlayerGameStat &stats, const gameui::FramePicture *framePicture)
  : Frame(/*theCommander->getWindowFramePicture()*/framePicture), m_stats(stats),
    m_statTitle("Combos"), m_comboLabel("1x")
{
    //add(&m_statTitle);
    //m_comboLine.add(&m_comboLabel);
    //m_comboLine.add(&m_progressBar);
    //add(&m_comboLine);
}

