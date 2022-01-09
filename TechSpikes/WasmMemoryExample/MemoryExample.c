
// clang -O2 --target=wasm32 --no-standard-libraries -Wl,--export-all -Wl,--no-entry -o MemoryExample.wasm MemoryExample.c

extern unsigned char __heap_base;

__attribute__ ((visibility("default"))) 
void *get_heap_base(void) 
{
	return &__heap_base;
}

void *MyHeap = (void *) 0;

int jmalign(int size)
{
	if (size & 3) return (size | 3) + 1;
	return size;
}

void *jmmalloc(int size)
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

void *create_block()
{
	return jmmalloc(10);
}
