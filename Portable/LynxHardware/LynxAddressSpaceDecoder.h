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

#include "../Z80/IZ80ExternalHandler.h"
#include "LynxHardwareCommon.h"
#include "LynxComputerInterface.h"
#include "LynxROMsAndRAMs.h"
#include "LynxScreen.h"
#include "LynxKeyboard.h"

namespace Jynx
{
	class LynxAddressSpaceDecoder: public JynxZ80::IZ80ExternalHandler
	{
	public:
	
		LynxAddressSpaceDecoder();
		
		uint32_t *GetScreenBitmapBaseAddress();
		volatile uint8_t *GetLynxKeyboardArrayAddress();
		
		void OnHardwareReset();
		
		// The Z80 calls this to write a byte to the address space.
		// Writes can hit multiple devices depending on which banks are active.
		virtual void Z80_AddressWrite( uint16_t address, uint8_t dataByte ) override;

		// The Z80 calls this to read a byte from the address space.
		virtual uint8_t Z80_AddressRead( uint16_t address ) override;

		// The Z80 calls this to write a byte to the I/O space.
		virtual void Z80_IOSpaceWrite( uint16_t portNumber, uint8_t dataByte ) override;

		// The Z80 calls this to read from the I/O space.
		// The cassette and keyboard are primarily readable via the I/O space.
		virtual uint8_t Z80_IOSpaceRead( uint16_t portNumber ) override;

		virtual void OnAboutToBranch() override; //TODO
		virtual void OnAboutToReturn() override; //TODO
		
		void RecomposeWholeHostScreenRGBAsIfPending();

	private:
	
		void SyncAddressSpaceFromPorts();

	private:

		LynxMachineType::Enum _machineType;

		uint8_t          _devicePort;         // Port '0x80' see DEVICEPORT_ #defines
		uint8_t          _bankPort;           // Port '0xFFFF' see BANKPORT_ #defines

		LynxROMsAndRAMs  _memory;             // The ROM and program-RAM storage
		LynxScreen       _screen;             // The Screen RAM and associated host-format bitmap rendering
		LynxKeyboard     _keyboard;           // The LYNX keyboard ports array.
	
		//
		// ADDRESS SPACE
		//
		// - The following arrays select what the Z80 sees in the address space.
		// - There are four overlapping 64KB address spaces, enabled by the bank switching ports.
		//

		ADDRESS_SPACE _addressSpaceREAD;
		ADDRESS_SPACE _addressSpaceWRITE1; // First place to write to
		ADDRESS_SPACE _addressSpaceWRITE2; // Additional place to write to if non-NULL
		ADDRESS_SPACE _addressSpaceWRITE3; // Additional place to write to if non-NULL

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

		//
		// An ADDRESS_SPACE is described as an array of 8 x CHIPs,
		// occupying the following addresses:
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
	
	};
}
