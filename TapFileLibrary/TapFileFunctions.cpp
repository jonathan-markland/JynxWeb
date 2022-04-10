
#include "TapFileFunctions.h"

namespace Jynx
{
    uint32_t CountFilesOnTape(
        const uint8_t* nulTermFileImageStart,
        const uint8_t* nulTermFileImageEnd)
    {
        uint32_t count = 0;

        if (JynxTapFileParser::ForEachTapeFileDo(
            nulTermFileImageStart,
            nulTermFileImageEnd,
            [&count](const Jynx::TapFileInfo& tapFileInfo)
            {
                ++count;
            }))
        {
            return count;
        }

            return 0; // TODO: parse error.
    }



    JynxFramework::String  LynxLoadCommandForFile(
        const Jynx::TapFileInfo& tapFileInfo)
    {
        auto fileType = tapFileInfo.FileTypeLetter;

        if (fileType == 'B')
        {
            return JynxFramework::String("LOAD \"");
        }
        else if (fileType == 'M')
        {
            return JynxFramework::String("MLOAD \"");
        }
        else
        {
            return JynxFramework::String("UNKNOWN FILE TYPE \"");  // should never happen.
        }
    }



    JynxFramework::String  TapeDirectory(
        const uint8_t* nulTermFileImageStart,
        const uint8_t* nulTermFileImageEnd)
    {
        JynxFramework::StringBuilder sb;

        JynxTapFileParser::ForEachTapeFileDo(
            nulTermFileImageStart,
            nulTermFileImageEnd,
            [&sb](const Jynx::TapFileInfo& tapFileInfo)
            {
                JynxFramework::Slice<const char>  fileName((const char*)tapFileInfo.FileName, tapFileInfo.FileNameLength);

                sb.Append("REM ") // so is suitable for automated "typing" into the Lynx.
                    .Append(LynxLoadCommandForFile(tapFileInfo))
                    .Append(fileName)
                    .Append("\"\r"); // Lynx compatible line ending.
            });

        if (sb.Empty())
        {
            sb.Append("REM No files on tape.\r");  // TODO: Or parse error on the file.
        }

        return sb.ToString();
    }



    JynxFramework::String  LoadCommandForFirstFile(
        const uint8_t* nulTermFileImageStart,
        const uint8_t* nulTermFileImageEnd)
    {
        JynxFramework::StringBuilder sb;
        bool first = true;

        JynxTapFileParser::ForEachTapeFileDo(
            nulTermFileImageStart,
            nulTermFileImageEnd,
            [&](const Jynx::TapFileInfo& tapFileInfo)
            {
                if (first)
                {
                    JynxFramework::Slice<const char>  fileName((const char*)tapFileInfo.FileName, tapFileInfo.FileNameLength);

                    sb.Append(LynxLoadCommandForFile(tapFileInfo)) // TODO: If the file is unknown, there won't be a REM prefix.
                        .Append(fileName)
                        .Append("\"\r"); // Lynx compatible line ending.
                }
                first = false; // TODO: We cannot request stopping the ForEach
            });

        if (sb.Empty())
        {
            sb.Append("REM No files on tape.\r");  // TODO: Or parse error on the file.
        }

        return sb.ToString();
    }




    uint32_t  CountRLEDataPointsForFile(
        const Jynx::TapFileInfo& tapFileInfo, 
        const Jynx::SignalLengths& lengths)
    {
        uint32_t rleCount = 0;

        JynxTapFileSignalGenerator::ForTapeBytesDo(
            tapFileInfo,
            lengths,
            [&rleCount](uint16_t /*lowLength*/) { ++rleCount; },
            [&rleCount](uint16_t /*highLength*/) { ++rleCount; });

        return rleCount;
    }



    JynxFramework::Array<SignalRLE>  GetRleArrayForFile(
        const Jynx::TapFileInfo& tapFileInfo,
        const Jynx::SignalLengths& lengths)
    {
        auto rleRecordCount = CountRLEDataPointsForFile(tapFileInfo, lengths);
        JynxFramework::ArrayBuilder<SignalRLE> builder(rleRecordCount);

        JynxTapFileSignalGenerator::ForTapeBytesDo(
            tapFileInfo,
            lengths,
            [&builder](uint16_t lowLength)  { builder.Add( SignalRLE(lowLength) ); },
            [&builder](uint16_t highLength) { builder.Add( SignalRLE(highLength | 0x8000) ); });

        return builder.MoveToArray();
    }
}

