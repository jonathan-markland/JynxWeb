#include <stdint.h>
#include "JynxFrameworkPanic.h"
#include "WasmNeverFreeingMemoryAllocator.h"

// This is a scaffold for a WebAssembly without LibC context.
// A heap manger that, simply, never frees anything.

extern unsigned char __heap_base;

namespace WasmNeverFreeingMemoryAllocator
{
	const int BytesPerMachineWord = 4;  // TODO: Possibly get from architecture, but we probably don't care for 32-bit WASM.

	static uintptr_t g_AllocatorNext = 0;
	static uintptr_t g_AllocatorEnd  = 0;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void Init()
	{
		if (g_AllocatorNext != 0 || g_AllocatorEnd != 0)
		{
			JynxFramework::Panic("WasmNeverFreeingMemoryAllocator should only be initialised once.");
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
		// NB: Panics in the out of memory cases.
		
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
				JynxFramework::Panic("WasmNeverFreeingMemoryAllocator needs initialisation!");
			}
			else
			{
				JynxFramework::Panic("WasmNeverFreeingMemoryAllocator out of memory.");
			}
			return 0; // Never executed.
		}
	}

	static void ShuntForwardToNextAddressMultipleOf(uintptr_t powerOfTwo)
	{
		// TODO: We don't check that powerOfTwo really is that!

		uintptr_t mask = powerOfTwo - 1;
		auto moduloAlignment = g_AllocatorNext & mask;
		if (moduloAlignment != 0)
		{
			auto shuntDistance = powerOfTwo - moduloAlignment;
			AllocWithUncheckedAlignment(shuntDistance);
		}
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void *malloc(uintptr_t size)
{
	WasmNeverFreeingMemoryAllocator::ShuntForwardToNextAddressMultipleOf(
		WasmNeverFreeingMemoryAllocator::BytesPerMachineWord);
		
	return (void *) WasmNeverFreeingMemoryAllocator::AllocWithUncheckedAlignment(size);
}

void free(void *) noexcept
{
	// No operation with never-freeing allocator!
}

