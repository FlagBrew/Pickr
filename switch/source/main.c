#include <switch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "draw.h"
#include "types.h"
#include "heart48_bin.h"

u8* g_framebuf;
u32 g_framebufWidth;

color_t color_black;
color_t color_red;

static struct {
    int t;
    int dt;
    int dc;
    u8 lives;
    int games;
    int score;
    int bestScore;
    bool pattern[16];
    u8 color[3];
} game;

void gameInit(void)
{
    game.lives = 5;
    game.dt = 4;
    game.t = 1280;
    game.dc = 10;
    game.score = 0;
}

void gameUpdateD(void)
{
    game.dc = 10;
}

static void newPattern(void)
{
    for (u8 i = 0; i < 16; i++)
    {
        game.pattern[i] = false;
    }
    game.pattern[rand() % 16] = true;
    game.color[0] = rand() % (255 - 2*game.dc) + game.dc;
    game.color[1] = rand() % (255 - 2*game.dc) + game.dc;
    game.color[2] = rand() % (255 - 2*game.dc) + game.dc;
}

static void gameEnd(void)
{
    static const char* end = "End game!";
    static const char* button = "Press A to restart.";
    char tmp1[26], tmp2[20], tmp3[20];
    snprintf(tmp1, 26, "Levels completed: %d", game.score);
    snprintf(tmp2, 20, "Best score: %d", game.bestScore);
    snprintf(tmp3, 20, "Games played: %d", game.games);

    u32 end_w, button_w, tmp1_w, tmp2_w, tmp3_w;
    GetTextDimensions(font42, end, &end_w, NULL);
    GetTextDimensions(font24, button, &button_w, NULL);
    GetTextDimensions(font24, tmp1, &tmp1_w, NULL);
    GetTextDimensions(font24, tmp2, &tmp2_w, NULL);
    GetTextDimensions(font24, tmp3, &tmp3_w, NULL);

    while(appletMainLoop() && !(hidKeysDown(CONTROLLER_P1_AUTO) & KEY_A))
    {
        hidScanInput();

        g_framebuf = gfxGetFramebuffer(&g_framebufWidth, NULL);
        memset(g_framebuf, 245, gfxGetFramebufferSize());

        DrawText(font42, ceil((1280 - end_w) / 2), 260, color_black, end);
        DrawText(font24, ceil((1280 - tmp1_w) / 2), 364, color_black, tmp1);
        DrawText(font24, ceil((1280 - tmp2_w) / 2), 410, color_red, tmp2);
        DrawText(font24, ceil((1280 - tmp3_w) / 2), 456, color_black, tmp3);
        DrawText(font24, ceil((1280 - button_w) / 2), 636, color_black, button);

        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();
    }
}

static void level(void)
{
    int x, y;
    bool win = false;
    char score[16];
    snprintf(score, 16, "Score: %d", game.score);

    game.t = 1280;
    newPattern();
    game.dc *= (rand() % 2) ? 1 : -1;

    static const int l = 96, spacing = 16;
    while(appletMainLoop() && game.t > 0 && game.lives > 0)
    {
        hidScanInput();
        touchPosition touch;
        hidTouchRead(&touch, 0);

        g_framebuf = gfxGetFramebuffer(&g_framebufWidth, NULL);
        memset(g_framebuf, 245, gfxGetFramebufferSize());

        // draw hearts
        for (u8 i = 0, max = game.lives < 5 ? 5 : game.lives; i < max; i++)
        {
            if (i < game.lives)
            {
                DrawImage(8 + i * (48 + 8), 8, 48, 48, heart48_bin, IMAGE_MODE_RGBA32);
            }
            else
            {
                DrawImageBlend(8 + i * (48 + 8), 8, 48, 48, heart48_bin, IMAGE_MODE_RGBA32, color_black);
            }
        }

        // draw level counter
        u32 score_w;
        GetTextDimensions(font24, score, &score_w, NULL);
        DrawText(font24, 1280 - 16 - score_w, 8, color_red, score);

        // draw rectangles
        y = ceil((720 - l*4 - spacing*3) / 2);
        for (u8 i = 0; i < 4; i++)
        {
            x = ceil((1280 - l*4 - spacing*3) / 2);
            for (u8 j = 0; j < 4; j++)
            {
                rectangle(x, y, l, l, (game.pattern[i*4+j]) ? MakeColor(game.color[0] - game.dc, game.color[1] - game.dc, game.color[2] - game.dc, 255) : MakeColor(game.color[0], game.color[1], game.color[2], 255));
                x += l + spacing;
            }
            y += l + spacing;
        }

        // draw time counter
        rectangle(0, 720 - 24, game.t, 24, game.t > 1280 / 4 ? MakeColor(0, 255, 0, 255) : MakeColor(255, 0, 0, 255));

        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();

        // win logic
        if (hidKeysDown(CONTROLLER_P1_AUTO) & KEY_TOUCH)
        {
            y = ceil((720 - l*4 - spacing*3) / 2);
            for (u8 i = 0; i < 4; i++)
            {
                x = ceil((1280 - l*4 - spacing*3) / 2);
                for (u8 j = 0; j < 4; j++)
                {
                    if (touch.px > x && touch.px < x + l && touch.py > y && touch.py < y + l)
                    {
                        if (game.pattern[i*4+j])
                        {
                            win = true;
                        }
                        else
                        {
                            game.lives--;
                        }
                    }
                    x += l + spacing;
                }
                y += l + spacing;
            }
        }

        // refresh lives and time if time finishes
        game.t -= game.dt;
        if (game.t <= 0 && game.lives > 0)
        {
            game.t = 1280;
            game.lives--;
        }

        // create a new level if you won
        if (win)
        {
            if (++game.score % 10 == 0 && game.lives < 10)
            {
                game.lives++;
            }
            gameUpdateD();
            game.bestScore = game.score >= game.bestScore ? game.score : game.bestScore;
            level();
        }
    }
}

int main()
{
    color_black = MakeColor(0, 0, 0, 255);
    color_red = MakeColor(200, 0, 0, 255);

    static const char* title = "PICKR";
    static const char* description = "Try to pick the different color square!";
    static const char* buttons = "Press A to start a new game. Press + to exit.";
    static const char* credits1 = "Based on Sean M. Tracey's idea";
    static const char* credits2 = "Made with       by Bernardo Giordano";

    u32 title_w, description_w, buttons_w;
    GetTextDimensions(font42, title, &title_w, NULL);
    GetTextDimensions(font24, description, &description_w, NULL);
    GetTextDimensions(font24, buttons, &buttons_w, NULL);

    srand(time(NULL));
    gfxInitDefault();
    gameInit();

    while(appletMainLoop() && !(hidKeysDown(CONTROLLER_P1_AUTO) & KEY_PLUS))
    {
        hidScanInput();

        if (hidKeysDown(CONTROLLER_P1_AUTO) & KEY_A)
        {
            level();
            game.games++;
            gameEnd();
            gameInit();
        }

        g_framebuf = gfxGetFramebuffer(&g_framebufWidth, NULL);
        memset(g_framebuf, 245, gfxGetFramebufferSize());

        DrawText(font42, ceil((1280 - title_w) / 2), 260, color_black, title);
        DrawText(font24, ceil((1280 - description_w) / 2), 364, color_red, description);
        DrawText(font24, ceil((1280 - buttons_w) / 2), 410, color_black, buttons);

        DrawText(font20, 10, 636, color_black, credits1);
        DrawText(font20, 10, 678, color_black, credits2);
        DrawImage(144, 672, 48, 48, heart48_bin, IMAGE_MODE_RGBA32);

        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();
    }

    gfxExit();
}