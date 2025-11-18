#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf(".png file required\n");
        return -1;
    }

    int w;
    int h;
    int channels;

    unsigned char *img;
    img = stbi_load(argv[1], &w, &h, &channels, 4);

    if (!img)
    {
        printf("Failed to load image\n");
        return 1;
    }

    int count;
    count = w * h;

    uint32_t *original_pixels;
    uint32_t *pixels;

    original_pixels = malloc(count * sizeof(uint32_t));
    pixels          = malloc(count * sizeof(uint32_t));

    int i;
    for (i = 0; i < count; i++)
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;

        r = img[i * 4 + 0];
        g = img[i * 4 + 1];
        b = img[i * 4 + 2];
        a = img[i * 4 + 3];

        // pixel bits      = RRRRRRRR    GGGGGGGG    BBBBBBBB   AAAAAAAA
        original_pixels[i] = (r << 24) | (g << 16) | (b << 8) | a;
    }

    stbi_image_free(img);

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    window = SDL_CreateWindow(
        "Image Processing",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        w,
        h,
        0
    );

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        w,
        h
    );

    int       running;
    int       mode;
    Uint32    last_switch;
    SDL_Event e;
    float     phase;
    float     speed;

    running     = 1;
    last_switch = SDL_GetTicks();
    mode        = 0;
    phase       = 0.0f;
    speed       = 0.5f; // controls how fast the cycle runs

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                running = 0;
            }
        }

        Uint32 now;
        float delta_phase;

        now = SDL_GetTicks();
        delta_phase = (now - last_switch) / 1000.0f * speed;

        phase = phase + delta_phase;
        last_switch = now;

        if (phase >= 3.0f)
        {
            phase = phase - 3.0f;
        }

        int seg;
        float t;

        seg = (int)phase;  // 0,1,2
        t   = phase - seg;

        if (now - last_switch > 1000)
        {
            mode = (mode + 1) % 3;
            last_switch = now;
        }

        for (i = 0; i < count; i++)
        {
            uint32_t src;
            src = original_pixels[i];

            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;

            // r = (src >> 24) & 0xFF;
            // g = (src >> 16) & 0xFF;
            // b = (src >> 8)  & 0xFF;
            // a =  src        & 0xFF;

            // uint8_t gray;

            // gray = (r + g + b) / 3;

            // pixels[i] = (gray << 24) | (gray << 16) | (gray << 8) | a;

            // uint32_t src;
            // src = original_pixels[i];

            // uint8_t r;
            // uint8_t g;
            // uint8_t b;
            // uint8_t a;

            r = (src >> 24) & 0xFF;
            g = (src >> 16) & 0xFF;
            b = (src >> 8)  & 0xFF;
            a =  src        & 0xFF;

            float mr; float mg; float mb;

            if (seg == 0)          // red → green
            {
                mr = 1.0f - t;
                mg = t;
                mb = 0.0f;
            }
            else if (seg == 1)     // green → blue
            {
                mr = 0.0f;
                mg = 1.0f - t;
                mb = t;
            }
            else                   // blue → red
            {
                mr = t;
                mg = 0.0f;
                mb = 1.0f - t;
            }

            uint8_t nr;
            uint8_t ng;
            uint8_t nb;

            nr = (uint8_t)(r * mr);
            ng = (uint8_t)(g * mg);
            nb = (uint8_t)(b * mb);

            pixels[i] = (nr << 24) | (ng << 16) | (nb << 8) | a;
        }

        SDL_UpdateTexture(texture, NULL, pixels, w * sizeof(uint32_t));

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    free(pixels);
    free(original_pixels);

    return 0;
}
