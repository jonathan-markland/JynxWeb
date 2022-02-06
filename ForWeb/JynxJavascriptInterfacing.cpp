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

#include "JynxJavascriptInterfacing.h"
#include "../Portable/LynxHardware/LynxComputer.h"
#include "../Portable/LynxHardware/LynxScreen.h"

static Jynx::LynxComputer *g_LynxComputerSingletonInstance = nullptr;

static float g_SilenceBuffer[128];  // TODO: remove this ultimately.  It will be in an object, like _lynxScreen


extern "C" void CreateJynxEmulatorSingleton()
{
	if (g_LynxComputerSingletonInstance == nullptr)
	{
		g_LynxComputerSingletonInstance = new Jynx::LynxComputer();
		
		for (int i=0; i<128; i++)
		{
			g_SilenceBuffer[i] = 0.0;  // TODO: no sound for now.  Remove this ultimately.
		}
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

extern "C" void RunTimeslice()
{
	g_LynxComputerSingletonInstance->OnTimeSlice();
}

extern "C" void ResetGuest()
{
	g_LynxComputerSingletonInstance->OnHardwareReset();
}

extern "C" float *GetSingletonSoundBufferBaseAddress()
{
	return g_SilenceBuffer;  // TODO: no sound for now.
}
