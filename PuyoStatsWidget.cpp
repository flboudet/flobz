/*
 *  StatsWidget.cpp
 *  flobopuyo
 *
 *  Created by Florent Boudet on 26/05/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "PuyoStatsWidget.h"
#include "PuyoCommander.h"

#define LINE_DURATION 0.8

using namespace gameui;

static StatsResources *res;

StatsResources::StatsResources()
{
    res = this;
    rope_elt = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/progressbar/rope.png"));
    ring_left = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/progressbar/ring.png"));
    ring_right = iim_surface_mirror_h(ring_left);
    puyo_left[0][0] = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/progressbar/puyo_left_1.png"));
    puyo_left[0][1] = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/progressbar/puyo_left_2.png"));
    puyo_left[0][2] = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/progressbar/puyo_left_3.png"));
    puyo_left[0][3] = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/progressbar/puyo_left_4.png"));
    puyo_left_mask = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/progressbar/puyo_left_mask.png"));
    puyo_right[0][0] = iim_surface_mirror_h(puyo_left[0][0]);
    puyo_right[0][1] = iim_surface_mirror_h(puyo_left[0][1]);
    puyo_right[0][2] = iim_surface_mirror_h(puyo_left[0][2]);
    puyo_right[0][3] = iim_surface_mirror_h(puyo_left[0][3]);
    puyo_right_mask = iim_surface_mirror_h(puyo_left_mask);

    for (int i = 0 ; i < 4 ; i++) {
        puyo_left[1][i] = iim_surface_shift_hue_masked(puyo_left[0][i], puyo_left_mask, 30.);
        puyo_right[1][i] = iim_surface_shift_hue_masked(puyo_right[0][i], puyo_right_mask, 30.);
        puyo_left[2][i] = iim_surface_shift_hue_masked(puyo_left[0][i], puyo_left_mask, 60.);
        puyo_right[2][i] = iim_surface_shift_hue_masked(puyo_right[0][i], puyo_right_mask, 60.);
        puyo_left[3][i] = iim_surface_shift_hue_masked(puyo_left[0][i], puyo_left_mask, 90.);
        puyo_right[3][i] = iim_surface_shift_hue_masked(puyo_right[0][i], puyo_right_mask, 90.);
    }
    separator = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/separator.png"));
    stats_bg_winner = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/stats-bg.png"));
    stats_bg_loser = iim_surface_shift_hue(stats_bg_winner, 180);
    // TODO: Combos images
}

StatsResources::~StatsResources()
{
    IIM_Free(rope_elt);
    for (int i = 0 ; i < 4 ; i++) {
        for (int j = 0 ; j < 4 ; j++) {
            IIM_Free(puyo_left[i][j]);
            IIM_Free(puyo_right[i][j]);
        }
    }
    IIM_Free(puyo_left_mask);
    IIM_Free(puyo_right_mask);
    IIM_Free(stats_bg_winner);
    IIM_Free(stats_bg_loser);
    IIM_Free(separator);
}

ProgressBarWidget::ProgressBarWidget(Action *associatedAction)
  : m_value(0.), m_targetValue(0.), m_progressive(false), m_progressiveDuration(LINE_DURATION), m_visible(true),
    m_associatedAction(associatedAction), m_positiveAttitude(true)
{
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
    static const int IMG_RING_WIDTH = 0;
    static const int IMG_ROPE_ELT_WIDTH = 8;
    static const int IMG_ROPE_ELT_HEIGHT = 8;

    int rope_targetSize = (bsize.x - IMG_RING_WIDTH - IMG_PUYO_WIDTH) * m_targetValue;
    int rope_size = (bsize.x - IMG_RING_WIDTH - IMG_PUYO_WIDTH) * m_value;
    int n_rope_elt = 3 * rope_targetSize / (2 * IMG_ROPE_ELT_WIDTH);
    if (n_rope_elt < 8) n_rope_elt = 8;

    if (m_dir == LEFT_TO_RIGHT)
        dstrect.x = bpos.x;
    else
        dstrect.x = bpos.x + bsize.x - 10;
    dstrect.y = bpos.y;
    dstrect.h = bsize.y;
    dstrect.w = 10;
    if (m_dir == LEFT_TO_RIGHT)
        SDL_BlitSurface(res->ring_right->surf, NULL, screen, &dstrect);
    else
        SDL_BlitSurface(res->ring_left->surf, NULL, screen, &dstrect);
    for (int i=1; i<=n_rope_elt; ++i) {

        dstrect.w = IMG_ROPE_ELT_WIDTH;
        dstrect.h = IMG_ROPE_ELT_HEIGHT;

        double f = i / (double)n_rope_elt;
        double c = (1.54 - cosh(1.0 - 2.0 * f)) / (1.0+3.0*m_value/m_targetValue);
        double oscil = 10.0 * c * sin(bpos.y + 8.1*m_t + f*rope_size/8) + 10.0 * c * sin(bpos.y-5.0*m_t - f*rope_size/32);
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
        if (m_dir == LEFT_TO_RIGHT) {
            dstrect.x = bpos.x + dstrect.x + IMG_RING_WIDTH;
        }
        else {
            dstrect.x = bpos.x + (bsize.x - dstrect.x - IMG_ROPE_ELT_WIDTH) - IMG_RING_WIDTH - 1;
        }
        SDL_BlitSurface(res->rope_elt->surf, NULL, screen, &dstrect);
    }
    dstrect.w = res->puyo_right[0][0]->w;
    dstrect.h = res->puyo_right[0][0]->h;
    dstrect.y = bpos.y;
    if (m_dir == LEFT_TO_RIGHT) {
        dstrect.x = bpos.x + rope_size + IMG_RING_WIDTH - 4;
        SDL_BlitSurface(res->puyo_right[m_colorIndex][m_progressive ? (fmod(m_t, 0.120) > 0.06 ? 0 : 1) : (m_positiveAttitude ? 2 : 3)]->surf, NULL, screen, &dstrect);
    }
    else {
        dstrect.x = bpos.x + bsize.x - rope_size - IMG_RING_WIDTH - IMG_PUYO_WIDTH + 4;
        SDL_BlitSurface(res->puyo_left[m_colorIndex][m_progressive ? (fmod(m_t, 0.120) > 0.06 ? 0 : 1) : (m_positiveAttitude ? 2 : 3)]->surf, NULL, screen, &dstrect);
    }
}

void ProgressBarWidget::idle(double currentTime)
{
    m_t = currentTime;
    if (m_progressive) {
        double deltaT = currentTime - m_targetBaseTime;
        if (deltaT < m_progressiveDuration) {
            m_value = m_fromValue + (m_targetValue - m_fromValue) * sin(0.5 * M_PI * deltaT / m_progressiveDuration);
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

void ProgressBarWidget::setDirection(PuyoStatsDirection dir)
{
   m_dir = dir;
}

void ProgressBarWidget::setVisible(bool visible)
{
    m_visible = visible;
    requestDraw();
}

StatsFormat::StatsFormat(PlayerGameStat &playerAStats, PlayerGameStat &playerBStats)
{
    int j = 0;
    for (int i = 0 ; i < MAX_DISPLAYED_COMBOS+1 ; i++)
        m_comboIndirection[i] = -1;
    for (int i = 0 ; (i < 24) && (j < MAX_DISPLAYED_COMBOS+1) ; i++) {
        if ((playerAStats.combo_count[i] > 0) || (playerBStats.combo_count[i] > 0))
            m_comboIndirection[j++] = i;
    }
}


StatsWidget::StatsWidget(StatsFormat &statsFormat,
                                 PlayerGameStat &stats, PlayerGameStat &opponentStats,
                                 const gameui::FramePicture *framePicture, PuyoStatsDirection dir,
                                 bool showGlobalScore,
                                 gameui::Action *action)
  : m_dir(dir), m_showGlobalScore(showGlobalScore),
    m_action(action), m_statsFormat(statsFormat),
    m_stats(stats), m_opponentStats(opponentStats),
    m_statTitle("Combos"), m_maxCombo(0), m_startTime(-1)
{
    setPolicy(USE_MIN_SIZE);
    setInnerMargin(15);

    // Create Layout
    Text *txt = new Text(stats.is_winner?"WINNER":"LOSER");
    Image *img1 = new Image(res->separator);
    img1->setPreferedSize(Vec3(128, 50));
    add(txt);
    add(img1);
    for (int i = 0 ; i < MAX_DISPLAYED_COMBOS ; i++) {
        add(&m_comboLines[i]);
    }
    Separator *sep = new Separator();
    sep->setPreferedSize(Vec3(128, 24));
    add(sep);
    Image *img2 = new Image(res->separator);
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
    m_score.setValue("0");
    m_globalScore.setValue("0");
    m_score.setAutoSize(false);
    m_globalScore.setAutoSize(false);
    m_score.setPreferedSize(Vec3(0., 0.));
    m_globalScore.setPreferedSize(Vec3(0., 0.));
    m_score.setTextAlign(TEXT_RIGHT_ALIGN);
    m_globalScore.setTextAlign(TEXT_RIGHT_ALIGN);
    scorebox->add(score);
    scorebox->add(&m_score);
    if (m_showGlobalScore) {
        totalscorebox->add(globalScore);
        totalscorebox->add(&m_globalScore);
    }
    box->add(scorebox);
    box->add(totalscorebox);
    add(box);

    // Prepare un-allocation
    widgetAutoReleasePool.add(sep);
    widgetAutoReleasePool.add(img1);
    widgetAutoReleasePool.add(img2);
    widgetAutoReleasePool.add(txt);
    widgetAutoReleasePool.add(score);
    widgetAutoReleasePool.add(globalScore);
    widgetAutoReleasePool.add(scorebox);
    widgetAutoReleasePool.add(totalscorebox);
    widgetAutoReleasePool.add(box);

    // Looking for the biggest combo
    for (int i = 0 ; i < 24 ; i++) {
        if (m_stats.combo_count[i] > m_maxCombo)
            m_maxCombo = m_stats.combo_count[i];
    }
    for (int i = 0 ; i < 24 ; i++) {
        if (m_opponentStats.combo_count[i] > m_maxCombo)
            m_maxCombo = m_opponentStats.combo_count[i];
    }
}

void StatsWidget::action(Widget *sender, int actionType, GameControlEvent *event)
{
    if (m_action != NULL)
        m_action->action(this, actionType+1, event);
    int comboIndirectionIndex = actionType + 1;
    if (comboIndirectionIndex >= MAX_DISPLAYED_COMBOS)
        return;
    int currentComboIndex = m_statsFormat.m_comboIndirection[comboIndirectionIndex];
    if (currentComboIndex != -1)
        m_comboLines[comboIndirectionIndex].setComboLineInfos(m_dir, comboIndirectionIndex, String(""),
                                                        m_stats.combo_count[currentComboIndex],
                                                        m_opponentStats.combo_count[currentComboIndex],
                                                        m_maxCombo, this);
}

void StatsWidget::startAnimation()
{
    int currentComboIndex = m_statsFormat.m_comboIndirection[0];
    if (currentComboIndex != -1)
        m_comboLines[0].setComboLineInfos(m_dir, 0, String(""),
                                          m_stats.combo_count[currentComboIndex],
                                          m_opponentStats.combo_count[currentComboIndex],
                                          m_maxCombo, this);
    if (m_action != NULL)
        m_action->action(this, 0, NULL);
}

StatsWidget::ComboLine::ComboLine()
  : m_progressBar(this)
{
    //add(&m_comboLabel);
}

void StatsWidget::ComboLine::setComboLineInfos(PuyoStatsDirection dir, int tag, String comboText,
                                                   int numberOfCombos, int vsNumberOfCombos,
                                                   int totalNumOfCombos, Action *progressionCompleteAction)
{
    /*if (numberOfCombos == 0)
        return;*/
    m_dir = dir;

    m_currentValue.setAutoSize(false);
    m_currentValue.setPreferedSize(Vec3(50));

    if (m_dir == LEFT_TO_RIGHT) {
        m_currentValue.setTextAlign(TEXT_LEFT_ALIGN);
        add(&m_progressBar);
        add(&m_currentValue);
    }
    else {
        m_currentValue.setTextAlign(TEXT_RIGHT_ALIGN);
        add(&m_currentValue);
        add(&m_progressBar);
    }
    m_progressBar.setVisible(false);

    m_progressBar.setDirection(dir);
    m_tag = tag;
    m_progressionCompleteAction = progressionCompleteAction;
    m_totalNumOfCombos = totalNumOfCombos;
    float progressBarValue = (float)(1+numberOfCombos) / (float)(1+m_totalNumOfCombos);
    m_comboLabel.setValue(comboText);
    m_progressBar.setVisible(true);
    m_progressBar.setValue(progressBarValue, true);
    m_progressBar.setPositiveAttitude((numberOfCombos >= vsNumberOfCombos) && (numberOfCombos > 0) ? true : false);
    int bof = (int)(progressBarValue * 3.0f);
    if (bof > 3) bof = 3;
    m_progressBar.setColorIndex( bof);
}

