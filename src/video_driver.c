#include <SDL2/SDL.h>
#include <stdbool.h>

#include "emulator8080.h"
#include "video_driver.h"

#define SCREEN_OFFSET 0x2400
#define SCREEN_W_ORIG 256
#define SCREEN_H_ORIG 224
#define SCREEN_W SCREEN_H_ORIG
#define SCREEN_H SCREEN_W_ORIG

static SDL_Window *win;
static SDL_Renderer *ren;
static SDL_Texture *tex;

static SDL_Event evt;

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



void video_init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return;
    }

    win = SDL_CreateWindow("Intel 8080 Emulator",
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     SCREEN_W,
                                     SCREEN_H,
                                     0);

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
}

bool video_exec(state8080 *state)
{
    while (SDL_PollEvent(&evt) > 0) {
        if (evt.type == SDL_QUIT) {
            return true;
        } else if (evt.type == SDL_KEYDOWN) {
            switch (evt.key.keysym.sym) {
                case SDLK_LEFT:
                    printf("Left\n");
                    port1 |= 0x20;
                    break;
                case SDLK_RIGHT:
                    printf("rIGGHT\n");
                    port1 |= 0x40;
                    break;
                case SDLK_SPACE:
                    printf("Space bar\n");
                    port1 |= 0x10;
                    break;
                case SDLK_c:
                    printf("C\n");
                    port1 |= 0x01;
                    break;
                case SDLK_RETURN:
                    printf("Enter\n");
                    port1 |= 0x04;
                    break;
            }
        } else if (evt.type == SDL_KEYUP) {
            switch (evt.key.keysym.sym) {
                case SDLK_LEFT:
                    printf("Left Up\n");
                    port1 &= 0xdf;
                    break;
                case SDLK_RIGHT:
                    printf("rIGGHT Up\n");
                    port1 &= 0xbf;
                    break;
                case SDLK_SPACE:
                    printf("Space bar Up\n");
                    port1 &= 0xef;
                    break;
                case SDLK_c:
                    printf("C Up\n");
                    port1 &= 0xfe;
                    break;
                case SDLK_RETURN:
                    printf("Enter\n");
                    port1 &= 0xfb;
                    break;
            }
        }
    }

    draw_screen(state->memory);
    return false;
}

void video_quit()
{
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}
