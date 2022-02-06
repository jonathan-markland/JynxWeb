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

#include "LynxAddressSpaceDecoder.h"

namespace Jynx
{
	void MapReflections( ADDRESS_SPACE &addressSpace, CHIP *chipOne, CHIP *chipTwo )
	{
		addressSpace[0] = chipOne;
		addressSpace[1] = chipOne;
		addressSpace[2] = chipTwo;
		addressSpace[3] = chipTwo;
		addressSpace[4] = chipOne;
		addressSpace[5] = chipOne;
		addressSpace[6] = chipTwo;
		addressSpace[7] = chipTwo;
	}



	void ZeroBANK( ADDRESS_SPACE &addressSpace )
	{
		InitialiseAllArrayElements( addressSpace, (CHIP *) nullptr );
	}



	LynxAddressSpaceDecoder::LynxAddressSpaceDecoder()
	{
		OnHardwareReset();
	}



	void LynxAddressSpaceDecoder::OnHardwareReset()
	{
		_devicePort = DEVICEPORT_INITIALISATION_VALUE;
		_bankPort   = BANKPORT_INITIALISATION_VALUE;
		
		_memory.OnHardwareReset();
		_screen.OnHardwareReset();
		_screen.OnDevicePortValueChanged(_devicePort);
	}



	void LynxAddressSpaceDecoder::SyncAddressSpaceFromPorts()
	{
		//
		// Updates the following address space variables from the state of _devicePort and _bankPort:
		//
		// _addressSpaceREAD
		// _addressSpaceWRITE1
		// _addressSpaceWRITE2
		// _addressSpaceWRITE3
		//

		_lynxScreen.OnDevicePortValueChanged(_devicePort);

		bool bCasEnBank2 = (_devicePort & DEVICEPORT_NOT_CASEN_BANK2) == 0;
		bool bCasEnBank3 = (_devicePort & DEVICEPORT_NOT_CASEN_BANK3) == 0;

		//
		// ADDRESS SPACE - WRITING
		//

		// Is Bank 1 going to be enabled for writes?

		if( (_bankPort & BANKPORT_NOT_WREN1) == 0 )
		{
			if( _machineType == LynxMachineType::LYNX_48K )
			{
				MapReflections( _addressSpaceWRITE1, _memory.GetRAM_8000(), _memory.GetRAM_6000() );
			}
			else
			{
				assert( _machineType == LynxMachineType::LYNX_96K || _machineType == LynxMachineType::LYNX_96K_Scorpion );
				_addressSpaceWRITE1[0] = _memory.GetRAM_0000();
				_addressSpaceWRITE1[1] = _memory.GetRAM_2000();
				_addressSpaceWRITE1[2] = _memory.GetRAM_4000();
				_addressSpaceWRITE1[3] = _memory.GetRAM_6000();
				_addressSpaceWRITE1[4] = _memory.GetRAM_8000();
				_addressSpaceWRITE1[5] = _memory.GetRAM_A000();
				_addressSpaceWRITE1[6] = _memory.GetRAM_C000();
				_addressSpaceWRITE1[7] = _memory.GetRAM_E000();
			}
		}
		else
		{
			MapReflections( _addressSpaceWRITE1, nullptr, nullptr );
		}

		// Is Bank 2 going to be enabled for writes?

		if( _bankPort & BANKPORT_WREN2 && bCasEnBank2 )
		{
			MapReflections( _addressSpaceWRITE2, _screen.GetBlueRAM(), _screen.GetRedRAM() );
		}
		else
		{
			MapReflections( _addressSpaceWRITE2, nullptr, nullptr );
		}

		// Is Bank 3 going to be enabled for writes?

		if( _bankPort & BANKPORT_WREN3 && bCasEnBank3 )
		{
			MapReflections( _addressSpaceWRITE3, _screen.GetAltGreenRAM(), _screen.GetGreenRAM() );
		}
		else
		{
			MapReflections( _addressSpaceWRITE3, nullptr, nullptr );
		}

		//
		// ADDRESS SPACE - READING
		//

		uint8_t readCount = 0;

		if( (_bankPort & BANKPORT_NOT_RDEN1) == 0 )
		{
			// Bank1 is switched IN.

			if( _machineType == LynxMachineType::LYNX_48K )
			{
				MapReflections( _addressSpaceREAD, _memory.GetRAM_8000(), _memory.GetRAM_6000() );
			}
			else
			{
				assert( _machineType == LynxMachineType::LYNX_96K || _machineType == LynxMachineType::LYNX_96K_Scorpion );
				_addressSpaceREAD[0] = _memory.GetRAM_0000();
				_addressSpaceREAD[1] = _memory.GetRAM_2000();
				_addressSpaceREAD[2] = _memory.GetRAM_4000();
				_addressSpaceREAD[3] = _memory.GetRAM_6000();
				_addressSpaceREAD[4] = _memory.GetRAM_8000();
				_addressSpaceREAD[5] = _memory.GetRAM_A000();
				_addressSpaceREAD[6] = _memory.GetRAM_C000();
				_addressSpaceREAD[7] = _memory.GetRAM_E000();
			}

			++readCount;
		}

		if( _bankPort & BANKPORT_RDEN2_3 )
		{
			if( bCasEnBank2 )
			{
				// Bank 2 is switched in
				MapReflections( _addressSpaceREAD, _screen.GetBlueRAM(), _screen.GetRedRAM() );
				++readCount;
			}
			if( bCasEnBank3 )
			{
				// Bank 3 is switched in
				MapReflections( _addressSpaceREAD, _screen.GetAltGreenRAM(), _screen.GetGreenRAM() );
				++readCount;
			}
		}

		if( readCount == 0 )
		{
			ZeroBANK( _addressSpaceREAD );
		}

		//
		// ROM -- If enabled, this appears in the READable address-space, overlaying the RAM.
		//

		if( (_bankPort & BANKPORT_NOT_RDEN0) == 0 )
		{
			// ROM is enabled for reading

			_addressSpaceREAD[0] = GetROM_0000();
			_addressSpaceREAD[1] = GetROM_2000();
                                   
			if( _machineType == LynxMachineType::LYNX_48K )
			{
				_addressSpaceREAD[2] = nullptr; // extended ROM not present, MUST return 0xFF for the region.
			}
			else
			{
				assert( _machineType == LynxMachineType::LYNX_96K || _machineType == LynxMachineType::LYNX_96K_Scorpion );
				_addressSpaceREAD[2] = GetROM_4000();  // The 96K machine has an extended ROM.
			}
		}
	}
}

