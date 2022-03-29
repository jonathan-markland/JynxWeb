// CassetteFPRefactor.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <stdint.h>
#include "JynxFramework.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//  TAP FILE ITERATOR
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

namespace Jynx
{
    struct TapFileInfo
    {
        TapFileInfo(
            char fileTypeLetter,
            const uint8_t* fileName,
            const uint32_t fileNameLength,
            const uint8_t* body,
            const uint32_t bodyLength)
                : FileTypeLetter(fileTypeLetter)
                , FileName(fileName)
                , FileNameLength(fileNameLength)
                , Body(body)
                , BodyLength(bodyLength)
        {
        }

        const char     FileTypeLetter;
        const uint8_t* FileName;
        const uint32_t FileNameLength;
        const uint8_t* Body;
        const uint32_t BodyLength;
    };
}

namespace JynxTapFileLexer
{
    // File too big => not likely to be a real concatenated TAP file!
    const int MaxTapeFileSize = 100000;

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


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//    TAPE FILE ITERATOR
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

namespace Jynx
{
    // Parse a concatenated-TAP file image, and call the action for each discovered file.
    // The image file must have an additional NUL terminator byte 0x00.
    // Returns true if all parsed successfully, else returns false.
    template<typename ACTION>
    bool ForEachTapeFileDo(
        const uint8_t* fileImageStart,
        const uint8_t* fileImageEnd,
        ACTION action)
    {
        return JynxTapFileLexer::ForEachTapeFileDo(fileImageStart, fileImageEnd, action);
    }
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   File RLE signal generator
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

namespace Jynx
{
    // Run-length encoding datum for TAPE (square-wave) waveforms.
    // Bit 15 has the square wave level:  0=low / 1=high level.
    // Bits 14..0 have the duration in Z80 cycles.
    class SignalRLE
    {
    public:

        SignalRLE(uint16_t rawValue) : Datum(rawValue) {}

        inline uint16_t Duration() const { return Datum & 0x7FFF; }
        inline uint8_t  BitValue() const { return Datum >> 15; }

        const uint16_t Datum;
    };

    // Used by ForTapeRLEDataDo(), not really of interest elsewhere.
    class SignalLengthInfo
    {
    public:

        SignalLengthInfo(uint16_t lowCycles, uint16_t highCyclesAfterBits7to1, uint16_t highCyclesAfterBit0)
            : LowCycles(lowCycles)
            , HighCyclesAfterBits7to1(highCyclesAfterBits7to1)
            , HighCyclesAfterBit0(highCyclesAfterBit0)
        {
        }

        const uint16_t  LowCycles;                   // Start by writing this many cycles LOW VALUE.
        const uint16_t  HighCyclesAfterBits7to1;     // If we are writing bits 7..1 of the byte, write this many cycles HIGH.
        const uint16_t  HighCyclesAfterBit0;         // If we are writing bit 0 of the byte, write this many cycles HIGH.
    };

    class SignalLengthSeeds
    {
    public:

        // Seed constants deduced from analysis of the signal tape files.  (At Lynx TAPE 0, 600bps).
        // "Bits per second" is from Camputers documentation, it may or may not have ever been accurate, I don't know.
        // We don't need to care - everything is calculated in Z80 cycles anyway, and I'm only interested in RATIOs.

        const uint32_t ZeroSeed;
        const uint32_t OneSeed;

        explicit SignalLengthSeeds(uint32_t bitsPerSecond)
            : ZeroSeed( (0x80C * 600) / bitsPerSecond )
            , OneSeed( (0xFB1 * 600) / bitsPerSecond )
        {
        }
    };

    // Signal length information (in Z80 cycles) for 0s and 1s
    class SignalLengths
    {
    public:

        SignalLengthInfo  SignalLengthsForOnes;
        SignalLengthInfo  SignalLengthsForZeroes;

