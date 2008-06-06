/*
 *  PuyoStatsWidget.cpp
 *  flobopuyo
 *
 *  Created by Florent Boudet on 26/05/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "PuyoStatsWidget.h"
#include "PuyoCommander.h"

using namespace gameui;

static StatsResources *res;

StatsResources::StatsResources() 
{
    res = this;
    rope_elt = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/progressbar/rope.png"));
    puyo_right = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/progressbar/puyo_right.png"));
    puyo_left = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/progressbar/puyo_left.png"));
    separator = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/separator.png"));
    stats_bg = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/stats-bg.png"));
    // TODO: Combos images
}

StatsResources::~StatsResources()
{
    IIM_Free(rope_elt);
    IIM_Free(puyo_left);
    IIM_Free(puyo_right);
    IIM_Free(stats_bg);
    IIM_Free(separator);
}

ProgressBarWidget::ProgressBarWidget(Action *associatedAction)
  : m_value(0.), m_targetValue(0.), m_progressive(false), m_progressiveDuration(0.6), m_visible(true),
    m_associatedAction(associatedAction)
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
    //if (rope_size < n_rope_elt*2) rope_size = n_rope_elt*2;

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
    dstrect.w = res->puyo_right->w;
    dstrect.h = res->puyo_right->h;
    dstrect.y = bpos.y;
    if (m_dir == LEFT_TO_RIGHT) {
        dstrect.x = bpos.x + rope_size + IMG_RING_WIDTH - 4;
        SDL_BlitSurface(res->puyo_right->surf, NULL, screen, &dstrect);
    }
    else {
        dstrect.x = bpos.x + bsize.x - rope_size - IMG_RING_WIDTH - IMG_PUYO_WIDTH + 4;
        SDL_BlitSurface(res->puyo_left->surf, NULL, screen, &dstrect);
    }
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

void ProgressBarWidget::setDirection(PuyoStatsDirection dir)
{
   m_dir = dir; 
}

void ProgressBarWidget::setVisible(bool visible)
{
    m_visible = visible;
    requestDraw();
}

PuyoStatsFormat::PuyoStatsFormat(PlayerGameStat &playerAStats, PlayerGameStat &playerBStats)
{
    int j = 0;
    for (int i = 0 ; i < MAX_DISPLAYED_COMBOS+1 ; i++)
        m_comboIndirection[i] = -1;
    for (int i = 0 ; (i < 24) && (j < MAX_DISPLAYED_COMBOS+1) ; i++) {
        if ((playerAStats.combo_count[i] >= 0) || (playerBStats.combo_count[i] >= 0))
            m_comboIndirection[j++] = i;
    }
}


PuyoStatsWidget::PuyoStatsWidget(PuyoStatsFormat &statsFormat,
                                 PlayerGameStat &stats, PlayerGameStat &opponentStats,
                                 const gameui::FramePicture *framePicture, PuyoStatsDirection dir,
                                 gameui::Action *action)
  : m_dir(dir), m_action(action), m_statsFormat(statsFormat),
    m_stats(stats), m_opponentStats(opponentStats),
    m_statTitle("Combos"), m_maxCombo(0)
{
    setPolicy(USE_MIN_SIZE);
    setInnerMargin(20);

    // Create Layout
    Text *txt = new Text(stats.is_winner?"WINNER":"LOOSER");
    /*txt->setPreferedSize(Vec3(128, 32));*/
    Image *img1 = new Image(res->separator);
    img1->setPreferedSize(Vec3(128, 32));
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

    // Prepare un-allocation
    widgetAutoReleasePool.add(sep);
    widgetAutoReleasePool.add(img1);
    widgetAutoReleasePool.add(img2);
    widgetAutoReleasePool.add(txt);

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

void PuyoStatsWidget::action(Widget *sender, int actionType, GameControlEvent *event)
{
    if (m_action != NULL)
        m_action->action(this, actionType+1, event);
    int comboIndirectionIndex = actionType + 1;
    if (comboIndirectionIndex >= MAX_DISPLAYED_COMBOS)
        return;
    int currentComboIndex = m_statsFormat.m_comboIndirection[comboIndirectionIndex];
    if (currentComboIndex != -1)
        m_comboLines[comboIndirectionIndex].setComboLineInfos(m_dir, comboIndirectionIndex, String(""), m_stats.combo_count[currentComboIndex], m_maxCombo, this);
}

void PuyoStatsWidget::startAnimation()
{
    int currentComboIndex = m_statsFormat.m_comboIndirection[0];
    if (currentComboIndex != -1)
        m_comboLines[0].setComboLineInfos(m_dir, 0, String(""), m_stats.combo_count[currentComboIndex], m_maxCombo, this);
    if (m_action != NULL)
        m_action->action(this, 0, NULL);
}

PuyoStatsWidget::ComboLine::ComboLine()
  : m_progressBar(this)
{
    //add(&m_comboLabel);
}

