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

#include "LynxHardwareCommon.h"

#define LYNX_FRAMEBUF_WIDTH            256   // NOTE: These are not really changeable at all!  If we ever did the 128K machine, these would need to be variables anyway...
#define LYNX_FRAMEBUF_HEIGHT           256   // NOTE: These are not really changeable at all!
#define LYNX_FRAMEBUF_PIXEL_COUNT      (LYNX_FRAMEBUF_WIDTH * LYNX_FRAMEBUF_HEIGHT)

namespace Jynx
{
	class LynxScreen
	{
	public:

		LynxScreen();

		LynxScreen(const LynxScreen &) = delete;
		void operator=(const LynxScreen &) = delete;
		
		void SetPalette(LynxColourSet::Enum colourSet);
		void OnDevicePortValueChanged(uint8_t devicePortValue);
		void OnScreenRamWrite(CHIP *ramChip, uint16_t addressIndex, uint8_t dataByte);
		void OnHardwareReset();

		void MarkWholeScreenInvalid();

		void MarkHostScreenRGBAsAsNeedingRecompose();
		void RecomposeWholeHostScreenRGBAsIfPending();
		
		// TODO: Get address of invalidation flags for Javascript to directly read from the SharedArrayBuffer.
		// TODO: Get address of _hostScreenImage for Javascript to directly read from the SharedArrayBuffer.

		inline CHIP  *GetRedRAM()       { return &_lynxRedRAM; }         // A000 .. BFFF
		inline CHIP  *GetBlueRAM()      { return &_lynxBlueRAM; }        // C000 .. DFFF
		inline CHIP  *GetAltGreenRAM()  { return &_lynxAltGreenRAM; }    // A000 .. BFFF
		inline CHIP  *GetGreenRAM()     { return &_lynxGreenRAM; }       // C000 .. DFFF

	private:
	
		void ComposeHostBitmapPixelsForLynxScreenAddress( uint32_t addressOffset );

	private:
	
		//
		// The RGBA pixels for the host.
		//
	
		uint32_t _hostScreenImage[LYNX_FRAMEBUF_PIXEL_COUNT];

		//
		// The Lynx's chip set  (8K ROMs/RAMs)
		//

		///
		/// Lynx's bank #2   (Video RAM RED and BLUE banks)
		///

		CHIP            _lynxRedRAM;         // A000 .. BFFF
		CHIP            _lynxBlueRAM;        // C000 .. DFFF

		///
		/// Lynx's bank #3   (Video RAM ALTERNATE GREEN and GREEN)
		///

		CHIP            _lynxAltGreenRAM;    // A000 .. BFFF
		CHIP            _lynxGreenRAM;       // C000 .. DFFF
	
		//
		// Video image composition selectors
		//
		// These specify the source data for RED GREEN and BLUE image composition.
		// We support these being NULL to hide that colour from the display.
		//

		CHIP *_sourceVideoRED;
		CHIP *_sourceVideoGREEN;
		CHIP *_sourceVideoBLUE;
	
		//
		// COLOUR PALETTE (indexed by lynx colour 0..7)
		// Interpretation depends on the host.
		//

		LynxColourSet::Enum   _colourSet;  // Added to support Level 9 games.
		uint32_t   _colourPalette[8];  // TODO: persistence?
		uint32_t   _translatedColourPalette[8];  // Translated by host after most recent palette change.

		//
		// Screen area invalidation recording system (guest coordinate space):
		//
		
		enum { INV_ROWS = 32 };          // The vertical height of the screen is divided into this many rows for invalidation recording.  (This is irrespective of 6845 start address alteration).
		volatile bool  _invalidateRow[INV_ROWS];  // Set true when an individual bit is drawn

		//
		// Host-screen RGBAs (_hostScreenImage) recomposition.
		//

		// If we need to recomposite whole screen because of change in Lynx screen register that affects the whole display.
		bool  _recompositeWholeHostRGBAs;   

	};
}