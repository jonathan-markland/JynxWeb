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
#include "LynxKeyboard.h"
#include "LynxHardwareCommon.h"


namespace Jynx
{
	LynxKeyboard::LynxKeyboard()
	{
		OnHardwareReset();
	}
	
	
	
	void LynxKeyboard::OnHardwareReset()
	{
		JynxFramework::InitialiseAllArrayElementsVolatile( _keyboard, (uint8_t) 0xFF );  // -ve logic
		JynxFramework::InitialiseAllArrayElements( _keyboardSweepDetect, false );
	}



	volatile uint8_t *LynxKeyboard::GetLynxKeyboardArrayAddress()
	{
		return _keyboard;
	}



	uint8_t  LynxKeyboard::ReadLynxKeyboard( uint16_t portNumber )
	{
		// Bits A11..A8 are the index into _keyboard[].
		// Bits A15..A12 are not decoded.
		// TODO:  assert( (portNumber & DEVICEPORT_DECODING_MASK) == 0x80 ); // Should have been checked by the caller.

		portNumber &= DEVICEPORT_KEYBOARD_DECODING_MASK;  // The Lynx doesn't decode all the bits.

		//
		// Handle port reading when text player active:
		//

		/* TODO:  if( _textPlayer.HasText() )
		{
			// When active, the _textPlayer disables direct keyboard reading, so we lie and say "result 0xFF".
			// If the user allows, we also assert speed-max mode with the text player:
			if( _canEnableSpeedMaxModeWhenInBetweenConsoleCommands == true )
			{
				_speedMaxModeBecauseWeAreInBetweenConsoleCommands = true;
			}
			return 0xFF;
		} */

		//
		// Handle normal port reading:
		//

		uint32_t portIndex = portNumber >> 8;

		if( portIndex < 10 )
		{
			auto result = _keyboard[portIndex];   // NB: volatile read

			/* TODO:   auto cleanSweepSeen = DoKeyboardCleanSweepDetection( portIndex );
			if( cleanSweepSeen )
			{
				// The Lynx has just done a clean-sweep read of the keyboard ports
				// resulting in a NO KEYS HELD DOWN result.  This our signal that
				// a command-prompt has returned, so let's disable speed max mode:
				_speedMaxModeBecauseWeAreInBetweenConsoleCommands = false;
			}

			// If the user allows, then enable speed-max mode when RETURN is pressed
			// (as we assume this is entering a console command for which the user wants
			// host-speed performance).
			if( _canEnableSpeedMaxModeWhenInBetweenConsoleCommands == true )
			{
				if( portNumber == 0x0980 && ((_keyboard[9] & 0x08) == 0) )  // RETURN key DOWN?
				{
					_speedMaxModeBecauseWeAreInBetweenConsoleCommands = true;
				}
			}

			// (NB: Doing the test the above order means that holding RETURN down does not
			// cause the screen to rapidly wipe out).
			*/

			// Return the keyboard port byte:
			return result;
		}

		return 0xFF;  // Port out of range of keyboard.
	}
}
