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
	class LynxROMsAndRAMs
	{
	public:
	
		LynxROMsAndRAMs();

		LynxROMsAndRAMs(const LynxROMsAndRAMs &) = delete;
		void operator=(const LynxROMsAndRAMs &) = delete;

		// Set machine type.  Wipes RAM to zeroes, and copies in the appropriate ROMs.
		void SetMachineType( LynxMachineType::Enum machineType );
		
		// Wipes RAM to zeroes, and copies in the appropriate ROMs for current machine type.
		void OnHardwareReset();

		inline CHIP *GetROM_0000()   { return &_lynxROM_0000; }
		inline CHIP *GetROM_2000()   { return &_lynxROM_2000; }
		inline CHIP *GetROM_4000()   { return &_lynxROM_4000; }
		
		inline CHIP *GetRAM_0000()   { return &_lynxRAM_0000; }
		inline CHIP *GetRAM_2000()   { return &_lynxRAM_2000; }
		inline CHIP *GetRAM_4000()   { return &_lynxRAM_4000; }
		inline CHIP *GetRAM_6000()   { return &_lynxRAM_6000; }
		inline CHIP *GetRAM_8000()   { return &_lynxRAM_8000; }
		inline CHIP *GetRAM_A000()   { return &_lynxRAM_A000; }
		inline CHIP *GetRAM_C000()   { return &_lynxRAM_C000; }
		inline CHIP *GetRAM_E000()   { return &_lynxRAM_E000; }
		
	private:
	
		// Machine type being emulated

		LynxMachineType::Enum  _machineType;
	
		//
		// The Lynx's chip set  (8K ROMs/RAMs)
		//

		///
		/// Lynx's Bank #0
		///
		/// Bank 0 contains the ROMs and is a special case.
		/// When enabled, this overlays everything else for READS
		/// between 0000..5FFF and E000..FFFF.
		///

		CHIP            _lynxROM_0000;         // 0000 .. 1FFF
		CHIP            _lynxROM_2000;         // 2000 .. 3FFF
		CHIP            _lynxROM_4000;         // 4000 .. 5FFF   (LYNX 96K) Basic extension / Scorpion ROM -- not on 48K machine.
											   // E000 .. FFFF   (Any extension rom -- not used on this emulator, default address decoding returns 0xFF).

		///
		/// Lynx's bank #1   ("User RAM")
		///
		/// Bank 1 contains the user RAM (read / write)
		///

		CHIP            _lynxRAM_0000;    // 0000 .. 1FFF  (LYNX 96K only)
		CHIP            _lynxRAM_2000;    // 2000 .. 3FFF  (LYNX 96K only)
		CHIP            _lynxRAM_4000;    // 4000 .. 5FFF  (LYNX 96K only)
		CHIP            _lynxRAM_6000;    // 6000 .. 7FFF  (LYNX 48K and 96K)
		CHIP            _lynxRAM_8000;    // 8000 .. 9FFF  (LYNX 48K and 96K)
		CHIP            _lynxRAM_A000;    // A000 .. BFFF  (LYNX 96K only)
		CHIP            _lynxRAM_C000;    // C000 .. DFFF  (LYNX 96K only)
		CHIP            _lynxRAM_E000;    // E000 .. FFFF  (LYNX 96K only)

	};
}