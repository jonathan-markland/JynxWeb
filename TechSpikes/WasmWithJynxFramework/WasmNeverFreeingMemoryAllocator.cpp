#include <stdint.h>
#include "JynxFrameworkPanic.h"
#include "WasmNeverFreeingMemoryAllocator.h"

// This is a scaffold for a WebAssembly without LibC context.
// A heap manger that, simply, never frees anything.

extern unsigned char __heap_base;

namespace WasmNeverFreeingMemoryAllocator
{
	const int BytesPerMachineWord = 4;

	static uintptr_t g_AllocatorNext = 0;
	static uintptr_t g_AllocatorEnd  = 0;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void Init()
	{
		if (g_AllocatorNext != 0 || g_AllocatorEnd != 0)
		{
			Panic("WasmNeverFreeingMemoryAllocator should only be initialised once.");
		}
		g_AllocatorNext = (uintptr_t) &__heap_base;
		g_AllocatorEnd = 16 * 1048576; // TODO: 16MB space assumption hack
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	static uintptr_t HeapRemaining()
	{
		return g_AllocatorEnd - g_AllocatorNext;
	}

	static uintptr_t AllocWithUncheckedAlignment(uintptr_t size)
	{
		if (size <= HeapRemaining())
		{
			auto address = g_AllocatorNext;
			g_AllocatorNext += size;
			return address;
		}
		else
		{
			if (g_AllocatorNext == 0 && g_AllocatorEnd == 0)
			{
				Panic("WasmNeverFreeingMemoryAllocator::Init() must be called before memory allocations can take place.");
			}
			else
			{
				Panic("WasmNeverFreeingMemoryAllocator out of memory.");
			}
		}
	}

	static bool ShuntForwardToNextAddressMultipleOf(uintptr_t powerOfTwo)
	{
		// TODO: We don't check that powerOfTwo really is that!

		uintptr_t mask = powerOfTwo - 1;
		auto moduloAlignment = g_AllocatorNext & mask;
		if (moduloAlignment != 0)
		{
			auto shuntDistance = powerOfTwo - moduloAlignment;
			return AllocWithUncheckedAlignment(shuntDistance) != nullptr;
		}
		else
		{
			return true; // No change needed
		}
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void *malloc(uintptr_t size)
{
	if (ShuntForwardToNextAddressMultipleOf(BytesPerMachineWord))
	{
		return (void *) AllocWithUncheckedAlignment(size);
	}
	return (void *) nullptr;
}

void free(void *) noexcept
{
	// No operation with never-freeing allocator!
}

