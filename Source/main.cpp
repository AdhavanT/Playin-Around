#include "platform.h"


struct AppMemory
{
	union
	{
		vec2ui dim;
		struct
		{
			uint32 height;
			uint32 width;
		};
	};
	MSlice<uint32> frame_buffer;
};

void init(PL& pl, AppMemory* ap)
{

	PL_initialize_timing(pl.time);
	
	PL_Audio_Format af =
	{
		2,
		16,
		44100,
		0,
		1.f
	};
	pl.audio.input.format = af;
	pl.audio.input.is_loopback = TRUE;
	pl.audio.input.only_update_every_new_buffer = FALSE;
	PL_initialize_audio_capture(pl.audio.input);

	PL_initialize_input_mouse(pl.input.mouse);
	PL_initialize_input_keyboard(pl.input.kb);
	


	ap->dim =
	{
		1280,720
	};
	
	pl.window.width = ap->dim.x;
	pl.window.height = ap->dim.y;
	pl.window.user_resizable = FALSE;
	pl.window.title = (char*)"Playin' Around";
	pl.window.window_bitmap.bytes_per_pixel = 4;
	
	ap->frame_buffer.init_and_allocate(&pl.memory.main_arena, ap->dim.x * ap->dim.y, "window frame buffer");
	PL_initialize_window(pl.window);
	
}

struct Player
{
	vec2f pos;
};

void update(PL* pl, AppMemory* ap)
{

}

void shutdown(PL& pl, AppMemory* ap)
{
	ap->frame_buffer.clear(&pl.memory.main_arena);
}

void PL_entry_point(PL& pl)
{
	pl.memory.main_arena.capacity = Megabytes(10);
	pl.memory.main_arena.overflow_addon_size = 0;
	pl.memory.main_arena.top = 0;
	pl.memory.main_arena.base = pl_arena_buffer_alloc(pl.memory.main_arena.capacity);
	add_monitoring(&pl.memory.main_arena);

	pl.memory.temp_arena.capacity = Megabytes(65);
	pl.memory.temp_arena.overflow_addon_size = 0;
	pl.memory.temp_arena.top = 0;
	pl.memory.temp_arena.base = pl_arena_buffer_alloc(pl.memory.temp_arena.capacity);
	add_monitoring(&pl.memory.temp_arena);

	pl.initialized = FALSE;
	pl.running = TRUE;

	AppMemory* app_memory = (AppMemory*)MARENA_PUSH(&pl.memory.main_arena, sizeof(AppMemory), "App Memory Struct");

	init(pl, app_memory);


	while (pl.running)
	{
		PL_poll_timing(pl.time);
		PL_poll_window(pl.window);
		PL_poll_input_mouse(pl.input.mouse, pl.window);
		PL_poll_input_keyboard(pl.input.kb);

		update(&pl, app_memory);

		if (pl.input.keys[PL_KEY::ALT].down && pl.input.keys[PL_KEY::F4].down || pl.input.keys[PL_KEY::ESCAPE].down)
		{
			pl.running = FALSE;
			update(&pl, app_memory);

		}
		//Refreshing the FPS counter in the window title bar. Comment out to turn off. 
		static f64 timing_refresh = 0;
		static char buffer[256];
		if (pl.time.fcurrent_seconds - timing_refresh > 0.1)//refreshing at a tenth(0.1) of a second.
		{
			int32 frame_rate = (int32)(pl.time.cycles_per_second / pl.time.delta_cycles);
			pl_format_print(buffer, 256, "Time per frame: %.*fms , %dFPS ; Mouse Pos: [x,y]:[%i,%i]\n", 2, (f64)pl.time.fdelta_seconds * 1000, frame_rate, pl.input.mouse.position_x, pl.input.mouse.position_y);
			pl.window.title = buffer;
			timing_refresh = pl.time.fcurrent_seconds;
		}
		PL_push_window(pl.window, TRUE);

	}

	shutdown(pl, app_memory);

	MARENA_POP(&pl.memory.main_arena, sizeof(AppMemory), "App Memory Struct");
}