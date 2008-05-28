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
    ProgressBarWidget(gameui::Action *associatedAction);
    virtual ~ProgressBarWidget() {}
    void draw(SDL_Surface *screen);
    void idle(double currentTime);
    IdleComponent *getIdleComponent() { return this; }
    void setValue(float value, bool progressive = false);
    void setVisible(bool visible);
    enum {
        VALUE_CHANGED,
        PROGRESSION_COMPLETE
    };
private:
    float m_value, m_fromValue, m_targetValue;
    bool m_progressive;
    double m_targetBaseTime;
    double m_progressiveDuration;
    bool m_visible;
    gameui::Action *m_associatedAction;
};

class PuyoStatsWidget : public gameui::Frame, gameui::Action {
public:
    PuyoStatsWidget(PlayerGameStat &stats, const gameui::FramePicture *framePicture);
    virtual ~PuyoStatsWidget() {}
    virtual void action(Widget *sender, int actionType, GameControlEvent *event);
private:
    class ComboLine : public gameui::HBox, gameui::Action {
    public:
        ComboLine();
        virtual ~ComboLine() {}
        void setComboLineInfos(String comboText, int numberOfCombos, int opponentNumberOfCombos);
        virtual void action(Widget *sender, int actionType, GameControlEvent *event);
    private:
        gameui::Text m_comboLabel;
        ProgressBarWidget m_progressBar;
    };
    PlayerGameStat &m_stats;
    gameui::Text m_statTitle;
    gameui::Text m_currentValue;
    ComboLine m_comboLines[24];
};



#endif // _PUYO_STATS_WIDGET_H

