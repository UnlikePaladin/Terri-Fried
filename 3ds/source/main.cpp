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
#include <cwav.h>
#include <tuple>
#include <dirent.h>

#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240

static C2D_SpriteSheet spriteSheet;
static C2D_Sprite playerSprite, lavaSprite, platformSprite, coinSprite, scoreBoxSprite, logo, splashEggSprite;

static const char* fileList[] = {
        "romfs:/sounds/splash.bcwav",
        "romfs:/sounds/select.bcwav",
        "romfs:/sounds/die.bcwav",
        "romfs:/sounds/coin.bcwav",
        "romfs:/sounds/click.bcwav",
        "romfs:/sounds/launch.bcwav",
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
    FILE *scorefile = fopen("/3ds/terri-fried/terri-fried-score.bin", "rb");
    if(!scorefile)
        return 0;

    int ret = getw(scorefile);
    fclose(scorefile);
    return ret;
}
void saveHighscore(int val)
{
    FILE *scorefile = fopen("/3ds/terri-fried/terri-fried-score.bin", "wb");
    putw(val, scorefile);
    fclose(scorefile);
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
C2D_Font font;
bool quit = false;

void pause(C3D_RenderTarget* top, C3D_RenderTarget* bottom){
    if (!titleScreen){
        C3D_FrameEnd(0);
        C2D_Text g_pauseText[2];
        C2D_TextBuf g_pauseBuf  = C2D_TextBufNew(4096);

        C2D_TextFontParse(&g_pauseText[0], font, g_pauseBuf, "PAUSED");
        C2D_TextOptimize(&g_pauseText[0]);
        C2D_TextFontParse(&g_pauseText[1], font, g_pauseBuf, "Press START to exit the game");
        C2D_TextOptimize(&g_pauseText[1]);

        while(aptMainLoop()) {
            C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
            C2D_SceneBegin(top);

            hidScanInput();
            u32 kDown = hidKeysDown();
            if(kDown & KEY_SELECT){
                    break;
                }
            C2D_DrawText(&g_pauseText[0], C2D_AtBaseline | C2D_WithColor, SCREEN_WIDTH/2 - 60, SCREEN_HEIGHT / 2, 1.0f, 0.9f, 0.9f, C2D_Color32f(0.70f,0.59f,0.49f,0.4f));
            C2D_SceneBegin(bottom);
            C2D_DrawText(&g_pauseText[1], C2D_AtBaseline | C2D_WithColor, (double)SCREEN_WIDTH/2 - 150, (double)SCREEN_HEIGHT/2 + 10 + 90, 1.0f, 0.6f, 0.6f, C2D_Color32f(0.0f,0.0f,0.0f,1.0f));
            if(kDown & KEY_START){
                quit = true;
                break;
            }
            C3D_FrameEnd(0);
        }
        C2D_TextBufDelete(g_pauseBuf);
    }
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

//Audio Library and Example Code from PabloMK7's libcwav
std::vector<std::tuple<std::string, CWAV*>> cwavList;
void populateCwavList()
{

    for (u32 i = 0; i < sizeof(fileList) / sizeof(char*); i++)
    {

        CWAV* cwav = (CWAV*)malloc(sizeof(CWAV));

        FILE* file = fopen(fileList[i], "rb");
        if (!file)
        {
            cwavFree(cwav);
            free(cwav);
            continue;
        }

        fseek(file, 0, SEEK_END);
        u32 fileSize = ftell(file);
        void* buffer = linearAlloc(fileSize);
        if (!buffer) // This should never happen (unless we load a file too big to fit)
            svcBreak(USERBREAK_PANIC);

        fseek(file, 0, SEEK_SET);
        fread(buffer, 1, fileSize, file);
        fclose(file);

        // Since we manually allocated the buffer, we use cwavLoad directly...
        cwavLoad(cwav, buffer, 4);
        cwav->dataBuffer = buffer; // (We store the buffer pointer here for convinience, but it's not required.)
        // or, we could have let the library handle it...
        // cwavFileLoad(cwav, fileList[i], maxSPlayList[i]);

        if (cwav->loadStatus == CWAV_SUCCESS)
        {
            cwavList.push_back(std::make_tuple(std::string(fileList[i]), cwav));
        }
        else
        {
            // Manually de-allocating the buffer
            cwavFree(cwav);
            linearFree(cwav->dataBuffer);
            // or, if we used cwavFileLoad, let the library handle it.
            // cwavFileFree(cwav);

            free(cwav);
        }
    }
}

void freeCwavList()
{
    for (auto it = cwavList.begin(); it != cwavList.end(); it++)
    {
        // Manually de-allocating the buffer
        cwavFree(std::get<1>(*it));
        void* buffer = std::get<1>(*it)->dataBuffer;
        if (buffer)
            linearFree(buffer);
        // or, if we used cwavFileLoad, let the library handle it.
        // cwavFileFree(std::get<1>(*it));
        free(std::get<1>(*it));
    }
}
//End of audio code

int main(void)
{
    srand (time(NULL));
    mkdir("/3ds", 0777);
    mkdir("/3ds/terri-fried/", 0777);

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

    C2D_TextBuf g_staticBuf, g_dynamicBuf;
    C2D_Text g_staticText[3];

    romfsInit();
    cfguInit();
    gfxInitDefault();
    cwavUseEnvironment(CWAV_ENV_DSP);
    ndspInit();
    gfxSet3D(true);

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

    populateCwavList();

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
    player.setX(platforms[0].getX() + platforms[0].getWidth()/2 - 26/2);
    player.setY(platforms[0].getY() - player.getHeight());
    while (aptMainLoop())
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
        if ((kDown & KEY_START) || quit)
        {
            saveHighscore(highscoreInt);
            break;
        }
        if (kDown & KEY_SELECT)
        {
            pause(top, bottom);
        }
        if(titleScreen && (kHold & KEY_X) && (kHold & KEY_L) && (kHold & KEY_R)) {
            highscoreInt = 0;
            saveHighscore(0);
            quit = true;
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
                    CWAV* cwav = std::get<1>(cwavList[1]);
                    if (cwav->numChannels == 2)
                    {
                        cwavPlay(cwav, 0, 1);
                    }
                    else
                    {
                        cwavPlay(cwav, 0, -1);
                    }
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
                    CWAV* cwav = std::get<1>(cwavList[1]);
                    if (cwav->numChannels == 2)
                    {
                        cwavPlay(cwav, 0, 1);
                    }
                    else
                    {
                        cwavPlay(cwav, 0, -1);
                    }

                    titleScreen = false;
                    mouseDownX = touchX;
                    mouseDownY = touchY;
                }
            } else {
                if (!playedSplash)
                {
                    CWAV* cwav = std::get<1>(cwavList[0]);
                    if (cwav->numChannels == 2)
                    {
                        cwavPlay(cwav, 0, 1);
                    }
                    else
                    {
                        cwavPlay(cwav, 0, -1);
                    }

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
                CWAV* cwav = std::get<1>(cwavList[3]);
                if (cwav->numChannels == 2)
                {
                    cwavPlay(cwav, 0, 1);
                }
                else
                {
                    cwavPlay(cwav, 0, -1);
                }
                playCoinFX = false;
            }
            if ((kDown & KEY_TOUCH) && player.isOnGround())
            {
                    CWAV* cwav = std::get<1>(cwavList[4]);
                if (cwav->numChannels == 2)
                {
                    cwavPlay(cwav, 0, 1);
                }
                else
                {
                    cwavPlay(cwav, 0, -1);
                }
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
                    CWAV* cwav = std::get<1>(cwavList[5]);
                    if (cwav->numChannels == 2)
                    {
                        cwavPlay(cwav, 0, 1);
                    }
                    else
                    {
                        cwavPlay(cwav, 0, -1);
                    }

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
                CWAV* cwav = std::get<1>(cwavList[2]);
                if (cwav->numChannels == 2)
                {
                    cwavPlay(cwav, 0, 1);
                }
                else
                {
                    cwavPlay(cwav, 0, -1);
                }
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

            C2D_DrawText(&dynText[0], C2D_AtBaseline, 13.0f, 39.0f, 1.0f, 0.9f, 1.3f);
            C2D_DrawText(&dynText[1], C2D_AtBaseline, 10.0f, 60.0f, 1.0f, 0.6f, 0.6f);

            C2D_TargetClear(bottom, C2D_Color32f(1.0f, 0.67f, 0.53f, 1.0f));
            C2D_SceneBegin(bottom);
            C3D_FrameEnd(0);
        }
    }
    freeCwavList();

    // Delete graphics
    C2D_SpriteSheetFree(spriteSheet);
    C2D_TextBufDelete(g_dynamicBuf);
    C2D_TextBufDelete(g_staticBuf);
    C2D_FontFree(font);
    ndspExit();
    // Deinit libs
    C2D_Fini();
    C3D_Fini();
    romfsExit();
    cfguExit();
    gfxExit();
    return 0;
}
