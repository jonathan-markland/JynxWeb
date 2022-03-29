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
// #include "../../JynxFramework.h"
// #include "../../ResultType.h"
#include "TapFileInfo.h"

namespace JynxTapFileLexer
{
    // File too big => not likely to be a real concatenated TAP file!
    const int MaxTapeFileSize = 100000;



    // Parse a concatenated-TAP file image, and call the action for each discovered file.
    // The image file must have an additional NUL terminator byte 0x00.
    // Returns true if all parsed successfully, else returns false, although action handler 
	// may have already been called during this in the case of a later failure.
    template<typename ACTION>
    bool ForEachTapeFileDo(
        const uint8_t* fileImageStart,
        const uint8_t* fileImageEnd,
        ACTION action)
    {
        if (fileImageEnd <= fileImageStart) return false;

        const uint8_t* _position = fileImageStart;
        const uint8_t* _endPosition = fileImageEnd - 1;

        auto spaceRemaining = [&]()
        {
            return _endPosition - _position;
        };

        auto atEnd = [&]()
        {
            return _position == _endPosition;
        };

        if (spaceRemaining() > MaxTapeFileSize) return false; 

        while (!atEnd())
        {
            //
            // START
            //

            // Some TAPs have a spurious A5 at the start:
            if (_position[0] == 0xA5)
            {
                ++_position;  // consume the 0xA5 only.  These sort of redundant A5s will be re-constituted by the replay, and do not need to be part of the data.
            }

            //
            // FILENAME
            //
            // In the block after the FIRST sync, the Lynx stores the file name, in quotes.
            // 'A' files have no file name portion ('A' is used by Level-9 adventures),
            // so we'll be on the letter 'A' here, not '"'
            //

            auto fileName = _position;
            auto fileNameEnd = _position;

            if (_position[0] == '\"')
            {
                ++_position; // skip first "

                fileName = _position;

                while (true)
                {
                    auto byte = *_position;
                    if (byte == 0) return false; //  Fail<String>("Unexpected TAP end");
                    if (byte == '\"') break;
                    ++_position;
                }

                fileNameEnd = _position;

                ++_position; // skip second "
            }

            //
            // FILE BODY
            //

            auto positionOfFileStart = _position;

            auto fileTypeLetter = _position[0];

            // Get file length, and skip rest of header:
            uint8_t  lengthLow = 0;
            uint8_t  lengthHigh = 0;
            if (fileTypeLetter == 'A') // 'A' type files used by Level 9 games, possibly others?
            {
                if (spaceRemaining() < 5) return false; // Fail<Array<uint8_t>>("End of tape while reading final fields of type A file.");
                lengthLow = _position[3];
                lengthHigh = _position[4];
                _position += 5;
            }
            else if (fileTypeLetter == 'B' || fileTypeLetter == 'M')  // Lynx basic or machine code.
            {
                if (spaceRemaining() < 3) return false; // Fail<Array<uint8_t>>("End of tape while reading final fields of type B or M file.");
                lengthLow = _position[1];
                lengthHigh = _position[2];
                _position += 3;
            }
            else return false; // Fail<Array<uint8_t>>("Unrecognised TAP file type.");

            // Determine the image length (as recorded in the file):
            auto payloadLength = (uint32_t)((lengthHigh << 8) | lengthLow);

            // Increase the length to include other known data:
            if (fileTypeLetter == 'B') payloadLength += 3;   // Basic files have 3 extra bytes after the payload
            if (fileTypeLetter == 'M') payloadLength += 7;   // "Machine code" files have 7 extra bytes after the payload
            if (fileTypeLetter == 'A') payloadLength += 12;  // "A" files have 12 extra bytes after the payload

            // Check it fits the file:
            if (payloadLength > spaceRemaining()) return false; // Fail<Array<uint8_t>>("End of tape while reading main payload.");

            // Copy out the raw file data, starting at the file type byte:
            _position += payloadLength;
            auto positionOfFileEnd = _position; //  auto fileBodySlice = Slice<const uint8_t>(positionOfFileStart, _position);

            // Unfortunately, a minority of TAPs, particularly BASIC ones, have a spurious zero
            // at the end.  However, we can skip this, if found, because TAP files always begin
            // with 0x22 0x41 (or the also-spurious 0xA5), never 0x00.
            while (spaceRemaining() > 0)  // Must do this test, so we don't mistake the overall NUL terminator for a spurious zero!
            {
                if (*_position != 0) break;
                ++_position;  // move past spurious zero
            }

            //
            // ACTION
            //

            auto tapFileInfo = Jynx::TapFileInfo(
                fileTypeLetter,
                fileName,
                (uint32_t)(fileNameEnd - fileName),
                positionOfFileStart,
                (uint32_t)(positionOfFileEnd - positionOfFileStart));

            action(tapFileInfo);
        }

        return true;
    }
}


