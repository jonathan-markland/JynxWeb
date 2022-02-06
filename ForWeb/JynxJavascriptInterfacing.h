//
// JynxZ80 - Jonathan's Z80 Emulator - Initially for Camputers Lynx Emulation Project.
// Copyright (C) 2014  Jonathan Markland
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//		jynx_emulator {at} yahoo {dot} com
//

#pragma once

#include <stdint.h>


		// TODO:  Do Reset with a volatile flag the host sets and the guest clears.


// Obtains address of a pointer that will be null if no panic, otherwise
// will be the address of the panic text nul-terminated string, when the
// program is in the panic infinite loop.
extern "C" volatile const char **GetPanicMessagePointerAddress();

// This needs to be called before the compiler-generated __wasm_call_ctors()
extern "C" void InitBeforeCtorsCalled();

// Get guest screen width.  This is a runtime constant.
extern "C" int32_t GetGuestScreenWidthPixels();

// Get guest screen height.  This is a runtime constant.
extern "C" int32_t GetGuestScreenHeightPixels();

// Ensure the emulator singleton is created.
// Must be done before functions below are called.
extern "C" void CreateJynxEmulatorSingleton();

// Once the singleton is created, the address of the host
// screen bitmap can be obtained.  This is 32-bit RGBA format
// and has dimensions given by the GetGuestScreenWidth/HeightPixels functions.
extern "C" uint32_t *GetSingletonScreenBitmapBaseAddress();

// Once the singleton is created, obtains the address of the
// sound buffer.  This is an array of 128 x 32-bit floats, per the
// web browser Audio Worklet specification.
extern "C" float *GetSingletonSoundBufferBaseAddress();

// Once the singleton is created, we can advance the state of
// the system by 1 x timeslice.  This is the amount of time required
// by the Javacsript Audio Worklet handler (128 x PCM samples of time).
extern "C" void RunTimeslice();

// Once the singleton is created, we can find the key code translation table.
extern "C" int GetWebBrowserKeycodeTranslationTableSize();

// Once the singleton is created, we can find the key code translation table.
extern "C" uint8_t *GetWebBrowserKeycodeTranslationTableAddress();

// Translate by taking the web browser key code, lookup up in the translation
// table given by GetWebBrowserKeycodeTranslationTableAddress(), then use 
// the found-index as as a bit-index into the following array ONLY if the 
// key code was found:
extern "C" volatile uint8_t *GetLynxKeyboardArrayAddress();

