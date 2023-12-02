/*
 *  StatsWidget.h
 *  flobopop
 *
 *  Created by Florent Boudet on 26/05/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _FLOBO_STATS_WIDGET_H
#define _FLOBO_STATS_WIDGET_H

#include "FPCommander.h"
#include "gameloop.h"
#include "gameui.h"
#include "Frame.h"
#include "FloboGameAbstract.h"

enum StatsDirection {
    RIGHT_TO_LEFT,
    LEFT_TO_RIGHT
};

struct StatsWidgetDimensions {
    StatsWidgetDimensions(float height, float legendWidth,
                          float comboLineValueWidth,
                          Vec3 leftBackgroundOffset, Vec3 rightBackgroundOffset)
        : m_height(height), m_legendWidth(legendWidth),
          m_comboLineValueWidth(comboLineValueWidth),
          m_leftBackgroundOffset(leftBackgroundOffset),
          m_rightBackgroundOffset(rightBackgroundOffset) {}
    StatsWidgetDimensions()
        : m_height(0), m_legendWidth(0),
          m_comboLineValueWidth(0) {}
    float m_height, m_legendWidth;
    float m_comboLineValueWidth;
    Vec3  m_leftBackgroundOffset, m_rightBackgroundOffset;
};

#define MAX_DISPLAYED_COMBOS 5 // TODO: More
class StatsResources {
public:
    IosSurfaceRef rope_elt;
    IosSurfaceRef ring_left;
    IosSurfaceRef originalFloboLeft[4];
    std::unique_ptr<IosSurface> ring_right;
    IosSurface * flobo_right[4][4];
    IosSurface *flobo_left[4][4];
    IosSurfaceRef flobo_left_mask;
    std::unique_ptr<IosSurface> flobo_right_mask;
    IosSurfaceRef stats_bg_winner;
    std::unique_ptr<IosSurface> stats_bg_loser;
    IosSurfaceRef titleImage;
    IosSurfaceRef comboImage[MAX_DISPLAYED_COMBOS];
public:
    StatsResources();
    ~StatsResources();
};

class ProgressBarWidget : public gameui::Widget, IdleComponent {
public:
    ProgressBarWidget(StatsResources &res, gameui::Action *associatedAction);
    virtual ~ProgressBarWidget() {}
    void draw(DrawTarget *dt);
    void idle(double currentTime);
    IdleComponent *getIdleComponent() { return this; }
    void setValue(float value, bool progressive = false);
    float getValue() const { return m_value; }
    void setVisible(bool visible);
    void setDirection(StatsDirection dir);
    void setPositiveAttitude(bool positiveAttitude) { m_positiveAttitude = positiveAttitude; }
    void setColorIndex(int colorIndex) { m_colorIndex = colorIndex; }
    enum {
        VALUE_CHANGED,
        PROGRESSION_COMPLETE
    };
private:
    StatsResources &m_res;
    StatsDirection m_dir;
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

struct StatsFormat {
    StatsFormat(PlayerGameStat &playerAStats, PlayerGameStat &playerBStats);
    int m_comboIndirection[MAX_DISPLAYED_COMBOS+1];
};

class StatsWidget : public gameui::VBox, gameui::Action, IdleComponent {
public:
    StatsWidget(StatsResources &res, StatsFormat &statsFormat,
                StatsWidgetDimensions &dimensions,
                PlayerGameStat &stats, PlayerGameStat &opponentStats,
                const gameui::FramePicture *framePicture, StatsDirection dir,
                bool showGlobalScore = false,
                gameui::Action *action = NULL);
    virtual ~StatsWidget() {}
    virtual void action(Widget *sender, int actionType, event_manager::GameControlEvent *event);
    void startAnimation();
    virtual void idle(double currentTime);
    virtual IdleComponent *getIdleComponent() { return this; }
    bool isWinner() const { return m_stats.is_winner; }
private:
    StatsResources &m_res;
    StatsDirection m_dir;
    bool m_showGlobalScore;
    gameui::Action *m_action;
    class ComboLine : public gameui::HBox, gameui::Action {
    public:
        ComboLine(StatsResources &res);
        virtual ~ComboLine() {}
        void setComboLineInfos(StatsDirection dir, int tag, String comboText,
                               int numberOfCombos, int vsNumberOfCombos,
                               int totalNumOfCombos, gameui::Action *progressionCompleteAction);
        void setDimensions(StatsWidgetDimensions &dimensions) {
            m_dimensions = dimensions;
        }
        virtual void action(Widget *sender, int actionType, event_manager::GameControlEvent *event);
    private:
        StatsWidgetDimensions m_dimensions;
        StatsDirection m_dir;
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
    std::vector<ComboLine *> m_comboLines;
    int m_maxCombo;
    gameui::Text m_score, m_globalScore;
    double m_startTime;
    SelfVector<Widget*> widgetAutoReleasePool;
};

class StatsLegendWidget : public gameui::Frame, public gameui::Action {
public:
  StatsLegendWidget(StatsFormat &statsFormat, StatsWidget &guideWidget,
                    const gameui::FramePicture *framePicture, StatsResources &res);
  virtual ~StatsLegendWidget() {}
  virtual void onWidgetVisibleChanged(bool visible);
  virtual void action(Widget *sender, int actionType, event_manager::GameControlEvent *event);
private:
  gameui::Image m_statsImage;
  StatsFormat &m_statsFormat;
  StatsWidget &m_guideWidget;
  gameui::Separator m_barSeparator, m_bottomSeparator;
  gameui::SliderContainer m_legendSlider[MAX_DISPLAYED_COMBOS];
  gameui::HBox m_legendCell[MAX_DISPLAYED_COMBOS];
  gameui::Image m_legendImage[MAX_DISPLAYED_COMBOS];
};

class TwoPlayersStatsWidget : public gameui::ZBox, gameui::SliderContainerListener {
public:
    TwoPlayersStatsWidget(PlayerGameStat &leftPlayerStats, PlayerGameStat &rightPlayerStats,
                              bool showLeftGlobalScore, bool showRightGlobalScore,
                              const gameui::FramePicture *framePicture, StatsWidgetDimensions &dimensions);
    virtual ~TwoPlayersStatsWidget() {}
    virtual void onWidgetVisibleChanged(bool visible);
    /**
     * Notified when the slider is inside the screen, at the end of its sliding movement
     */
    virtual void onSlideInside(gameui::SliderContainer &slider);
private:
    StatsResources m_res;
    StatsFormat m_statsFormat;
    gameui::SliderContainer m_leftSlider, m_rightSlider, m_legendSlider;
    StatsLegendWidget m_legend;
    StatsWidget m_leftStats, m_rightStats;
    float m_height, m_legendWidth;
    SelfVector<Widget*> widgetAutoReleasePool;
};

#endif // _FLOBO_STATS_WIDGET_H

