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
#include "sprites.h"
#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240

static C2D_SpriteSheet spriteSheet;
static C2D_Sprite playerSprite, lavaSprite, platformSprite, coinSprite, scoreBoxSprite, logo, splashEggSprite;

static const char* soundList[] = {
        "romfs:/sounds/splash.wav",
        "romfs:/sounds/select.wav",
        "romfs:/sounds/die.wav",
        "romfs:/sounds/coin.wav",
        "romfs:/sounds/click.wav",
        "romfs:/sounds/launch.wav",
};

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

    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");

    C2D_SpriteFromSheet(&playerSprite, spriteSheet, sprites_egg_idx);
    C2D_SpriteFromSheet(&lavaSprite, spriteSheet, sprites_lava_idx);
    C2D_SpriteFromSheet(&platformSprite, spriteSheet, sprites_platform_idx);
    C2D_SpriteFromSheet(&coinSprite, spriteSheet, sprites_coin_idx);
    C2D_SpriteFromSheet(&scoreBoxSprite, spriteSheet, sprites_scorebox_idx);
    C2D_SpriteFromSheet(&logo, spriteSheet, sprites_logo_idx);
    C2D_SpriteFromSheet(&splashEggSprite, spriteSheet, sprites_splash_egg_idx);
    C2D_SpriteSetDepth(&logo, 0.6f);
    C2D_SpriteSetDepth(&scoreBoxSprite, 0.6f);
    C2D_SpriteSetDepth(&splashEggSprite, 0.6f);

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
    C2D_TextFontParse(&g_staticText[3], font, g_staticBuf, "Press START to exit");

    C2D_TextOptimize(&g_staticText[0]);
    C2D_TextOptimize(&g_staticText[1]);
    C2D_TextOptimize(&g_staticText[2]);
    C2D_TextOptimize(&g_staticText[3]);

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
                C2D_DrawText(&g_staticText[0], C2D_AtBaseline | C2D_WithColor, (double)screenWidth/2 - 160, (double)screenHeight/2 + 10 + 32, 1.0f, 0.9f, 0.9f, C2D_Color32f(0.70f,0.59f,0.49f,0.4f));
                C2D_DrawText(&g_staticText[1], C2D_AtBaseline, (double)screenWidth/2 - 46, (double)screenHeight/2 + 50 + 32, 1.0f, 0.9f, 0.9f);

                C2D_TargetClear(bottom, C2D_Color32f(0.933f, 0.894f, 0.822f, 1.0f));
                C2D_SceneBegin(bottom);
                C2D_DrawText(&g_staticText[3], C2D_AtBaseline | C2D_WithColor, (double)screenWidth/2 - 150, (double)screenHeight/2 + 10 + 90, 1.0f, 0.9f, 0.9f, C2D_Color32f(0.70f,0.59f,0.49f,0.4f));

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

                C2D_DrawText(&g_staticText[2], C2D_AtBaseline | C2D_WithColor, (double)screenWidth/2 - 51, (double)screenHeight/2 + 10 + 32, 0.0f, 1.0f, 1.0f, C2D_Color32f(0.84f,0.50f,0.35f,1.0f));
                C2D_SpriteSetPos(&splashEggSprite, screenWidth/2 - 15, screenHeight/2 - 16 - 23);
                C2D_DrawSprite(&splashEggSprite);

                C2D_TargetClear(bottom, C2D_Color32f(0.933f, 0.894f, 0.822f, 1.0f));
                C2D_SceneBegin(bottom);
                C3D_FrameEnd(0);
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

                    player.setVelocity(velocityX*.08, velocityY*.08);
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
                C2D_DrawLine(mouseDownX + (player.getX()- mouseDownX) + (player.getWidth()/2), mouseDownY + ((player.getY() + 10) - mouseDownY) + (player.getHeight()/2),  C2D_Color32f(0.90f,0.85f,0.79f,1.0f), touchX + (player.getX() - mouseDownX) + (player.getWidth()/2), touchY + + (player.getY() - mouseDownY) + (player.getHeight()/2), C2D_Color32f(0.90f,0.85f,0.79f,1.0f), 2.0f, 0.3f);
            }  
            for (int i = 0; i < 4; i++)
            {
                C2D_DrawSprite(&platformSprite);
                C2D_SpriteSetPos(&platformSprite, platforms[i].getX(), platforms[i].getY());
                C2D_SpriteSetDepth(&platformSprite, 0.6f);
                if (platforms[i].getHasCoin())
                {
                    C2D_SpriteSetPos(&coinSprite, platforms[i].getCoinX(), platforms[i].getCoinY());
                    C2D_SpriteSetDepth(&coinSprite, 0.5f);
                    C2D_DrawSprite(&coinSprite);

                }
            }
            C2D_SpriteSetPos(&playerSprite, player.getX() + 5, player.getY() + 15);
            C2D_SpriteSetDepth(&playerSprite, 0.5f);
            C2D_DrawSprite(&playerSprite);

            C2D_SpriteSetPos(&lavaSprite, 0, lavaY + 12);
            C2D_SpriteSetDepth(&lavaSprite, 0.7f);
            C2D_DrawSprite(&lavaSprite);

            C2D_SpriteSetPos(&scoreBoxSprite, 10, 10);
            C2D_SpriteSetDepth(&scoreBoxSprite, 1.0f);
            C2D_DrawSprite(&scoreBoxSprite);

            C2D_DrawText(&dynText[0], C2D_AtBaseline, 14.5f, 42.0f, 1.0f, 0.9f, 1.3f);
            C2D_DrawText(&dynText[1], C2D_AtBaseline, 10.0f, 60.0f, 1.0f, 0.6f, 0.6f);

            C2D_TargetClear(bottom, C2D_Color32f(1.0f, 0.67f, 0.53f, 1.0f));
            C2D_SceneBegin(bottom);
            C3D_FrameEnd(0);
        }
    }

    // Delete graphics
    C2D_SpriteSheetFree(spriteSheet);
    C2D_TextBufDelete(g_dynamicBuf);
    C2D_TextBufDelete(g_staticBuf);
    C2D_FontFree(font);

    // Deinit libs
    C2D_Fini();
    C3D_Fini();
    romfsExit();
    cfguExit();
    gfxExit();
    return 0;
}
