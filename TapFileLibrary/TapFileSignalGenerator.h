
#pragma once

#include "../JynxFrameworkLibrary/JynxFramework.h"
#include "TapFileParser.h"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   File RLE signal generator
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

namespace Jynx
{
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

        uint16_t  LowCycles;                   // Start by writing this many cycles LOW VALUE.
        uint16_t  HighCyclesAfterBits7to1;     // If we are writing bits 7..1 of the byte, write this many cycles HIGH.
        uint16_t  HighCyclesAfterBit0;         // If we are writing bit 0 of the byte, write this many cycles HIGH.
    };

    class SignalLengthSeeds
    {
    public:

        // Seed constants deduced from analysis of the signal tape files.  (At Lynx TAPE 0, 600bps).
        // "Bits per second" is from Camputers documentation, it may or may not have ever been accurate, I don't know.
        // We don't need to care - everything is calculated in Z80 cycles anyway, and I'm only interested in RATIOs.

        uint32_t ZeroSeed;
        uint32_t OneSeed;

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
            : SignalLengthsForZeroes(seeds.ZeroSeed, seeds.ZeroSeed + 0x57, seeds.ZeroSeed + 0x11F)  // Signal length information (in Z80 cycles) for a ZERO  (at "TAPE 0" speed).
            , SignalLengthsForOnes(seeds.OneSeed, seeds.OneSeed + 0x121, seeds.OneSeed + 0x1F7)      // Signal length information (in Z80 cycles) for a ONE   (at "TAPE 0" speed).
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
