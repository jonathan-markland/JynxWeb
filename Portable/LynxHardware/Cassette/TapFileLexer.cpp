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

#include "TapFileLexer.h"

using namespace JynxFramework;

namespace Jynx
{
	TapFileLexer::TapFileLexer()
	{
		Clear();
	}



	void TapFileLexer::Clear()
	{
		_position    = nullptr;
		_endPosition = nullptr;
	}
	

			
	bool TapFileLexer::Open( const Array<uint8_t> &tapFileImage )
	{
		// assert( tapFileImage.size() >= 1 );  // must be
		// assert( tapFileImage.back() == 0 );  // must be a 0 terminator at the end (eases parsing)
		auto startPosition = (const uint8_t *) tapFileImage.FirstElementVoidPointer();
		_position    = startPosition;
		_endPosition = startPosition + (tapFileImage.Count() - 1);  // subtract one to stop our 0 terminator being consumed by block readers.
	}



	bool TapFileLexer::End()
	{
		if( _position > _endPosition ) return Fail();
		return _position == _endPosition;
	}



	uintptr_t TapFileLexer::SpaceRemaining() const
	{
		return _endPosition - _position;
	}



	bool TapFileLexer::ExpectFileName( String *result )
	{
		// Default result is empty string:
		result->Clear();
		
		// Some TAPs have a spurious A5 at the start:
		if( _position[0] == 0xA5 )
		{
			++_position;  // consume the 0xA5 only.  These sort of redundant A5s will be re-constituted by the replay, and do not need to be part of the data.
		}

		// Check for type 'A' files (no file name portion):
		if( _position[0] == 'A' )
		{
			return true; // There is no name string with an 'A' format TAP (as used by Level-9 adventures)
		}

		// In the block after the FIRST sync, the Lynx stores the file name, in quotes.

		if( _position[0] == '\"' )
		{
			++_position;
			StringBuilder fileNameBuilder;
			while(true)
			{
				auto byte = *_position;
				if( byte == 0 ) return Fail();
				if( byte == '\"' ) break;
				fileNameBuilder.AppendChar(byte);
				++_position;
			}
			++_position; // skip second "

			// Success
			*result = fileNameBuilder.ToString();
			return true;
		}

		return Fail();
	}



	bool TapFileLexer::ExpectFileBody( Array<uint8_t> *result )
	{
		// Default result
		result->Clear();
		
		// In the block after the second sync, the type-stamp, data-length and raw data are encoded.

		auto positionOfFileStart = _position;

		// Read file type letter:
		if( SpaceRemaining() < 1 ) return Fail();
		auto fileTypeLetter = _position[0];

		// Read header now we know the type:
		uint8_t  lengthLow  = 0;
		uint8_t  lengthHigh = 0;
		if( fileTypeLetter == 'A' ) // 'A' type files used by Level 9 games, possibly others?
		{
			if( SpaceRemaining() < 5 ) return Fail();
			lengthLow  = _position[3];
			lengthHigh = _position[4];
			_position += 5;
		}
		else if( fileTypeLetter == 'B' || fileTypeLetter == 'M' )  // Lynx basic or machine code.
		{
			if( SpaceRemaining() < 3 ) return Fail();
			lengthLow  = _position[1];
			lengthHigh = _position[2];
			_position += 3;
		}
		else return Fail();

		// Determine the image length (as recorded in the file):
		auto payloadLength = (uint32_t) ((lengthHigh << 8) | lengthLow);

		// Increase the length to include other known data:
		if( fileTypeLetter == 'B' ) payloadLength += 3;   // Basic files have 3 extra bytes after the payload
		if( fileTypeLetter == 'M' ) payloadLength += 7;   // "Machine code" files have 7 extra bytes after the payload
		if( fileTypeLetter == 'A' ) payloadLength += 12;  // "A" files have 12 extra bytes after the payload

		// Check it fits the file:
		auto spaceRemaining = SpaceRemaining();
		if( payloadLength > spaceRemaining ) return Fail();

		// Copy out the raw file data, starting at the file type byte:
		_position += payloadLength;
		auto fileBody = Array<uint8_t>( Slice<const uint8_t>(positionOfFileStart, _position) );

		// Unfortunately, a minority of TAPs, particularly BASIC ones, have a spurious zero
		// at the end.  However, we can skip this, if found, because TAP files always begin
		// with 0x22 0x41 (or the also-spurious 0xA5), never 0x00.
		while( SpaceRemaining() > 0 )  // Must do this test, so we don't mistake our own NUL terminator for a spurious zero!
		{
			if( *_position != 0 ) break;
			++_position;  // move past spurious zero
		}

		// Success
		*result = fileBody;
		return true;
	}



	bool TapFileLexer::Fail()
	{
		// Convenience error handling.
		// Reset the state and return false.
		Clear();
		return false;
	}

} // end namespace Jynx
