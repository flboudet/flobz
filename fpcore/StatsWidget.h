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
        : _height(height), _legendWidth(legendWidth),
          _comboLineValueWidth(comboLineValueWidth),
          _leftBackgroundOffset(leftBackgroundOffset),
          _rightBackgroundOffset(rightBackgroundOffset) {}
    StatsWidgetDimensions()
        : _height(0), _legendWidth(0),
          _comboLineValueWidth(0) {}
    float _height, _legendWidth;
    float _comboLineValueWidth;
    Vec3  _leftBackgroundOffset, _rightBackgroundOffset;
};

#define MAX_DISPLAYED_COMBOS 5 // TODO: More
class StatsResources {
public:
    IosSurfaceRef _rope_elt;
    IosSurfaceRef _ring_left;
    IosSurfaceRef _originalFloboLeft[4];
    std::unique_ptr<IosSurface> _ring_right;
    IosSurface * _flobo_right[4][4];
    IosSurface *_flobo_left[4][4];
    IosSurfaceRef _flobo_left_mask;
    std::unique_ptr<IosSurface> _flobo_right_mask;
    IosSurfaceRef _stats_bg_winner;
    std::unique_ptr<IosSurface> _stats_bg_loser;
    IosSurfaceRef _titleImage;
    IosSurfaceRef _comboImage[MAX_DISPLAYED_COMBOS];
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
    float getValue() const { return _value; }
    void setVisible(bool visible);
    void setDirection(StatsDirection dir);
    void setPositiveAttitude(bool positiveAttitude) { _positiveAttitude = positiveAttitude; }
    void setColorIndex(int colorIndex) { _colorIndex = colorIndex; }
    enum {
        VALUE_CHANGED,
        PROGRESSION_COMPLETE
    };
private:
    StatsResources &_res;
    StatsDirection _dir;
    float _value, _fromValue, _targetValue;
    bool _progressive;
    double _targetBaseTime;
    double _progressiveDuration;
    bool _visible;
    gameui::Action *_associatedAction;
    double _t;
    bool _positiveAttitude;
    int _colorIndex;
};

struct StatsFormat {
    StatsFormat(PlayerGameStat &playerAStats, PlayerGameStat &playerBStats);
    int _comboIndirection[MAX_DISPLAYED_COMBOS+1];
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
    bool isWinner() const { return _stats.is_winner; }
private:
    StatsResources &_res;
    StatsDirection _dir;
    bool _showGlobalScore;
    gameui::Action *_action;
    class ComboLine : public gameui::HBox, gameui::Action {
    public:
        ComboLine(StatsResources &res);
        virtual ~ComboLine() {}
        void setComboLineInfos(StatsDirection dir, int tag, const std::string &comboText,
                               int numberOfCombos, int vsNumberOfCombos,
                               int totalNumOfCombos, gameui::Action *progressionCompleteAction);
        void setDimensions(StatsWidgetDimensions &dimensions) {
            _dimensions = dimensions;
        }
        virtual void action(Widget *sender, int actionType, event_manager::GameControlEvent *event);
    private:
        StatsWidgetDimensions _dimensions;
        StatsDirection _dir;
        int _tag;
        gameui::Text _comboLabel;
        Action *_progressionCompleteAction;
        ProgressBarWidget _progressBar;
        gameui::Text _currentValue;
        int _totalNumOfCombos;
    };
    StatsFormat &_statsFormat;
    PlayerGameStat &_stats, &_opponentStats;
    gameui::Text _statTitle;
    std::vector<ComboLine *> _comboLines;
    int _maxCombo;
    gameui::Text _score, _globalScore;
    double _startTime;
    std::vector<std::unique_ptr<Widget>> _widgetAutoReleasePool;
};

class StatsLegendWidget : public gameui::Frame, public gameui::Action {
public:
  StatsLegendWidget(StatsFormat &statsFormat, StatsWidget &guideWidget,
                    const gameui::FramePicture *framePicture, StatsResources &res);
  virtual ~StatsLegendWidget() {}
  virtual void onWidgetVisibleChanged(bool visible);
  virtual void action(Widget *sender, int actionType, event_manager::GameControlEvent *event);
private:
  gameui::Image _statsImage;
  StatsFormat &_statsFormat;
  StatsWidget &_guideWidget;
  gameui::Separator _barSeparator, _bottomSeparator;
  gameui::SliderContainer _legendSlider[MAX_DISPLAYED_COMBOS];
  gameui::HBox _legendCell[MAX_DISPLAYED_COMBOS];
  gameui::Image _legendImage[MAX_DISPLAYED_COMBOS];
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
    StatsResources _res;
    StatsFormat _statsFormat;
    gameui::SliderContainer _leftSlider, _rightSlider, _legendSlider;
    StatsLegendWidget _legend;
    StatsWidget _leftStats, _rightStats;
    float _height, _legendWidth;
    std::vector<std::unique_ptr<Widget>> _widgetAutoReleasePool;
};

#endif // _FLOBO_STATS_WIDGET_H

