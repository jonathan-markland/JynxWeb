
#pragma once

#include <stdint.h>

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

