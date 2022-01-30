// shared.c

// The clang command line to build is now in buildcpp.sh

#define SOUND_BUFFER_SIZE   128   // x PCM samples at target rate.   This cannot be changed because of browser specifications.
#define GUEST_SCREEN_WIDTH  256   // If changed, must also change the Javascript const of the same name.
#define GUEST_SCREEN_HEIGHT 256   // If changed, must also change the Javascript const of the same name.

float sound_buffer[SOUND_BUFFER_SIZE];
int screen[GUEST_SCREEN_WIDTH * GUEST_SCREEN_HEIGHT];
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
	for(int i=0; i < (GUEST_SCREEN_WIDTH * GUEST_SCREEN_HEIGHT); i++)
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

	// TODO: for now fiddle with the top quarter of the screen also:
	next_colour = (next_colour + 1) & 0xFF;
	int paint_colour = next_colour | 0xFF000000;
	for(int i=0; i < (GUEST_SCREEN_WIDTH * (GUEST_SCREEN_HEIGHT / 4)); i++)
	{
		screen[i] = paint_colour;
	}
}
