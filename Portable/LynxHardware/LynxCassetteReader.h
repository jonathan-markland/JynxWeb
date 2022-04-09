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

		// *** TODO: Want a TAP -> PCM wave tool ***

#pragma once

#include <stdint.h>
#include "../../ExternalModules/JynxFrameworkLibrary/JynxFramework.h"

namespace Jynx
{
	class LynxCassetteReader
	{
	public:
	
		LynxCassetteReader();

		void OnHardwareReset();

		uint8_t ReadCurrentBit();
		
		// The static tap file image must have an additional NUL terminator appended
		// after the on-disc file data.
		void SetStaticTapeImageData(const uint8_t *tapFileImage, const uint8_t *tapFileImageEnd);
		
		// TODO: What about motor on/off messages?
		
	private:

		uint32_t _fileIndex;
		

	};
	
	// Need a function to:
	// Given
	//     - A TAP file image (possibly concatenations)
	//     - An offset from the start of the tape in Z80 cycles.
	// Returns
	//     - Wave level above|below the comparator line.
	
	
	
	
	
}
