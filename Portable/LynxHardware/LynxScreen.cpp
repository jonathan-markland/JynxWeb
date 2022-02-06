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

#include "LynxScreen.h"

namespace Jynx
{
	LynxScreen::LynxScreen()
	{
		OnHardwareReset();
	}
	
	
	
	void LynxScreen::OnHardwareReset()
	{
		_colourSet        = LynxColourSet::NormalRGB;
		
		_sourceVideoRED   = &_lynxRedRAM;
		_sourceVideoGREEN = &_lynxGreenRAM;  // TODO:  kinda assuming we know the port register value on a reset here.  (very minor)
		_sourceVideoBLUE  = &_lynxBlueRAM;
		
		_recompositeWholeHostRGBAs = false;

		InitialiseAllArrayElementsVolatile( _invalidateRow, true );

		for (uint32_t i=0; i < LYNX_FRAMEBUF_PIXEL_COUNT; i++)
		{
			_hostScreenImage[i] = 0xFFFFFFFFFF;
		}
		
		ZeroInitialiseMemory( _lynxRedRAM );
		ZeroInitialiseMemory( _lynxBlueRAM );
		ZeroInitialiseMemory( _lynxAltGreenRAM );
		ZeroInitialiseMemory( _lynxGreenRAM );

		SetPalette(LynxColourSet::NormalRGB);

		MarkScreenAsNeedingRecompose();
	}
	
	
	
	void LynxScreen::SetPalette(LynxColourSet::Enum colourSet)
	{
		// Fill the palette with RGBA values.
		// Then ask the host to translate the palette to target values.
		// We will quote the target values through the pixel rendering main interface.

		_colourSet = colourSet;

		if( colourSet == LynxColourSet::NormalRGB )
		{
			_colourPalette[0] = 0xFF000000;
			_colourPalette[1] = 0xFFFF0000;
			_colourPalette[2] = 0xFF0000FF;
			_colourPalette[3] = 0xFFFF00FF;
			_colourPalette[4] = 0xFF00FF00;
			_colourPalette[5] = 0xFFFFFF00;
			_colourPalette[6] = 0xFF00FFFF;
			_colourPalette[7] = 0xFFFFFFFF;
		}
		else if( colourSet == LynxColourSet::BlackAndWhiteTV )
		{
			_colourPalette[0] = 0xFF000000;
			_colourPalette[1] = 0xFF444444;
			_colourPalette[2] = 0xFF666666;
			_colourPalette[3] = 0xFF888888;
			_colourPalette[4] = 0xFFAAAAAA;
			_colourPalette[5] = 0xFFCCCCCC;
			_colourPalette[6] = 0xFFDDDDDD;
			_colourPalette[7] = 0xFFFFFFFF;
		}
		else if( colourSet == LynxColourSet::GreenScreenMonitor )
		{
			_colourPalette[0] = 0xFF000000;
			_colourPalette[1] = 0xFF004400;
			_colourPalette[2] = 0xFF006600;
			_colourPalette[3] = 0xFF008800;
			_colourPalette[4] = 0xFF00AA00;
			_colourPalette[5] = 0xFF00CC00;
			_colourPalette[6] = 0xFF00DD00;
			_colourPalette[7] = 0xFF00FF00;
		}
		else if( colourSet == LynxColourSet::GreenOnly )
		{
			_colourPalette[0] = 0xFF000000;
			_colourPalette[1] = 0xFF000000;
			_colourPalette[2] = 0xFF000000;
			_colourPalette[3] = 0xFF000000;
			_colourPalette[4] = 0xFF00FF00;
			_colourPalette[5] = 0xFF00FF00;
			_colourPalette[6] = 0xFF00FF00;
			_colourPalette[7] = 0xFF00FF00;
		}
		else if( colourSet == LynxColourSet::Level9 )
		{
			_colourPalette[0] = 0xFF000000;
			_colourPalette[1] = 0xFF200000;
			_colourPalette[2] = 0xFF000020;
			_colourPalette[3] = 0xFF200020;
			_colourPalette[4] = 0xFFCCFFFF;
			_colourPalette[5] = 0xFFDDFFFF;
			_colourPalette[6] = 0xFFEEFFFF;
			_colourPalette[7] = 0xFFFFFFFF;
		}

		// Establish default translations:
		for( uint32_t  i=0; i < 8; i++ )
		{
			_translatedColourPalette[i] = _colourPalette[i];
		}

		// Allow hosts to translate the values to whatever they desire to use
		// directly as pixel values / indices:
		// TODO:  Do when doing JynxII Desktop version, (the above colours are the Web browser ones):   
			// TODO:  _hostObject->TranslateRGBXColourPaletteToHostValues( _colourPalette, _translatedColourPalette );
			
		MarkHostScreenRGBAsAsNeedingRecompose();
	}
	
	
	
