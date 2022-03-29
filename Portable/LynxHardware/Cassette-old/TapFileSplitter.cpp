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

#include "TapFileSplitter.h"
#include "TapFileLexer.h"
#include "SignalWriter.h"
#include "FileLoader.h"
#include "../../JynxFramework.h"

using namespace JynxFramework;

namespace Jynx
{
	Array<Pointer<NamedTapFile>>  ParseMultiTapFile(const Array<uint8_t>& sourceTapFile)
	{
		ArrayBuilder<Pointer<NamedTapFile>>  filesListBuilder;

		TapFileLexer  lexer;
		
		if (lexer.Open(sourceTapFile))
		{
			while (!lexer.End())
			{
				auto namedTapFile = MakeNew<NamedTapFile>();
				namedTapFile->FileName = lexer.ExpectFileName();
				namedTapFile->ContentImage = lexer.ExpectFileBody();
				filesListBuilder.Add(namedTapFile);
			}
		}

		return filesListBuilder.MoveToArray();
	}
}




namespace Jynx
{
	TapFileSplitter::TapFileSplitter()
	{
		Clear();
	}



	// Clears the tape.
	void TapFileSplitter::Clear()
	{
		_filesOnTape.Clear();
	}
	


	// Attempts to set the tape to the content of a TAP file.
	bool TapFileSplitter::SetTape( IFileReader *tapFile )
	{
		// Load file:
		// Adding a kind of "NUL terminator" byte, because having a terminator 
		// makes lexical analysis code easy -- there is always at least 1 byte to read!

		Array<uint8_t> fileImage;
		if (LoadFileIntoVector( tapFileOpener, true, &fileImage ))
		{
			_filesOnTape = ParseMultiTapFile(fileImage);
			if (_filesOnTape.Count() > 0)
			{
				return true;
			}
		}
		
		Clear();
		return false;
	}






	size_t TapFileSplitter::GetNumberOfFiles() const
	{
		return _filesOnTape.Count();
	}



	JynxFramework::Array<SignalRLE>  TapFileSplitter::GenerateWaveformForFile( size_t fileIndex, uint32_t bitsPerSecond )
	{
		// Seed constants deduced from analysis of the signal tape files.  (At Lynx TAPE 0, 600bps).
		// "Bits per second" is from Camputers documentation, it may or may not have ever been accurate, I don't know.
		// We don't need to care - everything is calculated in Z80 cycles anyway, and I'm only interested in RATIOs.

		uint32_t zeroSeed = (0x80C * 600) / bitsPerSecond;
		uint32_t oneSeed  = (0xFB1 * 600) / bitsPerSecond;

		SignalWriter  signalWriter(
			SignalLengthInfo( zeroSeed,  zeroSeed + 0x57,  zeroSeed + 0x11F ),    // Signal length information (in Z80 cycles) for a ZERO  (at "TAPE 0" speed).
			SignalLengthInfo( oneSeed,   oneSeed + 0x121,  oneSeed + 0x1F7) );    // Signal length information (in Z80 cycles) for a ONE   (at "TAPE 0" speed).

			//SignalLengthInfo( 0x80C,  0x863,  0x92B ),    // Signal length information (in Z80 cycles) for a ZERO  (at "TAPE 0" speed).
			//SignalLengthInfo( 0xFB1, 0x10D2, 0x11A8 ) );  // Signal length information (in Z80 cycles) for a ONE   (at "TAPE 0" speed).

		auto &thisFile = _filesOnTape[fileIndex];

		// Initial SYNC + A5 applies to all file types:
		signalWriter.WriteSyncAndA5();

		// Types 'B' and 'M' require the file name portion and a second SYNC + A5:
		if( !thisFile->ContentImage.Empty() )  // avoid library assert on address-take if vector empty.
		{
			auto fileTypeLetter = thisFile->ContentImage[0];
			if( fileTypeLetter == 'B' || fileTypeLetter == 'M' )
			{
				signalWriter.WriteByte( 0x22 );
				signalWriter.WriteBytes( (const uint8_t *) (thisFile->FileName.c_str()), thisFile->FileName.Length() );
				signalWriter.WriteByte( 0x22 );
				signalWriter.WriteExtraHighCycles( 0x1F1 );
				signalWriter.WriteSyncAndA5();
			}
		}

		// All types emit the file payload:
		if( ! thisFile->ContentImage.Empty() )  // avoid library assert on address-take if vector empty.
		{
			signalWriter.WriteBytes(
				(const uint8_t *) thisFile->ContentImage.FirstElementVoidPointer(), 
				thisFile->ContentImage.Count());
		}

		return signalWriter.GetRLETapeData();
	}



	String  TapFileSplitter::GetTapeDirectory( TapeDirectoryStyle::Enum styleRequired ) const
	{
		// Retrieves text giving tape content and file types (LOAD / MLOAD).

		StringBuilder sb;

		if(_filesOnTape.Empty() )
		{
			sb.Append("REM No files on tape.\r");
		}
		else if( styleRequired == TapeDirectoryStyle::REMCommandListing )
		{
			for( size_t  i=0; i< _filesOnTape.Count(); i++ )
			{
				sb.Append("REM ") // so is suitable for automated "typing" into the Lynx.
				  .Append(LynxLoadCommandForFile(i))
				  .Append(_filesOnTape[i]->FileName)
				  .Append("\"\r"); // Lynx compatible line ending.
			}
		}
		else if( styleRequired == TapeDirectoryStyle::LoadCommands )
		{
			sb.Append("TAPE 5\r")
			  .Append(LynxLoadCommandForFile(0))
			  .Append(_filesOnTape[0]->FileName)
			  .Append("\"\r"); // Lynx compatible line ending.
		}
		// TODO: else assert(false);

		return sb.ToString();
	}



	String TapFileSplitter::LynxLoadCommandForFile( size_t fileIndex ) const
	{
		// TODO: assert(_filesOnTape[fileIndex].ContentImage[0].Count() > 0); // should always be from the LoadAndParseTapFile() result.

		auto fileType = _filesOnTape[fileIndex]->ContentImage[0];

		if( fileType == 'B' )
		{
			return String("LOAD \"");
		}
		else if( fileType == 'M' )
		{
			return String("MLOAD \"");
		}
		else
		{
			return String("UNKNOWN FILE TYPE \"");  // should never happen.
		}
	}



} // end namespace Jynx
