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
//   KEYBOARD
// ------------------------------------------------------------------------------------------------------------

#define NUMBER_OF_KEYS (8*11)



uint8_t  SdlKeycodesLookupTable[NUMBER_OF_KEYS] =
{
    // The Lynx has 10 x 8-bit registers that are connected to the keys.
    // The Lynx does not have the exact same keyboard that a PC has, so
    // we approximate the mappings of the Lynx keys to the PC.

    // - The values are Web browser key codes.
    // - Indices are bit-indexes into the Lynx Keyboard Array and registers

    //  SHIFT |  ESC  | DOWN  |  UP   | CAPS  |       |       |   1   |
    //        |       |   C   |   D   |   X   |   E   |   4   |   3   |
    //        |  CTRL |   A   |   S   |   Z   |   W   |   Q   |   2   |
    //        |       |   F   |   G   |   V   |   T   |   R   |   5   |
    //        |       |   B   |   N   | SPACE |   H   |   Y   |   6   |
    //        |       |   J   |       |   M   |   U   |   8   |   7   |
    //        |       |   K   |       |  , <  |   O   |   I   |   9   |
    //        |       |  ; :  |       |  . >  |   L   |   P   |   0   |
    //        |       |  = +  |       |  / ?  |  [ {  |  ' @  |  - _  |
    //        |       | RIGHT |       | ENTER | LEFT  |  ] }  | BKSPC |

    SDL_SCANCODE_LSHIFT , SDL_SCANCODE_ESCAPE , SDL_SCANCODE_DOWN      , SDL_SCANCODE_UP , SDL_SCANCODE_CAPSLOCK ,            0             ,        0                  , SDL_SCANCODE_1         ,
             0          ,         0           , SDL_SCANCODE_C         , SDL_SCANCODE_D  , SDL_SCANCODE_X        , SDL_SCANCODE_E           , SDL_SCANCODE_4            , SDL_SCANCODE_3         ,
             0          , SDL_SCANCODE_LCTRL  , SDL_SCANCODE_A         , SDL_SCANCODE_S  , SDL_SCANCODE_Z        , SDL_SCANCODE_W           , SDL_SCANCODE_Q            , SDL_SCANCODE_2         ,
             0          ,         0           , SDL_SCANCODE_F         , SDL_SCANCODE_G  , SDL_SCANCODE_V        , SDL_SCANCODE_T           , SDL_SCANCODE_R            , SDL_SCANCODE_5         ,
             0          ,         0           , SDL_SCANCODE_B         , SDL_SCANCODE_N  , SDL_SCANCODE_SPACE    , SDL_SCANCODE_H           , SDL_SCANCODE_Y            , SDL_SCANCODE_6         ,
             0          ,         0           , SDL_SCANCODE_J         ,       0         , SDL_SCANCODE_M        , SDL_SCANCODE_U           , SDL_SCANCODE_8            , SDL_SCANCODE_7         ,
             0          ,         0           , SDL_SCANCODE_K         ,       0         , SDL_SCANCODE_COMMA    , SDL_SCANCODE_O           , SDL_SCANCODE_I            , SDL_SCANCODE_9         ,
             0          ,         0           , SDL_SCANCODE_SEMICOLON ,       0         , SDL_SCANCODE_PERIOD   , SDL_SCANCODE_L           , SDL_SCANCODE_P            , SDL_SCANCODE_0         ,
             0          ,         0           , SDL_SCANCODE_EQUALS    ,       0         , SDL_SCANCODE_SLASH    , SDL_SCANCODE_LEFTBRACKET , SDL_SCANCODE_APOSTROPHE   , SDL_SCANCODE_MINUS     ,
             0          ,         0           , SDL_SCANCODE_RIGHT     ,       0         , SDL_SCANCODE_RETURN   , SDL_SCANCODE_LEFT        , SDL_SCANCODE_RIGHTBRACKET , SDL_SCANCODE_BACKSPACE
};



SDL_Scancode  CombineScancodes(SDL_Scancode sdlKeyCode)
{
    // (Since the table can only hold one keycode per slot)

    if (sdlKeyCode == SDL_SCANCODE_RSHIFT) return SDL_SCANCODE_LSHIFT;
    if (sdlKeyCode == SDL_SCANCODE_RCTRL) return SDL_SCANCODE_LCTRL;
    if (sdlKeyCode == SDL_SCANCODE_KP_ENTER) return SDL_SCANCODE_RETURN;
    return sdlKeyCode;
}



