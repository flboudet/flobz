/* FloboPuyo
 * Copyright (C) 2004
 *   Florent Boudet        <flobo@ios-software.com>,
 *   Jean-Christophe Hoelt <jeko@ios-software.com>,
 *   Guillaume Borios      <gyom@ios-software.com>
 *
 * iOS Software <http://www.ios-software.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *
 */

#include "PuyoStarter.h"
#include "PuyoView.h"
#include "PuyoNetworkView.h"
#include "PuyoNetworkGame.h"

#include "SDL_Painter.h"
#include "IosImgProcess.h"

using namespace ios_fc;

extern SDL_Painter painter;

IIM_Surface *grid;
IIM_Surface *perso[2];
int currentPerso;
IIM_Surface *live[4];
IIM_Surface *speedImg;
IIM_Surface *speedBlackImg;
IIM_Surface *gameScreen;
IIM_Surface *shrinkingPuyo[5][5];
IIM_Surface *explodingPuyo[5][5];
extern IIM_Surface *bigNeutral;
IIM_Surface *puyoEye[3];
extern IIM_Surface *puyoEyes;
IIM_Surface *puyoEyesSwirl[4];
extern IIM_Surface *puyoFaces[5][16];

int gameLevel;
int GAME_ACCEL = 1250;
static const int NB_PERSO_STATE = 2;

const char *p1name;
const char *p2name;

extern Menu *menu_pause;
static char *BACKGROUND[NB_MUSIC_THEME] = { "Background.jpg", "BackgroundDark.jpg" };
extern IIM_Surface *background, *fallingBlue, *fallingRed, *fallingGreen, *fallingViolet, *fallingYellow, *neutral;

PuyoPureStarter::PuyoPureStarter(PuyoCommander *commander) : commander(commander)
{
    paused = false;
    stopRendering = false;
    gameAborted = false;
}

PuyoPureStarter::~PuyoPureStarter()
{
}

void PuyoPureStarter::handleEvent(SDL_Event &event)
{
    /* Check for usual events */
    GameControlEvent controlEvent;
    getControlEvent(event, &controlEvent);
    switch (controlEvent.cursorEvent) {
        case GameControlEvent::kStart:
            startPressed();
            break;
        case GameControlEvent::kBack:
            backPressed();
            break;
        default:
            break;
    }
    /* Check for SDL special events */
    if(event.type == SDL_QUIT) {
        if (menu_active_is(commander->gameOverMenu, "YES"))
            menu_next_item(commander->gameOverMenu);
        //quit = 1;
        exit(0);
    }
}

void PuyoPureStarter::startPressed()
{
    //if (gameover) {
        //if (menu_active_is(commander->gameOverMenu, "NO"))
        //    menu_next_item(commander->gameOverMenu);
        //quit = true;
    //}
}

void PuyoPureStarter::backPressed()
{
/*
    if (!gameover) {
        if (!paused) {
            menu_show(menu_pause);
            paused = true;
            stopRender();
        }
        else {
            paused = false;
            restartRender();
            menu_hide(menu_pause);
        }
    }
    else {
        if (menu_active_is(commander->gameOverMenu, "NO"))
            menu_next_item(commander->gameOverMenu);
        quit = 1;
    }
    */
}
    
void PuyoPureStarter::stopRender()
{
    this->stopRendering = true;
    iim_surface_convert_to_gray(painter.gameScreen);
}


void PuyoPureStarter::restartRender()
{
    this->stopRendering = false;
    painter.redrawAll();
}






static void loadShrinkXplode2(int i, float dec)
{
    for (int j=1;j<=4;++j)
    {
        shrinkingPuyo[j-1][i] = iim_surface_shift_hue(shrinkingPuyo[j-1][3],dec);
        explodingPuyo[j-1][i] = iim_surface_shift_hue(explodingPuyo[j-1][3],dec);
    }
}

static void loadShrinkXplode(void)
{
    for (int j=1;j<=4;++j)
    {
        char f[20];
        sprintf(f,"Shrink%d.png", j);
        shrinkingPuyo[j-1][3] = IIM_Load_DisplayFormatAlpha(f);
        sprintf(f,"Explode%d.png", j);
        explodingPuyo[j-1][3] = IIM_Load_DisplayFormatAlpha(f);
    }
    
    loadShrinkXplode2(0,-65.0f);
    loadShrinkXplode2(1,100.0f);
    loadShrinkXplode2(2,-150.0f);
    loadShrinkXplode2(4,140.0f);
}


class PuyoLocalGameFactory : public PuyoGameFactory {
public:
    PuyoLocalGameFactory(PuyoRandomSystem *attachedRandom): attachedRandom(attachedRandom) {}
    PuyoGame *createPuyoGame(PuyoFactory *attachedPuyoFactory) {
        return new PuyoLocalGame(attachedRandom, attachedPuyoFactory);
    }
private:
    PuyoRandomSystem *attachedRandom;
};

class PuyoNetworkGameFactory : public PuyoGameFactory {
public:
    PuyoNetworkGameFactory(PuyoRandomSystem *attachedRandom, MessageBox &msgBox): attachedRandom(attachedRandom), msgBox(msgBox) {}
    PuyoGame *createPuyoGame(PuyoFactory *attachedPuyoFactory) {
        return new PuyoNetworkGame(attachedPuyoFactory, msgBox);
    }
private:
    PuyoRandomSystem *attachedRandom;
    MessageBox &msgBox;
};

