#pragma once

#include <stdint.h>
#include "JynxFrameworkPanic.h"

#ifdef _WIN32
#include <new>
#endif

#ifdef __clang_major__
// We are not using a LIBC with Clang.
typedef decltype(nullptr) nullptr_t;
#include "WasmNeverFreeingMemoryAllocator.h"
inline void* operator new(uintptr_t, void* p) noexcept { return p; }
#endif

template<typename T> 
inline T &&forward(T& v) 
{ 
	return static_cast<T&&>(v); 
}

namespace JynxFramework
{
	template<typename T>
	inline T&& Move(T& source)
	{
		return static_cast<T&&>(source);
	}

	template<typename T>
	void RawBlockCopy(const T* source, T* dest, uintptr_t count)
	{
		while (count > 0)
		{
			*dest++ = *source++;
			--count;
		}
	}

	template<typename T, typename PRED>
	uintptr_t RawCountUntil(const T* source, PRED pred)
	{
		uintptr_t count = 0;
		while (! pred(*source))
		{
			++source;
			++count;
		}
		return count;
	}

	// Return the length of the NUL terminated string.
	uintptr_t StringLengthOf(const char*);

	// Return true if the strings are identical, else false.
	bool StringCompareSame(const char*, const char*);

	// Return the span size in bytes.
	inline uintptr_t SpanSizeInBytes(const void* start, const void* end)
	{
		return ((uintptr_t)end) - ((uintptr_t)start);
	}

	template<typename ELEMENT>
	void DefaultConstruct(void* uninitialisedMemoryBlock, uintptr_t count)
	{
		// TODO: Exception safety
		auto destination = reinterpret_cast<ELEMENT*>(uninitialisedMemoryBlock);
		while (count > 0)
		{
			new ((void *) destination) ELEMENT();
			++destination;
			--count;
		}
	}

	template<typename ELEMENT>
	void CopyConstructDisjoint(const ELEMENT* source, const ELEMENT* end, void* uninitialisedMemoryBlock)
	{
		// TODO: Exception safety
		auto destination = reinterpret_cast<ELEMENT*>(uninitialisedMemoryBlock);
		while (source < end)
		{
			new (destination) ELEMENT(*source);
			++source;
			++destination;
		}
	}

	template<typename ELEMENT>
	void MoveConstructDisjoint(ELEMENT* source, void * uninitialisedMemoryBlock, uintptr_t count)
	{
		// TODO: Exception safety
		auto destination = reinterpret_cast<ELEMENT*>(uninitialisedMemoryBlock);
		while (count > 0)
		{
			new (destination) ELEMENT(Move(* source));
			++source;
			++destination;
			--count;
		}
	}

	template<typename ELEMENT>
	void Destruct(ELEMENT* target, uintptr_t count)
	{
		// TODO: Exception safety
		while (count > 0)
		{
			target->~ELEMENT();
			++target;
			--count;
		}
	}

	// Make a copy of the source elements as an array on the heap.
	template<typename ELEMENT>
	ELEMENT *CopyToNewHeapArray(const ELEMENT* source, const ELEMENT *end)
	{
		auto block = malloc(SpanSizeInBytes(source, end));
		PanicIfNull(block, "Out of memory");
		CopyConstructDisjoint(source, end, block);
		return (ELEMENT*)block;
	}

	// Marker used to call unsafe overloaded constructors.
	// Only for low-level interfacing, not for general use.
	class UnsafeAccept {};

	// A view of a portion of an array.
	template<typename T>
	class Slice
	{
	public:
		
		Slice(T* start, T* end)            : _start(start), _end(end) {}
		Slice(T* start, uintptr_t count)   : _start(start), _end(start + count) {}
		
		// Get start of slice.
		T* Start() const { return _start; }

		// Get end of slice (non-inclusive in the range).
		T* End()   const { return _end; }

		// Get number of elements in the slice.
		uintptr_t Count() const { return _end - _start; }

		// Return true if this is empty, else false.
		bool Empty() const { return Count() == 0; }

		// TODO: First(n)
		// TODO: Last(n)
		// TODO: From(n)
		// TODO: Range(n, count)

