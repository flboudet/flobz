/*
 *  StatsWidget.cpp
 *  flobopop
 *
 *  Created by Florent Boudet on 26/05/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "StatsWidget.h"
#include "FPCommander.h"

#define LINE_DURATION 0.8

using namespace gameui;
using namespace event_manager;

StatsResources::StatsResources()
{
    _rope_elt = theCommander->getSurface(IMAGE_RGBA, "gfx/progressbar/rope.png");
    _ring_left = theCommander->getSurface(IMAGE_RGBA, "gfx/progressbar/ring.png", IMAGE_READ);
    _ring_right.reset(_ring_left.get()->mirrorH());
    _ring_right.reset(_ring_left.get()->mirrorH());
    _originalFloboLeft[0] = theCommander->getSurface(IMAGE_RGBA, "gfx/progressbar/flobo_left_1.png", IMAGE_READ);
    _originalFloboLeft[1] = theCommander->getSurface(IMAGE_RGBA, "gfx/progressbar/flobo_left_2.png", IMAGE_READ);
    _originalFloboLeft[2] = theCommander->getSurface(IMAGE_RGBA, "gfx/progressbar/flobo_left_3.png", IMAGE_READ);
    _originalFloboLeft[3] = theCommander->getSurface(IMAGE_RGBA, "gfx/progressbar/flobo_left_4.png", IMAGE_READ);
    _flobo_left[0][0] = _originalFloboLeft[0];
    _flobo_left[0][1] = _originalFloboLeft[1];
    _flobo_left[0][2] = _originalFloboLeft[2];
    _flobo_left[0][3] = _originalFloboLeft[3];
    _flobo_left_mask = theCommander->getSurface(IMAGE_RGBA, "gfx/progressbar/flobo_left_mask.png");
    for (int i=0; i<4; ++i)
        _flobo_right[0][i] = _flobo_left[0][i]->mirrorH();
    _flobo_right_mask.reset(_flobo_left_mask.get()->mirrorH());

    for (int i = 0 ; i < 4 ; i++) {
		for (int j=1; j<4; ++j) {
			_flobo_left[j][i] = _flobo_left[0][i]->shiftHue(30.*j, _flobo_left_mask);
			_flobo_right[j][i] = _flobo_right[0][i]->shiftHue(30.*j, _flobo_right_mask.get());
		}
	}
    _titleImage = theCommander->getSurface(IMAGE_RGBA, "gfx/stats_title.png");
    for (int numCombo = 0 ; numCombo < MAX_DISPLAYED_COMBOS ; numCombo++) {
        std::string pictureName = std::string("gfx/combo") + std::to_string(numCombo+1) + "x_stat.png";
        _comboImage[numCombo] = theCommander->getSurface(IMAGE_RGBA, pictureName.c_str());
    }
    _stats_bg_winner = theCommander->getSurface(IMAGE_RGBA, "gfx/stats-bg.png", IMAGE_READ);
    _stats_bg_loser.reset(_stats_bg_winner.get()->shiftHue(180));
}

StatsResources::~StatsResources()
{
    for (int i = 0 ; i < 4 ; i++) {
        for (int j = 0 ; j < 4 ; j++) {
            if (i != 0) // Not deleting the original images since they come from the resource manager
                delete _flobo_left[i][j];
            delete _flobo_right[i][j];
        }
    }
}

ProgressBarWidget::ProgressBarWidget(StatsResources &res, Action *associatedAction)
  : _res(res), _value(0.), _targetValue(0.), _progressive(false),
    _progressiveDuration(LINE_DURATION), _visible(true),
    _associatedAction(associatedAction), _positiveAttitude(true), _t(0.)
{
    setPreferedSize(Vec3(0, 32));
}

void ProgressBarWidget::draw(DrawTarget *dt)
{
    if (!_visible)
        return;
    Vec3 bsize = getSize();
    Vec3 bpos = getPosition();
    IosRect dstrect;
    dstrect.x = bpos.x;
    dstrect.y = bpos.y+bsize.y-1;
    dstrect.w = bsize.x;
    dstrect.h = 1;
    RGBA bgColor(0x11, 0x66, 0x66, 0x66);
    dt->fillRect(&dstrect, bgColor);

    static const int IMG_FLOBO_WIDTH = 52;
    static const int IMG_RING_WIDTH = 0;
    static const int IMG_ROPE_ELT_WIDTH = 8;
    static const int IMG_ROPE_ELT_HEIGHT = 8;

    int rope_targetSize = (bsize.x - IMG_RING_WIDTH - IMG_FLOBO_WIDTH) * _targetValue;
    int rope_size = (bsize.x - IMG_RING_WIDTH - IMG_FLOBO_WIDTH) * _value;
    int n_rope_elt = 3 * rope_targetSize / (2 * IMG_ROPE_ELT_WIDTH);
    if (n_rope_elt < 8) n_rope_elt = 8;

    if (_dir == LEFT_TO_RIGHT)
        dstrect.x = bpos.x;
    else
        dstrect.x = bpos.x + bsize.x - 10;
    dstrect.y = bpos.y;
    dstrect.h = bsize.y;
    dstrect.w = 10;
    if (_dir == LEFT_TO_RIGHT)
        dt->draw(_res._ring_right.get(), NULL, &dstrect);
    else
        dt->draw(_res._ring_left, NULL, &dstrect);
    for (int i=1; i<=n_rope_elt; ++i) {

        dstrect.w = IMG_ROPE_ELT_WIDTH;
        dstrect.h = IMG_ROPE_ELT_HEIGHT;

        double f = i / (double)n_rope_elt;
        double c = (1.54 - cosh(1.0 - 2.0 * f)) / (1.0+3.0*_value/_targetValue);
        double oscil = 10.0 * c * sin(bpos.y + 8.1*_t + f*rope_size/8) + 10.0 * c * sin(bpos.y-5.0*_t - f*rope_size/32);
        dstrect.y = bsize.y / 3 + 75.0 * c;
        if (dstrect.y > bsize.y - dstrect.h)
            dstrect.y = bsize.y - dstrect.h;
        else {
            dstrect.y += oscil;
            if (dstrect.y > bsize.y - dstrect.h)
                dstrect.y = bsize.y - dstrect.h;
        }

        dstrect.x = rope_size * f;//- i*(IMG_ROPE_ELT_WIDTH-2);
        //if (dstrect.x < (n_rope_elt-i)*1) {
        //    dstrect.x = (n_rope_elt-i)*1;
        //}
        //if (dstrect.x > rope_size) {
        //    dstrect.x = rope_size;
        //}

        dstrect.y += bpos.y;
        if (_dir == LEFT_TO_RIGHT) {
            dstrect.x = bpos.x + dstrect.x + IMG_RING_WIDTH;
        }
        else {
            dstrect.x = bpos.x + (bsize.x - dstrect.x - IMG_ROPE_ELT_WIDTH) - IMG_RING_WIDTH - 1;
        }
        dt->draw(_res._rope_elt, NULL, &dstrect);
    }
    dstrect.w = _res._flobo_right[0][0]->w;
    dstrect.h = _res._flobo_right[0][0]->h;
    dstrect.y = bpos.y;
    if (_dir == LEFT_TO_RIGHT) {
        dstrect.x = bpos.x + rope_size + IMG_RING_WIDTH - 4;
        dt->draw(_res._flobo_right[_colorIndex][_progressive ? (fmod(_t, 0.120) > 0.06 ? 0 : 1) : (_positiveAttitude ? 2 : 3)], NULL, &dstrect);
    }
    else {
        dstrect.x = bpos.x + bsize.x - rope_size - IMG_RING_WIDTH - IMG_FLOBO_WIDTH + 4;
        dt->draw(_res._flobo_left[_colorIndex][_progressive ? (fmod(_t, 0.120) > 0.06 ? 0 : 1) : (_positiveAttitude ? 2 : 3)], NULL, &dstrect);
    }
}

void ProgressBarWidget::idle(double currentTime)
{
    _t = currentTime;
    if (_progressive) {
        double deltaT = currentTime - _targetBaseTime;
        if (deltaT < _progressiveDuration) {
            _value = _fromValue + (_targetValue - _fromValue) * sin(0.5 * M_PI * deltaT / _progressiveDuration);
            _associatedAction->action(this, VALUE_CHANGED, NULL);
        }
        else {
            _value = _targetValue;
            _progressive = false;
            _associatedAction->action(this, VALUE_CHANGED, NULL);
            _associatedAction->action(this, PROGRESSION_COMPLETE, NULL);
        }
        requestDraw(false);
    }
}

void ProgressBarWidget::setValue(float value, bool progressive)
{
    if (_value == value)
        return;
    _progressive = progressive;
    if (!progressive) {
        _value = value;
        requestDraw(false);
        _associatedAction->action(this, VALUE_CHANGED, NULL);
    }
    else {
        _fromValue = _value;
        _targetValue = value;
        _targetBaseTime = GameLoop::getCurrentTime();
    }
}

void ProgressBarWidget::setDirection(StatsDirection dir)
{
   _dir = dir;
}

void ProgressBarWidget::setVisible(bool visible)
{
    _visible = visible;
    requestDraw();
}

StatsFormat::StatsFormat(PlayerGameStat &playerAStats, PlayerGameStat &playerBStats)
{
    int j = 0;
    for (int i = 0 ; i < MAX_DISPLAYED_COMBOS+1 ; i++)
        _comboIndirection[i] = -1;
    for (int i = 0 ; (i < 24) && (j < MAX_DISPLAYED_COMBOS+1) ; i++) {
        if ((playerAStats.combo_count[i] > 0) || (playerBStats.combo_count[i] > 0))
            _comboIndirection[j++] = i;
    }
}


StatsWidget::StatsWidget(StatsResources &res, StatsFormat &statsFormat,
                         StatsWidgetDimensions &dimensions,
                         PlayerGameStat &stats, PlayerGameStat &opponentStats,
                         const gameui::FramePicture *framePicture, StatsDirection dir,
                         bool showGlobalScore,
                         gameui::Action *action)
  : _res(res), _dir(dir), _showGlobalScore(showGlobalScore),
    _action(action), _statsFormat(statsFormat),
    _stats(stats), _opponentStats(opponentStats),
    _statTitle("Combos"),
    _maxCombo(0), _startTime(-1)
{
    setPolicy(USE_MIN_SIZE);
    setInnerMargin(15);

    // Create Layout
    Text *txt = new Text(stats.is_winner?"WINNER":"LOSER");
    Frame *img1 = new Frame(theCommander->getSeparatorFramePicture());
    Separator *sep1 = new Separator();
    sep1->setPreferedSize(Vec3(128, 42));
    img1->setPreferedSize(Vec3(128, 8));
    add(txt);
    add(img1);
    add(sep1);
    for (int i = 0 ; i < MAX_DISPLAYED_COMBOS ; i++) {
        ComboLine *newComboLine = new ComboLine(res);
        newComboLine->setDimensions(dimensions);
        add(newComboLine);
        _widgetAutoReleasePool.emplace_back(newComboLine);
        _comboLines.push_back(newComboLine);
    }
    Separator *sep = new Separator();
    sep->setPreferedSize(Vec3(128, 24));
    add(sep);
    Frame *img2 = new Frame(theCommander->getSeparatorFramePicture());
    img2->setPreferedSize(Vec3(128, 8));
    add(img2);

    VBox *box = new VBox();
    HBox *scorebox = new HBox();
    HBox *totalscorebox = new HBox();
    Text *score = new Text("Score:");
    Text *globalScore = new Text("Total:");
    score->setAutoSize(false);
    globalScore->setAutoSize(false);
    score->setPreferedSize(Vec3(200., 0.));
    globalScore->setPreferedSize(Vec3(200., 0.));
    _score.setValue("0");
    _globalScore.setValue("0");
    _score.setAutoSize(false);
    _globalScore.setAutoSize(false);
    _score.setPreferedSize(Vec3(0., 0.));
    _globalScore.setPreferedSize(Vec3(0., 0.));
    _score.setTextAlign(TEXT_RIGHT_ALIGN);
    _globalScore.setTextAlign(TEXT_RIGHT_ALIGN);
    scorebox->add(score);
    scorebox->add(&_score);
    if (_showGlobalScore) {
        totalscorebox->add(globalScore);
        totalscorebox->add(&_globalScore);
    }
    box->add(scorebox);
    box->add(totalscorebox);
    add(box);

    // Prepare un-allocation
    _widgetAutoReleasePool.emplace_back(sep);
    _widgetAutoReleasePool.emplace_back(sep1);
    _widgetAutoReleasePool.emplace_back(img1);
    _widgetAutoReleasePool.emplace_back(img2);
    _widgetAutoReleasePool.emplace_back(txt);
    _widgetAutoReleasePool.emplace_back(score);
    _widgetAutoReleasePool.emplace_back(globalScore);
    _widgetAutoReleasePool.emplace_back(scorebox);
    _widgetAutoReleasePool.emplace_back(totalscorebox);
    _widgetAutoReleasePool.emplace_back(box);

    // Looking for the biggest combo
    for (int i = 0 ; i < 24 ; i++) {
        if (_stats.combo_count[i] > _maxCombo)
            _maxCombo = _stats.combo_count[i];
    }
    for (int i = 0 ; i < 24 ; i++) {
        if (_opponentStats.combo_count[i] > _maxCombo)
            _maxCombo = _opponentStats.combo_count[i];
    }
}

void StatsWidget::action(Widget *sender, int actionType, GameControlEvent *event)
{
    if (_action != NULL)
        _action->action(this, actionType+1, event);
    int comboIndirectionIndex = actionType + 1;
    if (comboIndirectionIndex >= MAX_DISPLAYED_COMBOS)
        return;
    int currentComboIndex = _statsFormat._comboIndirection[comboIndirectionIndex];
    if (currentComboIndex != -1)
        _comboLines[comboIndirectionIndex]->setComboLineInfos(_dir, comboIndirectionIndex, "",
                                                        _stats.combo_count[currentComboIndex],
                                                        _opponentStats.combo_count[currentComboIndex],
                                                        _maxCombo, this);
}

void StatsWidget::startAnimation()
{
    int currentComboIndex = _statsFormat._comboIndirection[0];
    if (currentComboIndex != -1)
        _comboLines[0]->setComboLineInfos(_dir, 0, "",
                                          _stats.combo_count[currentComboIndex],
                                          _opponentStats.combo_count[currentComboIndex],
                                          _maxCombo, this);
    if (_action != NULL)
        _action->action(this, 0, NULL);
}

StatsWidget::ComboLine::ComboLine(StatsResources &res)
  : _progressBar(res, this)
{
    //add(&_comboLabel);
}

void StatsWidget::ComboLine::setComboLineInfos(StatsDirection dir, int tag, const std::string &comboText,
                                                   int numberOfCombos, int vsNumberOfCombos,
                                                   int totalNumOfCombos, Action *progressionCompleteAction)
{
    /*if (numberOfCombos == 0)
        return;*/
    _dir = dir;

    _currentValue.setAutoSize(false);
    _currentValue.setPreferedSize(Vec3(_dimensions._comboLineValueWidth));

    if (_dir == LEFT_TO_RIGHT) {
        _currentValue.setTextAlign(TEXT_LEFT_ALIGN);
        add(&_progressBar);
        add(&_currentValue);
    }
    else {
        _currentValue.setTextAlign(TEXT_RIGHT_ALIGN);
        add(&_currentValue);
        add(&_progressBar);
    }
    _progressBar.setVisible(false);

    _progressBar.setDirection(dir);
    _tag = tag;
    _progressionCompleteAction = progressionCompleteAction;
    _totalNumOfCombos = totalNumOfCombos;
    float progressBarValue = (float)(1+numberOfCombos) / (float)(1+_totalNumOfCombos);
    _comboLabel.setValue(comboText);
    _progressBar.setVisible(true);
    _progressBar.setValue(progressBarValue, true);
    _progressBar.setPositiveAttitude((numberOfCombos >= vsNumberOfCombos) && (numberOfCombos > 0) ? true : false);
    int bof = (int)(progressBarValue * 3.0f);
    if (bof > 3) bof = 3;
    _progressBar.setColorIndex( bof);
}