        explicit SignalLengths(SignalLengthSeeds seeds)
            : SignalLengthsForOnes(seeds.ZeroSeed, seeds.ZeroSeed + 0x57, seeds.ZeroSeed + 0x11F)    
            , SignalLengthsForZeroes(seeds.OneSeed, seeds.OneSeed + 0x121, seeds.OneSeed + 0x1F7)    
        {
        }

    };
}

namespace JynxTapFileSignalGenerator
{
    template<typename ACTION_LOW, typename ACTION_HIGH>
    void ForTapeByteDo(
        uint8_t byteValue,
        const Jynx::SignalLengths& lengths,
        ACTION_LOW low,
        ACTION_HIGH high)
    {
        // Loop for bits 7..1 inclusive:
        uint8_t bitMask = 0x80;
        while (bitMask != 1)
        {
            if (byteValue & bitMask)
            {
                low(lengths.SignalLengthsForOnes.LowCycles);
                high(lengths.SignalLengthsForOnes.HighCyclesAfterBits7to1);
            }
            else
            {
                low(lengths.SignalLengthsForZeroes.LowCycles);
                high(lengths.SignalLengthsForZeroes.HighCyclesAfterBits7to1);
            }
            bitMask >>= 1;
        }

        // Finally do bit 0:
        if (byteValue & bitMask)
        {
            low(lengths.SignalLengthsForOnes.LowCycles);
            high(lengths.SignalLengthsForOnes.HighCyclesAfterBit0);
        }
        else
        {
            low(lengths.SignalLengthsForZeroes.LowCycles);
            high(lengths.SignalLengthsForZeroes.HighCyclesAfterBit0);
        }
    }


    // Parse a concatenated-TAP file image, and call the action for each discovered file.
    // The image file must have an additional NUL terminator byte 0x00.
    // Returns true if all parsed successfully, else returns false.
    template<typename ACTION>
    void ForTapeBytesDo(
        const Jynx::TapFileInfo& tapFileInfo,
        const Jynx::SignalLengths &lengths,
        ACTION action)
    {
        // Tape is a square wave.
        // The level is in bit #15 (1=high, 0=low).  Bits 14..0 are the duration in cycles.
        // (The duration encodes whether its a 0 or a 1 being recorded on tape.  1s are longer than 0s).
    
        // Idea: Call the action handler for each data byte, passing the Z80 cycle time offset
        //       at which the byte starts on the tape.  Gaps are HIGHs.

        uint32_t elapsedCycles = 0;

        auto high = [&](uint16_t repeatCount)
        {
            elapsedCycles += repeatCount;
        };

        auto byte = [&](uint8_t byteValue)
        {
            action(elapsedCycles, byteValue);

            ForTapeByteDo(byteValue, lengths,
                [&elapsedCycles](uint16_t lowLength)  { elapsedCycles += lowLength; },
                [&elapsedCycles](uint16_t highLength) { elapsedCycles += highLength; });
        };

        auto bytes = [&](const uint8_t *data, uint32_t length)
        {
            auto end = data + length;
            while (data < end)
            {
                byte(*data);
                ++data;
            }
        };

        auto syncAndA5 = [&]()
        {
            for (int i = 0; i < 768; i++)
            {
                byte(0x00);
            }
            high(0x15);
            byte(0xA5);
        };

        // Types 'B' and 'M' require the file name portion and a second SYNC + A5:
        auto fileTypeLetter = tapFileInfo.FileTypeLetter;
        if (fileTypeLetter == 'B' || fileTypeLetter == 'M')
        {
            syncAndA5();
            byte(0x22);
            bytes((const uint8_t*)(tapFileInfo.FileName), tapFileInfo.FileNameLength);
            byte(0x22);
            high(0x1F1);
        }

        // All types emit the file body:
        syncAndA5();
        bytes(tapFileInfo.Body, tapFileInfo.BodyLength);
    }
}





// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   TAPE DIRECTORY
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 


namespace Jynx
{
    uint32_t CountFilesOnTape(
        const uint8_t* nulTermFileImageStart,
        const uint8_t* nulTermFileImageEnd)
    {
        uint32_t count = 0;

        if (JynxTapFileLexer::ForEachTapeFileDo(
            nulTermFileImageStart,
            nulTermFileImageEnd,
            [&count](const Jynx::TapFileInfo& tapFileInfo)
            {
                ++count;
            }))
        {
            return count;
        }

        return 0; // TODO: parse error.
    }