void StatsWidget::idle(double currentTime)
{
    if (m_startTime < 0.0) m_startTime = currentTime;
    const double duration = MAX_DISPLAYED_COMBOS * LINE_DURATION + 0.5;
    int points = m_stats.points * sin(1.5708 * (currentTime-m_startTime) / duration);
    if (currentTime-m_startTime > duration) points = m_stats.points;
    m_score.setValue(String() + points);
    m_globalScore.setValue(String() + (m_stats.total_points + points));
}

void StatsWidget::ComboLine::action(Widget *sender, int actionType, GameControlEvent *event)
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

PuyoStatsLegendWidget::PuyoStatsLegendWidget(StatsFormat &statsFormat, StatsWidget &guideWidget, const gameui::FramePicture *framePicture)
  : Frame(framePicture), m_statsImage(), m_statsFormat(statsFormat), m_guideWidget(guideWidget)
{
    setPolicy(USE_MIN_SIZE);
    setInnerMargin(10);
    // Load title image
    IIM_Surface *titleImage = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath(String("gfx/stats_title.png")));
    m_statsImage.setImage(titleImage);
    m_statsImage.setAlign(IMAGE_CENTERED);
    add(&m_statsImage);
    for (int i = 0 ; i < MAX_DISPLAYED_COMBOS ; i++) {
        int numCombo = m_statsFormat.m_comboIndirection[i];
        if (numCombo != -1) {
            String pictureName = theCommander->getDataPathManager().getPath(String("gfx/combo") + ((numCombo+1) == 1 ? 1 : numCombo+1) + String("x.png"));
            IIM_Surface *comboImage = IIM_Load_Absolute_DisplayFormatAlpha(pictureName);
            m_legendImage[i].setImage(comboImage);
            m_legendImage[i].setAlign(IMAGE_CENTERED);
            m_legendCell[i].add(&m_legendImage[i]);
        }
        m_legendSlider[i].setSlideSide(SliderContainer::SLIDE_FROM_BOTTOM);
        add(&m_legendSlider[i]);
    }
    add(&m_bottomSeparator);
}

