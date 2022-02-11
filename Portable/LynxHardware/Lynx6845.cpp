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
#include "Lynx6845.h"

namespace Jynx
{
	Lynx6845::Lynx6845()
	{
		OnHardwareReset();
	}



	void Lynx6845::OnHardwareReset()
	{
		_mc6845Select = 0;
		JynxFramework::InitialiseAllArrayElements( _mc6845Regs, (uint8_t) 0 );   // The Lynx ROM initialises the 6845 in the first instance.
		Recalculate6845VariablesFromPorts();
	}
	
	
	
	uint8_t Lynx6845::GetRegister(uint8_t registerNumber)
	{
		return _mc6845Regs[registerNumber & 31];
	}
	
	
	
	void Lynx6845::SetRegister(uint8_t registerNumber, uint8_t value)
	{
		auto index = registerNumber & 31;
		if ( value != _mc6845Regs[index] )
		{
			_mc6845Regs[index] = value;
			Recalculate6845VariablesFromPorts();
		}
	}

	
	
	void Lynx6845::Recalculate6845VariablesFromPorts()
	{
		// TODO: Placeholder for future stuff that will involve other devices.
	}
}

