#include <iostream>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_timer.h"

#include "../../Portable/LynxHardware/LynxComputer.h"

// TODO: I linked SDL_Image anyway, which we might not need.  Need to decide what SDL2 binaries we really need.




// ------------------------------------------------------------------------------------------------------------
//   CONSTANTS
// ------------------------------------------------------------------------------------------------------------

const int GUEST_SCREEN_WIDTH = 256;         // If changed, must also change the C++ #define of the same name.
const int GUEST_SCREEN_HEIGHT = 256;        // If changed, must also change the C++ #define of the same name.
const int INV_ROWS = 32;                    // The number of entries in globalWasmRowDirtyCountersArray
const int SOUND_BUFFER_SAMPLE_COUNT = 128;  // As fixed by the browser, so using this on the Desktop too.


// ------------------------------------------------------------------------------------------------------------
//   SOUND THREAD DATA  (THE EMULATION ITSELF)
// ------------------------------------------------------------------------------------------------------------

struct SoundThreadData
{
    SoundThreadData()
    {
        lynxComputer = JynxFramework::MakeNew<Jynx::LynxComputer>();
    }

    JynxFramework::Pointer<Jynx::LynxComputer> lynxComputer;
};




// ------------------------------------------------------------------------------------------------------------
//   AUDIO CALLBACK
// ------------------------------------------------------------------------------------------------------------

void AudioCallback(void* userdata, Uint8* stream, int len)
{
    /*
        userdata
        an application-specific parameter saved in the SDL_AudioSpec structure's userdata field

        stream
        a pointer to the audio data buffer filled in by SDL_AudioCallback()

        len
        the length of that buffer in bytes
    */

    if (len == (SOUND_BUFFER_SAMPLE_COUNT * sizeof(float)))
    {
        auto &soundThreadData = *(SoundThreadData*) userdata;

        // Advance the emulation, and we copy the sound buffer into the SDL's buffer.  TODO: Enhance the interface to allow guest to generate into the SDL's buffer.

        soundThreadData.lynxComputer->OnTimeSlice();

        auto sourceSoundBuffer = soundThreadData.lynxComputer->GetSoundBufferBaseAddress();

        JynxFramework::RawBlockCopy((float*)sourceSoundBuffer, (float*)stream, SOUND_BUFFER_SAMPLE_COUNT);  // TODO: hacking off the volatile, it shouldn't be there in the design!
    }
}



// ------------------------------------------------------------------------------------------------------------
//   50Hz TIMER CALLBACK
// ------------------------------------------------------------------------------------------------------------

Uint32 __cdecl TimerCallback(Uint32 interval, void* param)
{
    SDL_Event event;

    event.type = SDL_EventType::SDL_USEREVENT;
    event.user.code  = 0;
    event.user.data1 = 0;
    event.user.data2 = 0;

    SDL_PushEvent(&event);

    return interval;  // We can return 0 to cancel the timer here, or interval to keep it going.
}




// ------------------------------------------------------------------------------------------------------------
//   MAIN
// ------------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    //
    // Create emulator instance
    //

    JynxZ80::Z80::InitialiseGlobalTables();
    SoundThreadData soundThreadData;

    //
    // Init SDL
    //

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }

    //
    // Create window
    //

    auto win = 
        SDL_CreateWindow(
            "Jynx II - Web and Desktop",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            GUEST_SCREEN_WIDTH * 2, GUEST_SCREEN_HEIGHT * 2, 0);  // TODO: sort out dimensions

    Uint32 render_flags = 
        SDL_RENDERER_ACCELERATED;

    auto rend = 
        SDL_CreateRenderer(win, -1, render_flags);

    // auto guestScreenSurface =
    //     SDL_CreateRGBSurfaceWithFormat(0, GUEST_SCREEN_WIDTH, GUEST_SCREEN_HEIGHT, 32, SDL_PIXELFORMAT_ARGB8888);

    auto guestScreenTexture = 
        SDL_CreateTexture(rend, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, GUEST_SCREEN_WIDTH, GUEST_SCREEN_HEIGHT);

    //
    // 50Hz timer
    // 20ms timer installed so that the main event loop receives 'SDL.SDL_EventType.SDL_USEREVENT' every 20ms (1/50th second)
    //

    auto timerID =
        SDL_AddTimer(20, TimerCallback, 0);

    if (timerID == 0)
    {
        printf("Failed to install the gameplay timer.");
        return 1; // TODO: close down
    }
    

    //
    // Audio
    //

    SDL_AudioSpec want;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = 44100;
    want.format = AUDIO_F32;
    want.channels = 1;
    want.samples = 128;
    want.callback = AudioCallback;
    want.userdata = &soundThreadData;

    SDL_AudioSpec have;

    auto audioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0); // SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (audioDevice == 0)
    {
        printf("SDL_OpenAudioDevice() failed");
        return 1;
    }

    SDL_PauseAudioDevice(audioDevice, 0); // start audio playing.

    // -----------------------------------------------------------------------------------

    bool quit = false;

    while (!quit) 
    {
        SDL_Event event;

        // Events management
        while (SDL_PollEvent(&event) != 0 && !quit) 
        {
            switch (event.type)
            {
            case SDL_QUIT:
                quit = true;
                break;

            case SDL_KEYDOWN:
                // HandleKeyDownEvent(event.key.keysym.scancode, lynxComputer);
                break;

            case SDL_KEYUP:
                // HandleKeyUpEvent(event.key.keysym.scancode, lynxComputer);
                break;

            case SDL_USEREVENT:
                if (JynxFramework::IsInPanicState())
                {
                    printf("Jynx framework panic: %s", *JynxFramework::GetPanicMessagePointerAddress());
                    quit = true;
                }
                else
                {
                    // RefreshScreen(rend, lynxComputer);
                }
                break;
            }
        }

        /*
        // clears the screen
        SDL_RenderClear(rend);
        // SDL_RenderCopy(rend, tex, NULL, &dest);

        // triggers the double buffers
        // for multiple rendering
        SDL_RenderPresent(rend);

        // calculates to 60 fps
        SDL_Delay(1000 / 60);
        */
    }

    SDL_CloseAudioDevice(audioDevice);
    // SDL_FreeSurface(guestScreenSurface);
    SDL_DestroyTexture(guestScreenTexture);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
