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

#include "../../ExternalModules/JynxFrameworkLibrary/JynxFramework.h"
#include "LynxCassetteReader.h"

// TODO: Post MVP:  In the MVP we statically link a single cassette file only.
extern unsigned char BuiltInCassetteImage[];
extern unsigned int  BuiltInCassetteImageLength;

namespace Jynx
{
	LynxCassetteReader::LynxCassetteReader()
		: _tapeBitStreamSupplier(
				BuiltInCassetteImage, 
				BuiltInCassetteImageLength)
	{
	}
}

