/**
 * 3D Spinning Cube Demo for PSP
 *
 * Created by Claude Code (Anthropic)
 */

#include <pspkernel.h>
#include <pspctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

typedef struct
{
    float x, y, z;
} Vec3;

typedef struct
{
    int a, b;
} Edge;

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

Vec3 rotateX(Vec3 v, float angle)
{
    Vec3 result;
    result.x = v.x;
    result.y = v.y * cosf(angle) - v.z * sinf(angle);
    result.z = v.y * sinf(angle) + v.z * cosf(angle);
    return result;
}

Vec3 rotateY(Vec3 v, float angle)
{
    Vec3 result;
    result.x = v.x * cosf(angle) + v.z * sinf(angle);
    result.y = v.y;
    result.z = -v.x * sinf(angle) + v.z * cosf(angle);
    return result;
}

Vec3 rotateZ(Vec3 v, float angle)
{
    Vec3 result;
    result.x = v.x * cosf(angle) - v.y * sinf(angle);
    result.y = v.x * sinf(angle) + v.y * cosf(angle);
    result.z = v.z;
    return result;
}

void project(Vec3 v, int *x, int *y, float distance)
{
    float factor = distance / (distance + v.z);
    *x = (int)(v.x * factor) + SCREEN_WIDTH / 2;
    *y = (int)(v.y * factor) + SCREEN_HEIGHT / 2;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

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

    TTF_Font *font = TTF_OpenFont("Orbitron-Regular.ttf", 20);
    if (!font)
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    Text title = createText(renderer, font, (SDL_Color){255, 255, 255, 255}, "3D Spinning Cube Demo");
    Text credit = createText(renderer, font, (SDL_Color){180, 180, 180, 255}, "Made by Claude Code (Anthropic)");
    Text controls = createText(renderer, font, (SDL_Color){150, 150, 150, 255}, "START to exit");

    Vec3 vertices[8] = {
        {-50, -50, -50},
        {50, -50, -50},
        {50, 50, -50},
        {-50, 50, -50},
        {-50, -50, 50},
        {50, -50, 50},
        {50, 50, 50},
        {-50, 50, 50}};

    Edge edges[12] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};

    float angleX = 0.0f;
    float angleY = 0.0f;
    float angleZ = 0.0f;

    SceCtrlData pad;
    int running = 1;

    while (running)
    {
        sceCtrlReadBufferPositive(&pad, 1);

        if (pad.Buttons & PSP_CTRL_START)
            running = 0;

        angleX += 0.02f;
        angleY += 0.025f;
        angleZ += 0.015f;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        Vec3 rotated[8];
        int projected[8][2];

        for (int i = 0; i < 8; i++)
        {
            rotated[i] = vertices[i];
            rotated[i] = rotateX(rotated[i], angleX);
            rotated[i] = rotateY(rotated[i], angleY);
            rotated[i] = rotateZ(rotated[i], angleZ);
            project(rotated[i], &projected[i][0], &projected[i][1], 200.0f);
        }

        SDL_SetRenderDrawColor(renderer, 0, 200, 255, 255);
        for (int i = 0; i < 12; i++)
        {
            int a = edges[i].a;
            int b = edges[i].b;
            SDL_RenderDrawLine(renderer,
                               projected[a][0], projected[a][1],
                               projected[b][0], projected[b][1]);
        }

        for (int i = 0; i < 8; i++)
        {
            SDL_Rect point = {projected[i][0] - 2, projected[i][1] - 2, 4, 4};
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &point);
        }

        drawText(renderer, &title, 10, 10);
        drawText(renderer, &credit, 10, 35);
        drawText(renderer, &controls, 10, SCREEN_HEIGHT - 30);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    freeText(&title);
    freeText(&credit);
    freeText(&controls);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    sceKernelExitGame();

    return 0;
}
