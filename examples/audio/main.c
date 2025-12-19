#include <pspkernel.h>
#include <pspctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

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

// Generate a simple sine wave beep sound
void generateBeepSound(const char *filename, int frequency, int duration_ms)
{
    int sample_rate = 22050;
    int samples = (sample_rate * duration_ms) / 1000;

    SDL_AudioSpec spec;
    spec.freq = sample_rate;
    spec.format = AUDIO_S16SYS;
    spec.channels = 1;
    spec.samples = 4096;
    spec.callback = NULL;

    Sint16 *buffer = (Sint16 *)malloc(samples * sizeof(Sint16));
    if (!buffer)
        return;

    // Generate sine wave
    for (int i = 0; i < samples; i++)
    {
        double t = (double)i / sample_rate;
        double envelope = 1.0;

        // Add envelope to prevent clicking
        if (i < sample_rate / 100) // 10ms fade in
            envelope = (double)i / (sample_rate / 100);
        else if (i > samples - sample_rate / 100) // 10ms fade out
            envelope = (double)(samples - i) / (sample_rate / 100);

        buffer[i] = (Sint16)(32767 * 0.3 * envelope * sin(2.0 * M_PI * frequency * t));
    }

    // Save as WAV
    SDL_RWops *file = SDL_RWFromFile(filename, "wb");
    if (file)
    {
        // WAV header
        SDL_RWwrite(file, "RIFF", 4, 1);
        Uint32 chunk_size = 36 + samples * 2;
        SDL_RWwrite(file, &chunk_size, 4, 1);
        SDL_RWwrite(file, "WAVE", 4, 1);

        // fmt chunk
        SDL_RWwrite(file, "fmt ", 4, 1);
        Uint32 fmt_size = 16;
        SDL_RWwrite(file, &fmt_size, 4, 1);
        Uint16 audio_format = 1; // PCM
        SDL_RWwrite(file, &audio_format, 2, 1);
        Uint16 num_channels = 1;
        SDL_RWwrite(file, &num_channels, 2, 1);
        SDL_RWwrite(file, &sample_rate, 4, 1);
        Uint32 byte_rate = sample_rate * 2;
        SDL_RWwrite(file, &byte_rate, 4, 1);
        Uint16 block_align = 2;
        SDL_RWwrite(file, &block_align, 2, 1);
        Uint16 bits_per_sample = 16;
        SDL_RWwrite(file, &bits_per_sample, 2, 1);

        // data chunk
        SDL_RWwrite(file, "data", 4, 1);
        Uint32 data_size = samples * 2;
        SDL_RWwrite(file, &data_size, 4, 1);
        SDL_RWwrite(file, buffer, samples * 2, 1);

        SDL_RWclose(file);
    }

    free(buffer);
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    // Initialize PSP controls
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
        return 1;

    if (TTF_Init() < 0)
    {
        SDL_Quit();
        return 1;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0)
    {
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "Audio Demo",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        0);

    if (!win)
    {
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, 0);
    if (!renderer)
    {
        SDL_DestroyWindow(win);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("Orbitron-Regular.ttf", 18);
    if (!font)
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color green = {0, 255, 0, 255};

    // Generate sound files
    generateBeepSound("beep.wav", 440, 200);    // A4 note
    generateBeepSound("beep2.wav", 880, 150);   // A5 note
    generateBeepSound("music.wav", 523, 1000);  // C5 note (longer for "music")

    // Load sounds
    Mix_Chunk *beep1 = Mix_LoadWAV("beep.wav");
    Mix_Chunk *beep2 = Mix_LoadWAV("beep2.wav");
    Mix_Music *music = Mix_LoadMUS("music.wav");

    // UI text
    Text title = createText(renderer, font, white, "PSP Audio Demo");
    Text line1 = createText(renderer, font, green, "X - Play Beep 1 (440 Hz)");
    Text line2 = createText(renderer, font, green, "O - Play Beep 2 (880 Hz)");
    Text line3 = createText(renderer, font, green, "[] - Play/Pause Music");
    Text line4 = createText(renderer, font, green, "^ - Stop Music");
    Text line5 = createText(renderer, font, green, "L/R - Volume Down/Up");
    Text line6 = createText(renderer, font, green, "START - Quit");

    char status_buffer[64];
    Text status_text = {NULL, 0, 0};

    SceCtrlData pad;
    int running = 1;
    int music_playing = 0;
    int volume = MIX_MAX_VOLUME / 2;
    int button_pressed = 0;

    Mix_VolumeMusic(volume);
    Mix_Volume(-1, volume);

    while (running)
    {
        sceCtrlReadBufferPositive(&pad, 1);

        if (pad.Buttons)
        {
            if (!button_pressed)
            {
                if (pad.Buttons & PSP_CTRL_START)
                {
                    running = 0;
                }
                else if (pad.Buttons & PSP_CTRL_CROSS)
                {
                    if (beep1)
                        Mix_PlayChannel(-1, beep1, 0);
                }
                else if (pad.Buttons & PSP_CTRL_CIRCLE)
                {
                    if (beep2)
                        Mix_PlayChannel(-1, beep2, 0);
                }
                else if (pad.Buttons & PSP_CTRL_SQUARE)
                {
                    if (music)
                    {
                        if (music_playing)
                        {
                            if (Mix_PausedMusic())
                            {
                                Mix_ResumeMusic();
                            }
                            else
                            {
                                Mix_PauseMusic();
                            }
                        }
                        else
                        {
                            Mix_PlayMusic(music, -1); // Loop forever
                            music_playing = 1;
                        }
                    }
                }
                else if (pad.Buttons & PSP_CTRL_TRIANGLE)
                {
                    Mix_HaltMusic();
                    music_playing = 0;
                }
                else if (pad.Buttons & PSP_CTRL_LTRIGGER)
                {
                    volume -= 16;
                    if (volume < 0)
                        volume = 0;
                    Mix_VolumeMusic(volume);
                    Mix_Volume(-1, volume);
                }
                else if (pad.Buttons & PSP_CTRL_RTRIGGER)
                {
                    volume += 16;
                    if (volume > MIX_MAX_VOLUME)
                        volume = MIX_MAX_VOLUME;
                    Mix_VolumeMusic(volume);
                    Mix_Volume(-1, volume);
                }

                button_pressed = 1;
            }
        }
        else
        {
            button_pressed = 0;
        }

        // Update status
        const char *music_status = "Stopped";
        if (Mix_PlayingMusic())
        {
            if (Mix_PausedMusic())
                music_status = "Paused";
            else
                music_status = "Playing";
        }

        snprintf(status_buffer, sizeof(status_buffer),
                 "Music: %s | Volume: %d%%",
                 music_status,
                 (volume * 100) / MIX_MAX_VOLUME);

        freeText(&status_text);
        status_text = createText(renderer, font, white, status_buffer);

        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 50, 255);
        SDL_RenderClear(renderer);

        drawText(renderer, &title, 10, 10);
        drawText(renderer, &line1, 20, 50);
        drawText(renderer, &line2, 20, 75);
        drawText(renderer, &line3, 20, 100);
        drawText(renderer, &line4, 20, 125);
        drawText(renderer, &line5, 20, 150);
        drawText(renderer, &line6, 20, 175);
        drawText(renderer, &status_text, 10, 220);

        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ~60 FPS
    }

    // Cleanup
    freeText(&title);
    freeText(&line1);
    freeText(&line2);
    freeText(&line3);
    freeText(&line4);
    freeText(&line5);
    freeText(&line6);
    freeText(&status_text);

    if (beep1)
        Mix_FreeChunk(beep1);
    if (beep2)
        Mix_FreeChunk(beep2);
    if (music)
        Mix_FreeMusic(music);

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
    sceKernelExitGame();

    return 0;
}
