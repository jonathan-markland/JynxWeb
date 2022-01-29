
// WasmWithJynxFramework

// clang --std=c++17 -O2 --target=wasm32 --no-standard-libraries -matomics -mbulk-memory -Wl,--max-memory=16777216 -Wl,--shared-memory -Wl,--import-memory -Wl,--export-all -Wl,--no-entry -o WasmWithJynxFramework.wasm WasmWithJynxFramework.cpp JynxFramework.cpp JynxFrameworkPanic.cpp WasmNeverFreeingMemoryAllocator.cpp

#include <stdint.h>
#include "WasmNeverFreeingMemoryAllocator.h"

extern "C" void InitWasmProgram()
{
	WasmNeverFreeingMemoryAllocator::Init();
}

extern "C" void *CreateBlock()
{
	return malloc(10);
}

