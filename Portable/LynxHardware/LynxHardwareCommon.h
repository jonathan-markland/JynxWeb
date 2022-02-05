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

namespace Jynx
{
	typedef uint8_t CHIP[8192];
	typedef CHIP *CHIP_PTR;
	typedef CHIP_PTR ADDRESS_SPACE[8];

} // end namespace Jynx

//
// DEVICE PORT
// ~~~~~~~~~~~
// "PORT 80" : Actual I/O space decoding is:  A15..A0 resp:  [xxxx xxxx 10xx x00x]   x=don't care
//

#define DEVICEPORT_INITIALISATION_VALUE  0x0C

#define DEVICEPORT_FRAME_BLANKING  0x80  // Must be kept zeroed
#define DEVICEPORT_LINE_BLANKING   0x40  // If set high, Z80 will be frozen until next line blanking period  [TODO: Or not in the case of this emulator, but then it doesn't emulate snow, either!]
#define DEVICEPORT_CPU_ACCESS      0x20  // Display sharing sync: Halt Z80 until end of scan line.  CPU gets access to video RAM when high.
#define DEVICEPORT_USE_ALT_GREEN   0x10  // Basic returns this to GREEN after a screen access
#define DEVICEPORT_NOT_CASEN_BANK3 0x08  // When high, reading and writing to GREEN and ALT GREEN is impossible.  GREEN & ALT GREEN are not shown
#define DEVICEPORT_NOT_CASEN_BANK2 0x04  // When high, reading and writing to RED and BLUE is impossible.  RED & BLUE are not shown
#define DEVICEPORT_CASSETTE_MOTOR  0x02  // Casette motor on/off
#define DEVICEPORT_SPEAKER         0x01  // Speaker on/off

#define DEVICEPORT_DECODING_MASK   0x00C6
#define DEVICEPORT_KEYBOARD_DECODING_MASK   0x0FC6

