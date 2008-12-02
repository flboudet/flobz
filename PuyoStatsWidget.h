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

#include "gameloop.h"
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
    void setPositiveAttitude(bool positiveAttitude) { m_positiveAttitude = positiveAttitude; }
    void setColorIndex(int colorIndex) { m_colorIndex = colorIndex; }
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
    bool m_positiveAttitude;
    int m_colorIndex;
};

#define MAX_DISPLAYED_COMBOS 7
struct PuyoStatsFormat {
    PuyoStatsFormat(PlayerGameStat &playerAStats, PlayerGameStat &playerBStats);
    int m_comboIndirection[MAX_DISPLAYED_COMBOS+1];
};

class PuyoStatsWidget : public gameui::VBox, gameui::Action, IdleComponent {
public:
    PuyoStatsWidget(PuyoStatsFormat &statsFormat,
                    PlayerGameStat &stats, PlayerGameStat &opponentStats,
                    const gameui::FramePicture *framePicture, PuyoStatsDirection dir,
                    gameui::Action *action = NULL);
    virtual ~PuyoStatsWidget() {}
    virtual void action(Widget *sender, int actionType, GameControlEvent *event);
    void startAnimation();
    virtual void idle(double currentTime);
    virtual IdleComponent *getIdleComponent() { return this; }
private:
    PuyoStatsDirection m_dir;
    gameui::Action *m_action;
    class ComboLine : public gameui::HBox, gameui::Action {
    public:
        ComboLine();
        virtual ~ComboLine() {}
        void setComboLineInfos(PuyoStatsDirection dir, int tag, String comboText,
                               int numberOfCombos, int vsNumberOfCombos,
                               int totalNumOfCombos, gameui::Action *progressionCompleteAction);
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
    gameui::Text m_score;
    double m_startTime;
    SelfVector<Widget*> widgetAutoReleasePool;
};

class PuyoStatsLegendWidget : public gameui::Frame, public gameui::Action {
public:
  PuyoStatsLegendWidget(PuyoStatsFormat &statsFormat, PuyoStatsWidget &guideWidget, const gameui::FramePicture *framePicture);
  virtual ~PuyoStatsLegendWidget() {}
  virtual void onWidgetVisibleChanged(bool visible);
  virtual void action(Widget *sender, int actionType, GameControlEvent *event);
private:
  PuyoStatsFormat &m_statsFormat;
  PuyoStatsWidget &m_guideWidget;
  gameui::Separator m_titleSeparator, m_barSeparator, m_bottomSeparator;
  gameui::SliderContainer m_legendSlider[MAX_DISPLAYED_COMBOS];
  gameui::HBox m_legendCell[MAX_DISPLAYED_COMBOS];
  gameui::Image m_legendImage[MAX_DISPLAYED_COMBOS];
};

class StatsResources {
    public:
        IIM_Surface *rope_elt;
        IIM_Surface *ring_left, *ring_right;
        IIM_Surface *puyo_right[4][4];
        IIM_Surface *puyo_left[4][4];
        IIM_Surface *puyo_left_mask, *puyo_right_mask;
        IIM_Surface *stats_bg;
        IIM_Surface *separator;
        StatsResources();
        ~StatsResources();
};

class PuyoTwoPlayersStatsWidget : public gameui::VBox, gameui::SliderContainerListener {
public:
    PuyoTwoPlayersStatsWidget(PlayerGameStat &leftPlayerStats, PlayerGameStat &rightPlayerStats, const gameui::FramePicture *framePicture);
    virtual ~PuyoTwoPlayersStatsWidget() {}
    virtual void onWidgetVisibleChanged(bool visible);
    /**
     * Notified when the slider is inside the screen, at the end of its sliding movement
     */
    virtual void onSlideInside(gameui::SliderContainer &slider);
private:
    StatsResources m_res;
    PuyoStatsFormat m_statsFormat;
    gameui::SliderContainer m_topSlider, m_leftSlider, m_rightSlider, m_legendSlider;
    gameui::Text m_title;
    PuyoStatsLegendWidget m_legend;
    PuyoStatsWidget m_leftStats, m_rightStats;
    SelfVector<Widget*> widgetAutoReleasePool;
};

#endif // _PUYO_STATS_WIDGET_H

