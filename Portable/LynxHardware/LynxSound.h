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
#include "../Z80/JynxZ80.h"  // only needed for elapsed time   TODO: refactor to use an interface only.

#define BROWSER_SOUND_BUFFER_LENGTH 128   // fixed by AudioWorklet spec

namespace Jynx
{
	class LynxSound
	{
	public:
	
		LynxSound();
		void SetCPU( JynxZ80::Z80 *processor );
		
		volatile float *GetSoundBufferBaseAddress()    { return _pcmBuffer; }
		
		void OnHardwareReset();
		void OnQuantumStart();
		void OnQuantumEnd();

		// Change speaker level.
		// Called when speaker is written, and the Z80 elapsed time is quoted
		// in the form of the timeslice length, and countdown (distance to end of the ideal slice).
		void SetLevelAtTime( uint8_t lynxSpeakerLevel );
		
	private:

		volatile float _pcmBuffer[BROWSER_SOUND_BUFFER_LENGTH];
		void DrawTo( int index );
		
	private:
		
		int _bufferPosition;
		float _level;
		JynxZ80::Z80 *_processor;

	};
}
