#pragma once

#include <stdint.h>

namespace JynxFramework
{
	// To be called on worker thread (non-main thread).
	// Enters "Blue Screen Of Death" condition, with a message string.
	// This function never returns.
	void Panic(const char* message);

	// To be called on worker thread (non-main thread).
	// Enters "Blue Screen Of Death" condition if the value is nullptr.  Message string included.
	// This function never returns.
	void PanicIfNull(const void* p, const char* message);

	// To be called on the monitoring thread.
	// Indicates unrecoverable "Blue Screen Of Death" condition.
	bool IsInPanicState();
	
	// Obtain address of panic-message-pointer global variable.
	// Used to allow external monitoring.
	volatile const char **GetPanicMessagePointerAddress();
}
