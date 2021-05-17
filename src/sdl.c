#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

#include "SDL2/SDL_audio.h"
#include "SDL2/SDL_render.h"
#include "SDL2/SDL_surface.h"
#include "SDL2/SDL_video.h"
#include "emulator8080.h"
#include "sdl.h"

#define SCREEN_OFFSET 0x2400
#define SCREEN_W_ORIG 256
#define SCREEN_H_ORIG 224
#define GAME_RECT_W 872
#define GAME_RECT_H 1024
#define SCREEN_W 1000
#define SCREEN_H 1000
#define SDL_PORT 0x01

SDL_Window *win;
SDL_Renderer *ren;
SDL_Texture *tex;
SDL_Event evt;
SDL_Texture *background; 

#define NUM_WAV 9
Mix_Chunk *wav_buf[NUM_WAV];

static void draw_screen(uint8_t *mem)
{
    int pitch;
    uint8_t *pixels;
    
    SDL_RenderCopy(ren, background, NULL, NULL);

    SDL_LockTexture(tex, NULL, (void **) &pixels, &pitch);

    for (uint32_t lin = 0; lin < SCREEN_H_ORIG; ++lin) {
        for (uint32_t col = 0; col < SCREEN_W_ORIG; ++col) {
            uint32_t byte = lin * (SCREEN_W_ORIG >> 3) + (col >> 3);
            uint8_t bit_off = col & 0x07;
            uint8_t bit = (mem[SCREEN_OFFSET + byte] >> bit_off) & 0x01;
            uint8_t normalized_val = (uint8_t) (-bit & 0xff);

            uint32_t tex_coordinate = (col << 2) + lin * (uint32_t) pitch;
            pixels[tex_coordinate] = normalized_val;     // b
            pixels[tex_coordinate + 1] = normalized_val;     // b
            pixels[tex_coordinate + 2] = normalized_val; // g
            pixels[tex_coordinate + 3] = normalized_val; // r
        }
    }

    SDL_UnlockTexture(tex);

    SDL_Rect dst_rect = (SDL_Rect) {
        .x = 0,
        .y = GAME_RECT_H,
        .w = GAME_RECT_H,
        .h = GAME_RECT_W
    };

    SDL_Point orig = (SDL_Point) { .x = 0, .y = 0 };
    SDL_RenderCopyEx(ren, tex, NULL, &dst_rect, 270.0, &orig, SDL_FLIP_NONE);

    SDL_RenderPresent(ren);
}

static void video_init()
{
    win = SDL_CreateWindow("Intel 8080 Emulator",
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            SCREEN_W,
                            SCREEN_H,
                            SDL_WINDOW_ALLOW_HIGHDPI);

    if (!win) {
        fprintf(stderr, "Failed to create SDL window.\n");
        return;
    }

    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    
    if (!ren) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        return;
    }

    tex = SDL_CreateTexture(ren,
                            SDL_PIXELFORMAT_RGBA8888,
                            SDL_TEXTUREACCESS_STREAMING,
                            SCREEN_W_ORIG,
                            SCREEN_H_ORIG
                            );
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    if (!tex) {
        SDL_Log("Unable to create texture: %s", SDL_GetError());
        return;
    }


    IMG_Init(IMG_INIT_PNG);
    SDL_Surface *bg = IMG_Load("image/invaders.png");
    if (!bg) {
        SDL_Log("Unable to open image: %s\n", IMG_GetError());
    }

    background = SDL_CreateTextureFromSurface(ren, bg);

    SDL_FreeSurface(bg);

}

static void video_quit()
{
    SDL_DestroyTexture(tex);
    SDL_DestroyTexture(background);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    IMG_Quit();
}

static void sound_init()
{
    if (Mix_OpenAudioDevice(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024,
                        0, SDL_AUDIO_ALLOW_ANY_CHANGE) == -1) {

        fprintf(stderr, "Failed open audio device.\n");
    }

    if (Mix_AllocateChannels(NUM_WAV) < 0) {
        fprintf(stderr, "Failed to allocate audio channels.\n");
    }

    for (uint32_t i = 0; i < NUM_WAV; ++i) {
        char filename[12];
        sprintf(filename, "sound/%u.wav", i);
        wav_buf[i] = Mix_LoadWAV(filename);
    }
}

void stop_loop(uint8_t c)
{
    Mix_HaltChannel(c);
}

void play_sound(uint8_t c)
{
    Mix_PlayChannel(c, wav_buf[c], 0);
}

void loop_sound(uint8_t c)
{
    Mix_PlayChannel(c, wav_buf[c], -1);
}

static void sound_quit()
{
    for (uint32_t i = 0; i < NUM_WAV; ++i) {
        Mix_FreeChunk(wav_buf[i]);
    }
    Mix_CloseAudio();
}

void sdl_init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return;
    }

    video_init();
    sound_init();
}

bool sdl_exec(state8080 *state)
{
    while (SDL_PollEvent(&evt) > 0) {
        if (evt.type == SDL_QUIT) {
            return true;
        } else if (evt.type == SDL_KEYDOWN) {
            switch (evt.key.keysym.sym) {
                case SDLK_LEFT:
                    ports[SDL_PORT] |= 0x20;
                    break;
                case SDLK_RIGHT:
                    ports[SDL_PORT] |= 0x40;
                    break;
                case SDLK_SPACE:
                    ports[SDL_PORT] |= 0x10;
                    break;
                case SDLK_c:
                    ports[SDL_PORT] |= 0x01;
                    break;
                case SDLK_RETURN:
                    ports[SDL_PORT] |= 0x04;
                    break;
            }
        } else if (evt.type == SDL_KEYUP) {
            switch (evt.key.keysym.sym) {
                case SDLK_LEFT:
                    ports[SDL_PORT] &= 0xdf;
                    break;
                case SDLK_RIGHT:
                    ports[SDL_PORT] &= 0xbf;
                    break;
                case SDLK_SPACE:
                    ports[SDL_PORT] &= 0xef;
                    break;
                case SDLK_c:
                    ports[SDL_PORT] &= 0xfe;
                    break;
                case SDLK_RETURN:
                    ports[SDL_PORT] &= 0xfb;
                    break;
            }
        }
    }

    draw_screen(state->memory);

    return false;
}

void sdl_quit()
{
    video_quit();
    sound_quit();
    SDL_Quit();
}