void PuyoStarter::draw()
{
    if (stopRendering) {
        SDL_BlitSurface(painter.gameScreen->surf,NULL,display,NULL);
    }
    else {
        SDL_Rect drect;
        
        areaA->render();
        areaB->render();
        
        drect.x = 21;
        drect.y = -1;
        drect.w = grid->w;
        drect.h = grid->h;
        painter.requestDraw(grid, &drect);
        drect.x = 407;
        drect.y = -1;
        drect.w = grid->w;
        drect.h = grid->h;
        painter.requestDraw(grid, &drect);
        
        areaA->renderNeutral();
        areaB->renderNeutral();
        if ((randomPlayer)&&(currentPerso>=0))
        {
            drect.x = 320 - perso[currentPerso]->w/2;
            drect.y = 280 - perso[currentPerso]->h/2;
            drect.w = perso[currentPerso]->w;
            drect.h = perso[currentPerso]->h;
            painter.requestDraw(perso[currentPerso], &drect);
        }
        
        if ((randomPlayer)&&(lives>=0)&&(lives<=3))
        {
            drect.x = painter.gameScreen->w / 2 - live[lives]->w / 2;
            drect.y = 436;
            drect.w = live[lives]->w;
            drect.h = live[lives]->h;
            painter.requestDraw(live[lives], &drect);
        }
        
        painter.draw(painter.gameScreen->surf);
    }
    char text[256];
    if (!randomPlayer)
    {
        sprintf(text, "Win %d", score1);
        SoFont_CenteredString_XY (commander->smallFont, display,
                                  50, 460,   text, NULL);
        sprintf(text, "Win %d", score2);
        SoFont_CenteredString_XY (commander->smallFont, display,
                                  590, 460, text, NULL);
    }
    
    SoFont *font = (stopRendering?commander->darkFont:commander->menuFont);
    SoFont_CenteredString_XY (font, display, 510, 460,   p1name, NULL);
    SoFont_CenteredString_XY (font, display, 130, 460,   p2name, NULL);
    
    int gameSpeedCpy = gameSpeed;
    if (gameSpeed == 1) gameSpeedCpy = 0;
    
    SDL_Rect speedRect;
    speedRect.x = 0;
    speedRect.w = speedImg->w;
    speedRect.h = (20 - gameSpeedCpy) * 6;
    speedRect.y = speedImg->h - speedRect.h;
    
    SDL_Rect drect;
    drect.x = 320 - speedRect.w / 2;
    drect.y = 170 - speedRect.h;
    drect.w = speedRect.w;
    drect.h = speedRect.h;
    
    SDL_Rect speedBlackRect = speedRect;
    SDL_Rect drectBlack     = drect;
    
    speedBlackRect.h = speedImg->h - speedRect.h;
    speedBlackRect.y = 0;
    drectBlack.y = 50;
    drectBlack.h = speedBlackRect.h;
    
    SDL_BlitSurface(speedBlackImg->surf,&speedBlackRect,display,&drectBlack);
    if (stopRendering)
        SDL_BlitSurface(speedBlackImg->surf,&speedRect,display,&drect);
    else
        SDL_BlitSurface(speedImg->surf,&speedRect,display,&drect);
    
    SoFont *fontBl = NULL;
    if ((blinkingPointsA % 2) == 0)
        fontBl = commander->smallFont;
    else
        fontBl = commander->menuFont;
    
    sprintf(text, "<< %d", attachedGameA->getPoints());
    SoFont_CenteredString_XY (fontBl, display,
                              300, 380,   text, NULL);
    
    if ((blinkingPointsB % 2) == 0)
        fontBl = commander->smallFont;
    else
        fontBl = commander->menuFont;
    
    sprintf(text, "%d >>", attachedGameB->getPoints());
    SoFont_CenteredString_XY (fontBl, display,
                              340, 395, text, NULL);
}


