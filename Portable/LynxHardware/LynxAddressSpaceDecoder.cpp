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

#include "../../ExternalModules/JynxFrameworkLibrary/JynxFramework.h"
#include "LynxAddressSpaceDecoder.h"
#include "LynxHardwareCommon.h"

// TODO: Post MVP:  In the MVP we hack setting a palette, on the 
//                  basis of knowledge of the statically-linked cassette image.
extern bool BuiltInCassetteNeedsLevel9Palette;

namespace Jynx
{
	LynxAddressSpaceDecoder::LynxAddressSpaceDecoder()
	{
		_processor = nullptr;
		_machineType = LynxMachineType::LYNX_96K; // TODO: Parameterise this externally.
		_timesliceStartCount = 0;  // Does not need reset on hardware reset (used for differencing).
		OnHardwareReset();
	}



	void LynxAddressSpaceDecoder::OnHardwareReset()
	{
		_devicePort = DEVICEPORT_INITIALISATION_VALUE;
		_bankPort   = BANKPORT_INITIALISATION_VALUE;
		_memory.OnHardwareReset();
		_sound.OnHardwareReset();
		_screen.OnHardwareReset();
		_screen.OnDevicePortValueChanged(_devicePort);
		_6845.OnHardwareReset();
		_cassetteReader.OnHardwareReset();
		_screen.SetPalette(BuiltInCassetteNeedsLevel9Palette ? LynxColourSet::Level9 : LynxColourSet::NormalRGB);  // TODO: This is only for the MVP (static link of cassette image)
		SyncAddressSpaceFromPorts();
	}



	void LynxAddressSpaceDecoder::OnQuantumStart()
	{
		_cycleCountBefore = _processor->GetRemainingCycles();
		_screen.OnQuantumStart();
		_sound.OnQuantumStart();
	}



	void LynxAddressSpaceDecoder::OnQuantumEnd()
	{
		auto cycleCountAfter = _processor->GetRemainingCycles();
		_timesliceStartCount += (cycleCountAfter - _cycleCountBefore) + _processor->GetTimesliceLength();
		_screen.OnQuantumEnd();
		_sound.OnQuantumEnd();
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
		}
		else if( _bankPort & BANKPORT_RDEN2_3 )
		{
			if( bCasEnBank2 )
			{
				// Bank 2 is switched in
				_addressSpaceREAD.MapReflections( _screen.GetBlueRAM(), _screen.GetRedRAM() );
			}
			else if( bCasEnBank3 )
			{
				// Bank 3 is switched in
				_addressSpaceREAD.MapReflections( _screen.GetAltGreenRAM(), _screen.GetGreenRAM() );
			}
			else
			{
				_addressSpaceREAD.MapReflections( _memory.GetROM_FFs(), _memory.GetROM_FFs() );
			}
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
				_addressSpaceREAD.Chips[2] = _memory.GetROM_FFs();   // This ROM is missing on the 48K.
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
		auto dataByte = chipToReadFrom->RamBytes[address & 0x1FFF];
		return dataByte;
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

				if( (oldSetting ^ dataByte) & DEVICEPORT_CASSETTE_MOTOR )
				{
					if( dataByte & DEVICEPORT_CASSETTE_MOTOR )
					{
						_cassetteReader.TapeMotorOn(600); // TODO: We used to get the setting from the Lynx
					}
					else
					{
						_cassetteReader.TapeMotorOff();
					}
				}
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
		
		//
		// 6845
		//

		else if( (portNumber & 0xC7) == 0x86 )
		{
			_6845.SetSelect( dataByte );
		}
		else if( (portNumber & 0xC7) == 0x87 )
		{
			_6845.SetSelectedRegister( dataByte );
		}

		//
		// CASSETTE / LOUDSPEAKER - 6 bit D/A
		//

		else if( (portNumber & DEVICEPORT_DECODING_MASK) == 0x84 )
		{
			// If the casette motor is enabled, we are saving to tape:
			// The Lynx has a 6-bit D-A converter.

			// Bits 5..0 contain the level:
			auto level = dataByte & 0x3F;

			if( _6845.GetRegister(12) & 0x10 )  // Camputers use 6845 output MA12 as a switch to enable outputting.
			{
				/* CassetteWrite( level );

				if( _hearTapeSounds )
				{
					// Listen to tape saving (quieten it a bit!):
					SpeakerWrite( level >> 2 );
				} */
			}
			else // if( _devicePort & DEVICEPORT_SPEAKER ) // <-- Hmm... interesting... this disabled the sound on Invaders!
			{
				auto cyclesDoneInTimeslice = _processor->GetCyclesDoneInTimeslice();
				auto timesliceLength = _processor->GetTimesliceLength();
				_sound.SetLevelAtTime( level, cyclesDoneInTimeslice, timesliceLength );
			}
		}
	}
	
	
	
	uint8_t LynxAddressSpaceDecoder::Z80_IOSpaceRead( uint16_t portNumber )
	{
		if( (portNumber & DEVICEPORT_DECODING_MASK) == 0x80 )
		{
			// If the cassette motor is enabled, we are loading from tape.
			// Bit 0 is a "level sensor" which detects whether the level is below or above the middle.

			// Since the keyboard shares the same port as the cassette we must include the key states
			// -- although I have insufficient documentation on this!  I deduced when the supply the
			// cassette value in bit 0 of port 0x0080

			if( _6845.GetRegister(12) & 0x20 ) // Camputers use this output of the 6845 as a switch to enable cassette reading on the keyboard port (if MA13 is high).
			{
				if( (portNumber & 0xFC6) == 0x0080 ) // <-- Mask per Lynx User Magazine Issue 1.  The lynx appears to only read from this port specifically, when reading tapes.
				{
					auto elaspedThisQuantum = _processor->GetTimesliceLength() - _processor->GetRemainingCycles();
					auto z80CycleCountNow   = _timesliceStartCount + elaspedThisQuantum;
					auto cassetteBit0       = _cassetteReader.ReadCurrentBit(z80CycleCountNow);
					/* TODO:  if( _hearTapeSounds )
					{
						// Listen to tape loading (quieten it a bit):
						auto cyclesDoneInTimeslice = _processor->GetCyclesDoneInTimeslice();
						auto timesliceLength = _processor->GetTimesliceLength();
						_sound.SetLevelAtTime( cassetteBit0 << 3, cyclesDoneInTimeslice, timesliceLength );
					} */

					// (It seems cassette loading terminates immediately unless the key information is
					// returned here.  Fixing the top 7 bits at "0"s wasn't a good idea!).
					return (_keyboard.ReadLynxKeyboard(portNumber) & 0xFE) | cassetteBit0;    // The AND mask probably isn't needed.
				}
			}

			// Read of keyboard only (cassette motor not active):
			return _keyboard.ReadLynxKeyboard(portNumber);
		}
		
		/* TODO: remove when JynxII emulator more mature:
		
		else if( (portNumber & CRTCPORT_DECODING_MASK) == 0x86 )
		{
			return 0xFF; // spec says 6845 display generator select isn't readable
		}
		else if( (portNumber & CRTCPORT_DECODING_MASK) == 0x87 )
		{
			return 0xFF; // spec says 6845 register isn't readable
		}
		else if( (portNumber & BANKPORT_DECODING_MASK) == BANKPORT_DECODING_MASK )
		{
			// It's the so-called "PORT FFFF":
			return _bankPort; // I think the spec says this isn't readable either.  This doesn't seem to get executed.
		}*/

		return 0xFF;   // Nobody decoded this I/O space address.
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

