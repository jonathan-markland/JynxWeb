#include <iostream>

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_timer.h"


void MyAudioCallback(void *userdata, Uint8 *stream, int len)
{
    /*
        userdata
        an application-specific parameter saved in the SDL_AudioSpec structure's userdata field

        stream
        a pointer to the audio data buffer filled in by SDL_AudioCallback()

        len
        the length of that buffer in bytes
    */

    auto floatStream = (float*)stream;

    int floatCount = len / sizeof(float);

    auto halfWay   = floatStream + (floatCount / 2);
    auto endBuffer = floatStream + floatCount;

    for (auto p = floatStream; p < halfWay; p++)
    {
        *p = -0.3F;
    }
    for (auto p = halfWay; p < endBuffer; p++)
    {
        *p = +0.3F;
    }
}



const int WindowWidth = 500;
const int WindowHeight = 500;



int main(int argc, char* argv[])
{
    // returns zero on success else non-zero
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow("GAME", // creates a window
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WindowWidth, WindowHeight, 0);

    // triggers the program that controls
    // your graphics hardware and sets flags
    Uint32 render_flags = SDL_RENDERER_ACCELERATED;

    // creates a renderer to render our images
    SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags);

    // creates a surface to load an image into the main memory
    SDL_Surface* surface;

    // please provide a path for your image
    surface = IMG_Load("Anubis.png");

    // loads image to our graphics hardware memory.
    SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surface);

    // clears main-memory
    SDL_FreeSurface(surface);

    // let us control our image position
    // so that we can move it with our keyboard.
    SDL_Rect dest;

    // connects our texture with dest to control position
    SDL_QueryTexture(tex, NULL, NULL, &dest.w, &dest.h);

    // adjust height and width of our image box.
    dest.w /= 2;
    dest.h /= 2;

    // sets initial x-position of object
    dest.x = (WindowWidth - dest.w) / 2;

    // sets initial y-position of object
    dest.y = (WindowHeight - dest.h) / 2;

    // controls animation loop
    int close = 0;

    // speed of box
    int speed = 300;

    // -----------------------------------------------------------------------------------

    SDL_AudioSpec want;
    SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
    want.freq = 48000;
    want.format = AUDIO_F32;
    want.channels = 2;
    want.samples = 512;
    want.callback = MyAudioCallback; /* you wrote this function elsewhere -- see SDL_AudioSpec for details */

    SDL_AudioSpec have;

    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (dev != 0) 
    {
        if (have.format != want.format) 
        {
            /* we let this one thing change. */
            printf("We didn't get Float32 audio format.");
            return 1;
        }

        SDL_PauseAudioDevice(dev, 0); /* start audio playing. */
        SDL_Delay(5000); /* let the audio callback play some sound for 5 seconds. */
        SDL_CloseAudioDevice(dev);
    }
    else
    {
        printf("SDL_OpenAudioDevice() failed");
        return 1;
    }

    // -----------------------------------------------------------------------------------

    // animation loop
    while (!close) {
        SDL_Event event;

        // Events management
        while (SDL_PollEvent(&event)) {
            switch (event.type) {

            case SDL_QUIT:
                // handling of close button
                close = 1;
                break;

            case SDL_KEYDOWN:
                // keyboard API for key pressed
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    dest.y -= speed / 30;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    dest.x -= speed / 30;
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    dest.y += speed / 30;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    dest.x += speed / 30;
                    break;
                default:
                    break;
                }
            }
        }

        // right boundary
        if (dest.x + dest.w > 1000)
            dest.x = WindowWidth - dest.w;

        // left boundary
        if (dest.x < 0)
            dest.x = 0;

        // bottom boundary
        if (dest.y + dest.h > 1000)
            dest.y = WindowHeight - dest.h;

        // upper boundary
        if (dest.y < 0)
            dest.y = 0;

        // clears the screen
        SDL_RenderClear(rend);
        SDL_RenderCopy(rend, tex, NULL, &dest);

        // triggers the double buffers
        // for multiple rendering
        SDL_RenderPresent(rend);

        // calculates to 60 fps
        SDL_Delay(1000 / 60);
    }

    // destroy texture
    SDL_DestroyTexture(tex);

    // destroy renderer
    SDL_DestroyRenderer(rend);

    // destroy window
    SDL_DestroyWindow(win);

    // close SDL
    SDL_Quit();

    return 0;
}