		Slice(Slice&&) = default;
		
		void operator=(Slice&& s) = delete;
		Slice(const Slice&) = delete;
		void operator=(const Slice&) = delete;

	private:

		T* _start;
		T* _end;

	};

	// NUL terminated string to slice. Slice excludes the NUL terminator.
	Slice<char> All(const char*);

	// Immutable string on heap. The object size is simply a pointer.
	class String
	{
	public:

		// Default construction is empty string.
		String();

		// Construction taking a copy of the source string.
		String(const String&);

		// Construction by moving from source, leaving source as empty string.
		String(String&&) noexcept;

		// Construction taking a copy of the source string.
		explicit String(const char* s);

		// Construction taking a copy of the content of the source slice.
		explicit String(const Slice<const char>&);

		// Construction concatenating the two strings.
		String(const char* s1, const char *s2);

		// Low-level construction, where s must be a free()able heap block
		// containing the NUL-terminated string.  This should not be generally used.
		String(UnsafeAccept, const char* s); // TODO: Slice version?

		// Release heap block containing string.
		~String();

		// Return address of NUL-terminated character array, in heap block.
		const char* c_str() const;

		// Return true if string is empty.
		bool Empty() const;

		// Return length of string.
		uintptr_t Length() const;

		// Return string as Slice.
		Slice<char> All() const;

		// Return a new string that is the concatenation result.
		String operator+(const char* s) const;

		// Return a new string that is the concatenation result.
		String operator+(const String &) const;

		// Compare strings for exact match.
		bool operator==(const char* s) const;

		// Compare strings for exact match.
		bool operator==(const String&) const;

		// Compare strings for NOT exact match.
		template<typename T>
		bool operator!=(T&& t) const { return !operator==(t); }

		void operator=(String&&) noexcept;
		void operator=(const String&) = delete;

	private:

		void Release();
		void ConstructFrom(const char*);
		void ConstructFromPart(const char*, uintptr_t length);
		const char *_string;

	};

	class VirtualDestructibleObject
	{
	public:
		virtual ~VirtualDestructibleObject() {}
	};

	// Heap Object -- usage counted

	// Base class for usage-countable object.
	// These can still be stored by containment or on the stack since
	// the usage count will remain 0.
	class UsageCounted : VirtualDestructibleObject
	{
	public:
		UsageCounted() : _usageCount(0) {}
		int _usageCount;
	};

	// A box resides on the heap and contains a regular type.
	// A usage counter and virtual destructor is added.
	template<typename BOXTYPE>
	class Boxed : public UsageCounted
	{
	public:

		template<typename ...Args>
		Boxed(UnsafeAccept, Args&&... args) : Contained(forward<Args>(args)...) {}

		Boxed() = delete;
		Boxed(Boxed&&) = delete;
		Boxed(const Boxed&) = delete;
		void operator=(Boxed&&) = delete;
		void operator=(const Boxed&) = delete;

		BOXTYPE Contained;
	};

	// Usage-counted pointer.

	void IncUsage(UsageCounted*);
	void DecUsage(UsageCounted*);

	// Usage-counted pointer to heap-based objects derived from class Object.
	// Single-threaded app only.
	template<typename T>
	class Pointer
	{
	public:
		
		// Null pointer construction.
		Pointer()
		{
			_ptr = nullptr;
		}

		// Copy construction (exact type match)
		Pointer(const Pointer<T>& p)
		{
			_ptr = p._ptr;
			IncUsage(_ptr);
		}

		// Copy construction from polymorphic match.
		template<typename Matching> 
		Pointer(const Pointer<Matching>&p)
		{
			_ptr = p._ptr;
			IncUsage(_ptr);
		}

		// Move construction from exact type match.
		// Moved-from Pointer will be nullptr.
		Pointer(Pointer<T>&& p)
		{
			_ptr = p._ptr;
			p._ptr = nullptr;
		}

		// Move construction from polymorphic match.
		// Moved-from Pointer will be nullptr.
		template<typename Matching> 
		Pointer(Pointer<Matching>&&p)
		{
			_ptr = p._ptr;
			p._ptr = nullptr;
		}

