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
#include "../../JynxFramework.h"
#include "../../ResultType.h"

namespace Jynx
{
	// Information about an individual file located with a TAP file.
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