void StatsWidget::idle(double currentTime)
{
    if (_startTime < 0.0) _startTime = currentTime;
    const double duration = MAX_DISPLAYED_COMBOS * LINE_DURATION + 0.5;
    int points = _stats.points * sin(1.5708 * (currentTime-_startTime) / duration);
    if (currentTime-_startTime > duration) points = _stats.points;
    _score.setValue(std::to_string(points));
    _globalScore.setValue(std::to_string(_stats.total_points + points));
}

void StatsWidget::ComboLine::action(Widget *sender, int actionType, GameControlEvent *event)
{
    switch (actionType) {
        case ProgressBarWidget::VALUE_CHANGED:
            //_currentValue.setValue(std::string("") + (int)(_progressBar.getValue() * (float)_totalNumOfCombos) + " "); // TODO: string
            break;
        case ProgressBarWidget::PROGRESSION_COMPLETE:
            _progressionCompleteAction->action(this, _tag, event);
            break;
        default:
            break;
    }
}

StatsLegendWidget::StatsLegendWidget(StatsFormat &statsFormat, StatsWidget &guideWidget,
                                     const gameui::FramePicture *framePicture, StatsResources &res)
  : Frame(framePicture), _statsImage(), _statsFormat(statsFormat), _guideWidget(guideWidget)
{
    setPolicy(USE_MIN_SIZE);
    setInnerMargin(10);
    // Load title image
    _statsImage.setImage(res._titleImage);
    _statsImage.setAlign(IMAGE_CENTERED);
    add(&_statsImage);
    for (int i = 0 ; i < MAX_DISPLAYED_COMBOS ; i++) {
        int numCombo = _statsFormat._comboIndirection[i];
        if (numCombo != -1) {
            _legendImage[i].setImage(res._comboImage[numCombo]);
            _legendImage[i].setAlign(IMAGE_CENTERED);
            _legendCell[i].add(&_legendImage[i]);
        }
        _legendSlider[i].setSlideSide(SliderContainer::SLIDE_FROM_BOTTOM);
        add(&_legendSlider[i]);
    }
    add(&_bottomSeparator);
}