		// Construction by moving object 'o' to new heap block.
		Pointer(T&& o)
		{
			Boxed<T>* block = (Boxed<T>*) malloc(sizeof(Boxed<T>));   // NB: Matches free in DecUsage().
			PanicIfNull(block, "Out of memory");
			new ((void*)block) Boxed<T>(UnsafeAccept(), Move(o));
			_ptr = (Boxed<T>*)block;
			IncUsage(_ptr);
		}

		// Construction with forced address. Unsafe and unverifiable.
		Pointer(UnsafeAccept, Boxed<T>* unverifiableAddress)
		{
			_ptr = unverifiableAddress;
			IncUsage(_ptr);
		}

		// Destruction. Decremement usage on any referred object.
		~Pointer()
		{
			auto p = _ptr;
			_ptr = nullptr; // pedantic
			DecUsage(p);
		}

		// Assign exact same type.
		void operator=(const Pointer<T>& p)
		{
			auto old = _ptr;
			_ptr = p._ptr;
			IncUsage(_ptr);
			DecUsage(old);
		}

		// Assign polymorphic matching type.
		template<typename TDerived>
		void operator=(const Pointer<TDerived>& p)
		{
			static_cast<T*>((TDerived *) nullptr); // TDerived must be derived from T.  // TODO: template constraint?
			auto old = _ptr;
			_ptr = (Boxed<T>*) p._ptr;
			IncUsage(_ptr);
			DecUsage(old);
		}

		// Move-assign same type or polymorphic matching type.
		// Moved-from Pointer will be nullptr.
		template<typename TDerived>
		void operator=(Pointer<TDerived>&& p)
		{
			static_cast<T*>((TDerived*) nullptr); // TDerived must be derived from T.  // TODO: template constraint?
			auto old = _ptr;
			_ptr = (Boxed<T>*) p._ptr;
			p._ptr = nullptr;
			DecUsage(old);
		}

		T* operator->() const 
		{ 
			return &_ptr->Contained; 
		}
		
		T* operator*() const 
		{ 
			return &_ptr->Contained; 
		}

		bool operator==(const nullptr_t) const
		{
			return _ptr == nullptr;
		}

		bool operator!=(const nullptr_t) const
		{
			return _ptr != nullptr;
		}

		// Address comparison.
		template<typename TOther>
		bool operator==(const Pointer<TOther>& rhs) const
		{
			return (void*)_ptr == (void*)rhs._ptr;
		}
		
		// Address comparison.
		template<typename TOther>
		bool operator!=(const Pointer<TOther>& rhs) const
		{
			return (void*)_ptr != (void*)rhs._ptr;
		}

		// Address comparison.
		template<typename TOther>
		bool operator<(const Pointer<TOther>& rhs) const
		{
			return (void*)_ptr < (void*)rhs._ptr;
		}

		// Delegate indexing to support array-like targets.
		template<typename INDEX>
		auto &operator[](INDEX index)
		{
			return (_ptr->Contained)[index];
		}

		// Obtain usage count for debugging.
		uintptr_t UsageCount() const
		{
			return _ptr->_usageCount;
		}

		// Returns the address of the Boxed<T>, not the T itself!
		void* ToVoidPointer() const
		{
			return _ptr;
		}

	public:  // TODO: hack because a Pointer-to-derived class cannot access this in the base pointer!

		Boxed<T>* _ptr;

	};

	// Allocate an instance of object of type T on the heap,
	// and call its constructor with the given arguments.
	template<typename T, typename... ConstructionArgs>
	Pointer<T> MakeNew(ConstructionArgs... args)
	{
		Boxed<T>* block = (Boxed<T>*) malloc(sizeof(Boxed<T>));   // NB: Matches free in DecUsage().
		PanicIfNull(block, "Out of memory");
		new ((void*)block) Boxed<T>(UnsafeAccept(), args...);
		return Pointer<T>(UnsafeAccept(), block);
	}

	// Move existing object to the heap, wrapping it with Pointer.
	template<typename T>
	inline Pointer<T> MoveToHeap(T&object)
	{
		return Pointer<T>(Move(object));
	}