    JynxFramework::String  LynxLoadCommandForFile(const Jynx::TapFileInfo& tapFileInfo)
    {
        auto fileType = tapFileInfo.FileTypeLetter;

        if (fileType == 'B')
        {
            return JynxFramework::String("LOAD \"");
        }
        else if (fileType == 'M')
        {
            return JynxFramework::String("MLOAD \"");
        }
        else
        {
            return JynxFramework::String("UNKNOWN FILE TYPE \"");  // should never happen.
        }
    }



    JynxFramework::String TapeDirectory(
        const uint8_t* nulTermFileImageStart,
        const uint8_t* nulTermFileImageEnd)
    {
        JynxFramework::StringBuilder sb;

        JynxTapFileLexer::ForEachTapeFileDo(
            nulTermFileImageStart,
            nulTermFileImageEnd,
            [&sb](const Jynx::TapFileInfo& tapFileInfo)
            {
                JynxFramework::Slice<const char>  fileName((const char*)tapFileInfo.FileName, tapFileInfo.FileNameLength);

                sb.Append("REM ") // so is suitable for automated "typing" into the Lynx.
                    .Append(LynxLoadCommandForFile(tapFileInfo))
                    .Append(fileName)
                    .Append("\"\r"); // Lynx compatible line ending.
            });

        if (sb.Empty())
        {
            sb.Append("REM No files on tape.\r");  // TODO: Or parse error on the file.
        }

        return sb.ToString();
    }



    JynxFramework::String LoadCommandForFirstFile(
        const uint8_t* nulTermFileImageStart,
        const uint8_t* nulTermFileImageEnd)
    {
        JynxFramework::StringBuilder sb;
        bool first = true;

        JynxTapFileLexer::ForEachTapeFileDo(
            nulTermFileImageStart,
            nulTermFileImageEnd,
            [&](const Jynx::TapFileInfo& tapFileInfo)
            {
                if (first)
                {
                    JynxFramework::Slice<const char>  fileName((const char*)tapFileInfo.FileName, tapFileInfo.FileNameLength);

                    sb.Append(LynxLoadCommandForFile(tapFileInfo)) // TODO: If the file is unknown, there won't be a REM prefix.
                      .Append(fileName)
                      .Append("\"\r"); // Lynx compatible line ending.
                }
                first = false; // TODO: We cannot request stopping the ForEach
            });

        if (sb.Empty())
        {
            sb.Append("REM No files on tape.\r");  // TODO: Or parse error on the file.
        }

        return sb.ToString();
    }
}




// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

uint8_t FileBuffer[100000];

#include <string>

int main()
{
    auto filename = "C:\\Users\\Jonathan\\Documents\\Jynx for DropBox\\Camputers Lynx\\Tapes\\LOAD COLOSSAL.TAP";
    auto len = 30025;

    FILE* fileHandle = nullptr;
    if (fopen_s(&fileHandle, filename, "rb"))
        return 1;

    if (!fileHandle) return 1;

    fread(FileBuffer, 1, len, fileHandle);

    FileBuffer[len] = 0;

    // Function given a TAP file to return the lynx's load command string for 
    // the first file on the tape.

    // Function given TAP file to return REM commands directory listing.

    // Count the number of files on the tape.  Check for parse error.
    // Given a tape index:
    // - Return info record
    // - Count the number of LOW records needed in the RLE scheme.
    // - Return RLE array

    auto result = 
        JynxTapFileLexer::ForEachTapeFileDo(
            FileBuffer, FileBuffer + (len + 1), 
            [](const Jynx::TapFileInfo& tapFileInfo) 
            {
                std::string fileName((const char *) tapFileInfo.FileName, tapFileInfo.FileNameLength);
                printf("File: %c '%s' %d\n", tapFileInfo.FileTypeLetter, fileName.c_str(), tapFileInfo.BodyLength);

                auto bitsPerSecond = 600;
                auto signalLengths = Jynx::SignalLengths(Jynx::SignalLengthSeeds(bitsPerSecond));

                int count = 0;

                JynxTapFileSignalGenerator::ForTapeBytesDo(tapFileInfo, signalLengths,
                    [&](uint32_t z80CycleMark, uint8_t byteValue)
                    {
                        printf("      Z80-Time: %10d  Byte: %02x\n", z80CycleMark, byteValue);
                        ++count;
                    });

                printf("      Rle count: %d\n", count);
            });
}




/*

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


---------------------------------------------------------------------------------


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

*/