#include <iostream>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_timer.h"

// TODO: I linked SDL_Image anyway, which we might not need.  Need to decide what SDL2 binaries we really need.



// ------------------------------------------------------------------------------------------------------------
//   CONSTANTS
// ------------------------------------------------------------------------------------------------------------

const int GUEST_SCREEN_WIDTH = 256;    // If changed, must also change the C++ #define of the same name.
const int GUEST_SCREEN_HEIGHT = 256;   // If changed, must also change the C++ #define of the same name.
const int INV_ROWS = 32;               // The number of entries in globalWasmRowDirtyCountersArray




// ------------------------------------------------------------------------------------------------------------
//   MAIN
// ------------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }

    //
    // Create window
    //

    auto win = SDL_CreateWindow(
        "Jynx II - Web and Desktop",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        GUEST_SCREEN_WIDTH * 2, GUEST_SCREEN_HEIGHT * 2, 0);  // TODO: sort out dimensions

    Uint32 render_flags = SDL_RENDERER_ACCELERATED;

    auto rend = SDL_CreateRenderer(win, -1, render_flags);

    //
    // Audio
    //
    /*
    SDL_AudioSpec want;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = 48000;
    want.format = AUDIO_F32;
    want.channels = 2;
    want.samples = 512;
    want.callback = MyAudioCallback;

    SDL_AudioSpec have;

    auto dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (dev != 0)
    {
        if (have.format != want.format)
        {
            // we let this one thing change.
            printf("We didn't get Float32 audio format.");
            return 1;
        }

        SDL_PauseAudioDevice(dev, 0); // start audio playing.
        SDL_Delay(5000); // let the audio callback play some sound for 5 seconds. 
        SDL_CloseAudioDevice(dev);
    }
    else
    {
        printf("SDL_OpenAudioDevice() failed");
        return 1;
    }
    */

    // -----------------------------------------------------------------------------------

    bool quit = false;

    while (!quit) 
    {
        SDL_Event event;

        // Events management
        while (SDL_PollEvent(&event)) 
        {
            switch (event.type) 
            {
            case SDL_QUIT:
                quit = true;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode) 
                {
                case SDL_SCANCODE_W:
                default:
                    break;
                }
            }
        }

        // clears the screen
        SDL_RenderClear(rend);
        // SDL_RenderCopy(rend, tex, NULL, &dest);

        // triggers the double buffers
        // for multiple rendering
        SDL_RenderPresent(rend);

        // calculates to 60 fps
        SDL_Delay(1000 / 60);
    }

    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
