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

// - See Z80ExternalImplementation.h for user tasks.
//
// - This was made for the Camputers Lynx emulator, but does NOT have Lynx-specific code!
//
// - The Lynx had almost no use for interrupts anyway, and no use at all for them in the
//   parts of the Lynx that I emulate!  So, the implementation of interrupts is
//   work-in-progress.  I will need to substitute into another oper-source emulator to
//   properly develop interrupts.  Sorry!
//
// - This is restricted to little-endian machines at present.  TheLoByte() and HiByte()
//   functions are responsible for this restriction.  I cannot test big-endian.
//

#pragma once

#include <stdint.h>

namespace JynxZ80
{
	// User-defined interface.  Handles Z80 events.
	class IZ80ExternalHandler
	{
	public:
		virtual void Z80_AddressWrite( uint16_t address, uint8_t dataByte ) = 0;
		virtual uint8_t Z80_AddressRead( uint16_t address ) = 0;
		virtual void Z80_IOSpaceWrite( uint16_t portNumber, uint8_t dataByte ) = 0;
		virtual uint8_t Z80_IOSpaceRead( uint16_t portNumber ) = 0;
		virtual void OnAboutToBranch() = 0;
		virtual void OnAboutToReturn() = 0;
	};

} // end namespace
