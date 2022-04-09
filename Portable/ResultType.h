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

namespace JynxFramework
{
	template<typename SUCCESS_TYPE>
	struct Result
	{
	public:
	
		explicit Result( SUCCESS_TYPE &&result )     : SuccessValue( static_cast<SUCCESS_TYPE &&>(result)), StaticFailMessage(nullptr) {}
		Result( const char *msg, const void * )      : StaticFailMessage(msg) {}
	
		Result( const Result<SUCCESS_TYPE> & ) = delete;
		void operator=( const Result<SUCCESS_TYPE> & ) = delete;
		
		// Success indicator for use with eg: if()
		inline operator bool() const                 { return StaticFailMessage != nullptr; }
	
		SUCCESS_TYPE  SuccessValue;
		const char   *StaticFailMessage;

	};
	
	
	template<typename SUCCESS_TYPE>
	inline Result<SUCCESS_TYPE> Success( SUCCESS_TYPE &&result )
	{
		return Result<SUCCESS_TYPE>( static_cast<SUCCESS_TYPE &&>(result) );
	}

	template<typename SUCCESS_TYPE>
	inline auto Fail( const char *staticMessage )
	{
		return Result<SUCCESS_TYPE>(staticMessage, nullptr);
	}

} // end namespace Jynx