void StatsLegendWidget::onWidgetVisibleChanged(bool visible)
{
  // Set the size of the different rows
  for (int i = 0 ; i < this->getNumberOfChilds() - 2 ; i++) {
    Vec3 elementSize = _guideWidget.getChild(i+2)->getSize();
    getChild(i)->setPreferedSize(Vec3(0, elementSize.y));
  }
  // Special case: the title should have the size of the first 2 items of the guide
  // +10 is to compensate the difference in inner margin
  getChild(0)->setPreferedSize(Vec3(0, getChild(0)->getPreferedSize().y + _guideWidget.getChild(0)->getSize().y + 10));
}

void StatsLegendWidget::action(Widget *sender, int actionType, GameControlEvent *event)
{
    if (actionType < MAX_DISPLAYED_COMBOS)
        _legendSlider[actionType].transitionToContent(&_legendCell[actionType]);
}

TwoPlayersStatsWidget::TwoPlayersStatsWidget(PlayerGameStat &leftPlayerStats, PlayerGameStat &rightPlayerStats,
                                                     bool showLeftGlobalScore, bool showRightGlobalScore,
                                                     const gameui::FramePicture *framePicture, StatsWidgetDimensions &dimensions)
  : _statsFormat(leftPlayerStats, rightPlayerStats),
    _legend(_statsFormat, _leftStats, NULL, _res),
    _leftStats(_res, _statsFormat, dimensions, leftPlayerStats, rightPlayerStats, NULL, RIGHT_TO_LEFT, showLeftGlobalScore, &_legend),
    _rightStats(_res, _statsFormat, dimensions, rightPlayerStats, leftPlayerStats, NULL, LEFT_TO_RIGHT, showRightGlobalScore),
    _height(dimensions._height), _legendWidth(dimensions._legendWidth)
{
    _legendSlider.setPreferedSize(Vec3(_legendWidth, _height));
    _leftSlider.setPreferedSize(Vec3(0., _height));
    _rightSlider.setPreferedSize(Vec3(0., _height));
    _leftSlider.setBackgroundOffset(dimensions._leftBackgroundOffset);
    _rightSlider.setBackgroundOffset(dimensions._rightBackgroundOffset);
    HBox *backBox = new HBox();
    VBox *v1 = new VBox();
    Separator *sep1 = new Separator();
    v1->add(&_leftSlider);
    v1->add(sep1);
    backBox->add(v1);
    VBox *v3 = new VBox();
    v3->setPreferedSize(Vec3(_legendWidth, 0.));
    backBox->add(v3);
    VBox *v2 = new VBox();
    Separator *sep2 = new Separator();
    v2->add(&_rightSlider);
    v2->add(sep2);
    backBox->add(v2);

    HBox *frontBox = new HBox();
    VBox *f1 = new VBox();
    frontBox->add(f1);
    VBox *f3 = new VBox();
    Separator *sep3 = new Separator();
    f3->setPreferedSize(Vec3(_legendWidth, 0.));
    f3->add(&_legendSlider);
    f3->add(sep3);
    frontBox->add(f3);
    VBox *f2 = new VBox();
    frontBox->add(f2);

    add(backBox);
    add(frontBox);

    _leftSlider.addListener(*this);
    _legendSlider.addListener(*this);
    _rightSlider.addListener(*this);

    _widgetAutoReleasePool.emplace_back(backBox);
    _widgetAutoReleasePool.emplace_back(v1);
    _widgetAutoReleasePool.emplace_back(v2);
    _widgetAutoReleasePool.emplace_back(v3);
    _widgetAutoReleasePool.emplace_back(sep1);
    _widgetAutoReleasePool.emplace_back(sep2);
    _widgetAutoReleasePool.emplace_back(sep3);
    _widgetAutoReleasePool.emplace_back(frontBox);
    _widgetAutoReleasePool.emplace_back(f1);
    _widgetAutoReleasePool.emplace_back(f2);
    _widgetAutoReleasePool.emplace_back(f3);
}

void TwoPlayersStatsWidget::onWidgetVisibleChanged(bool visible)
{
    _leftSlider.setSlideSide(SliderContainer::SLIDE_FROM_LEFT);
    _rightSlider.setSlideSide(SliderContainer::SLIDE_FROM_RIGHT);
    _legendSlider.setSlideSide(SliderContainer::SLIDE_FROM_BOTTOM);
    _leftSlider.transitionToContent(&_leftStats);
    _rightSlider.transitionToContent(&_rightStats);
    _legendSlider.transitionToContent(&_legend);
    _leftSlider.setBackground(_leftStats.isWinner() ? _res._stats_bg_winner.get() : _res._stats_bg_loser.get());
    _rightSlider.setBackground(_rightStats.isWinner() ? _res._stats_bg_winner.get() : _res._stats_bg_loser.get());
}

void TwoPlayersStatsWidget::onSlideInside(SliderContainer &slider)
{
    if (&slider == &_leftSlider)
        _leftStats.startAnimation();
    if (&slider == &_rightSlider)
        _rightStats.startAnimation();
}

