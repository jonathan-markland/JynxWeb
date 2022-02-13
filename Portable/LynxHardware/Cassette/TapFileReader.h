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
// #include "ISerialiser.h"
#include "../../IFileOpener.h"
#include "../../JynxFramework.h"
#include "SignalRLE.h"
#include "TapFileSplitter.h"
#include "ITapeSpeedSupplier.h"

namespace Jynx
{
	class TapFileReader
	{
	public:

		// A TAP file reader.
		// - TAP files seem to have just one Lynx file in them, as far as I can see.  (No Spec!)
		// Must construct afresh for each file to load!
		// Can rewind though!

		explicit TapFileReader( ITapeSpeedSupplier * );
			// The constructor for an empty tape.

		TapFileReader( IFileOpener *tapFileOpener, ITapeSpeedSupplier * );

		// Motor
		void CassetteMotorOn();
		void CassetteMotorOff();

		// Playback
		void RewindPlaybackPosition();
		uint8_t GetSampleAtTime( uint64_t cycleCounterNow );

		// Get Tape Directory
		JynxFramework::String  GetTapeDirectory( TapeDirectoryStyle::Enum styleRequired ) const;

	private:

		TapFileSplitter _tapFileSplitter;

		// Wave data representation for the *current* file:
		JynxFramework::Array<SignalRLE>  _waveData;

		// External supplier of the tape speed.
		// Called at the last minute, when we make the waveform.
		ITapeSpeedSupplier *_tapeSpeedSupplier;

		// Playback:
		bool      _playbackResyncWithCycleCounterOnNextRead;
		uint32_t  _bitsPerSecond;
		size_t    _playbackFileIndex;
		size_t    _playbackWaveDataIndex;
		uint64_t  _playbackTimeOffset;
		uint64_t  _playbackCycleCounterOrigin;

		void SetPositionToFile( size_t fileIndex );

	};

} // end namespace Jynx