	// Array

	template<typename T>
	class Array
	{
	public:

		Array()
		{
			_data = nullptr;
			_count = 0;
		}

		// Construction from slice.  Content is copied to fresh heap block.
		template<typename U>
		Array(const Slice<U>& slice) // template to allow const/non-const source
		{
			auto start = slice.Start();
			auto end = slice.End();
			_data = CopyToNewHeapArray(start, end);
			_count = end - start;
		}

		// Move constructor.
		// Moved-from Array will be nullptr with count 0.
		Array(Array&& a)
		{
			_data = a._data;
			_count = a._count;
			a._count = 0;
			a._data = nullptr;
		}

		// Construction with forced array address and count. Unsafe and unverifiable.
		Array(UnsafeAccept, T* unverifiableBaseAddress, uintptr_t unverifiableCount)
		{
			_data = unverifiableBaseAddress;
			_count = unverifiableCount;
		}

		// Destruct, releasing array immediately.
		~Array()
		{
			DeleteArray();
		}

		// Move assignment.
		// Moved-from Array will be nullptr with count 0.
		void operator=(Array&& a) noexcept
		{
			DeleteArray();
			_data = a._data;
			_count = a._count;
			a._count = 0;
			a._data = nullptr;
		}

		// Checked indexing.
		T& operator[](uintptr_t index)
		{
			if (index < _count)
			{
				return _data[index];
			}
			else
			{
				Panic("Array index out of bounds.");
				return *(T*)nullptr;
			}
		}

		// Checked indexing.
		const T& operator[](uintptr_t index) const
		{
			if (index < _count)
			{
				return _data[index];
			}
			else
			{
				Panic("Array index out of bounds.");
				return *(const T*)nullptr;
			}
		}

		// Returns number of elements in the array.
		uintptr_t Count() const
		{
			return _count;
		}

		// Return true if this is empty, else false.
		bool Empty() const { return Count() == 0; }

		// Returns a reference to the whole array as a Slice.
		Slice<T> All() const
		{
			return Slice<T>(_data, _data + _count);
		}

		// For each iteration.
		template<typename ACTION>
		void ForEachDo(ACTION action) // TODO: const correctness?
		{
			auto ptr = _data;
			auto end = _data + _count;
			while (ptr < end)
			{
				action(*ptr);
				++ptr;
			}
		}

		// For each iteration with index.
		template<typename ACTION>
		void ForEachIndexDo(ACTION action) // TODO: const correctness?
		{
			auto ptr = _data;
			auto end = _data + _count;
			uintptr_t index = 0;
			while (ptr < end)
			{
				action(index, *ptr);
				++ptr;
				++index;
			}
		}

		// Low-level support: Return address of first element.
		// May not be appropriate for all types.
		void* FirstElementVoidPointer() const
		{
			return _data;
		}

		// Low-level support: Return size of array in bytes.
		// May not be appropriate for all types.
		uintptr_t SizeBytes() const
		{
			return _count * sizeof(T);
		}

		// Low-level support: Detach the owned array on heap.
		// Leaves this Array<T> instance at address nullptr, count 0.
		T* Detach()
		{
			auto buffer = _data;
			_data = nullptr;
			_count = 0;
			return buffer;
		}

		Array(const Array&) = delete; // not offering yet, may reconsider.
		void operator=(const Array&) = delete; // not offering yet, may reconsider.

	private:

		void DeleteArray()
		{
			Destruct(_data, _count);
			free(_data);
			_data = nullptr;
			_count = 0;
		}

		T* _data;
		uintptr_t _count;

	};

	// Make Array where each element is constructed with the same construction arguments.
	template<typename T, typename... ConstructionArgs>
	Array<T> ArrayInit(uintptr_t count, ConstructionArgs... args)
	{
		T* block = (T*)malloc(sizeof(T) * count);   // matches free in DecUsage().
		PanicIfNull(block, "Out of memory");
		auto ptr = block;
		auto end = ptr + count;
		while (ptr < end)
		{
			new (ptr) T(args...); // TODO: Does this move where type T can?
			++ptr;
		}
		return Array<T>(UnsafeAccept(), (T*)block, count);
	}

