//
// Jynx - Jonathan's Lynx Emulator (Camputers Lynx 48K/96K models).
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

#include "../ExternalModules/JynxFrameworkLibrary/JynxFrameworkPanic.h"
#include "../ExternalModules/JynxFrameworkLibrary/WasmNeverFreeingMemoryAllocator.h"
#include "../Portable/Z80/JynxZ80.h"
#include "../Portable/LynxHardware/LynxComputer.h"
#include "../Portable/LynxHardware/LynxScreen.h"
#include "JynxJavascriptInterfacing.h"



extern "C" volatile const char **GetPanicMessagePointerAddress()
{
	return JynxFramework::GetPanicMessagePointerAddress();
}



extern "C" void InitBeforeCtorsCalled()
{
	// This needs to be called before the compiler-generated __wasm_call_ctors()
	
	WasmNeverFreeingMemoryAllocator::Init();
}



static Jynx::LynxComputer *g_LynxComputerSingletonInstance = nullptr;



extern "C" void CreateJynxEmulatorSingleton()
{
	if (g_LynxComputerSingletonInstance == nullptr)
	{
		JynxZ80::Z80::InitialiseGlobalTables();
		g_LynxComputerSingletonInstance = new Jynx::LynxComputer();
	}
}

extern "C" int32_t GetGuestScreenWidthPixels()
{
	return LYNX_FRAMEBUF_WIDTH;
}

extern "C" int32_t GetGuestScreenHeightPixels()
{
	return LYNX_FRAMEBUF_HEIGHT;
}

extern "C" uint32_t *GetSingletonScreenBitmapBaseAddress()
{
	return g_LynxComputerSingletonInstance->GetScreenBitmapBaseAddress();
}

extern "C" volatile uint8_t *GetSingletonRowDirtyCountersAddress()
{ 
	return g_LynxComputerSingletonInstance->GetRowDirtyCountersAddress();
}

extern "C" void RunTimeslice()
{
	g_LynxComputerSingletonInstance->OnTimeSlice();
}

extern "C" volatile float *GetSingletonSoundBufferBaseAddress()
{
	return g_LynxComputerSingletonInstance->GetSoundBufferBaseAddress();
}

#define NUMBER_OF_KEYS (8*11)

uint8_t  WebBrowserKeycodesLookupTable[NUMBER_OF_KEYS] =
{
	// Values are Web browser key codes.
	// Indices are bit-indexes into the Lynx Keyboard Array.
	16, 27,  40, 38,  20,   0,   0,  49,
	 0,  0,  67, 68,  88,  69,  52,  51,
	 0, 17,  65, 83,  90,  87,  81,  50,
	 0,  0,  70, 71,  86,  84,  82,  53,
	 0,  0,  66, 78,  32,  72,  89,  54,
	 0,  0,  74,  0,  77,  85,  56,  55,
	 0,  0,  75,  0, 188,  79,  73,  57,
	 0,  0, 186,  0, 190,  76,  80,  48,
	 0,  0, 187,  0, 191, 219, 222, 189,
	 0,  0,  39,  0,  13,  37, 221,   8,
};

extern "C" int GetWebBrowserKeycodeTranslationTableSize()
{
	return NUMBER_OF_KEYS;
}

extern "C" uint8_t *GetWebBrowserKeycodeTranslationTableAddress()
{
	return WebBrowserKeycodesLookupTable;
}

extern "C" volatile uint8_t *GetLynxKeyboardArrayAddress()
{
	return g_LynxComputerSingletonInstance->GetLynxKeyboardArrayAddress();
}
