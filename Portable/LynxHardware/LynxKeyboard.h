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

#include <stdint.h>

namespace Jynx
{
	class LynxKeyboard
	{
	public:
	
		LynxKeyboard();
		
		void OnHardwareReset();
		volatile uint8_t *GetLynxKeyboardArrayAddress();
		uint8_t  ReadLynxKeyboard( uint16_t portNumber );
		
	private:
	
		volatile uint8_t  _keyboard[10];       // Lynx keyboard ports (not persistent).
		bool     _keyboardSweepDetect[10];     // ports 0..9
	
	};
}