	// Make Array from a 'T generatorFunction(uintptr_t index)' that returns the 
	// element for the given index.
	template<typename T, typename FUNCTION>
	Array<T> ArrayGenerate(uintptr_t count, FUNCTION generatorFunction)
	{
		T* block = (T*)malloc(sizeof(T) * count);   // matches free in DecUsage().
		PanicIfNull(block, "Out of memory");
		auto ptr = block;
		auto end = ptr + count;
		uintptr_t index = 0;
		while (ptr < end)
		{
			new ((void*)ptr) T(generatorFunction(index));
			++ptr;
			++index;
		}
		return Array<T>(UnsafeAccept(), (T*)block, count);
	}

	// ArrayBuilder

	template<typename T>
	class ArrayBuilder
	{
	public:

		ArrayBuilder()
		{
			ApplyResetState();
		}

		explicit ArrayBuilder(uintptr_t startSizeHint)
		{
			ApplyResetState();
			Resize(startSizeHint);
		}

		~ArrayBuilder()
		{
			Destruct(_data, _appendIndex);
			ApplyResetState();
		}

		// Remove all content from the array.
		void Clear()
		{
			_appendIndex = 0;
		}

		// Append item to the array.
		ArrayBuilder &Add(const T &item)
		{
			if (_appendIndex < _capacity)
			{
				void* slotAddress = &_data[_appendIndex];
				new (slotAddress) T(item);
				++_appendIndex;
			}
			else
			{
				auto newCapacity = (_capacity * 3) / 2;
				if (newCapacity == 0) newCapacity = 10; // TODO: arbitrary choice
				Resize(newCapacity);
				Add(item);
			}
			return *this;
		}

		// Append copies of all elements of the Slice to the array.
		ArrayBuilder &Add(const Slice<T>& s) // TODO: add a Slice<const T> and suypport Slice<T> -> Slice<const T> conversion in the slice class?  See also StringBuilder.
		{
			auto ptr = s.Start();
			auto end = s.End();
			while (ptr < end)
			{
				Add(*ptr);  // TODO: optimise implementation.
				++ptr;
			}
			return *this;
		}

		// Low-level support: Return address of first element.
		// May not be appropriate for all types.
		void* FirstElementVoidPointer() const
		{
			return _data;
		}

		// Return number of items appended to the array thus far.
		uintptr_t Count() const
		{
			return _appendIndex;
		}

		// Return true if this is empty, else false.
		bool Empty() const { return Count() == 0; }

		// Return reallocation point.
		uintptr_t Capacity() const
		{
			return _capacity;
		}

		// Optimise storage space, and detach the content as an Array<T>.
		// This ArrayBuilder is then empty, available for re-use.
		Array<T> MoveToArray()
		{
			OptimiseSpace();
			auto data = _data;
			auto count = _appendIndex;
			ApplyResetState();
			return Array<T>(UnsafeAccept(), data, count);
		}

		// Trim the growth space by re-allocating if needed, to make
		// the Count and Capacity match.
		ArrayBuilder &OptimiseSpace()
		{
			Resize(_appendIndex);
			return *this;
		}

		// Design decision: Decided not to provide All() to return a Slice since
		// this re-allocates by nature, thus making Slice invalidation likely.

		ArrayBuilder(const ArrayBuilder&) = delete;
		void operator=(const ArrayBuilder&) = delete;

	private:

		void ApplyResetState()
		{
			_data = nullptr;
			_appendIndex = 0;
			_capacity = 0;
		}

		void Resize(uintptr_t capacity)
		{
			if (capacity != _capacity)
			{
				if (_appendIndex > capacity)
				{
					// The block is being shortened. Trim the losses to precisely fit.
					Destruct(_data + capacity, _appendIndex - capacity);
					_appendIndex = capacity;
				}

				void* newBlock = malloc(sizeof(T) * capacity);
				PanicIfNull(newBlock, "Out of memory");
				MoveConstructDisjoint(_data, newBlock, _appendIndex);
				free(_data);
				_data = (T*)newBlock;
				_capacity = capacity;
			}
		}