PuyoStarter::PuyoStarter(PuyoCommander *commander, bool aiLeft, int aiLevel, IA_Type aiType, int theme, MessageBox *mbox):PuyoPureStarter(commander), mbox(mbox)
{
    tickCounts = 0;
    
    blinkingPointsA = 0;
    blinkingPointsB = 0;
    savePointsA = 0;
    savePointsB = 0;
    
    background    = IIM_Load_DisplayFormat(BACKGROUND[theme]);
    
    painter.backGround = background;
    if (painter.gameScreen == NULL)
    {
        SDL_PixelFormat *fmt = background->surf->format;
        SDL_Surface *tmp = SDL_CreateRGBSurface(background->surf->flags,
                                                background->w, background->h, 32,
                                                fmt->Rmask, fmt->Gmask,
                                                fmt->Bmask, fmt->Amask);
        gameScreen = painter.gameScreen = IIM_RegisterImg(SDL_DisplayFormat(tmp), false);
        SDL_FreeSurface(tmp);
    }
    painter.redrawAll(painter.gameScreen->surf);
    
    static bool firstTime = true;
    if (firstTime) {

        IIM_Surface *fallingCross, *fallingCircle, *fallingVCircle, *fallingHCircle,*fallingSquare;
        fallingCircle  = IIM_Load_DisplayFormatAlpha("v0.png");
        fallingSquare  = IIM_Load_DisplayFormatAlpha("c0.png");
        fallingCross   = IIM_Load_DisplayFormatAlpha("g0.png");
        fallingHCircle = IIM_Load_DisplayFormatAlpha("h0.png");
        fallingVCircle = IIM_Load_DisplayFormatAlpha("p0.png");
        fallingViolet = iim_surface_shift_hue(fallingCircle, 0.0f);
        fallingRed    = iim_surface_shift_hue(fallingCross, 100.0f);
        fallingBlue   = iim_surface_shift_hue(fallingVCircle, -65.0f);
        fallingGreen  = iim_surface_shift_hue(fallingHCircle, -150.0f);
        fallingYellow = iim_surface_shift_hue(fallingSquare, 140.0f);
/*        fallingViolet = iim_surface_shift_hue(fallingSquare, 0.0f);
        fallingRed    = iim_surface_shift_hue(fallingSquare, 100.0f);
        fallingBlue   = iim_surface_shift_hue(fallingSquare, -65.0f);
        fallingGreen  = iim_surface_shift_hue(fallingSquare, -150.0f);
        fallingYellow = iim_surface_shift_hue(fallingSquare, 140.0f);*/
        IIM_Free(fallingCircle);
        IIM_Free(fallingSquare);
        IIM_Free(fallingCross);
        IIM_Free(fallingHCircle);
        IIM_Free(fallingVCircle);

        neutral       = IIM_Load_DisplayFormatAlpha("Neutral.png");
        bigNeutral    = IIM_Load_DisplayFormatAlpha("BigNeutral.png");
        speedImg      = IIM_Load_DisplayFormatAlpha("speed.png");
        speedBlackImg = IIM_Load_DisplayFormatAlpha("speed_black.png");

        loadShrinkXplode();
        
        puyoEye[0] = IIM_Load_DisplayFormatAlpha("eye0.png");
        puyoEye[1] = IIM_Load_DisplayFormatAlpha("eye1.png");
        puyoEye[2] = IIM_Load_DisplayFormatAlpha("eye2.png");
        puyoEyes = puyoEye[0];
        puyoEyesSwirl[0] = IIM_Load_DisplayFormatAlpha("twirleye0.png");
        puyoEyesSwirl[1] = IIM_Load_DisplayFormatAlpha("twirleye1.png");
        puyoEyesSwirl[2] = IIM_Load_DisplayFormatAlpha("twirleye2.png");
        puyoEyesSwirl[3] = IIM_Load_DisplayFormatAlpha("twirleye3.png");
        
        puyoFaces[0][0] = IIM_Load_DisplayFormatAlpha("v0.png");
        puyoFaces[0][1] = IIM_Load_DisplayFormatAlpha("v1a.png");
        puyoFaces[0][2] = IIM_Load_DisplayFormatAlpha("v1b.png");
        puyoFaces[0][3] = IIM_Load_DisplayFormatAlpha("v1c.png");
        puyoFaces[0][4] = IIM_Load_DisplayFormatAlpha("v1d.png");
        puyoFaces[0][5] = IIM_Load_DisplayFormatAlpha("v2ab.png");
        puyoFaces[0][6] = IIM_Load_DisplayFormatAlpha("v2ac.png");
        puyoFaces[0][7] = IIM_Load_DisplayFormatAlpha("v2ad.png");
        puyoFaces[0][8] = IIM_Load_DisplayFormatAlpha("v2bc.png");
        puyoFaces[0][9] = IIM_Load_DisplayFormatAlpha("v2bd.png");
        puyoFaces[0][10] = IIM_Load_DisplayFormatAlpha("v2cd.png");
        puyoFaces[0][11] = IIM_Load_DisplayFormatAlpha("v3abc.png");
        puyoFaces[0][12] = IIM_Load_DisplayFormatAlpha("v3abd.png");
        puyoFaces[0][13] = IIM_Load_DisplayFormatAlpha("v3acd.png");
        puyoFaces[0][14] = IIM_Load_DisplayFormatAlpha("v3bcd.png");
        puyoFaces[0][15] = IIM_Load_DisplayFormatAlpha("v4abcd.png");
        
        puyoFaces[4][0] = IIM_Load_DisplayFormatAlpha("c0.png");
        puyoFaces[4][1] = IIM_Load_DisplayFormatAlpha("c1a.png");
        puyoFaces[4][2] = IIM_Load_DisplayFormatAlpha("c1b.png");
        puyoFaces[4][3] = IIM_Load_DisplayFormatAlpha("c1c.png");
        puyoFaces[4][4] = IIM_Load_DisplayFormatAlpha("c1d.png");
        puyoFaces[4][5] = IIM_Load_DisplayFormatAlpha("c2ab.png");
        puyoFaces[4][6] = IIM_Load_DisplayFormatAlpha("c2ac.png");
        puyoFaces[4][7] = IIM_Load_DisplayFormatAlpha("c2ad.png");
        puyoFaces[4][8] = IIM_Load_DisplayFormatAlpha("c2bc.png");
        puyoFaces[4][9] = IIM_Load_DisplayFormatAlpha("c2bd.png");
        puyoFaces[4][10] = IIM_Load_DisplayFormatAlpha("c2cd.png");
        puyoFaces[4][11] = IIM_Load_DisplayFormatAlpha("c3abc.png");
        puyoFaces[4][12] = IIM_Load_DisplayFormatAlpha("c3abd.png");
        puyoFaces[4][13] = IIM_Load_DisplayFormatAlpha("c3acd.png");
        puyoFaces[4][14] = IIM_Load_DisplayFormatAlpha("c3bcd.png");
        puyoFaces[4][15] = IIM_Load_DisplayFormatAlpha("c4abcd.png");
        
        puyoFaces[2][0] = IIM_Load_DisplayFormatAlpha("p0.png");
        puyoFaces[2][1] = IIM_Load_DisplayFormatAlpha("p1a.png");
        puyoFaces[2][2] = IIM_Load_DisplayFormatAlpha("p1b.png");
        puyoFaces[2][3] = IIM_Load_DisplayFormatAlpha("p1c.png");
        puyoFaces[2][4] = IIM_Load_DisplayFormatAlpha("p1d.png");
        puyoFaces[2][5] = IIM_Load_DisplayFormatAlpha("p2ab.png");
        puyoFaces[2][6] = IIM_Load_DisplayFormatAlpha("p2ac.png");
        puyoFaces[2][7] = IIM_Load_DisplayFormatAlpha("p2ad.png");
        puyoFaces[2][8] = IIM_Load_DisplayFormatAlpha("p2bc.png");
        puyoFaces[2][9] = IIM_Load_DisplayFormatAlpha("p2bd.png");
        puyoFaces[2][10] = IIM_Load_DisplayFormatAlpha("p2cd.png");
        puyoFaces[2][11] = IIM_Load_DisplayFormatAlpha("p3abc.png");
        puyoFaces[2][12] = IIM_Load_DisplayFormatAlpha("p3abd.png");
        puyoFaces[2][13] = IIM_Load_DisplayFormatAlpha("p3acd.png");
        puyoFaces[2][14] = IIM_Load_DisplayFormatAlpha("p3bcd.png");
        puyoFaces[2][15] = IIM_Load_DisplayFormatAlpha("p4abcd.png");
        
        puyoFaces[3][0] = IIM_Load_DisplayFormatAlpha("h0.png");
        puyoFaces[3][1] = IIM_Load_DisplayFormatAlpha("h1a.png");
        puyoFaces[3][2] = IIM_Load_DisplayFormatAlpha("h1b.png");
        puyoFaces[3][3] = IIM_Load_DisplayFormatAlpha("h1c.png");
        puyoFaces[3][4] = IIM_Load_DisplayFormatAlpha("h1d.png");
        puyoFaces[3][5] = IIM_Load_DisplayFormatAlpha("h2ab.png");
        puyoFaces[3][6] = IIM_Load_DisplayFormatAlpha("h2ac.png");
        puyoFaces[3][7] = IIM_Load_DisplayFormatAlpha("h2ad.png");
        puyoFaces[3][8] = IIM_Load_DisplayFormatAlpha("h2bc.png");
        puyoFaces[3][9] = IIM_Load_DisplayFormatAlpha("h2bd.png");
        puyoFaces[3][10] = IIM_Load_DisplayFormatAlpha("h2cd.png");
        puyoFaces[3][11] = IIM_Load_DisplayFormatAlpha("h3abc.png");
        puyoFaces[3][12] = IIM_Load_DisplayFormatAlpha("h3abd.png");
        puyoFaces[3][13] = IIM_Load_DisplayFormatAlpha("h3acd.png");
        puyoFaces[3][14] = IIM_Load_DisplayFormatAlpha("h3bcd.png");
        puyoFaces[3][15] = IIM_Load_DisplayFormatAlpha("h4abcd.png");
        
        puyoFaces[1][0] = IIM_Load_DisplayFormatAlpha("g0.png");
        puyoFaces[1][1] = IIM_Load_DisplayFormatAlpha("g1a.png");
        puyoFaces[1][2] = IIM_Load_DisplayFormatAlpha("g1b.png");
        puyoFaces[1][3] = IIM_Load_DisplayFormatAlpha("g1c.png");
        puyoFaces[1][4] = IIM_Load_DisplayFormatAlpha("g1d.png");
        puyoFaces[1][5] = IIM_Load_DisplayFormatAlpha("g2ab.png");
        puyoFaces[1][6] = IIM_Load_DisplayFormatAlpha("g2ac.png");
        puyoFaces[1][7] = IIM_Load_DisplayFormatAlpha("g2ad.png");
        puyoFaces[1][8] = IIM_Load_DisplayFormatAlpha("g2bc.png");
        puyoFaces[1][9] = IIM_Load_DisplayFormatAlpha("g2bd.png");
        puyoFaces[1][10] = IIM_Load_DisplayFormatAlpha("g2cd.png");
        puyoFaces[1][11] = IIM_Load_DisplayFormatAlpha("g3abc.png");
        puyoFaces[1][12] = IIM_Load_DisplayFormatAlpha("g3abd.png");
        puyoFaces[1][13] = IIM_Load_DisplayFormatAlpha("g3acd.png");
        puyoFaces[1][14] = IIM_Load_DisplayFormatAlpha("g3bcd.png");
        puyoFaces[1][15] = IIM_Load_DisplayFormatAlpha("g4abcd.png");


        live[0] = IIM_Load_DisplayFormatAlpha("0live.png");
        live[1] = IIM_Load_DisplayFormatAlpha("1live.png");
        live[2] = IIM_Load_DisplayFormatAlpha("2live.png");
        live[3] = IIM_Load_DisplayFormatAlpha("3live.png");
        
        for (int i = 0 ; i < 16 ; i++) {
            IIM_Surface *tmp = puyoFaces[1][i];
            puyoFaces[1][i] = iim_surface_shift_hue(tmp, 100);
            IIM_Free(tmp);
        }
        for (int i = 0 ; i < 16 ; i++) {
            IIM_Surface *tmp = puyoFaces[2][i];
            puyoFaces[2][i] = iim_surface_shift_hue(tmp, -65);
            IIM_Free(tmp);
        }
        for (int i = 0 ; i < 16 ; i++) {
            IIM_Surface *tmp = puyoFaces[3][i];
            puyoFaces[3][i] = iim_surface_shift_hue(tmp, -150);
            IIM_Free (tmp);
        }
        for (int i = 0 ; i < 16 ; i++) {
            IIM_Surface *tmp = puyoFaces[4][i];
            puyoFaces[4][i] = iim_surface_shift_hue(tmp, 140);
            IIM_Free (tmp);
        }
        
        grid          = IIM_Load_DisplayFormatAlpha("grid.png");
        firstTime = false;
    }
    
    if (fallingBlue == NULL)
    {
        fprintf(stderr, "IMG_Load error:%s\n", SDL_GetError());
        exit(-1);
    }
    if (mbox == NULL) {
        attachedGameFactory = new PuyoLocalGameFactory(&attachedRandom);
        areaA = new PuyoView(attachedGameFactory, 1 + CSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + FSIZE, BSIZE+ESIZE);
        areaB = new PuyoView(attachedGameFactory, 1 + CSIZE + PUYODIMX*TSIZE + DSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + DSIZE - FSIZE - TSIZE, BSIZE+ESIZE);
    }
    else {
        attachedGameFactory = new PuyoLocalGameFactory(&attachedRandom);
        attachedNetworkGameFactory = new PuyoNetworkGameFactory(&attachedRandom, *mbox);
        areaA = new PuyoView(attachedNetworkGameFactory, 1 + CSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + FSIZE, BSIZE+ESIZE);
        areaB = new PuyoNetworkView(attachedGameFactory, 1 + CSIZE + PUYODIMX*TSIZE + DSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + DSIZE - FSIZE - TSIZE, BSIZE+ESIZE, mbox);
        //SDL_Delay(10000);
    }
    
    
    attachedGameA = areaA->getAttachedGame();
    attachedGameB = areaB->getAttachedGame();
    
    if (aiLeft) {
        randomPlayer = new PuyoIA(aiType, aiLevel, areaA);
        perso[0] = IIM_Load_DisplayFormatAlpha("perso1_1.png");
        perso[1] = IIM_Load_DisplayFormatAlpha("perso1_2.png");
    }
    else {
        randomPlayer = 0;
        perso[0] = NULL;
    }
    
    areaA->setEnemyGame(attachedGameB);
    areaB->setEnemyGame(attachedGameA);
}