void PuyoStatsLegendWidget::onWidgetVisibleChanged(bool visible)
{
  // Set the size of the different rows
  for (int i = 0 ; i < this->getNumberOfChilds() - 1 ; i++) {
    Vec3 elementSize = m_guideWidget.getChild(i+1)->getSize();
    getChild(i)->setPreferedSize(Vec3(0, elementSize.y));
  }
  // Special case: the title should have the size of the first 2 items of the guide
  // +10 is to compensate the difference in inner margin
  getChild(0)->setPreferedSize(Vec3(0, getChild(0)->getPreferedSize().y + m_guideWidget.getChild(0)->getSize().y + 10));
}

void PuyoStatsLegendWidget::action(Widget *sender, int actionType, GameControlEvent *event)
{
    if (actionType < MAX_DISPLAYED_COMBOS)
        m_legendSlider[actionType].transitionToContent(&m_legendCell[actionType]);
}

PuyoTwoPlayersStatsWidget::PuyoTwoPlayersStatsWidget(PlayerGameStat &leftPlayerStats, PlayerGameStat &rightPlayerStats,
                                                     bool showLeftGlobalScore, bool showRightGlobalScore,
                                                     const gameui::FramePicture *framePicture)
  : m_statsFormat(leftPlayerStats, rightPlayerStats),
    m_legend(m_statsFormat, m_leftStats, NULL/*framePicture*/),
    m_leftStats(m_statsFormat, leftPlayerStats, rightPlayerStats, NULL, RIGHT_TO_LEFT, showLeftGlobalScore, &m_legend),
    m_rightStats(m_statsFormat, rightPlayerStats, leftPlayerStats, NULL, LEFT_TO_RIGHT, showRightGlobalScore)
{
    m_legendSlider.setPreferedSize(Vec3(194., 416.));
    m_leftSlider.setPreferedSize(Vec3(0., 416.));
    m_rightSlider.setPreferedSize(Vec3(0., 416.));
    HBox *backBox = new HBox();
    VBox *v1 = new VBox();
    Separator *sep1 = new Separator();
    v1->add(&m_leftSlider);
    v1->add(sep1);
    backBox->add(v1);
    VBox *v3 = new VBox();
    v3->setPreferedSize(Vec3(194., 0.));
    backBox->add(v3);
    VBox *v2 = new VBox();
    Separator *sep2 = new Separator();
    v2->add(&m_rightSlider);
    v2->add(sep2);
    backBox->add(v2);
    
    HBox *frontBox = new HBox();
    VBox *f1 = new VBox();
    frontBox->add(f1);
    VBox *f3 = new VBox();
    Separator *sep3 = new Separator();
    f3->setPreferedSize(Vec3(194., 0.));
    f3->add(&m_legendSlider);
    f3->add(sep3);
    frontBox->add(f3);
    VBox *f2 = new VBox();
    frontBox->add(f2);
    
    add(backBox);
    add(frontBox);
    
    m_leftSlider.addListener(*this);
    m_legendSlider.addListener(*this);
    m_rightSlider.addListener(*this);

    widgetAutoReleasePool.add(backBox);
    widgetAutoReleasePool.add(v1);
    widgetAutoReleasePool.add(v2);
    widgetAutoReleasePool.add(v3);
    widgetAutoReleasePool.add(sep1);
    widgetAutoReleasePool.add(sep2);
    widgetAutoReleasePool.add(sep3);
    widgetAutoReleasePool.add(frontBox);
    widgetAutoReleasePool.add(f1);
    widgetAutoReleasePool.add(f2);
    widgetAutoReleasePool.add(f3);
}

void PuyoTwoPlayersStatsWidget::onWidgetVisibleChanged(bool visible)
{
    m_leftSlider.setSlideSide(SliderContainer::SLIDE_FROM_LEFT);
    m_rightSlider.setSlideSide(SliderContainer::SLIDE_FROM_RIGHT);
    m_legendSlider.setSlideSide(SliderContainer::SLIDE_FROM_BOTTOM);
    m_leftSlider.transitionToContent(&m_leftStats);
    m_rightSlider.transitionToContent(&m_rightStats);
    m_legendSlider.transitionToContent(&m_legend);
    m_leftSlider.setBackground(m_leftStats.isWinner() ? ::res->stats_bg_winner : ::res->stats_bg_loser);
    m_rightSlider.setBackground(m_rightStats.isWinner() ? ::res->stats_bg_winner : ::res->stats_bg_loser);
}

void PuyoTwoPlayersStatsWidget::onSlideInside(SliderContainer &slider)
{
    if (&slider == &m_leftSlider)
        m_leftStats.startAnimation();
    if (&slider == &m_rightSlider)
        m_rightStats.startAnimation();
}

