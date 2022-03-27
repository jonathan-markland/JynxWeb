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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   File RLE signal generator
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

namespace Jynx
{
    // Run-length encoding datum for TAPE (square-wave) waveforms.
    // Bit 15 has the square wave level:  0=low / 1=high level.
    // Bits 14..0 have the duration in Z80 cycles.
    /* TODO: not yet used: class SignalRLE
    {
    public:

        SignalRLE(uint16_t rawValue) : Datum(rawValue) {}

        inline uint16_t Duration() const { return Datum & 0x7FFF; }
        inline uint8_t  BitValue() const { return Datum >> 15; }

        const uint16_t Datum;
    };*/
	
	

	// Record containing values used to determine SignalLengths.
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
	
	

    // Signal length information (in Z80 cycles) for either 0s or 1s
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
    void ForTapeByteSignalDo(
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



	// For each byte in a single tape file, call the action handler, 
	// reporting the Z80 cycle count at which the byte starts,
	// and the byte value.  This behaviour includes the sync and
	// data.  Any time gaps are logic HIGH.  Hint: The caller could
	// use ForTapeByteSignalDo() to reconstitute the timings for the
	// bits within the byte.
    template<typename ACTION>
    void ForTapFileSignalBytesDo(
        const Jynx::TapFileInfo& tapFileInfo,
        const Jynx::SignalLengths &lengths,
        ACTION action)
    {
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
