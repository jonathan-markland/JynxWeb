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
	LynxComputer::LynxComputer()
	{
		_processor.SetExternalHandler( &_addressSpace );
		OnHardwareReset();
	}



	void LynxComputer::OnHardwareReset()
	{
		_processor.SetTimesliceLength(10449);  // TODO: Calc properly.  (4,000,000 * 128 / 44100) * 0.9  ...  (Z80 speed * samples per buffer / sound sample rate) * ROM slowdown factor
		_z80CycleCounter = 0;
		_addressSpace.OnHardwareReset();
		_processor.Reset();
	}
	
	
	
	void LynxComputer::OnTimeSlice()
	{
		// Execute Z80 code for this timeslice, and accumulate
		// the precise number of cycles elapsed (which may not
		// precisely be what we asked for, but the design supports
		// correcting this in later timeslices):

		auto cycleCountBefore = _processor.GetRemainingCycles();
		_processor.RunForTimeslice();
		auto cycleCountAfter  = _processor.GetRemainingCycles();
		_z80CycleCounter += (cycleCountAfter - cycleCountBefore) + _processor.GetTimesliceLength();

		_lynxScreen.RecomposeEntireScreenIfNeeded();
	}
}

