// shared.c

// clang -O2 --target=wasm32 --no-standard-libraries -Wl,--export-all -Wl,--no-entry -o shared.wasm shared.c

#define SOUND_BUFFER_SIZE 128

int swing_counter = 0;
float sound_buffer[SOUND_BUFFER_SIZE];

float *get_static_sound_buffer()
{
	return sound_buffer;
}

int next_change_index()
{
	return (SOUND_BUFFER_SIZE / 2);
	
	// int change_index = swing_counter;
	// 
	// ++swing_counter;
	// int swing_range = (SOUND_BUFFER_SIZE / 4);
	// if (swing_counter >= swing_range)
	// {
	// 	swing_counter = 0;
	// }
	// 
	// return (SOUND_BUFFER_SIZE / 2) + change_index;
}

void fill_sound_buffer(double gain)  // TODO: the gain isn't used in this example.
{
	int change_index = next_change_index();

	int i = 0;

	while (i < change_index) 
	{
		sound_buffer[i] = 0.1;  ++i;
	}

	while (i < SOUND_BUFFER_SIZE) 
	{
		sound_buffer[i] = -0.1;  ++i;
	}
}
