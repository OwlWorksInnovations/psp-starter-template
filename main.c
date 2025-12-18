#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

int drawText(SDL_Renderer *renderer, TTF_Font *font, SDL_Color color, const char *text, int x, int y)
{
    if (!font || !text || !renderer)
        return -1;

    SDL_Surface *surface = TTF_RenderText_Blended(font, text, color);
    if (!surface)
    {
        printf("Surface error: %s\n", TTF_GetError());
        return -1;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    int w = surface->w;
    int h = surface->h;
    SDL_FreeSurface(surface);  // Free after copying dimensions

    if (!texture)
    {
        printf("Texture error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Rect dst = {x, y, w, h};
    int result = SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_DestroyTexture(texture);

    return result;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
    {
        printf("SDL2 could not be initialized!\nSDL2 Error: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() < 0)
    {
        printf("SDL2_ttf could not be initialized!\nSDL2_ttf Error: %s\n", TTF_GetError());
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
        printf("Window could not be created!\nSDL_Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, 0);
    if (!renderer)
    {
        printf("Renderer could not be created!\nSDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Lazy load here (preload)
    TTF_Font *font = TTF_OpenFont("Orbitron-Regular.ttf", 24);
    if (!font)
    {
        printf("Font could not be loaded!\nTTF_Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    int running = 1;
    SDL_Event e;
    while (running)
    {
        while (SDL_PollEvent(&e))  // Process all pending events
        {
            switch (e.type)
            {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_CONTROLLERDEVICEADDED:
                SDL_GameControllerOpen(e.cdevice.which);
                break;
            case SDL_CONTROLLERBUTTONDOWN:
                if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START)
                    running = 0;
                break;
            }
        }

        // Draw here
        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
        SDL_RenderClear(renderer);
        drawText(renderer, font, (SDL_Color){0, 0, 0, 255}, "Hello PSP!", 0, 0);
        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();

    return 0;
}