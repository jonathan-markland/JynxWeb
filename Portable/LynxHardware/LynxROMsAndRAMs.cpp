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

#include "../JynxFramework.h"
#include "LynxROMsAndRAMs.h"

// - The snapshot format permits the user to fiddle the ROM images,
//   these are the original ones, loaded by the constructor, then
//   used as sources when resetting the machine, or changing the type.

extern unsigned char RomImageLynx48Image1[8192];
extern unsigned char RomImageLynx48Image2[8192];
extern unsigned char RomImageLynx96Image1[8192];
extern unsigned char RomImageLynx96Image2[8192];
extern unsigned char RomImageLynx96Image3[8192];
extern unsigned char RomImageLynx96ScorpionImage3[8192];



namespace Jynx
{
	LynxROMsAndRAMs::LynxROMsAndRAMs()
	{
		_machineType = LynxMachineType::LYNX_48K;
		OnHardwareReset();
	}
	
	
	
	void LynxROMsAndRAMs::SetMachineType( LynxMachineType::Enum machineType )
	{
		_machineType = machineType;
		OnHardwareReset();
	}
	
	
	
	void LynxROMsAndRAMs::OnHardwareReset()
	{
		JynxFramework::ZeroInitialiseArrayMemory( _lynxRAM_0000 );
		JynxFramework::ZeroInitialiseArrayMemory( _lynxRAM_2000 );
		JynxFramework::ZeroInitialiseArrayMemory( _lynxRAM_4000 );
		JynxFramework::ZeroInitialiseArrayMemory( _lynxRAM_6000 );
		JynxFramework::ZeroInitialiseArrayMemory( _lynxRAM_8000 );
		JynxFramework::ZeroInitialiseArrayMemory( _lynxRAM_A000 );
		JynxFramework::ZeroInitialiseArrayMemory( _lynxRAM_C000 );
		JynxFramework::ZeroInitialiseArrayMemory( _lynxRAM_E000 );

		//
		// Copy the appropriate ROMs in according to the _machineType
		//

		if( _machineType == LynxMachineType::LYNX_48K )
		{
			JynxFramework::CopyWholeArray( RomImageLynx48Image1, _lynxROM_0000 );
			JynxFramework::CopyWholeArray( RomImageLynx48Image2, _lynxROM_2000 );
			JynxFramework::ZeroInitialiseArrayMemory( _lynxROM_4000 );
		}
		else if( _machineType == LynxMachineType::LYNX_96K )
		{
			JynxFramework::CopyWholeArray( RomImageLynx96Image1, _lynxROM_0000 );
			JynxFramework::CopyWholeArray( RomImageLynx96Image2, _lynxROM_2000 );
			JynxFramework::CopyWholeArray( RomImageLynx96Image3, _lynxROM_4000 );
		}
		else if( _machineType == LynxMachineType::LYNX_96K_Scorpion )
		{
			JynxFramework::CopyWholeArray( RomImageLynx96Image1         , _lynxROM_0000 );
			JynxFramework::CopyWholeArray( RomImageLynx96Image2         , _lynxROM_2000 );
			JynxFramework::CopyWholeArray( RomImageLynx96ScorpionImage3 , _lynxROM_4000 );
		}
		// TODO: else assert(false);
	}
}