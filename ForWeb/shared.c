// shared.c

// clang -O2 --target=wasm32 --no-standard-libraries -matomics -mbulk-memory -Wl,--max-memory=655360 -Wl,--shared-memory -Wl,--import-memory -Wl,--export-all -Wl,--no-entry -o shared.wasm shared.c

#define SOUND_BUFFER_SIZE 128

float sound_buffer[SOUND_BUFFER_SIZE];
int screen[16 * 16];
int next_colour = 0;
float top_level = 0.1;

float *get_static_sound_buffer()
{
	return sound_buffer;
}

float *get_static_level_variable()
{
	return &top_level;
}

int *get_screen_base_address()
{
	// TODO: for now:
	for(int i=0; i<256; i++)
	{
		screen[i] = 0xFFFF00FF;
	}
	
	return screen;
}

void fill_sound_buffer(double gain)  // TODO: the gain isn't used in this example.
{
	int change_index = (SOUND_BUFFER_SIZE / 2);

	int i = 0;

	while (i < change_index) 
	{
		sound_buffer[i] = top_level;
		++i;
	}

	while (i < SOUND_BUFFER_SIZE) 
	{
		sound_buffer[i] = -top_level;
		++i;
	}

	// TODO: for now fiddle with the screen also:
	next_colour = (next_colour + 1) & 0xFF;
	int paint_colour = next_colour | 0xFF000000;
	for(int i=0; i<64; i++)
	{
		screen[i] = paint_colour;
	}
}
