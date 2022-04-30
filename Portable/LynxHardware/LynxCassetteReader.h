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
#include "../../ExternalModules/JynxFrameworkLibrary/JynxFramework.h"
#include "../../ExternalModules/TapFileLibrary/TapeBitStreamSupplier.h"

namespace Jynx
{
	class LynxCassetteReader
	{
	public:
	
		LynxCassetteReader();
		void OnHardwareReset()                              {}
		uint8_t ReadCurrentBit(uint64_t z80CycleCountNow)   { return _tapeBitStreamSupplier.ReadBit(z80CycleCountNow); }
		void TapeMotorOn(uint32_t bitsPerSecond)            { _tapeBitStreamSupplier.TapeMotorOn(bitsPerSecond); }
		void TapeMotorOff()                                 { _tapeBitStreamSupplier.TapeMotorOff(); }
		
	private:

		TapeBitStreamSupplier _tapeBitStreamSupplier;

	};
}
