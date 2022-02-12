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
#include "LynxComputer.h"

namespace Jynx
{
	LynxComputer::LynxComputer()
	{
		_processor.SetExternalHandler( &_addressSpace );
		_addressSpace.SetCPU( &_processor );
		OnHardwareReset();
	}



	void LynxComputer::OnHardwareReset()
	{
		_processor.SetTimesliceLength(10449);  // TODO: Calc properly.  (4,000,000 * 128 / 44100) * 0.9  ...  (Z80 speed * samples per buffer / sound sample rate) * ROM slowdown factor
		_addressSpace.OnHardwareReset();
		_processor.Reset();
	}
	
	
	
	void LynxComputer::OnTimeSlice()
	{
		_addressSpace.OnQuantumStart();
		_processor.RunForTimeslice();
		_addressSpace.OnQuantumEnd();
	}
}

