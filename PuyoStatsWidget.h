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
struct StatsFormat {
    StatsFormat(PlayerGameStat &playerAStats, PlayerGameStat &playerBStats);
    int m_comboIndirection[MAX_DISPLAYED_COMBOS+1];
};

class StatsWidget : public gameui::VBox, gameui::Action, IdleComponent {
public:
    StatsWidget(StatsFormat &statsFormat,
                    PlayerGameStat &stats, PlayerGameStat &opponentStats,
                    const gameui::FramePicture *framePicture, PuyoStatsDirection dir,
                    bool showGlobalScore = false,
                    gameui::Action *action = NULL);
    virtual ~StatsWidget() {}
    virtual void action(Widget *sender, int actionType, GameControlEvent *event);
    void startAnimation();
    virtual void idle(double currentTime);
    virtual IdleComponent *getIdleComponent() { return this; }
    bool isWinner() const { return m_stats.is_winner; }
private:
    PuyoStatsDirection m_dir;
    bool m_showGlobalScore;
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
    StatsFormat &m_statsFormat;
    PlayerGameStat &m_stats, &m_opponentStats;
    gameui::Text m_statTitle;
    ComboLine m_comboLines[MAX_DISPLAYED_COMBOS];
    int m_maxCombo;
    gameui::Text m_score, m_globalScore;
    double m_startTime;
    SelfVector<Widget*> widgetAutoReleasePool;
};

class PuyoStatsLegendWidget : public gameui::Frame, public gameui::Action {
public:
  PuyoStatsLegendWidget(StatsFormat &statsFormat, StatsWidget &guideWidget, const gameui::FramePicture *framePicture);
  virtual ~PuyoStatsLegendWidget() {}
  virtual void onWidgetVisibleChanged(bool visible);
  virtual void action(Widget *sender, int actionType, GameControlEvent *event);
private:
  gameui::Image m_statsImage;
  StatsFormat &m_statsFormat;
  StatsWidget &m_guideWidget;
  gameui::Separator m_barSeparator, m_bottomSeparator;
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
        IIM_Surface *stats_bg_winner, *stats_bg_loser;
        IIM_Surface *separator;
        StatsResources();
        ~StatsResources();
};

class PuyoTwoPlayersStatsWidget : public gameui::ZBox, gameui::SliderContainerListener {
public:
    PuyoTwoPlayersStatsWidget(PlayerGameStat &leftPlayerStats, PlayerGameStat &rightPlayerStats,
                              bool showLeftGlobalScore, bool showRightGlobalScore,
                              const gameui::FramePicture *framePicture);
    virtual ~PuyoTwoPlayersStatsWidget() {}
    virtual void onWidgetVisibleChanged(bool visible);
    /**
     * Notified when the slider is inside the screen, at the end of its sliding movement
     */
    virtual void onSlideInside(gameui::SliderContainer &slider);
private:
    StatsResources m_res;
    StatsFormat m_statsFormat;
    gameui::SliderContainer m_leftSlider, m_rightSlider, m_legendSlider;
    PuyoStatsLegendWidget m_legend;
    StatsWidget m_leftStats, m_rightStats;
    SelfVector<Widget*> widgetAutoReleasePool;
};

#endif // _PUYO_STATS_WIDGET_H

