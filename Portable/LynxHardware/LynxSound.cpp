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
#include "LynxSound.h"

//  #define DEBUG_EMIT_CONSTANT_SQUARE_WAVE

namespace Jynx
{
	LynxSound::LynxSound()
	{
		OnHardwareReset();
		
		#ifdef DEBUG_EMIT_CONSTANT_SQUARE_WAVE
		int i=0;
		for(; i<BROWSER_SOUND_BUFFER_LENGTH/2; i++) _pcmBuffer[i] = 0.0;
		for(; i<BROWSER_SOUND_BUFFER_LENGTH; i++) _pcmBuffer[i] = 0.3;
		#endif
	}
	
	
	
	void LynxSound::OnHardwareReset()
	{
		_level = 0.0;
		_bufferPosition = 0;
		
		#ifndef DEBUG_EMIT_CONSTANT_SQUARE_WAVE
		JynxFramework::InitialiseAllArrayElementsVolatile( _pcmBuffer, (float) 0.0 );
		#endif
	}



	void LynxSound::OnQuantumStart()
	{
		_bufferPosition = 0;
	}	
	
	
	
	void LynxSound::SetLevelAtTime( uint8_t lynxSpeakerLevel, int32_t cyclesDoneInTimeslice, int32_t timesliceLength )
	{
		auto drawToIndex = (BROWSER_SOUND_BUFFER_LENGTH * cyclesDoneInTimeslice) / timesliceLength;
				
		// This is kind of "write behind":  Only NOW do we know how long the previous
		// level was held for!  So let's flush the previous level:
		DrawTo( drawToIndex );

		// Record the new level for next time:
		_level = ((float) lynxSpeakerLevel) / 63.0;
	}

	

	void LynxSound::OnQuantumEnd()
	{
		// Ensure the most recent level is painted to the end of the sound buffer:
		DrawTo( BROWSER_SOUND_BUFFER_LENGTH );
	}
	
	
	
	void LynxSound::DrawTo( int index )
	{	
		if (index < 0)
		{
			index = 0;
		}
		else if (index > BROWSER_SOUND_BUFFER_LENGTH) // Z80 processing may overshoot timeslice
		{
			index = BROWSER_SOUND_BUFFER_LENGTH;
		}
		
		if (index > _bufferPosition)
		{
			for (int i = _bufferPosition; i < index; i++)
			{
				#ifndef DEBUG_EMIT_CONSTANT_SQUARE_WAVE
				_pcmBuffer[i] = _level;
				#endif
			}

			_bufferPosition = index;
		}
	}
}

