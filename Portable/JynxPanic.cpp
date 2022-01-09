
static volatile const char *g_JynxPanicMessage = 0;

void JynxPanic(const char *message)
{
	g_JynxPanicMessage = message;  // Read by host
	while(true) {}
}

bool JynxIsInSuspendedPanic()
{
	return g_JynxPanicMessage != 0;
}
