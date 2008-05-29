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

static IIM_Surface *rope_elt = NULL;
static IIM_Surface *puyo_right = NULL;

ProgressBarWidget::ProgressBarWidget(Action *associatedAction)
  : m_value(0.), m_targetValue(0.), m_progressive(false), m_progressiveDuration(1.), m_visible(true),
    m_associatedAction(associatedAction)
{
    if (rope_elt == NULL) {
        rope_elt = IIM_Load_Absolute_DisplayFormatAlpha("data/base.000/gfx/progressbar/rope.png");
        puyo_right = IIM_Load_Absolute_DisplayFormatAlpha("data/base.000/gfx/progressbar/puyo_right.png");
    }
    setPreferedSize(Vec3(0, 32));
}

void ProgressBarWidget::draw(SDL_Surface *screen)
{
    if (!m_visible)
        return;
    Vec3 bsize = getSize();
    Vec3 bpos = getPosition();
    SDL_Rect dstrect;

    dstrect.x = bpos.x;
    dstrect.y = bpos.y+bsize.y-1;
    dstrect.w = bsize.x;
    dstrect.h = 1;
    SDL_FillRect(screen, &dstrect, 0x11666666);

    static const int IMG_PUYO_WIDTH = 52;
    static const int IMG_RING_WIDTH = 16;
    static const int IMG_ROPE_ELT_WIDTH = 8;
    static const int IMG_ROPE_ELT_HEIGHT = 8;

    int rope_targetSize = (bsize.x - IMG_RING_WIDTH - IMG_PUYO_WIDTH) * m_targetValue;
    int rope_size = (bsize.x - IMG_RING_WIDTH - IMG_PUYO_WIDTH) * m_value;
    int n_rope_elt = rope_targetSize / IMG_ROPE_ELT_WIDTH;
    if (n_rope_elt < 4) n_rope_elt = 4;
//if (rope_size < n_rope_elt*2) rope_size = n_rope_elt*2;

    for (int i=1; i<=n_rope_elt; ++i) {
        /*
        dstrect.x = bpos.x + i;
        dstrect.y = bpos.y + bsize.y / 3 + 150.0 * m_value * m_value * (1.54 - cosh(1.0 - 2.0 * f));
        */
        dstrect.w = IMG_ROPE_ELT_WIDTH;
        dstrect.h = IMG_ROPE_ELT_HEIGHT;

        double f = i / (double)n_rope_elt;
        double c = (1.54 - cosh(1.0 - 2.0 * f)) / (1.0+3.0*m_value/m_targetValue);
        double oscil = 10.0 * c * sin(rope_size+5.0*m_t + f*rope_size/32);
        dstrect.y = bsize.y / 3 + 75.0 * c;
        if (dstrect.y > bsize.y - dstrect.h)
            dstrect.y = bsize.y - dstrect.h;
        else {
            dstrect.y += oscil;
            if (dstrect.y > bsize.y - dstrect.h)
                dstrect.y = bsize.y - dstrect.h;
        }

        dstrect.x = rope_size - i*IMG_ROPE_ELT_WIDTH;
        if (dstrect.x < (n_rope_elt-i)*2) {
            dstrect.x = (n_rope_elt-i)*2;
        }
        if (dstrect.x > rope_size) {
            dstrect.x = rope_size;
        }

        dstrect.x += bpos.x + IMG_RING_WIDTH;
        dstrect.y += bpos.y;

        //SDL_FillRect(screen, &dstrect, 0x11661166);
        SDL_BlitSurface(rope_elt->surf, NULL, screen, &dstrect);
    }
    dstrect.w = puyo_right->w;
    dstrect.h = puyo_right->h;
    dstrect.x = bpos.x + rope_size + IMG_RING_WIDTH - 4;
    dstrect.y = bpos.y;
    SDL_BlitSurface(puyo_right->surf, NULL, screen, &dstrect);
}

void ProgressBarWidget::idle(double currentTime)
{
    m_t = currentTime;
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
    setInnerMargin(20);
    add(&m_statTitle);
    for (int i = 0 ; i < 12 ; i++) {
        add(&m_comboLines[i]);
    }
    m_comboLines[0].setComboLineInfos(0, String("Combo x") + (1) + ": ", 9, 20, this);
}

void PuyoStatsWidget::action(Widget *sender, int actionType, GameControlEvent *event)
{
    int comboLineIndex = actionType + 1;
    if (comboLineIndex >= 12)
        return;
    m_comboLines[comboLineIndex].setComboLineInfos(comboLineIndex, String("Combo x") + (comboLineIndex+1) + ": ", rand()%20, 20, this);
}

PuyoStatsWidget::ComboLine::ComboLine()
  : m_progressBar(this)
{
    add(&m_comboLabel);
    add(&m_progressBar);
    add(&m_currentValue);
    m_progressBar.setVisible(false);
}

void PuyoStatsWidget::ComboLine::setComboLineInfos(int tag, String comboText,
                                                   int numberOfCombos, int totalNumOfCombos, Action *progressionCompleteAction)
{
    m_tag = tag;
    m_progressionCompleteAction = progressionCompleteAction;
    m_totalNumOfCombos = totalNumOfCombos;
    float progressBarValue = (float)numberOfCombos / (float)m_totalNumOfCombos;
    m_comboLabel.setValue(comboText);
    m_progressBar.setVisible(true);
    m_progressBar.setValue(progressBarValue, true);
}

void PuyoStatsWidget::ComboLine::action(Widget *sender, int actionType, GameControlEvent *event)
{
    switch (actionType) {
        case ProgressBarWidget::VALUE_CHANGED:
            m_currentValue.setValue(String("") + (int)(m_progressBar.getValue() * (float)m_totalNumOfCombos) + " ");
            break;
        case ProgressBarWidget::PROGRESSION_COMPLETE:
            m_progressionCompleteAction->action(this, m_tag, event);
            break;
        default:
            break;
    }
}