int SdlKeyCodeToLynxKeyIndex(SDL_Scancode sdlKeyCode)
{
    sdlKeyCode = CombineScancodes(sdlKeyCode);

    for (int i = 0; i < NUMBER_OF_KEYS; i++)
    {
        if (sdlKeyCode == SdlKeycodesLookupTable[i])
        {
            return i;
        }
    }

    return -1;
}



void HandleKeyDownEvent(SDL_Scancode sdlKeyCode, volatile uint8_t *sharedKeyboardRegisters)
{
    auto bitIndex = SdlKeyCodeToLynxKeyIndex(sdlKeyCode);
    if (bitIndex != -1)
    {
        auto portIndex = bitIndex >> 3;
        auto mask = 0x80 >> (bitIndex & 7);
        sharedKeyboardRegisters[portIndex] = (sharedKeyboardRegisters[portIndex] | mask) ^ mask;  // Clear bit in shared memory  (key Down -ve logic)
    }
}



void HandleKeyUpEvent(SDL_Scancode sdlKeyCode, volatile uint8_t* sharedKeyboardRegisters)
{
    auto bitIndex = SdlKeyCodeToLynxKeyIndex(sdlKeyCode);
    if (bitIndex != -1)
    {
        auto portIndex = bitIndex >> 3;
        auto mask = 0x80 >> (bitIndex & 7);
        sharedKeyboardRegisters[portIndex] = sharedKeyboardRegisters[portIndex] | mask;  // Set bit in shared memory (key Up -ve logic)
    }
}



// ------------------------------------------------------------------------------------------------------------
//   SCREEN
// ------------------------------------------------------------------------------------------------------------

void RefreshScreen(
    SDL_Renderer* rend, 
    SDL_Surface *guestScreenSurface,
    volatile uint8_t *sharedBandCounters,
    JynxFramework::Array<uint8_t> &bandCounters)
{
    // TODO:  We ignore the band flags detail altogether!
    // TODO:  Consider 6845 emulation, which will completely change Desktop and Web rendering anyway.

    bool changeDetectedAnywhere = false;

    for (int i = 0; i < INV_ROWS; i++)
    {
        auto n = sharedBandCounters[i];
        if (bandCounters[i] != n)
        {
            changeDetectedAnywhere = true;
            bandCounters[i] = n;
        }
    }

    if (changeDetectedAnywhere)
    {
        auto tex = SDL_CreateTextureFromSurface(rend, guestScreenSurface);
        SDL_RenderClear(rend);
        SDL_RenderCopy(rend, tex, NULL, NULL);
        SDL_RenderPresent(rend);
        SDL_DestroyTexture(tex);
    }
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
    SoundThreadData soundThreadData; // but we'll need the shared buffers from here.
    auto sharedKeyboardRegisters = soundThreadData.lynxComputer->GetLynxKeyboardArrayAddress();
    auto sharedDisplayBuffer = soundThreadData.lynxComputer->GetScreenBitmapBaseAddress();
    auto sharedBandCounters = soundThreadData.lynxComputer->GetRowDirtyCountersAddress();
    auto bandCounters = JynxFramework::ArrayInit<uint8_t>(INV_ROWS, 0);

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

    auto guestScreenSurface =
        SDL_CreateRGBSurfaceWithFormatFrom(sharedDisplayBuffer, GUEST_SCREEN_WIDTH, GUEST_SCREEN_HEIGHT, 32, GUEST_SCREEN_WIDTH * 4, SDL_PIXELFORMAT_ABGR8888);

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

    // 
    // Main loop
    //

    bool quit = false;

    while (!quit) 
    {
        SDL_Event event;

        while (SDL_WaitEvent(&event) != 0 && !quit)
        {
            switch (event.type)
            {
            case SDL_QUIT:
                quit = true;
                break;

            case SDL_KEYDOWN:
                HandleKeyDownEvent(event.key.keysym.scancode, sharedKeyboardRegisters);
                break;

            case SDL_KEYUP:
                HandleKeyUpEvent(event.key.keysym.scancode, sharedKeyboardRegisters);
                break;

            case SDL_USEREVENT:
                if (JynxFramework::IsInPanicState())
                {
                    printf("Jynx framework panic: %s", *JynxFramework::GetPanicMessagePointerAddress());
                    quit = true;
                }
                else
                {
                    RefreshScreen(rend, guestScreenSurface, sharedBandCounters, bandCounters);
                }
                break;
            }
        }
    }

    SDL_CloseAudioDevice(audioDevice);
    SDL_FreeSurface(guestScreenSurface);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
