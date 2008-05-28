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

ProgressBarWidget::ProgressBarWidget(Action *associatedAction)
  : m_value(0.), m_targetValue(0.), m_progressive(false), m_progressiveDuration(3.), m_visible(true),
    m_associatedAction(associatedAction)
{
    setPreferedSize(Vec3(0, 10));
}

void ProgressBarWidget::draw(SDL_Surface *screen)
{
    if (!m_visible)
        return;
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
            m_associatedAction->action(this, VALUE_CHANGED, NULL);
        }
        else {
            m_value = m_targetValue;
            m_progressive = false;
            m_associatedAction->action(this, VALUE_CHANGED, NULL);
            m_associatedAction->action(this, PROGRESSION_COMPLETE, NULL);
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
        m_associatedAction->action(this, VALUE_CHANGED, NULL);
    }
    else {
        m_fromValue = m_value;
        m_targetValue = value;
        m_targetBaseTime = GameLoop::getCurrentTime();
    }
}

void ProgressBarWidget::setVisible(bool visible)
{
    m_visible = visible;
    requestDraw();
}

PuyoStatsWidget::PuyoStatsWidget(PlayerGameStat &stats, const gameui::FramePicture *framePicture)
  : Frame(/*theCommander->getWindowFramePicture()*/framePicture), m_stats(stats),
    m_statTitle("Combos")
{
    add(&m_statTitle);
    for (int i = 0 ; i < 24 ; i++) {
        add(&m_comboLines[i]);
        m_comboLines[i].setComboLineInfos(String("Combo x") + (i+1), 9, 10);
    }
}

void PuyoStatsWidget::action(Widget *sender, int actionType, GameControlEvent *event)
{
}

PuyoStatsWidget::ComboLine::ComboLine()
  : m_progressBar(this)
{
    add(&m_comboLabel);
    add(&m_progressBar);
    m_progressBar.setVisible(false);
}

void PuyoStatsWidget::ComboLine::setComboLineInfos(String comboText, int numberOfCombos, int opponentNumberOfCombos)
{
    float progressBarValue = (float)numberOfCombos / (float)opponentNumberOfCombos;
    if (progressBarValue > 1.)
        progressBarValue = 1;
    m_comboLabel.setValue(comboText);
    m_progressBar.setVisible(true);
    m_progressBar.setValue(progressBarValue, true);
}

void PuyoStatsWidget::ComboLine::action(Widget *sender, int actionType, GameControlEvent *event)
{
    switch (actionType) {
        case ProgressBarWidget::VALUE_CHANGED:
            break;
        case ProgressBarWidget::PROGRESSION_COMPLETE:
            break;
        default:
            break;
    }
}