	void LynxScreen::OnDevicePortValueChanged(uint8_t devicePortValue)
	{
		_sourceVideoGREEN = (devicePortValue & DEVICEPORT_USE_ALT_GREEN) ? &_lynxAltGreenRAM : &_lynxGreenRAM;
		_sourceVideoBLUE  = &_lynxBlueRAM;
		_sourceVideoRED   = &_lynxRedRAM;
	}
	
	
	
	void LynxScreen::OnScreenRamWrite(CHIP *ramChip, uint16_t addressIndex, uint8_t dataByte)
	{
		// The caller (the decoder) reminds us of which of our RAM chips we're writing to.
		
		// TODO: We could do with knowing the colour the CHIP relates to, and only write that colour's byte in the RGBAs!
		
		auto ramLocation = ramChip + addressIndex;
		if( *ramLocation != dataByte ) // speed optimisation
		{
			*ramLocation = dataByte;
			ComposeHostBitmapPixelsForLynxScreenAddress( addressIndex );
		}		
	}
	
	
	
	void LynxScreen::ComposeHostBitmapPixelsForLynxScreenAddress( uint32_t addressOffset )
	{
		// Note: Volatile variable access to _invalidateRow[] -- no sync needed.

		// The Lynx's screen memory has just been written at address offset 'addressOffset'.
		// We compose the framebuffer equivalent from the sources, which can be NULL.

		// TODO: Consider: This overall design could cause multiple re-compositions as the banks are written to
		// *WITHIN* the time periods between repaints by the Host.  Just before invalidation
		// we could re-compose the invalid region.  If we did this, it would be desireable to
		// record HIGH RESOLUTION invalid regions, in case just a small section has changed.

		assert( addressOffset < 0x2000 );

		uint32_t  lynxRedByte   = (*_sourceVideoRED)[addressOffset];
		uint32_t  lynxGreenByte = (*_sourceVideoGREEN)[addressOffset];
		uint32_t  lynxBlueByte  = (*_sourceVideoBLUE)[addressOffset];

		uint32_t   pixelDataRGBA[8];

		auto pixelAddress = &pixelDataRGBA[0];
		uint8_t pixelMask = 0x80;
		while( pixelMask != 0 )
		{
			auto lynxInkNumber = 0;
			if( lynxBlueByte  & pixelMask )  lynxInkNumber |= 1;
			if( lynxRedByte   & pixelMask )  lynxInkNumber |= 2;
			if( lynxGreenByte & pixelMask )  lynxInkNumber |= 4;
			*pixelAddress = _translatedColourPalette[lynxInkNumber];
			pixelMask >>= 1;
			++pixelAddress;
		}

		_hostObject->PaintPixelsOnHostBitmap_OnEmulatorThread( addressOffset, pixelDataRGBA );

		/* TODO: 6845 not in MVP for web brower version     if( _rangeMaskedScreenStartAddress6845 != 0 )
		{
			// Since supporting register 12 and 13 on the 6845, we need to adjust for the 6845 start address,
			// because the invalidation recording does not know about the 6845, only the display "straight on".
			// - Convert the lynx byte-address written to a 6845-char address (assuming screen "straight on").
			// - Then adjust for the 6845's screen-start.
			// - Then map that to cartesian coordinates in invalidation-record space.
			auto horizontalChar = addressOffset & 0x1F;
			auto verticalChar   = (addressOffset / (32*4)) & 0x3F;
			auto charOffset     = (verticalChar * 32) + horizontalChar;
			auto adjustedOffset = (charOffset - _rangeMaskedScreenStartAddress6845) & LYNX_DISPLAY_DIMENSIONS_6845_CHARS_MASK;
			auto invRowIndex    = adjustedOffset / 64;

			assert( invRowIndex < INV_ROWS );
			_invalidateRow[ invRowIndex ] = true; // mark a row invalid
		}
		else */
		{
			// Original code for non-fiddled 6845 R12/R13.  (Just playing it super-safe for now until I regression test the above case more!).
			assert( (addressOffset >> 8) < INV_ROWS );
			_invalidateRow[addressOffset >> 8] = true; // mark a row invalid
		}
	}
	
	
	
	void LynxEmulatorGuest::MarkWholeScreenInvalid()
	{
		// (Called on Z80 thread and Main thread).
		// Volatile variable access -- no sync needed.

		InitialiseAllArrayElementsVolatile( _invalidateRow, true );
	}
	
	
	
	void LynxScreen::MarkHostScreenRGBAsAsNeedingRecompose()
	{
		_recompositeWholeHostRGBAs = true;
	}
	
	
	
	void LynxScreen::RecomposeWholeHostScreenRGBAsIfPending()
	{
		if (_recompositeWholeHostRGBAs)
		{
			_recompositeWholeHostRGBAs = false;
			
			for( uint32_t guestScreenAddressOffset = 0x0000; guestScreenAddressOffset < 0x2000; ++guestScreenAddressOffset )
			{
				ComposeHostBitmapPixelsForLynxScreenAddress( guestScreenAddressOffset );
			}
		}
	}

	
}

