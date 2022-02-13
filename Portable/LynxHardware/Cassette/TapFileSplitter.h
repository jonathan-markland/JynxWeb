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
#include "../../IFileOpener.h"
#include "../../JynxFramework.h"
#include "SignalRLE.h"

namespace Jynx
{
	namespace TapeDirectoryStyle
	{
		enum Enum
		{
			LoadCommands, REMCommandListing
		};
	}



    class TapFileLexerException: public std::runtime_error
    {
    public:
        TapFileLexerException( const char *message ) : std::runtime_error(message) {}
    };



	struct NamedTapFile
	{
		JynxFramework::String  FileName;
		JynxFramework::Array<uint8_t>  ContentImage;
	};



	class TapFileSplitter
	{
	public:

		TapFileSplitter();
			// The constructor for an empty tape.

		explicit TapFileSplitter( IFileOpener *tapFile );

		size_t GetNumberOfFiles() const;
			// Return number of files containing in this TAP.

		JynxFramework::Array<SignalRLE>  GenerateWaveformForFile( size_t fileIndex, uint32_t bitsPerSecond );
			// Returns the wave data for the file at the given index.
			// The return wave comprises:
			// - The initial SYNC
			// - The file name
			// - The second SYNC
			// - The main data block
			// ... as the Lynx uses.
			// - bitsPerSecond should be the "TAPE" speed the Lynx is currently
			//   expecting: { 600, 900, 1200, 1500, 1800, 2100 } for TAPE 0-5 resp.

		JynxFramework::String  GetTapeDirectory( TapeDirectoryStyle::Enum styleRequired ) const;
			// Retrieves text giving tape content and file types (LOAD / MLOAD).

	private:

		JynxFramework::Array<JynxFramework::Pointer<NamedTapFile>> _filesOnTape;

		void LoadAndParseTapFile( IFileOpener *tapFileOpener );
		JynxFramework::String LynxLoadCommandForFile( size_t fileIndex ) const;

	};

} // end namespace Jynx
