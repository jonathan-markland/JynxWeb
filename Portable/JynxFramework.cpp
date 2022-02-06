
#include "JynxFramework.h"

#ifdef _WIN32
#include <stdlib.h>
#endif

#ifdef __clang_major__

extern "C" int __cxa_atexit(void (*func) (void *), void *arg, void *d) 
{
	// We are not doing anything about calling at-exit functions because
	// we will not support exiting for Jynx, in lieu of no LibC.  
	// TODO: Sort out properly, if I progress this.
	return 0;
}

#endif


namespace JynxFramework
{
	void RawZeroInitialise( void *block, uintptr_t numBytes )
	{
		uint8_t *ptr = (uint8_t *) block;
		uint8_t *end = ptr + numBytes;
		while (ptr < end)
		{
			*ptr++ = 0;
		}
	}

	
	// Empty string instance used by String class when empty.
	// The String class knows NOT to free this address!
	static const char* EmptyString = "";

	uintptr_t StringLengthOf(const char* s)
	{
		auto length = RawCountUntil(s, [](char c) -> bool { return c == '\0'; });
		return length;
	}

	Slice<char> All(const char* s)
	{
		auto p = (char*)s;
		return Slice<char>(p, p + StringLengthOf(p));
	}

	bool StringCompareSame(const char*s1, const char*s2)
	{
		while (*s1 == *s2)
		{
			if (*s1 == '\0') return true;
			++s1; ++s2;
		}
		return false;
	}

	String::String()
	{
		_string = EmptyString;
	}

	String::String(const char* s)
	{
		ConstructFrom(s);
	}

	String::String(const char* s1, const char* s2)
	{
		auto length1 = StringLengthOf(s1);
		auto length2 = StringLengthOf(s2);
		auto heapBuffer = (char*)malloc(length1 + length2 + 1);
		RawBlockCopy(s1, heapBuffer, length1);
		RawBlockCopy(s2, heapBuffer + length1, length2 + 1);
		_string = heapBuffer;
	}

	String::String(const String& source)
	{
		ConstructFrom(source._string);
	}

	String::String(String &&source) noexcept
	{
		auto heapBuffer = source._string;
		source._string = EmptyString;
		_string = heapBuffer;
	}

	String::String(UnsafeAccept, const char* s)
	{
		_string = s;
	}

	void String::ConstructFrom(const char* s)
	{
		auto length = StringLengthOf(s);
		auto heapBuffer = (char*)malloc(length + 1);
		RawBlockCopy(s, heapBuffer, length + 1);
		_string = heapBuffer;
	}

	String::~String()
	{
		Release();
	}

	void String::Release()
	{
		if (_string != EmptyString) // Address comparison. We must not pass EmptyString to free()!
		{
			free((void*)_string);
			_string = EmptyString;
		}
	}

	void String::operator=(String&&source) noexcept
	{
		Release();
		_string = source._string;
		source._string = EmptyString;
	}

	const char* String::c_str() const
	{
		return _string;
	}

	bool String::Empty() const
	{
		return _string[0] == '\0';
	}

	uintptr_t String::Length() const
	{
		return StringLengthOf(_string);
	}

	Slice<char> String::All() const
	{
		return JynxFramework::All(_string);
	}

	String String::operator+(const char* s) const
	{
		return String(_string, s);
	}

	String String::operator+(const String&s) const
	{
		return String(_string, s.c_str());
	}

	bool String::operator==(const char* s) const
	{
		return StringCompareSame(_string, s);
	}

	bool String::operator==(const String&s) const
	{
		return StringCompareSame(_string, s.c_str());
	}
}



namespace JynxFramework
{
	void IncUsage(UsageCounted*o)
	{
		if (o != nullptr)
		{
			o->_usageCount++;
		}
	}

	void DecUsage(UsageCounted*o)
	{
		if (o != nullptr)
		{
			auto count = o->_usageCount;
			if (count == 1)
			{
				o->~UsageCounted();
				free(o);  // matches malloc in MakeNew().
			}
			else if (count > 1)
			{
				--o->_usageCount;
			}
			// Anything with count == 0 never got attached to a Pointer, so just leave it.
		}
	}
}



namespace JynxFramework
{
	StringBuilder& StringBuilder::AppendChar(char c)
	{
		_buffer.Add(c);
		return *this;
	}

	StringBuilder& StringBuilder::Append(const char* text)
	{
		while (true)
		{
			auto ch = *text;
			if (ch == '\0') break;
			_buffer.Add(ch);
			++text;
		}
		return *this;
	}

	StringBuilder& StringBuilder::Append(const Slice<char>&slice)
	{
		_buffer.Add(slice);
		return *this;
	}

	StringBuilder& StringBuilder::Append(const char* text, uintptr_t count)
	{
		while (count > 0)
		{
			auto ch = *text;
			_buffer.Add(ch);
			--count;
			++text;
		}
		return *this;
	}

	StringBuilder& StringBuilder::Append(const String &s)
	{
		return Append(s.c_str(), s.Length());
	}

	uintptr_t StringBuilder::Count() const
	{
		return _buffer.Count();
	}

	StringBuilder& StringBuilder::Clear()
	{
		_buffer.Clear();
		return *this;
	}

	void StringBuilder::EnsureNulTerminated()
	{
		auto len = _buffer.Count();
		const char* data = (const char*)_buffer.FirstElementVoidPointer();
		if (len == 0 || data[len - 1] != '\0')
		{
			_buffer.Add('\0');
		}
	}

	String StringBuilder::ToString()
	{
		EnsureNulTerminated();
		
		auto theHeapArray = 
			_buffer
				.OptimiseSpace()
				.MoveToArray()
				.Detach();

		return String(UnsafeAccept(), theHeapArray);
	}
}



namespace JynxFramework
{
	static char g_HexChars[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

	char NibbleToChar(uint32_t value)
	{
		return g_HexChars[value & 15];
	}

	uint32_t QuietlyGetValidNumberBase(uint32_t base)
	{
		if (base < 2) { return 2; }
		else if (base > 16) { return 16; }
		return base;
	}
}