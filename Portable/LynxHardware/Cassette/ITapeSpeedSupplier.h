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
	class ITapeSpeedSupplier
	{
	public:

		// Interface onto the party that will supply the TapFileReader with
		// the Lynx's current tape speed setting (TAPE command in BASIC).
		// For reliability, this needs to be done at the last moment, just
		// before creating the waveform.

		virtual uint32_t GetLynxTapeSpeedBitsPerSecond() = 0;
		virtual bool GetPauseAfterTapLoadEnable() = 0;
		virtual void SetPauseMode( bool pauseMode ) = 0;
	};

} // end namespace Jynx
