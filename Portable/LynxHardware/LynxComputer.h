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

#pragma once

#include <stdint.h>
#include "../Z80/JynxZ80.h"
#include "LynxAddressSpaceDecoder.h"

namespace Jynx
{
	class LynxComputer
	{
	public:
	
		LynxComputer();
		
		void OnHardwareReset();
		void OnTimeSlice();
		
		uint32_t *GetScreenBitmapBaseAddress();
		
	private:
	
		enum { LynxZ80ClockSpeedHz = 4000000 };

		JynxZ80::Z80   _processor;          // Z80 + Registers
		uint64_t       _z80CycleCounter;    // Total cycle counter // TODO: serialise -- but only the cassette creation relies on it, and we don't & can't easily serialise the state of that.

		LynxAddressSpaceDecoder _addressSpace;
	
	};
}
