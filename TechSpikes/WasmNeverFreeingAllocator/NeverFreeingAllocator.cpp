
// NeverFreeingAllocator

// clang --std=c++17 -O2 --target=wasm32 --no-standard-libraries -Wl,--export-all -Wl,--no-entry -o NeverFreeingAllocator.wasm NeverFreeingAllocator.cpp

extern unsigned char __heap_base;

__attribute__ ((visibility("default"))) 
extern "C" void *get_heap_base(void) 
{
	return &__heap_base;
}

void *MyHeap = (void *) 0;

extern "C" int jmalign(int size)
{
	if (size & 3) return (size | 3) + 1;
	return size;
}

extern "C" void *jmmalloc(int size)
{
	void *block = MyHeap;
	if (block == (void *) 0)
	{
		block = get_heap_base();
	}
	size = jmalign(size);
	MyHeap = (void *) (((int) block) + size);
	return block;
}

extern "C" void *create_block()
{
	return jmmalloc(10);
}
