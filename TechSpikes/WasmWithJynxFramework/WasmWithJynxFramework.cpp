
// WasmWithJynxFramework

// clang command is in build.sh

#include <stdint.h>
#include "WasmNeverFreeingMemoryAllocator.h"
#include "JynxFramework.h"

extern "C" void InitBeforeCtorsCalled()
{
	// This needs to be called before the compiler-generated __wasm_call_ctors()
	
	WasmNeverFreeingMemoryAllocator::Init();
}

extern "C" void *CreateBlock()
{
	return malloc(10);
}

extern "C" const void *ExerciseJynxFramework()
{
	auto str = JynxFramework::String("Hello!");
	return str.c_str(); // unsafe as memory is freed, but we can still see it immediately on JS side.
}

JynxFramework::String  g_exampleGlobalString("Global String!");

extern "C" const void *GetGlobalStringAddress()
{
	return g_exampleGlobalString.c_str();
}
