// CassetteFPRefactor.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <stdint.h>
#include "ExternalModules/JynxFrameworkLibrary/JynxFramework.h"
#include "ExternalModules/TapFileLibrary/TapFileFunctions.h"
#include "ExternalModules/TapFileLibrary/TapFileTo8BitWaveAudio.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

uint8_t FileBuffer[100000];

#include <string>

int main()
{
    //auto filename = "C:\\Users\\Jonathan\\Documents\\Jynx for DropBox\\Camputers Lynx\\Tapes\\LOAD COLOSSAL.TAP";
    //auto len = 30025;

    auto filename = "C:\\Users\\Jonathan\\Documents\\Jynx for DropBox\\Camputers Lynx\\Tapes\\LOAD GRIDTRAP.tap";
    auto len = 4708;

    FILE* fileHandle = nullptr;
    if (fopen_s(&fileHandle, filename, "rb"))
        return 1;

    if (!fileHandle) return 1;

    fread(FileBuffer, 1, len, fileHandle);

    fclose(fileHandle);

    FileBuffer[len] = 0;

    // Function given a TAP file to return the lynx's load command string for 
    // the first file on the tape.

    // Function given TAP file to return REM commands directory listing.

    // Count the number of files on the tape.  Check for parse error.
    // Given a tape index:
    // - Return info record
    // - Count the number of LOW records needed in the RLE scheme.
    // - Return RLE array

    auto nulTerminatedImageStart = FileBuffer;
    auto nulTermiantedImageEnd = FileBuffer + (len + 1);

    auto numFilesOnTape = 
        Jynx::CountFilesOnTape(nulTerminatedImageStart, nulTermiantedImageEnd);

    auto directory =
        Jynx::TapeDirectory(nulTerminatedImageStart, nulTermiantedImageEnd);

    auto loadCommand =
        Jynx::LoadCommandForFirstFile(nulTerminatedImageStart, nulTermiantedImageEnd);

    auto audioForWholeTapFile =
        Jynx::TapFileTo8BitWaveAudio(FileBuffer, len, 600, 44100, 4000000, 64);

    {
        FILE* fileHandle = nullptr;
        if (fopen_s(&fileHandle, "C:\\Users\\Jonathan\\Documents\\Work\\GRIDTRAP.raw", "wb"))
            return 1;

        if (!fileHandle) return 1;

        fwrite(audioForWholeTapFile.FirstElementVoidPointer(), 1, audioForWholeTapFile.Count(), fileHandle);

        fclose(fileHandle);
    }

    JynxTapFileParser::ForEachTapeFileDo(
        nulTerminatedImageStart,
        nulTermiantedImageEnd,
        [&](const Jynx::TapFileInfo& tapFileInfo)
        {
            std::string fileName((const char*)tapFileInfo.FileName, tapFileInfo.FileNameLength);
            printf("File: %c '%s' %d\n", tapFileInfo.FileTypeLetter, fileName.c_str(), tapFileInfo.BodyLength);

            auto bitsPerSecond = 600;
            auto signalLengths = Jynx::SignalLengths(Jynx::SignalLengthSeeds(bitsPerSecond));

            auto rleData = GetRleArrayForFile(tapFileInfo, signalLengths);
            auto count = rleData.Count();

            // for (int i=0; i<count; i++)
            // {
            //     printf("    %d  %10d\n", rleData[i].BitValue(), rleData[i].Duration());
            // }
        });
}

