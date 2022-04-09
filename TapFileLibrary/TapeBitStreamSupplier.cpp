
#include "TapeBitStreamSupplier.h"


namespace Jynx
{
    TapeBitStreamSupplier::TapeBitStreamSupplier(
        const uint8_t* tapFileStart,
        const uint32_t tapFileLength)
        : _currentSignalLengths(Jynx::SignalLengthSeeds(600))  // This initialisation satisfies the compiler -- is not actually used.
        , _tapeMotorOn(false)
        , _nextFileIndex(0)
        , _rleIndex(0)
        , _fineOffset(0)
        , _resyncOnTapeMotorOn(true)
        , _previousReadBitZ80Time(0)
        , _allDone(false)
    {
        auto nulTerminated = JynxFramework::ArrayInit<uint8_t>((uintptr_t)tapFileLength + 1, 0);
        for (uint32_t i = 0; i < tapFileLength; i++)
        {
            nulTerminated[i] = tapFileStart[i];
        }
        nulTerminated[tapFileLength] = 0;
        _tapFileWithNulTerminator = JynxFramework::Move(nulTerminated);
        _nulTermFileImageStart = _tapFileWithNulTerminator.All().Start();
        _nulTermFileImageEnd = _tapFileWithNulTerminator.All().End();
    }



    void TapeBitStreamSupplier::TapeMotorOn(uint32_t bitsPerSecond)
    {
        _tapeMotorOn = true;
        _currentSignalLengths = Jynx::SignalLengths(Jynx::SignalLengthSeeds(bitsPerSecond));
        _resyncOnTapeMotorOn = true;
    }



    void TapeBitStreamSupplier::TapeMotorOff()
    {
        _tapeMotorOn = false;
        _resyncOnTapeMotorOn = true;
    }



    uint8_t TapeBitStreamSupplier::ReadBit(uint64_t z80CycleCountNow)
    {
        if (_tapeMotorOn && !_allDone)
        {
            if (_resyncOnTapeMotorOn)
            {
                PrepareToStartReadingFile();
                _previousReadBitZ80Time = z80CycleCountNow;
                _resyncOnTapeMotorOn = false;
            }
            else
            {
                AdvancePositionByZ80Cycles(z80CycleCountNow - _previousReadBitZ80Time);
                _previousReadBitZ80Time = z80CycleCountNow;

                if (_rleIndex < _currentFileRle.Count())
                {
                    return _currentFileRle[_rleIndex].BitValue();
                }
            }
        }

        // Default to returning a HIGH signal level (arbitrary really, but should remain constant).
        return 1;
    }



    void TapeBitStreamSupplier::AdvancePositionByZ80Cycles(uint64_t z80Cycles)
    {
        while (_rleIndex < _currentFileRle.Count())
        {
            auto currentRleItemDuration = _currentFileRle[_rleIndex].Duration();
            auto datumRemainder = currentRleItemDuration - _fineOffset;
            if (z80Cycles >= datumRemainder)
            {
                // Move onto next datum
                z80Cycles -= datumRemainder;
                _fineOffset = 0;
                ++_rleIndex;
            }
            else
            {
                // Destination lies within this datum
                _fineOffset += (uint16_t)z80Cycles;  // always fits
                break;
            }
        }
    }



    void TapeBitStreamSupplier::PrepareToStartReadingFile()
    {
        uint32_t  index = 0;
        JynxFramework::Array<SignalRLE> foundRleData;

        if (JynxTapFileParser::ForEachTapeFileDo( // TODO: I don't cache the tapFileInfos, so re-parse
            _nulTermFileImageStart,
            _nulTermFileImageEnd,
            [&](const Jynx::TapFileInfo& tapFileInfo)
            {
                if (index == _nextFileIndex)
                {
                    foundRleData = GetRleArrayForFile(tapFileInfo, _currentSignalLengths);
                    // TODO: We cannot stop the enumeration == idealism but not essential.
                }
                ++index;
            }))
        {
            if (foundRleData.Empty())
            {
                _allDone = true;
            }
            else
            {
                _currentFileRle = JynxFramework::Move(foundRleData);
                ++_nextFileIndex; // for the next file
                _rleIndex = 0;
                _fineOffset = 0;
            }
        }
        else
        {
            _allDone = true;
        }
    }
}

