//#include <psp2/kernel/processmgr.h>
//#include <psp2/ctrl.h>
//#include <psp2/touch.h>
//#include <psp2/audioout.h>

#include <citro2d.h>
#include <3ds.h>

#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <stdio.h>
#include <time.h>
#include <fstream>

#include "player.h"
#include "platform.h"

//#include "soloud.h"
//#include "soloud_wav.h"
//#include "soloud_speech.h"
#include "sprites.h"
//#define click() (keysDown() & KEY_TOUCH)
//#define release() (keysUp() & KEY_TOUCH)
//#define hold() (keysHeld() & KEY_TOUCH)

#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240

static C2D_SpriteSheet spriteSheet;
static C2D_Sprite playerSprite, lavaSprite, platformSprite, coinSprite, scoreBoxSprite, logo, splashEggSprite;

//SoLoud::Soloud gSoloud;
//SoLoud::Wav fxLaunch;
//SoLoud::Wav fxClick;
//SoLoud::Wav fxDeath;
//SoLoud::Wav fxCoin;
//SoLoud::Wav fxSplash;
//SoLoud::Wav fxSelect;

const double pi = 3.1415926535897;
const double gravity = 0.5;

Platform platforms[4] = {{0}, {1}, {2}, {3}};
Player player(platforms[0].getX() + platforms[0].getWidth()/2 - 26/2, platforms[0].getY() - player.getHeight(), 26, 32);

int getHighscore();

int scoreInt = 0;
int highscoreInt = getHighscore();
char score[32];
char highscore[32];

bool titleScreen = true;
bool playCoinFX = false;

int getHighscore()
{
    //bds cjnwmds
    return 1;
}
void saveHighscore(int val)
{
    /*
    FILE *scorefile = fopen("sdmc:/config/terri-fried-score.bin", "wb");

    fwrite(&val, sizeof(int), 1, scorefile);
    fclose(scorefile);*/
    printf("test %d", val);
}
void addScore(int amount)
{
    scoreInt += amount;
    if (scoreInt < 10)
    {
        sprintf(score, "00%d", scoreInt);
    } else if (scoreInt < 100)
    {
        sprintf(score, "0%d", scoreInt);
    } else {
        sprintf(score, "%d", scoreInt);
    }
    if (scoreInt > highscoreInt)
    {
        highscoreInt = scoreInt;
        sprintf(highscore, "BEST: %d", highscoreInt);
    }
}
void resetScore()
{
    scoreInt = 0;
    sprintf(score, "00%d", scoreInt);
    saveHighscore(highscoreInt);
}
void resetGame()
{
    resetScore();
    for (int i = 0; i < 4; i++)
    {
        platforms[i].resetPosition(i);
    }
    player.setVelocity(0, 0);
    player.setX(platforms[0].getX() + platforms[0].getWidth()/2 - 26/2);
    player.setY(platforms[0].getY() - player.getHeight());
}
void checkPlayerCollision()
{
    bool onPlatform = false;
    for (int i = 0; i < 4; i++)
    {
        if (platforms[i].getHasCoin() && player.getX() + player.getWidth() - 1 > platforms[i].getCoinX() && player.getX() + 1 < platforms[i].getCoinX() + 12 && player.getY() + player.getHeight() - 1 > platforms[i].getCoinY() && player.getY() + 1 < platforms[i].getCoinY() + 12)
        {
            addScore(1);
            platforms[i].setHasCoin(false);
            playCoinFX = true;
        }
        if (player.getX() + 1 < platforms[i].getX() + platforms[i].getWidth() && player.getX() + player.getWidth() > platforms[i].getX() && player.getY() + player.getHeight() >= platforms[i].getY() && player.getY() < platforms[i].getY() + platforms[i].getHeight())
        {
            if (player.getY() > platforms[i].getY() + platforms[i].getHeight()/2)
            {
                player.setVelocity(player.getVelocityX(), 2.5);
            } 
            else if (player.getY() + player.getHeight() <  platforms[i].getY() + platforms[i].getHeight())
            {    
                onPlatform = true;
                player.setY(platforms[i].getY() - player.getHeight());
                player.setY(player.getY() + 0.5);
            }
        }
        
    }
    player.setOnPlatform(onPlatform);
}

