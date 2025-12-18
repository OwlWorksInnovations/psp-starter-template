#include <pspkernel.h>
#include <pspctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

typedef struct
{
    SDL_Texture *texture;
    int w;
    int h;
} Text;

Text createText(SDL_Renderer *renderer, TTF_Font *font, SDL_Color color, const char *text)
{
    Text t = {NULL, 0, 0};

    SDL_Surface *surface = TTF_RenderText_Blended(font, text, color);
    if (!surface)
        return t;

    t.texture = SDL_CreateTextureFromSurface(renderer, surface);
    t.w = surface->w;
    t.h = surface->h;
    SDL_FreeSurface(surface);

    return t;
}

void drawText(SDL_Renderer *renderer, Text *text, int x, int y)
{
    if (!text->texture)
        return;

    SDL_Rect dst = {x, y, text->w, text->h};
    SDL_RenderCopy(renderer, text->texture, NULL, &dst);
}

void freeText(Text *text)
{
    if (text->texture)
        SDL_DestroyTexture(text->texture);
    text->texture = NULL;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    // Initialize PSP controls
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return 1;

    if (TTF_Init() < 0)
    {
        SDL_Quit();
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "window",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        0);

    if (!win)
    {
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, 0);
    if (!renderer)
    {
        SDL_DestroyWindow(win);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("Orbitron-Regular.ttf", 24);
    if (!font)
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    Text welcome = createText(renderer, font, (SDL_Color){0, 0, 0, 255}, "Welcome to PSP clicker!");
    
    int clicks = 0;
    int wasPressed = 0;
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Clicks: %d", clicks);
    Text click_count = createText(renderer, font, (SDL_Color){0, 0, 0, 255}, buffer);

    SceCtrlData pad;
    int running = 1;

    while (running)
    {
        sceCtrlReadBufferPositive(&pad, 1);

        if (pad.Buttons & PSP_CTRL_START)
            running = 0;

        if (pad.Buttons & PSP_CTRL_CROSS)
        {
            if (!wasPressed)
            {
                clicks++;
                freeText(&click_count);
                snprintf(buffer, sizeof(buffer), "Clicks: %d", clicks);
                click_count = createText(renderer, font, (SDL_Color){0, 0, 0, 255}, buffer);
                wasPressed = 1;
            }
        }
        else
        {
            wasPressed = 0;
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        drawText(renderer, &welcome, 0, 0);
        drawText(renderer, &click_count, 0, 32);
        SDL_RenderPresent(renderer);
    }

    freeText(&welcome);
    freeText(&click_count);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    sceKernelExitGame();

    return 0;
}