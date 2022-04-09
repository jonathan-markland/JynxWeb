
#include "TapeBitStreamSupplier.h"
#include "TapFileTo8BitWaveAudio.h"

namespace Jynx
{
    template<typename LEVEL_FUNCTION>
    bool ForTapFileWaveAudioDo(
        const uint8_t* tapFileStart,
        const uint32_t tapFileLengthAsPersisted,
        uint32_t lynxBitsPerSecond,
        uint32_t wavAudioSampleRateHz,
        uint32_t interFilePauseLength,
        uint32_t z80CyclesPerSecond,
        LEVEL_FUNCTION levelFunction)
    {
        auto z80TimePerSample = (double)z80CyclesPerSecond / (double)wavAudioSampleRateHz;

        TapeBitStreamSupplier  bitStream(tapFileStart, tapFileLengthAsPersisted); // TODO: This wants it AS PERSISTED, not NUL TERMINATED but ForEachTapeFileDo wants NULL TERMINATED.

        double  cycleCount = 0.0;

        while (! bitStream.EndMet())
        {
            bitStream.TapeMotorOn(lynxBitsPerSecond);

            auto bitValue = bitStream.ReadBit((uint64_t)(cycleCount + 0.5));  // Causes first resync
            auto index = bitStream.GetNextFileIndex();   // Continue resampling until this changes.  [Can only obtain this after the first ReadBit call]

            while (bitStream.FileStillGoing())
            {
                levelFunction(bitValue);
                cycleCount += z80TimePerSample;
                bitValue = bitStream.ReadBit((uint64_t)(cycleCount + 0.5));
            }

            bitStream.TapeMotorOff();

            for (uint32_t  i = 0; i < interFilePauseLength; i++)
            {
                levelFunction(1);  // arbitrary level.
            }
        }

        return true; // TODO: for now.  We need to know if bitStream errored.
    }



    JynxFramework::Array<uint8_t>  TapFileTo8BitWaveAudio(
        const uint8_t* tapFileStart,
        const uint32_t tapFileLengthAsPersisted,
        uint32_t lynxBitsPerSecond,
        uint32_t wavAudioSampleRateHz,
        uint32_t z80CyclesPerSecond,
        uint8_t volumeLevel)
    {
        JynxFramework::ArrayBuilder<uint8_t>  wavBytesBuilder;

        const uint32_t interFilePauseLength = wavAudioSampleRateHz; // ie: one second at the target rate.

        auto success = ForTapFileWaveAudioDo(
            tapFileStart,
            tapFileLengthAsPersisted,
            lynxBitsPerSecond,
            wavAudioSampleRateHz,
            interFilePauseLength,
            z80CyclesPerSecond,
            [&](uint8_t bitValue)
            {
                wavBytesBuilder.Add(bitValue * volumeLevel);
            });

        return success ? wavBytesBuilder.MoveToArray() : JynxFramework::Array<uint8_t>();  // TODO: reconsider?
    }
}