int main(void)
{
    srand (time(NULL));
    resetScore();
    sprintf(highscore, "BEST: %d", highscoreInt);
    const int screenWidth = SCREEN_WIDTH;
    const int screenHeight = SCREEN_HEIGHT;

    int mouseDownX;
    int mouseDownY;
    double lavaY = screenHeight - 32;
    double timer = 0; 
    double splashTimer = 0;
    bool playedSplash = false;
    bool playedSelect = false;
    bool firstTime = true;

    C2D_Font font;

    C2D_TextBuf g_staticBuf, g_dynamicBuf;
    C2D_Text g_staticText[3];

    romfsInit();
    cfguInit();
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    consoleInit(GFX_BOTTOM, NULL);

    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");

 //   sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
 //   sceTouchEnableTouchForce(SCE_TOUCH_PORT_FRONT);

    C2D_SpriteFromSheet(&playerSprite, spriteSheet, sprites_egg_idx);
   // playerSprite = vita2d_load_PNG_file("app0:resources/egg.png");
    C2D_SpriteSetDepth(&playerSprite, 0.6f);

    C2D_SpriteFromSheet(&lavaSprite, spriteSheet, sprites_lava_idx);
   // lavaSprite = vita2d_load_PNG_file("app0:resources/lava.png");
    C2D_SpriteSetDepth(&lavaSprite, 0.6f);

    C2D_SpriteFromSheet(&platformSprite, spriteSheet, sprites_platform_idx);
    //platformSprite = vita2d_load_PNG_file("app0:resources/platform.png");
    C2D_SpriteSetDepth(&platformSprite, 0.6f);

    C2D_SpriteFromSheet(&coinSprite, spriteSheet, sprites_coin_idx);
    //coinSprite = vita2d_load_PNG_file("app0:resources/coin.png");
    C2D_SpriteSetDepth(&coinSprite, 0.6f);

    C2D_SpriteFromSheet(&scoreBoxSprite, spriteSheet, sprites_scorebox_idx);
    //scoreBoxSprite = vita2d_load_PNG_file("app0:resources/scorebox.png");
    C2D_SpriteSetDepth(&scoreBoxSprite, 0.6f);

    C2D_SpriteFromSheet(&logo, spriteSheet, sprites_logo_idx);
    //logo = vita2d_load_PNG_file("app0:resources/logo.png");
    C2D_SpriteSetDepth(&logo, 0.6f);

    C2D_SpriteFromSheet(&splashEggSprite, spriteSheet, sprites_splash_egg_idx);
    //splashEggSprite = vita2d_load_PNG_file("app0:resources/splash_egg.png");
    C2D_SpriteSetDepth(&splashEggSprite, 0.6f);

    //gspWaitForVBlank();

    //SceTouchData touch, oldtouch;
	//sceTouchPeek(0, &oldtouch, 1);

    //TODO: Add sound back
    //fxLaunch.load("app0:/resources/launch.ogg");
    //fxClick.load("app0:/resources/click.ogg");
    //fxDeath.load("app0:/resources/die.ogg");
    //fxCoin.load("app0:/resources/coin.ogg");
    //fxSplash.load("app0:/resources/splash.ogg");
    //fxSelect.load("app0:/resources/select.ogg");
    font = C2D_FontLoad("romfs:resources/font.otf");

    g_staticBuf  = C2D_TextBufNew(4096);
    g_dynamicBuf = C2D_TextBufNew(4096);

    C2D_TextFontParse(&g_staticText[0], font, g_staticBuf, "CLICK ANYWHERE TO BEGIN");
    C2D_TextFontParse(&g_staticText[1], font, g_staticBuf, highscore);
    C2D_TextFontParse(&g_staticText[2], font, g_staticBuf, "Polymars");

    C2D_TextOptimize(&g_staticText[0]);
    C2D_TextOptimize(&g_staticText[1]);
    C2D_TextOptimize(&g_staticText[2]);

    while (1)   
    {

        hidScanInput();
        //click
        u32 kDown = hidKeysDown();
        //release
        u32 kUp = hidKeysUp();
        //hold
        u32 kHold = hidKeysHeld();

        touchPosition touch;

        hidTouchRead(&touch);

        //sceTouchPeek(0, &touch, 1);
        if (kDown & KEY_START)
        {
            break;
        }

        int touchX;
        int touchY;

        if(kHold & KEY_TOUCH)
        {
            touchX = touch.px;
            touchY = touch.py;
        }

        if (titleScreen)
        {   if (splashTimer > 120)
            {
                if (!playedSelect)
                {
                    //gSoloud.play(fxSelect);
                    playedSelect = true;
                }
                C2D_SpriteSetPos(&logo, screenWidth/2 - 200, screenHeight/2 - 45 - 30);
                C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
                C2D_TargetClear(top, C2D_Color32f(0.933f, 0.894f, 0.822f, 1.0f));
                C2D_SceneBegin(top);
                C2D_DrawSprite(&logo);
                C2D_DrawText(&g_staticText[0], C2D_AtBaseline | C2D_WithColor, (double)screenWidth/2 - 150, (double)screenHeight/2 + 10 + 32, 0.0f, 0.9f, 0.9f, C2D_Color32f(0.70f,0.59f,0.49f,0.4f));
                C2D_DrawText(&g_staticText[1], C2D_AtBaseline, (double)screenWidth/2 - 46, (double)screenHeight/2 + 50 + 32, 0.0f, 1.0f, 1.0f);
                //vita2d_font_draw_text(font, screenWidth/2 - 146, screenHeight/2 + 50 + 32, RGBA8(178, 150, 125, 102), 32, "CLICK ANYWHERE TO BEGIN");
                //gfxFlushBuffers();
                //gfxSwapBuffers();
                C3D_FrameEnd(0);


                if (kDown & KEY_TOUCH)
                {
                    //gSoloud.play(fxSelect);
                    titleScreen = false;
                    mouseDownX = touchX;
                    mouseDownY = touchY;
                }
            } else {
                if (!playedSplash)
                {
                    //gSoloud.play(fxSplash);
                    playedSplash = true;
                }

                C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
                C2D_TargetClear(top, C2D_Color32f(0.933f, 0.894f, 0.822f, 1.0f));
                C2D_SceneBegin(top);

                //vita2d_font_draw_text(font, screenWidth/2 - 59, screenHeight/2 + 3 + 26, RGBA8(213, 128, 90, 255), 32, "POLYMARS");
                C2D_DrawText(&g_staticText[2], C2D_AtBaseline | C2D_WithColor, (double)screenWidth/2 - 41, (double)screenHeight/2 + 10 + 32, 0.0f, 1.0f, 1.0f, C2D_Color32f(0.84f,0.50f,0.35f,1.0f));
                C2D_SpriteSetPos(&splashEggSprite, screenWidth/2, screenHeight/2 - 16 - 23);
                C2D_DrawSprite(&splashEggSprite);
                C3D_FrameEnd(0);
                //vita2d_swap_buffers();
                splashTimer += 1;

            }
        } 
        else 
        {
            if (playCoinFX)
            {
                //gSoloud.play(fxCoin);
                playCoinFX = false;
            }
            if ((kDown & KEY_TOUCH) && player.isOnGround())
            {
                    //gSoloud.play(fxClick);
                    mouseDownX = touchX;
                    mouseDownY = touchY;
            }
            if ((kUp & KEY_TOUCH) && player.isOnGround())
            {
                if (firstTime)
                {
                    firstTime = false;
                }
                else 
                {
                    //gSoloud.play(fxLaunch);
                    if (player.isOnPlatform())
                    {
                        player.setY(player.getY() - 0.5);
                    }
                    int velocityX = touchX - mouseDownX;
   
                    int velocityY = touchY - mouseDownY;

                    player.setVelocity((double)velocityX*.08, (double)velocityY*.08);
                }
            }
            checkPlayerCollision();
            player.updatePosition();
            if (player.getY() > screenHeight)
            {
                //gSoloud.play(fxDeath);
                resetGame();
            }
            for (int i = 0; i < 4; i++)
            {
                platforms[i].updatePosition();
            }

            lavaY = screenHeight - 29 - sin(timer) * 2.5;
            timer += 0.05;
           // vita2d_start_drawing();
           // vita2d_clear_screen();
            C2D_TextBufClear(g_dynamicBuf);
            C2D_Text dynText[1];

            C2D_TextFontParse(&dynText[0], font, g_dynamicBuf, score);
            C2D_TextFontParse(&dynText[1], font, g_dynamicBuf, highscore);

            C2D_TextOptimize(&dynText[0]);
            C2D_TextOptimize(&dynText[1]);

            C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
            C2D_TargetClear(top, C2D_Color32f(0.933f, 0.894f, 0.822f, 1.0f));
            C2D_SceneBegin(top);

            if ((kHold & KEY_TOUCH) && player.isOnGround())
            {

                C2D_DrawLine(mouseDownX + (player.getX() - mouseDownX) + (player.getWidth()/2), mouseDownY + (player.getY() - mouseDownY) + (player.getHeight()/2),  C2D_Color32f(0.90f,0.85f,0.79f,1.0f), touchX + (player.getX() - mouseDownX) + (player.getWidth()/2), touchY + + (player.getY() - mouseDownY) + (player.getHeight()/2), C2D_Color32f(0.90f,0.85f,0.79f,1.0f), 4.0f, 1.0f);
                //vita2d_draw_line(mouseDownX + (player.getX() - mouseDownX) + (player.getWidth()/2), mouseDownY + (player.getY() - mouseDownY) + (player.getHeight()/2), touch.report[0].x + (player.getX() - mouseDownX) + (player.getWidth()/2), touch.report[0].y + (player.getY() - mouseDownY) + (player.getHeight()/2), RGBA8(231, 216, 201, 255));
                //vita2d_draw_line(mouseDownX + (player.getX() - mouseDownX) + (player.getWidth()/2), mouseDownY + (player.getY() - mouseDownY) + (player.getHeight()/2) + 1, touch.report[0].x + (player.getX() - mouseDownX) + (player.getWidth()/2), touch.report[0].y + (player.getY() - mouseDownY) + (player.getHeight()/2) + 1, RGBA8(231, 216, 201, 255));
                //C2D_DrawLine(mouseDownX + (player.getX() - mouseDownX) + (player.getWidth()/2), mouseDownY + (player.getY() - mouseDownY) + (player.getHeight()/2) + 1,  C2D_Color32f(0.90f,0.85f,0.79f,1.0f), touchX + (player.getX() - mouseDownX) + (player.getWidth()/2), touchY + + (player.getY() - mouseDownY) + (player.getHeight()/2) + 1, C2D_Color32f(0.90f,0.85f,0.79f,1.0f), 4.0f, 1.0f);
                //vita2d_draw_line(mouseDownX + (player.getX() - mouseDownX) + (player.getWidth()/2) + 1, mouseDownY + (player.getY() - mouseDownY) + (player.getHeight()/2), touch.report[0].x + (player.getX() - mouseDownX) + (player.getWidth()/2) + 1, touch.report[0].y + (player.getY() - mouseDownY) + (player.getHeight()/2), RGBA8(231, 216, 201, 255));
               // C2D_DrawLine(mouseDownX + (player.getX() - mouseDownX) + (player.getWidth()/2) + 1, mouseDownY + (player.getY() - mouseDownY) + (player.getHeight()/2),  C2D_Color32f(0.90f,0.85f,0.79f,1.0f), touchX + (player.getX() - mouseDownX) + (player.getWidth()/2) + 1, touchY + + (player.getY() - mouseDownY) + (player.getHeight()/2), C2D_Color32f(0.90f,0.85f,0.79f,1.0f), 4.0f, 1.0f);
                //C2D_DrawLine(mouseDownX + (player.getX() - mouseDownX) + (player.getWidth()/2) + 1, mouseDownY + (player.getY() - mouseDownY) + (player.getHeight()/2) + 1,  C2D_Color32f(0.90f,0.85f,0.79f,1.0f), touchX + (player.getX() - mouseDownX) + (player.getWidth()/2) + 1, touchY + + (player.getY() - mouseDownY) + (player.getHeight()/2) + 1, C2D_Color32f(0.90f,0.85f,0.79f,1.0f), 4.0f, 1.0f);
               //vita2d_draw_line(mouseDownX + (player.getX() - mouseDownX) + (player.getWidth()/2) + 1, mouseDownY + (player.getY() - mouseDownY) + (player.getHeight()/2) + 1, touch.report[0].x + (player.getX() - mouseDownX) + (player.getWidth()/2) + 1, touch.report[0].y + (player.getY() - mouseDownY) + (player.getHeight()/2) + 1, RGBA8(231, 216, 201, 255));
            }  
            for (int i = 0; i < 4; i++)
            {
                C2D_DrawSprite(&platformSprite);
                C2D_SpriteSetPos(&platformSprite, platforms[i].getX(), platforms[i].getY());
                //vita2d_draw_texture(platformSprite, platforms[i].getX(),  platforms[i].getY());
                if (platforms[i].getHasCoin())
                {
                    //vita2d_draw_texture(coinSprite, platforms[i].getCoinX(), platforms[i].getCoinY());
                    C2D_SpriteSetPos(&coinSprite, platforms[i].getCoinX(), platforms[i].getCoinY());
                    C2D_DrawSprite(&coinSprite);

                }
            }
            //vita2d_draw_texture(playerSprite, player.getX(), player.getY());
            C2D_SpriteSetPos(&playerSprite, player.getX(), player.getY() + 15);
            C2D_DrawSprite(&playerSprite);

            //vita2d_draw_texture(lavaSprite, 0, lavaY);
            C2D_SpriteSetPos(&lavaSprite, 0, lavaY);
            C2D_DrawSprite(&lavaSprite);

            //vita2d_draw_texture(scoreBoxSprite, 17, 17);
            C2D_SpriteSetPos(&scoreBoxSprite, 17, 17);
            C2D_DrawSprite(&scoreBoxSprite);

            //vita2d_font_draw_text(font, 28 - 2, 20 + 64 - 9, RGBA8(0, 0, 0, 255), 64, score);
            //vita2d_font_draw_text(font, 17, 90 + 32, RGBA8(0, 0, 0, 255), 32, highscore);
            C2D_DrawText(&dynText[0], C2D_AtBaseline, (double)28 - 2, (double)20 + 64 - 9, 0.0f, 1.0f, 1.0f);
            C2D_DrawText(&dynText[1], C2D_AtBaseline, (double)17, (double)90 + 32, 0.0f, 1.0f, 1.0f);

            C3D_FrameEnd(0);
            //vita2d_end_drawing();
            //vita2d_swap_buffers();
        }
        //oldtouch = touch;
        //gspWaitForVBlank();
    }
    


    C2D_TextBufDelete(g_dynamicBuf);
    C2D_TextBufDelete(g_staticBuf);
    C2D_FontFree(font);
    // Delete graphics
    C2D_SpriteSheetFree(spriteSheet);

    // Deinit libs
    C2D_Fini();
    C3D_Fini();
    romfsExit();
    cfguExit();
    gfxExit();
    return 0;
}
