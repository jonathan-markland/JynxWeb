#pragma once

// This is a scaffold for a WebAssembly without LibC context, and will only be included there.
// A heap manger that, simply, never frees anything.

#include <stdint.h>
#include "WasmNeverFreeingMemoryAllocator.h"

namespace WasmNeverFreeingMemoryAllocator
{
	void Init();
}

void *malloc(uintptr_t size);
void free(void *) noexcept;
void *operator new(uintptr_t sizeBytes);
void operator delete(void *block) noexcept;
