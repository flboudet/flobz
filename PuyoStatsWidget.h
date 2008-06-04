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

enum PuyoStatsDirection {
    RIGHT_TO_LEFT,
    LEFT_TO_RIGHT
};

class ProgressBarWidget : public gameui::Widget, IdleComponent {
public:
    ProgressBarWidget(gameui::Action *associatedAction);
    virtual ~ProgressBarWidget() {}
    void draw(SDL_Surface *screen);
    void idle(double currentTime);
    IdleComponent *getIdleComponent() { return this; }
    void setValue(float value, bool progressive = false);
    float getValue() const { return m_value; }
    void setVisible(bool visible);
    void setDirection(PuyoStatsDirection dir);
    enum {
        VALUE_CHANGED,
        PROGRESSION_COMPLETE
    };
private:
    PuyoStatsDirection m_dir;
    float m_value, m_fromValue, m_targetValue;
    bool m_progressive;
    double m_targetBaseTime;
    double m_progressiveDuration;
    bool m_visible;
    gameui::Action *m_associatedAction;
    double m_t;
};

#define MAX_DISPLAYED_COMBOS 7
struct PuyoStatsFormat {
    PuyoStatsFormat(PlayerGameStat &playerAStats, PlayerGameStat &playerBStats);
    int m_comboIndirection[MAX_DISPLAYED_COMBOS+1];
};

class PuyoStatsWidget : public gameui::VBox, gameui::Action {
public:
    PuyoStatsWidget(PuyoStatsFormat &statsFormat, 
                    PlayerGameStat &stats, PlayerGameStat &opponentStats,
                    const gameui::FramePicture *framePicture, PuyoStatsDirection dir);
    virtual ~PuyoStatsWidget() {}
    virtual void action(Widget *sender, int actionType, GameControlEvent *event);
    void startAnimation();
private:
    PuyoStatsDirection m_dir;
    class ComboLine : public gameui::HBox, gameui::Action {
    public:
        ComboLine();
        virtual ~ComboLine() {}
        void setComboLineInfos(PuyoStatsDirection dir, int tag, String comboText, int numberOfCombos, int totalNumOfCombos, gameui::Action *progressionCompleteAction);
        virtual void action(Widget *sender, int actionType, GameControlEvent *event);
    private:
        PuyoStatsDirection m_dir;
        int m_tag;
        gameui::Text m_comboLabel;
        Action *m_progressionCompleteAction;
        ProgressBarWidget m_progressBar;
        gameui::Text m_currentValue;
        int m_totalNumOfCombos;
    };
    PuyoStatsFormat &m_statsFormat;
    PlayerGameStat &m_stats, &m_opponentStats;
    gameui::Text m_statTitle;
    ComboLine m_comboLines[MAX_DISPLAYED_COMBOS];
    int m_maxCombo;
};

class PuyoStatsLegendWidget : public gameui::Frame {
public:
  PuyoStatsLegendWidget(PuyoStatsFormat &statsFormat, PuyoStatsWidget &guideWidget, const gameui::FramePicture *framePicture);
  virtual ~PuyoStatsLegendWidget() {}
private:
  PuyoStatsFormat &m_statsFormat;
  gameui::SliderContainer m_legendSlider[MAX_DISPLAYED_COMBOS];
  gameui::Text m_legendText[MAX_DISPLAYED_COMBOS];
};

class PuyoTwoPlayersStatsWidget : public gameui::HBox, gameui::SliderContainerListener {
public:
    PuyoTwoPlayersStatsWidget(PlayerGameStat &leftPlayerStats, PlayerGameStat &rightPlayerStats, const gameui::FramePicture *framePicture);
    virtual ~PuyoTwoPlayersStatsWidget() {}
    virtual void onWidgetVisibleChanged(bool visible);
    /**
     * Notified when the slider is inside the screen, at the end of its sliding movement
     */
    virtual void onSlideInside(gameui::SliderContainer &slider);
private:
    PuyoStatsFormat m_statsFormat;
    gameui::SliderContainer m_leftSlider, m_rightSlider, m_legendSlider;
    PuyoStatsWidget m_leftStats, m_rightStats;
    PuyoStatsLegendWidget m_legend;
};

#endif // _PUYO_STATS_WIDGET_H