void PuyoStatsWidget::ComboLine::setComboLineInfos(PuyoStatsDirection dir, int tag, String comboText,
                                                   int numberOfCombos, int totalNumOfCombos, Action *progressionCompleteAction)
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

PuyoStatsLegendWidget::PuyoStatsLegendWidget(PuyoStatsFormat &statsFormat, PuyoStatsWidget &guideWidget, const gameui::FramePicture *framePicture)
  : Frame(framePicture), m_statsFormat(statsFormat), m_guideWidget(guideWidget)
{
    //setPolicy(USE_MIN_SIZE);
    setInnerMargin(20);
    for (int i = 0 ; i < MAX_DISPLAYED_COMBOS ; i++) {
        int numCombo = m_statsFormat.m_comboIndirection[i];
        if (numCombo != -1) {
	  String pictureName = theCommander->getDataPathManager().getPath(String("gfx/combo") + ((numCombo+1) == 1 ? 2 : numCombo+1) + String("x.png"));
      IIM_Surface *comboImage = IIM_Load_Absolute_DisplayFormatAlpha(pictureName);
	  m_legendImage[i].setImage(comboImage);
	}
        m_legendSlider[i].setSlideSide(SliderContainer::SLIDE_FROM_BOTTOM);
        add(&m_legendSlider[i]);
    }
    //if (m_statsFormat.m_comboIndirection[MAX_DISPLAYED_COMBOS] != -1)
    //    m_legendText[MAX_DISPLAYED_COMBOS-1].setValue("Countless");
}

void PuyoStatsLegendWidget::onWidgetVisibleChanged(bool visible)
{
  // Set the size of the different rows
  for (int i = 0 ; i < this->getNumberOfChilds() ; i++)
    getChild(i)->setPreferedSize(Vec3(0, m_guideWidget.getChild(i)->getSize().y));
}

void PuyoStatsLegendWidget::action(Widget *sender, int actionType, GameControlEvent *event)
{
    if (actionType < MAX_DISPLAYED_COMBOS)
        m_legendSlider[actionType].transitionToContent(&m_legendImage[actionType]);
}

PuyoTwoPlayersStatsWidget::PuyoTwoPlayersStatsWidget(PlayerGameStat &leftPlayerStats, PlayerGameStat &rightPlayerStats, const gameui::FramePicture *framePicture)
  : m_statsFormat(leftPlayerStats, rightPlayerStats),
    m_legend(m_statsFormat, m_leftStats, NULL/*framePicture*/),
    m_leftStats(m_statsFormat, leftPlayerStats, rightPlayerStats, NULL, RIGHT_TO_LEFT, &m_legend),
    m_rightStats(m_statsFormat, rightPlayerStats, leftPlayerStats, NULL, LEFT_TO_RIGHT)
{
    m_legendSlider.setPreferedSize(Vec3(190., 416.));
    m_leftSlider.setPreferedSize(Vec3(0., 416.));
    m_rightSlider.setPreferedSize(Vec3(0., 416.));
    VBox *v1 = new VBox();
    Separator *sep1 = new Separator();
    v1->add(&m_leftSlider);
    v1->add(sep1);
    add(v1);
    VBox *v3 = new VBox();
    Separator *sep3 = new Separator();
    v3->add(&m_legendSlider);
    v3->add(sep3);
    v3->setPreferedSize(Vec3(190., 0.));
    add(v3);
    VBox *v2 = new VBox();
    Separator *sep2 = new Separator();
    v2->add(&m_rightSlider);
    v2->add(sep2);
    add(v2);
    m_leftSlider.addListener(*this);
    m_legendSlider.addListener(*this);
    m_rightSlider.addListener(*this);

    widgetAutoReleasePool.add(v1);
    widgetAutoReleasePool.add(v2);
    widgetAutoReleasePool.add(v3);
    widgetAutoReleasePool.add(sep1);
    widgetAutoReleasePool.add(sep2);
    widgetAutoReleasePool.add(sep3);
}

void PuyoTwoPlayersStatsWidget::onWidgetVisibleChanged(bool visible)
{
    m_leftSlider.setSlideSide(SliderContainer::SLIDE_FROM_LEFT);
    m_rightSlider.setSlideSide(SliderContainer::SLIDE_FROM_RIGHT);
    m_legendSlider.setSlideSide(SliderContainer::SLIDE_FROM_TOP);
    m_leftSlider.transitionToContent(&m_leftStats);
    m_rightSlider.transitionToContent(&m_rightStats);
    m_legendSlider.transitionToContent(&m_legend);
    m_leftSlider.setBackground(::res->stats_bg);
    m_rightSlider.setBackground(::res->stats_bg);
}

void PuyoTwoPlayersStatsWidget::onSlideInside(SliderContainer &slider)
{
    if (&slider == &m_leftSlider)
        m_leftStats.startAnimation();
    if (&slider == &m_rightSlider)
        m_rightStats.startAnimation();
}

