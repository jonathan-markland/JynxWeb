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
#include "LynxAddressSpaceDecoder.h"
#include "LynxHardwareCommon.h"

namespace Jynx
{
	LynxAddressSpaceDecoder::LynxAddressSpaceDecoder()
	{
		_machineType = LynxMachineType::LYNX_48K; // TODO: Parameterise this externally.
		OnHardwareReset();
	}



	uint32_t *LynxAddressSpaceDecoder::GetScreenBitmapBaseAddress()
	{
		return _screen.GetScreenBitmapBaseAddress();
	}
	
	
	
	void LynxAddressSpaceDecoder::RecomposeWholeHostScreenRGBAsIfPending()
	{
		_screen.RecomposeWholeHostScreenRGBAsIfPending();
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

		_screen.OnDevicePortValueChanged(_devicePort);

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
				_addressSpaceWRITE1.MapReflections( _memory.GetRAM_8000(), _memory.GetRAM_6000() );
			}
			else
			{
				// TODO: assert( _machineType == LynxMachineType::LYNX_96K || _machineType == LynxMachineType::LYNX_96K_Scorpion );
				_addressSpaceWRITE1.Chips[0] = _memory.GetRAM_0000();
				_addressSpaceWRITE1.Chips[1] = _memory.GetRAM_2000();
				_addressSpaceWRITE1.Chips[2] = _memory.GetRAM_4000();
				_addressSpaceWRITE1.Chips[3] = _memory.GetRAM_6000();
				_addressSpaceWRITE1.Chips[4] = _memory.GetRAM_8000();
				_addressSpaceWRITE1.Chips[5] = _memory.GetRAM_A000();
				_addressSpaceWRITE1.Chips[6] = _memory.GetRAM_C000();
				_addressSpaceWRITE1.Chips[7] = _memory.GetRAM_E000();
			}
		}
		else
		{
			_addressSpaceWRITE1.MapReflections( nullptr, nullptr );
		}

		// Is Bank 2 going to be enabled for writes?

		if( _bankPort & BANKPORT_WREN2 && bCasEnBank2 )
		{
			_addressSpaceWRITE2.MapReflections( _screen.GetBlueRAM(), _screen.GetRedRAM() );
		}
		else
		{
			_addressSpaceWRITE2.MapReflections( nullptr, nullptr );
		}

		// Is Bank 3 going to be enabled for writes?

		if( _bankPort & BANKPORT_WREN3 && bCasEnBank3 )
		{
			_addressSpaceWRITE3.MapReflections( _screen.GetAltGreenRAM(), _screen.GetGreenRAM() );
		}
		else
		{
			_addressSpaceWRITE3.MapReflections( nullptr, nullptr );
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
				_addressSpaceREAD.MapReflections( _memory.GetRAM_8000(), _memory.GetRAM_6000() );
			}
			else
			{
				// TODO: assert( _machineType == LynxMachineType::LYNX_96K || _machineType == LynxMachineType::LYNX_96K_Scorpion );
				_addressSpaceREAD.Chips[0] = _memory.GetRAM_0000();
				_addressSpaceREAD.Chips[1] = _memory.GetRAM_2000();
				_addressSpaceREAD.Chips[2] = _memory.GetRAM_4000();
				_addressSpaceREAD.Chips[3] = _memory.GetRAM_6000();
				_addressSpaceREAD.Chips[4] = _memory.GetRAM_8000();
				_addressSpaceREAD.Chips[5] = _memory.GetRAM_A000();
				_addressSpaceREAD.Chips[6] = _memory.GetRAM_C000();
				_addressSpaceREAD.Chips[7] = _memory.GetRAM_E000();
			}

			++readCount;
		}

		if( _bankPort & BANKPORT_RDEN2_3 )
		{
			if( bCasEnBank2 )
			{
				// Bank 2 is switched in
				_addressSpaceREAD.MapReflections( _screen.GetBlueRAM(), _screen.GetRedRAM() );
				++readCount;
			}
			if( bCasEnBank3 )
			{
				// Bank 3 is switched in
				_addressSpaceREAD.MapReflections( _screen.GetAltGreenRAM(), _screen.GetGreenRAM() );
				++readCount;
			}
		}

		if( readCount == 0 )
		{
			_addressSpaceREAD.SetAllNull();
		}

		//
		// ROM -- If enabled, this appears in the READable address-space, overlaying the RAM.
		//

		if( (_bankPort & BANKPORT_NOT_RDEN0) == 0 )
		{
			// ROM is enabled for reading

			_addressSpaceREAD.Chips[0] = _memory.GetROM_0000();
			_addressSpaceREAD.Chips[1] = _memory.GetROM_2000();
                                   
			if( _machineType == LynxMachineType::LYNX_48K )
			{
				_addressSpaceREAD.Chips[2] = nullptr; // extended ROM not present, MUST return 0xFF for the region.  // TODO: Use new solution: We initialised the ROM store to 0xFFs above!
			}
			else
			{
				// TODO: assert( _machineType == LynxMachineType::LYNX_96K || _machineType == LynxMachineType::LYNX_96K_Scorpion );
				_addressSpaceREAD.Chips[2] = _memory.GetROM_4000();  // The 96K machine has an extended ROM.
			}
		}
	}
	
	
	
	uint8_t LynxAddressSpaceDecoder::Z80_AddressRead( uint16_t address )
	{
		auto regionIndex = (address >> 13) & 7;
		auto chipToReadFrom = _addressSpaceREAD.Chips[regionIndex];

		if( chipToReadFrom != nullptr )  // TODO: Can we afford an 8KB slab of 0xFFs to avoid this branch?
		{
			auto dataByte = chipToReadFrom->RamBytes[address & 0x1FFF];
			return dataByte;
		}
		else
		{
			return 0xFF;  // Nobody decoded this address.
		}
	}



	void LynxAddressSpaceDecoder::Z80_AddressWrite( uint16_t address, uint8_t dataByte )
	{
		auto regionIndex = (address >> 13) & 7;
		auto addressOffset = address & 0x1FFF;

		// Writes can hit multiple devices depending on which banks are active.

		// Is bank 1 enabled to decode this?

		auto pChipInBank1 = _addressSpaceWRITE1.Chips[regionIndex];
		if( pChipInBank1 )
		{
			pChipInBank1->RamBytes[addressOffset] = dataByte;
		}

		// Is bank 2 enabled to decode this?

		auto pChipInBank2 = _addressSpaceWRITE2.Chips[regionIndex];
		if( pChipInBank2 )
		{
			_screen.OnScreenRamWrite(*pChipInBank2, addressOffset, dataByte);
		}

		// Is bank 3 enabled to decode this?

		auto pChipInBank3 = _addressSpaceWRITE3.Chips[regionIndex];
		if( pChipInBank3 )
		{
			_screen.OnScreenRamWrite(*pChipInBank3, addressOffset, dataByte);
		}
	}
	
	
	
	void LynxAddressSpaceDecoder::Z80_IOSpaceWrite( uint16_t portNumber, uint8_t dataByte )
	{
		//
		// AUDIO-VISUAL CONTROL PORT
		//

		if( (portNumber & DEVICEPORT_DECODING_MASK) == 0x80 )
		{
			auto oldSetting = _devicePort;

			dataByte &= 0x3F; // Bit 7 kept zero according to spec, and let's wire the mono-stable (bit 6) to 0   TODO: review monostable?

			if( oldSetting != dataByte ) // optimise by ignoring repeated writes of same value.
			{
				// Update our record of the port status (which may simultaneously change several things):
				// Must do this first, so any routines we call out to (below) see the new port settings:
				_devicePort = dataByte;

				// Use XOR to detect *change* in the DEVICEPORT_USE_ALT_GREEN bit:
				if( (oldSetting ^ dataByte) & DEVICEPORT_USE_ALT_GREEN )
				{
					_screen.OnDevicePortValueChanged(_devicePort);
				}

				// Use XOR to detect *change* in either/both of the DEVICEPORT_NOT_CASEN_BANK3 or DEVICEPORT_NOT_CASEN_BANK2 bits:
				if( (oldSetting ^ dataByte) & (DEVICEPORT_NOT_CASEN_BANK3 | DEVICEPORT_NOT_CASEN_BANK2) )
				{
					// Bits "NOT CASEN BANK3" or "NOT CASEN BANK2" have changed.
					// This affects bank switching:
					SyncAddressSpaceFromPorts();
				}

				/* TODO: cassette   // Use XOR to detect *change* in CASSETTE MOTOR control bit:
				if( (oldSetting ^ dataByte) & DEVICEPORT_CASSETTE_MOTOR )
				{
					if( dataByte & DEVICEPORT_CASSETTE_MOTOR )
					{
						CassetteMotorOn();
					}
					else
					{
						CassetteMotorOff();
					}
				}
				*/
			}
		}

		//
		// BANK SWITCH ( "Port 80" )
		//

		else if( (portNumber & 0x207F) == 0x207F )
		{
			// It's the so-called "PORT FFFF", although the 
			// Lynx hardware doesn't decode all the bits of the address!
			
			if( _bankPort != dataByte ) // optimise away repeated writes of same value
			{
				_bankPort = dataByte;
				SyncAddressSpaceFromPorts();
			}
		}
		
		// TODO: Support writing the volume level DAC.
	}
	
	
	
	uint8_t LynxAddressSpaceDecoder::Z80_IOSpaceRead( uint16_t portNumber )
	{
		return 0xFF;   // TODO: We support reading the keyboard and the cassette.
	}



	void LynxAddressSpaceDecoder::OnAboutToBranch()
	{
		// TODO
	}
	
	
	
	void LynxAddressSpaceDecoder::OnAboutToReturn()
	{
		// TODO
	}
}