PuyoStarter::~PuyoStarter()
{
    delete areaA;
    delete areaB;
    delete attachedGameA;
    delete attachedGameB;
    delete attachedGameFactory;
    //	SDL_FreeSurface(fallingBlue);
}




#define FPKEY_REPEAT        7
#define FPKEY_DELAY         5

#define FPKEY_keyNumber     10

#define FPKEY_P1_Down       0
#define FPKEY_P1_Left       1
#define FPKEY_P1_Right      2
#define FPKEY_P1_TurnLeft   3
#define FPKEY_P1_TurnRight  4
#define FPKEY_P2_Down       5
#define FPKEY_P2_Left       6
#define FPKEY_P2_Right      7
#define FPKEY_P2_TurnLeft   8
#define FPKEY_P2_TurnRight  9

#define repeatCondition(A) keysDown[A]++; if (((keysDown[A]-FPKEY_DELAY)>0) && ((keysDown[A]-FPKEY_DELAY)%FPKEY_REPEAT == 0)) 

void PuyoStarter::run(int _score1, int _score2, int lives, int point1, int point2)
{
    this->lives = lives;
    this->score1 = _score1;
    this->score2 = _score2;
    SDL_Rect drect;
    SDL_Event event;
    int quit = 0;
    SDL_EnableUNICODE(1);
    int keysDown[FPKEY_keyNumber] = {0,0,0,0,0,0,0,0,0,0};
    
    gameSpeed = 20;
    attachedGameB->points = point1;
    attachedGameA->points = point2;
    
    if (!randomPlayer) {
        int sc1=score1,sc2=score2;
        while ((sc1>0)&&(sc2>0)) { sc1--; sc2--; }
        attachedGameA->increaseNeutralPuyos(sc1 * PUYODIMX);
        attachedGameB->increaseNeutralPuyos(sc2 * PUYODIMX);
        attachedGameA->dropNeutrals();
        attachedGameB->dropNeutrals();
        attachedGameA->cycle();
        attachedGameB->cycle();
    }
    
    while (!quit) {
        bool left_danger = (attachedGameA->getMaxColumnHeight() > PUYODIMY - 5);
        bool right_danger = (attachedGameB->getMaxColumnHeight() > PUYODIMY - 5);
        bool danger = left_danger || right_danger;
        bool gameover = (!attachedGameA->isGameRunning() || !attachedGameB->isGameRunning());
        
        /*if (paused)
            audio_music_start(0);
        else */if (gameover)
            audio_music_start(3);
        else if (danger)
            audio_music_start(2);
        else
            audio_music_start(1);
        
        if (left_danger)
            currentPerso = 0;
        else
            currentPerso = 1;
        
        while (SDL_PollEvent(&event) == 1)
        {
            if (attachedGameA->isGameRunning() &&
                attachedGameB->isGameRunning()) {
                if (!paused) {
                    
                    /* Check for usual events */
                    GameControlEvent controlEvent;
                    getControlEvent(event, &controlEvent);
                    
                    if (controlEvent.isUp)
                    {
                        switch (controlEvent.gameEvent) {
                            case GameControlEvent::kPlayer1Down:
                                if (randomPlayer == 0) keysDown[FPKEY_P1_Down] = 0;
                                break;
                            case GameControlEvent::kPlayer1Left:
                                if (randomPlayer == 0) keysDown[FPKEY_P1_Left] = 0;
                                break;
                            case GameControlEvent::kPlayer1Right:
                                if (randomPlayer == 0) keysDown[FPKEY_P1_Right] = 0;
                                break;
                            case GameControlEvent::kPlayer1TurnLeft:
                                if (randomPlayer == 0) keysDown[FPKEY_P1_TurnLeft] = 0;
                                break;
                            case GameControlEvent::kPlayer1TurnRight:
                                if (randomPlayer == 0) keysDown[FPKEY_P1_TurnRight] = 0;
                                break;
                                
                            case GameControlEvent::kPlayer2Down:
                                keysDown[FPKEY_P2_Down] = 0;
                                break;
                            case GameControlEvent::kPlayer2Left:
                                keysDown[FPKEY_P2_Left] = 0;
                                break;
                            case GameControlEvent::kPlayer2Right:
                                keysDown[FPKEY_P2_Right] = 0;
                                break;
                            case GameControlEvent::kPlayer2TurnLeft:
                                keysDown[FPKEY_P2_TurnLeft] = 0;
                                break;
                            case GameControlEvent::kPlayer2TurnRight:
                                keysDown[FPKEY_P2_TurnRight] = 0;
                                break;
                        }
                    }
                    else {
                        switch (controlEvent.gameEvent) {
                            case GameControlEvent::kPlayer1Left:
                                if (randomPlayer == 0) {
                                    areaA->moveLeft();
                                    if (attachedGameA->isEndOfCycle()) keysDown[FPKEY_P1_Left] = 0;
                                    else keysDown[FPKEY_P1_Left]++; 
                                }
                                break;
                            case GameControlEvent::kPlayer1Right:
                                if (randomPlayer == 0) {
                                    areaA->moveRight();
                                    if (attachedGameA->isEndOfCycle()) keysDown[FPKEY_P1_Right] = 0;
                                    else keysDown[FPKEY_P1_Right]++; 
                                }
                                break;
                            case GameControlEvent::kPlayer1TurnLeft:
                                if (randomPlayer == 0) {
                                    areaA->rotateLeft();
                                    if (attachedGameA->isEndOfCycle()) keysDown[FPKEY_P1_TurnLeft] = 0;
                                    else keysDown[FPKEY_P1_TurnLeft]++; 
                                }
                                break;
                            case GameControlEvent::kPlayer1TurnRight:
                                if (randomPlayer == 0) {
                                    areaA->rotateRight();
                                    if (attachedGameA->isEndOfCycle()) keysDown[FPKEY_P1_TurnRight] = 0;
                                    else keysDown[FPKEY_P1_TurnRight]++;
                                }
                                break;
                            case GameControlEvent::kPlayer1Down:
                                if (randomPlayer == 0) {
                                    //attachedGameA->cycle(); desact flobo
                                    if (attachedGameA->isEndOfCycle()) keysDown[FPKEY_P1_Down] = 0;
                                    else keysDown[FPKEY_P1_Down]++;
                                }
                                break;
                                
                            case GameControlEvent::kPlayer2Left:
                                areaB->moveLeft();
                                if (attachedGameB->isEndOfCycle()) keysDown[FPKEY_P2_Left] = 0;
                                else keysDown[FPKEY_P2_Left]++;
                                break;
                            case GameControlEvent::kPlayer2Right:
                                areaB->moveRight();
                                if (attachedGameB->isEndOfCycle()) keysDown[FPKEY_P2_Right] = 0;
                                else keysDown[FPKEY_P2_Right]++;
                                break;
                            case GameControlEvent::kPlayer2TurnLeft:
                                areaB->rotateLeft();
                                if (attachedGameB->isEndOfCycle()) keysDown[FPKEY_P2_TurnLeft] = 0;
                                else keysDown[FPKEY_P2_TurnLeft]++;
                                break;
                            case GameControlEvent::kPlayer2TurnRight:
                                areaB->rotateRight();
                                if (attachedGameB->isEndOfCycle()) keysDown[FPKEY_P2_TurnRight] = 0;
                                else keysDown[FPKEY_P2_TurnRight]++;
                                break;
                            case GameControlEvent::kPlayer2Down:
                                //attachedGameB->cycle(); desact flobo
                                if (attachedGameB->isEndOfCycle()) keysDown[FPKEY_P2_Down] = 0;
                                else keysDown[FPKEY_P2_Down]++;
                                break;
                            default:
                                break;
                        }
                    }
                    
                    switch (event.type) {
                      case SDL_USEREVENT:
                        if (randomPlayer)
                          randomPlayer->cycle();
                        if (event.user.code == 1) {
                          drect.x = 0;
                          drect.y = 0;
                          drect.w = 640;
                          drect.h = 480;

                          if (attachedGameA->isEndOfCycle()) {
                            keysDown[FPKEY_P1_Down] = 0;
                            keysDown[FPKEY_P1_TurnLeft] = 0;
                            keysDown[FPKEY_P1_TurnRight] = 0;
                          }

                          areaA->cycleGame(); // a voir

                          if (attachedGameB->isEndOfCycle()) {
                            keysDown[FPKEY_P2_Down] = 0;
                            keysDown[FPKEY_P2_TurnLeft] = 0;
                            keysDown[FPKEY_P2_TurnRight] = 0;
                          }
                          areaB->cycleGame(); // a voir

                          switch (gameLevel)
                          {
                            case 1:
                              attachedGameB->points += 1;
                              attachedGameA->points += 1;
                              break;
                            case 2:
                              attachedGameB->points += 5;
                              attachedGameA->points += 5;
                              break;
                            case 3:
                              attachedGameB->points += 10;
                              attachedGameA->points += 10;
                              break;
                          }

                          if (attachedGameA->getPoints()/50000 > savePointsA/50000)
                            blinkingPointsA = 10;
                          if (attachedGameB->getPoints()/50000 > savePointsB/50000)
                            blinkingPointsB = 10;

                          if (blinkingPointsA > 0)
                            blinkingPointsA--;
                          if (blinkingPointsB > 0)
                            blinkingPointsB--;

                          savePointsB = attachedGameB->getPoints();
                          savePointsA = attachedGameA->getPoints();

                          if (savePointsA < 50000) blinkingPointsA=0;
                          if (savePointsB < 50000) blinkingPointsB=0;

                        } else {
                          if (keysDown[FPKEY_P2_Down]) {
                            if (attachedGameB->isEndOfCycle())
                              keysDown[FPKEY_P2_Down] = 0;
                            else
                              areaB->cycleGame(); // a voir
                          }
                          if (keysDown[FPKEY_P2_Left]) {
                            repeatCondition(FPKEY_P2_Left) areaB->moveLeft();
                          }
                          if (keysDown[FPKEY_P2_Right]) {
                            repeatCondition(FPKEY_P2_Right) areaB->moveRight();
                          }
                          if (keysDown[FPKEY_P2_TurnLeft]) {
                            repeatCondition(FPKEY_P2_TurnLeft) areaB->rotateLeft();
                            if (attachedGameB->isEndOfCycle())
                              keysDown[FPKEY_P2_TurnLeft] = 0;
                          }
                          if (keysDown[FPKEY_P2_TurnRight]) {
                            repeatCondition(FPKEY_P2_TurnRight) areaB->rotateRight();
                            if (attachedGameB->isEndOfCycle())
                              keysDown[FPKEY_P2_TurnRight] = 0;
                          }

                          if (keysDown[FPKEY_P1_Down]) {
                            if (attachedGameA->isEndOfCycle())
                              keysDown[FPKEY_P1_Down] = 0;
                            else
                              areaA->cycleGame(); // a voir
                          }
                          if (keysDown[FPKEY_P1_Left]) {
                            repeatCondition(FPKEY_P1_Left) areaA->moveLeft();
                            if (attachedGameA->isEndOfCycle())
                              keysDown[FPKEY_P1_Left] = 0;
                          }
                          if (keysDown[FPKEY_P1_Right]) {
                            repeatCondition(FPKEY_P1_Right) areaA->moveRight();
                            if (attachedGameA->isEndOfCycle())
                              keysDown[FPKEY_P1_Right] = 0;
                          }
                          if (keysDown[FPKEY_P1_TurnLeft]) {
                            repeatCondition(FPKEY_P1_TurnLeft) areaA->rotateLeft();
                            if (attachedGameA->isEndOfCycle())
                              keysDown[FPKEY_P1_TurnLeft] = 0;
                          }
                          if (keysDown[FPKEY_P1_TurnRight]) {
                            repeatCondition(FPKEY_P1_TurnRight) areaA->rotateRight();
                            if (attachedGameA->isEndOfCycle())
                              keysDown[FPKEY_P1_TurnRight] = 0;
                          }
                        }
                        break;
                      case SDL_KEYDOWN:
                        /* check for cheat-codes */
                        static int cheatcode = 0;
                        if (event.key.keysym.sym == SDLK_k) cheatcode  = 0;
                        if (event.key.keysym.sym == SDLK_i) cheatcode += 1;
                        if (event.key.keysym.sym == SDLK_e) cheatcode += 10;
                        if (event.key.keysym.sym == SDLK_f) cheatcode += 100;
                        if (event.key.keysym.sym == SDLK_t) cheatcode += 1000;
                        if (event.key.keysym.sym == SDLK_l) cheatcode += 10000;
                        if (cheatcode == 31111) {
                          attachedGameA->increaseNeutralPuyos(PUYODIMX * 12);
                          attachedGameA->dropNeutrals();
                          attachedGameA->increaseNeutralPuyos(PUYODIMX * 12);
                        }
                      default:
                        break;
                    }
                } // !Paused
                    else {
                        GameControlEvent controlEvent;
                        getControlEvent(event, &controlEvent);
                        switch (controlEvent.cursorEvent) {
                            case GameControlEvent::kUp:
                                menu_prev_item(menu_pause);
                                break;
                            case GameControlEvent::kDown:
                                menu_next_item(menu_pause);
                                break;
                            case GameControlEvent::kStart:
                                if (menu_active_is(menu_pause, kContinueGame)) {
                                    paused = false;
                                    restartRender();
                                    menu_hide(menu_pause);
                                }
                                if (menu_active_is(menu_pause, kAbortGame)) {
                                    if (menu_active_is(commander->gameOverMenu, "YES"))
                                        menu_next_item(commander->gameOverMenu);
                                    quit = 1;
                                    menu_hide(menu_pause);
                                }
                                if (menu_active_is(menu_pause, kOptions)) {
                                    menu_hide (menu_pause);
                                    commander->optionMenuLoop(this);
                                    menu_show (menu_pause);
                                }
                                break;
                        }
                    }
            } else // Not GameIsRunning
            {
                if (randomPlayer) {
                    if (rightPlayerWin()) {
                        if ((_score2 == 7) && (rightPlayerWin()))
                            commander->gameOverMenu = commander->finishedMenu;
                        else
                            commander->gameOverMenu = commander->nextLevelMenu;
                    }
                    else {
                        if (lives == 0) {
                            commander->gameOverMenu = commander->gameOver1PMenu;
                        }
                        else {
                            commander->gameOverMenu = commander->looserMenu;
                        }
                    }
                }
                else {
                    commander->gameOverMenu = commander->gameOver2PMenu;
                }
                
                if (!menu_visible(commander->gameOverMenu)) {
                    
                    if (leftPlayerWin()) score1 = _score1 + 1;
                    else if (rightPlayerWin()) score2 = _score2 + 1;
                    
                    if (commander->gameOverMenu == commander->gameOver2PMenu) {
                        char winner[256];
                        char score[256];
                        sprintf(winner,"%d Wins!!!",(leftPlayerWin()?1:2));
                        sprintf(score, "%d - %d", score1, score2);
                        menu_set_value(commander->gameOverMenu, kPlayer, winner);
                        menu_set_value(commander->gameOverMenu, kScore,  score);
                    }
                    else if (commander->gameOverMenu == commander->nextLevelMenu) {
                        char level[256];
                        extern char *AI_NAMES[];
                        sprintf(level, "Stage %d... Vs %s", score2+1, AI_NAMES[score2]);
                        menu_set_value(commander->gameOverMenu, kNextLevel, level);
                    }
                    else if (commander->gameOverMenu == commander->looserMenu) {
                        char level[256];
                        char cont[256];
                        sprintf(level, "Stage %d... Vs %s", score2+1, p2name);
                        sprintf(cont, "%d Left", lives);
                        menu_set_value(commander->gameOverMenu, kCurrentLevel, level);
                        menu_set_value(commander->gameOverMenu, kContinueLeft, cont);
                    }
                    else if (commander->gameOverMenu == commander->gameOver1PMenu) {
                        char level[256];
                        sprintf(level, "Stage %d... Vs %s", score2+1, p2name);
                        menu_set_value(commander->gameOverMenu, kYouGotToLevel, level);
                    }
                    commander->showGameOver();
                    stopRender();
                } // GameOver Visible
            }
                GameControlEvent controlEvent2;
                getControlEvent(event, &controlEvent2);
                switch (controlEvent2.cursorEvent) {
                    case GameControlEvent::kStart:
                        if (gameover)
                        {
                            if (menu_active_is(commander->gameOverMenu, "NO"))
                                menu_next_item(commander->gameOverMenu);
                            quit = 1;
                        }
                        break;
                    case GameControlEvent::kBack:
                        if (!gameover) {
                            if (!paused) {
                                menu_show(menu_pause);
                                paused = true;
                                stopRender();
                            }
                            else {
                                paused = false;
                                restartRender();
                                menu_hide(menu_pause);
                            }
                        }
                        else {
                            if (menu_active_is(commander->gameOverMenu, "NO"))
                                menu_next_item(commander->gameOverMenu);
                            quit = 1;
                        }
                        break;
                    default:
                        break;
                }
                if(event.type == SDL_QUIT) {
                    if (menu_active_is(commander->gameOverMenu, "YES"))
                        menu_next_item(commander->gameOverMenu);
                    quit = 1;
                    exit(0);
                }
        }
            commander->updateAll(this);
            if (!paused) {
                areaA->cycleAnimation();
                areaB->cycleAnimation();
                
                tickCounts++;
                event.user.type = SDL_USEREVENT;
                event.user.code = 0;
                SDL_PushEvent(&event);
                // Vitesse du jeu
                if (tickCounts % (gameSpeed + 5 * (3 - gameLevel)) == 0)
                {
                    event.user.type = SDL_USEREVENT;
                    event.user.code = 1;
                    SDL_PushEvent(&event);
                    if (!paused) {
                        if ((tickCounts > GAME_ACCEL) && (gameSpeed > 1)) {
                            tickCounts = 0;
                            gameSpeed--;
                        }
                    }
                }
            }
            
    }
	commander->hideGameOver();
	if (randomPlayer) {
            for (int i=0; i<NB_PERSO_STATE; ++i)
                IIM_Free(perso[i]);
	}
	SDL_SetClipRect(display,NULL);
}
