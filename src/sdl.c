#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>

#include "SDL2/SDL_audio.h"
#include "SDL2/SDL_video.h"
#include "emulator8080.h"
#include "sdl.h"

#define SCREEN_OFFSET 0x2400
#define SCREEN_W_ORIG 256
#define SCREEN_H_ORIG 224
#define SCREEN_W SCREEN_H_ORIG
#define SCREEN_H SCREEN_W_ORIG
#define SDL_PORT 0x01

static SDL_Window *win;
static SDL_Renderer *ren;
static SDL_Texture *tex;
static SDL_Event evt;

#define NUM_WAV 9
Mix_Chunk *wav_buf[NUM_WAV];
uint32_t wav_len[NUM_WAV];
SDL_AudioDeviceID dev_id;

static void draw_screen(uint8_t *mem)
{
    int pitch;
    uint8_t *pixels;
    SDL_LockTexture(tex, NULL, (void **) &pixels, &pitch);

    for (uint32_t lin = 0; lin < SCREEN_H; ++lin) {
        for (uint32_t col = 0; col < SCREEN_W; ++ col) {
            uint32_t byte = col * (SCREEN_W_ORIG >> 3) + ((SCREEN_W_ORIG - lin) >> 3);
            uint8_t bit_off = ~lin & 0x07;
            uint8_t bit = (mem[SCREEN_OFFSET + byte] >> bit_off) & 0x01;
            uint8_t normalized_val = (uint8_t) (-bit & 0xff);

            uint32_t tex_coordinate = (col << 2) + lin * (uint32_t) pitch;
            pixels[tex_coordinate] = normalized_val;     // b
            pixels[tex_coordinate + 1] = normalized_val; // g
            pixels[tex_coordinate + 2] = normalized_val; // r
        }
    }

    SDL_UnlockTexture(tex);
    SDL_RenderCopy(ren, tex, NULL, NULL);
    SDL_RenderPresent(ren);
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
                            SDL_PIXELFORMAT_RGB888,
                            SDL_TEXTUREACCESS_STREAMING,
                            SCREEN_W,
                            SCREEN_H
                            );

    if (!tex) {
        SDL_Log("Unable to create texture: %s", SDL_GetError());
        return;
    }

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
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    sound_quit();
    SDL_Quit();
}
