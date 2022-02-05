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

namespace Jynx
{
	class LynxAddressSpaceDecoder
	{
	public:
	

	private:
	
		//
		// BANK SWITCHING SELECTORS.
		//
		// - The following arrays select what the Z80 sees in the address space.
		//
		//   Index [0] is 0000 .. 1FFF
		//   Index [1] is 2000 .. 3FFF
		//   Index [2] is 4000 .. 5FFF
		//   Index [3] is 6000 .. 7FFF
		//   Index [4] is 8000 .. 9FFF
		//   Index [5] is A000 .. BFFF
		//   Index [6] is C000 .. DFFF
		//   Index [7] is E000 .. FFFF
		//
		// - An address-space READ will read from one CHIP,
		//   or return 0xFF if the array holds a NULL pointer.
		//
		// - An address-space WRITE will consider all three
		//   destinations, and write to the CHIPs referred to
		//   if the pointers are non-NULL.
		//
		// The code *never* lets the ROMs appear in the WRITE arrays!
		//

		ADDRESS_SPACE _addressSpaceREAD;
		ADDRESS_SPACE _addressSpaceWRITE1; // First place to write to
		ADDRESS_SPACE _addressSpaceWRITE2; // Additional place to write to if non-NULL
		ADDRESS_SPACE _addressSpaceWRITE3; // Additional place to write to if non-NULL
	
	};
}
