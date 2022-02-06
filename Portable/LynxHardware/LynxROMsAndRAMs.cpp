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
		_lynxRAM_0000.SetToAllZeroes();
		_lynxRAM_2000.SetToAllZeroes();
		_lynxRAM_4000.SetToAllZeroes();
		_lynxRAM_6000.SetToAllZeroes();
		_lynxRAM_8000.SetToAllZeroes();
		_lynxRAM_A000.SetToAllZeroes();
		_lynxRAM_C000.SetToAllZeroes();
		_lynxRAM_E000.SetToAllZeroes();

		//
		// Copy the appropriate ROMs in according to the _machineType
		//

		if( _machineType == LynxMachineType::LYNX_48K )
		{
			_lynxROM_0000.SetFrom( RomImageLynx48Image1 );
			_lynxROM_2000.SetFrom( RomImageLynx48Image2 );
			_lynxROM_4000.SetToAll(0xFF);
		}
		else if( _machineType == LynxMachineType::LYNX_96K )
		{
			_lynxROM_0000.SetFrom( RomImageLynx96Image1 );
			_lynxROM_2000.SetFrom( RomImageLynx96Image2 );
			_lynxROM_4000.SetFrom( RomImageLynx96Image3 );
		}
		else if( _machineType == LynxMachineType::LYNX_96K_Scorpion )
		{
			_lynxROM_0000.SetFrom( RomImageLynx96Image1         );
			_lynxROM_2000.SetFrom( RomImageLynx96Image2         );
			_lynxROM_4000.SetFrom( RomImageLynx96ScorpionImage3 );
		}
		// TODO: else assert(false);
	}
}