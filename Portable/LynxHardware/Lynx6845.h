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
	class Lynx6845
	{
	public:
	
		Lynx6845();
		
		void OnHardwareReset();
		
		uint8_t  GetRegister(uint8_t registerNumber);

	private:

		void Recalculate6845VariablesFromPorts();
		
	private:
		
		uint8_t  _mc6845Select;       // 6845 register selector
		uint8_t  _mc6845Regs[32];     // 6845 register values store

	};
}