		T* _data;
		uintptr_t _appendIndex;
		uintptr_t _capacity;

		// Invariant: If _data is nullptr, then _appendIndex and _capacity will always be zero.
		// Invariant: _appendIndex .. _capacity is unconstructed memory.

	};

	// StringBuilder

	class StringBuilder
	{
	public:

		StringBuilder() {}

		StringBuilder& AppendChar(char c);
		StringBuilder& Append(const char *text);
		StringBuilder& Append(const Slice<char>&);
		StringBuilder& Append(const Slice<const char>&); // TODO: sort out const overloads in slice class?
		StringBuilder& Append(const char *text, uintptr_t count); // TODO: remove?
		StringBuilder& Append(const String&);
		StringBuilder& Clear();

		// Returns the number of chars added to this builder.
		uintptr_t Count() const;

		// Return true if this is empty, else false.
		bool Empty() const { return Count() == 0; }

		// Return a String with the result.  This trims the space if needed, by re-allocation.
		// This StringBuilder is returned to the empty state, available for re-use.
		String ToString();

		StringBuilder(const StringBuilder&) = delete; // not offering yet, may reconsider.
		StringBuilder(StringBuilder&&) = delete; // not offering yet, may reconsider.
		void operator=(const StringBuilder&) = delete; // not offering yet, may reconsider.
		void operator=(StringBuilder&&) = delete; // not offering yet, may reconsider.

		// Design decision: Decided not to provide All() to return a Slice since
		// this re-allocates by nature, thus making Slice invalidation likely.

	private:

		void EnsureNulTerminated();

		ArrayBuilder<char> _buffer;

	};

	// Number to string

	char NibbleToChar(uint32_t value);
	uint32_t QuietlyGetValidNumberBase(uint32_t base);

	template<typename INTTYPE>
	class NumberAsString
	{
	public:
		NumberAsString(INTTYPE number, uint32_t base, uint32_t paddingLength, char paddingChar);
		operator const char* () const { return _dest; }
		uintptr_t size() const { return _size; }
	protected:
		enum { SpaceMax = (sizeof(INTTYPE) * 8) + 1 }; // Largest representation is binary.  Add 1 for the potential "-"
		char  _tmpStr[SpaceMax + 1]; // plus one for null terminator
		char* _dest;
		uintptr_t _size;
	};

	template<class INTTYPE>
	NumberAsString<INTTYPE>::NumberAsString(INTTYPE number, uint32_t base, uint32_t paddingLength, char paddingChar)
	{
		bool minusSign = (number < 0);

		if (paddingLength > SpaceMax) paddingLength = SpaceMax;
		auto padStartAddress = _tmpStr + (SpaceMax - paddingLength);

		base = QuietlyGetValidNumberBase(base);

		if (minusSign)
		{
			number = (~number) + 1;   // avoiding a compiler warning on negation (if INTTYPE is unsigned)
		}

		//
		// Start at the final slot, place a null terminator there:
		//

		char* dest = _tmpStr + SpaceMax;
		*dest = '\0';

		//
		// Squeeze out all of the digits:
		//

		for (;;)
		{
			--dest;
			*dest = NibbleToChar(uint8_t(number % base));
			number = number / base;
			if (number == 0) break; // all done
		}

		if (minusSign)
		{
			--dest;
			*dest = '-';
		}

		while (dest > padStartAddress)
		{
			--dest;
			*dest = paddingChar;
		}

		char* end = _tmpStr + SpaceMax;
		_size = end - dest;
		_dest = dest;
	}

	inline NumberAsString<int32_t>   ToString(int32_t  signedNumber, uint32_t base=10)   { return NumberAsString<int32_t>(signedNumber, base, 0, ' '); }
	inline NumberAsString<uint32_t>  ToString(uint32_t unsignedNumber, uint32_t base=10) { return NumberAsString<uint32_t>(unsignedNumber, base, 0, ' '); }

	// Swap

	template<typename T>
	inline void Swap(T& a, T& b)
	{
		auto tmp = a;
		a = b;
		b = tmp;
	}
}

