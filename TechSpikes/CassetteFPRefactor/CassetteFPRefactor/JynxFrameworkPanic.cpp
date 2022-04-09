
#ifdef _WIN32
#include "windows.h"
#endif

#include "JynxFrameworkPanic.h"

namespace JynxFramework
{
	// This is set when the panic state is entered.
	static volatile const char* g_JynxPanicMessage = nullptr;



	volatile const char **GetPanicMessagePointerAddress()
	{
		return &g_JynxPanicMessage;
	}
	


	void Panic(const char* message)
	{
		g_JynxPanicMessage = message;
		while (true) 
		{
			// Non-emulation thread will monitor the situation, and BSOD.

			#ifdef _WIN32
			::Sleep(100000);  // May as well not waste CPU time on the Desktop build.
			#endif
			
			#ifdef __clang_major__
			*(int*)0x20000000 = 0;  // TODO: hack to abort WASM machine in the browser context
			#endif

			// TODO: || defined(__linux__))
		} 
	}



	void PanicIfNull(const void* p, const char* message)
	{
		if (p == nullptr)
		{
			Panic(message);
		}
	}



	bool IsInPanicState()
	{
		return g_JynxPanicMessage != nullptr;
	}
}
