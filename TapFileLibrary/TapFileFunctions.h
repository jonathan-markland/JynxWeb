
#pragma once

#include "../JynxFrameworkLibrary/JynxFramework.h"
#include "TapFileInfo.h"
#include "TapFileParser.h"
#include "TapFileSignalGenerator.h"



namespace Jynx
{
    uint32_t  CountFilesOnTape(
        const uint8_t* nulTermFileImageStart,
        const uint8_t* nulTermFileImageEnd);



    JynxFramework::String  LynxLoadCommandForFile(
        const Jynx::TapFileInfo& tapFileInfo);



    JynxFramework::String  TapeDirectory(
        const uint8_t* nulTermFileImageStart,
        const uint8_t* nulTermFileImageEnd);



    JynxFramework::String  LoadCommandForFirstFile(
        const uint8_t* nulTermFileImageStart,
        const uint8_t* nulTermFileImageEnd);



    uint32_t  CountRLEDataPointsForFile(
        const Jynx::TapFileInfo& tapFileInfo, 
        const Jynx::SignalLengths& lengths);



    // Run-length encoding datum for TAPE (square-wave) waveforms.
    // Bit 15 has the square wave level:  0=low / 1=high level.
    // Bits 14..0 have the duration in Z80 cycles.
    class SignalRLE
    {
    public:

        explicit SignalRLE(uint16_t rawValue) : Datum(rawValue) {}

        inline uint16_t Duration() const { return Datum & 0x7FFF; }
        inline uint8_t  BitValue() const { return Datum >> 15; }

        const uint16_t Datum;
    };



    JynxFramework::Array<SignalRLE>  GetRleArrayForFile(
        const Jynx::TapFileInfo& tapFileInfo,
        const Jynx::SignalLengths& lengths);
}

