
#pragma once

#include "JynxFramework.h"

namespace Jynx
{
    JynxFramework::Array<uint8_t>  TapFileTo8BitWaveAudio(
        const uint8_t* tapFileStart,
        const uint32_t tapFileLengthAsPersisted,
        uint32_t lynxBitsPerSecond,
        uint32_t wavAudioSampleRateHz,
        uint32_t z80CyclesPerSecond,
        uint8_t volumeLevel);
}
