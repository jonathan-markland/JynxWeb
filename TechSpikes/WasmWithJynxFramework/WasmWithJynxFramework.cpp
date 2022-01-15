
// WasmWithJynxFramework

// clang --std=c++17 -O2 --target=wasm32 --no-standard-libraries -matomics -mbulk-memory -Wl,--max-memory=16777216 -Wl,--shared-memory -Wl,--import-memory -Wl,--export-all -Wl,--no-entry -o WasmWithJynxFramework.wasm WasmWithJynxFramework.cpp JynxFramework.cpp

#include <stdint.h>

extern unsigned char __heap_base;

// ==========================================================================================================

static uintptr_t g_AllocatorNext = 0;
static uintptr_t g_AllocatorEnd = 0;
const int BytesPerMachineWord = 4;

const uintptr_t NullAddress = 0x80000000;   // TODO: If we do WASM memory resizing, rather than have a fixed size, this needs attention.

void InitNeverFreeHeapAllocator()
{
	// TODO: assert not re-initialising
	g_AllocatorNext = (uintptr_t) &__heap_base;
	g_AllocatorEnd = 16 * 1048576; // TODO: 16MB space assumption hack
}

uintptr_t HeapRemaining()
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
	else return NullAddress;
}

static bool ShuntForwardToNextAddressMultipleOf(uintptr_t powerOfTwo)
{
	// TODO: We don't check that powerOfTwo really is that!

	uintptr_t mask = powerOfTwo - 1;
	auto moduloAlignment = g_AllocatorNext & mask;
	if (moduloAlignment != 0)
	{
		auto shuntDistance = powerOfTwo - moduloAlignment;
		return AllocWithUncheckedAlignment(shuntDistance) != NullAddress;
	}
	else
	{
		return true; // No change needed
	}
}

void *malloc(uintptr_t size)
{
	if (ShuntForwardToNextAddressMultipleOf(BytesPerMachineWord))
	{
		return (void *) AllocWithUncheckedAlignment(size);
	}
	return (void *) NullAddress;
}

void free(void *) noexcept
{
	// No operation with never-freeing allocator!
}

// ==========================================================================================================

extern "C" void InitWasmProgram()
{
	InitNeverFreeHeapAllocator();
}

extern "C" void *CreateBlock()
{
	return malloc(10);
}

// ==========================================================================================================

