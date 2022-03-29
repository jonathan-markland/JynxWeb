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



namespace Jynx
{

	// Run-length encoding datum for TAPE (square-wave) waveforms.
	// Bit 15 has the square wave level:  0=low / 1=high level.
	// Bits 14..0 have the duration in Z80 cycles.
	class SignalRLE
	{
	public:
		
		SignalRLE(uint16_t rawValue)           : Datum(rawValue) {}

		inline uint16_t Duration() const       { return Datum & 0x7FFF; }
		inline uint8_t  BitValue() const       { return Datum >> 15; }

		const uint16_t Datum;
	};

} // end namespace Jynx
