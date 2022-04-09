
#pragma once

#include <stdint.h>
#include "JynxFramework.h"
#include "TapFileSignalGenerator.h"  // SignalLengths
#include "TapFileFunctions.h"        // SignalRLE


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   Tape server state machine
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

namespace Jynx
{
    class TapeBitStreamSupplier
    {
    public:

        // Construct from buffer containing TAP file image (ie: as the file is persisted).
        TapeBitStreamSupplier(
            const uint8_t* tapFileStart,
            const uint32_t tapFileLength);

        // Notify tape motor switched on at given time, and rate set.
        void TapeMotorOn(uint32_t bitsPerSecond);

        // Notify tape motor stopped for this file.
        void TapeMotorOff();

        // Must be called with forward-increasing times.
        uint8_t ReadBit(uint64_t z80CycleCountNow);

        // Obtain index of *next* file.  Will be 1 beyond end when on the final file.
        uint32_t GetNextFileIndex() const { return _nextFileIndex; }

        // Return true if file exists and we're still going.
        bool FileStillGoing() const { return !EndMet() && _rleIndex < _currentFileRle.Count(); }

        // Overall end met, also if parse fail halted this.
        bool EndMet() const { return _allDone; }

    private:

        // Unpack RLE data for _nextFileIndex, and initialise for ReadBit() calls.
        void PrepareToStartReadingFile();

        // Advance _rleIndex and _fileOffset by z80Cycles, to the 
        // location of the sampling point.
        void AdvancePositionByZ80Cycles(uint64_t z80Cycles);

        // Copy of original image with NUL terminator added.
        // (Decided not to include NUL in on-disc file in JynxII preview MVP, just add the NUL terminator in memory).
        JynxFramework::Array<uint8_t> _tapFileWithNulTerminator;

        // Aliases _tapFileWithNulTerminator
        const uint8_t* _nulTermFileImageStart;

        // Aliases _tapFileWithNulTerminator
        const uint8_t* _nulTermFileImageEnd;

        bool _allDone;

        // Tape motor setting
        bool _tapeMotorOn;

        // Index of next file to select (may be beyond end).
        uint32_t _nextFileIndex;

        // Set when TapeMotorOn() called in readiness for next file.
        mutable Jynx::SignalLengths _currentSignalLengths;

        // Resync flag
        bool _resyncOnTapeMotorOn;

        // Time of previous ReadBit call(), set to start time on resync.
        uint64_t _previousReadBitZ80Time;

        // Set on the first ReadBit() after tape motor is turned on.
        JynxFramework::Array<SignalRLE> _currentFileRle;

        // Set on the first ReadBit() after tape motor is turned on.  (May be beyond end).
        uint32_t _rleIndex;

        // Z80 cycle count offset into _currentFileRle[_rleIndex].
        // Set to 0 on the first ReadBit() after tape motor is turned on.
        uint16_t _fineOffset;

    };
}
