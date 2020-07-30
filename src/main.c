#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdint.h>

#include "common.h"
#include "constants.h"
#include "complex.h"
#include "buffer.h"
#include "dsp.h"
#include "rx.h"

static rx_t rx;

void audio_callback(void* userdata, u8 *stream, i32 len){
	i16 *samples = stream;
	rx_add_samples(samples, len/sizeof(i16), &rx);
}

int main(int argc, char *argv[]){
	srand(0);
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window *window = SDL_CreateWindow("QPSK",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WIDTH, HEIGHT,
		NULL);
		
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

	rx = rx_init(DEFAULT_SAMP_RATE, DEFAULT_BAUD_RATE);
				
	SDL_AudioSpec want, have;
			
	SDL_memset(&want, 0, sizeof(want));
	want.freq = DEFAULT_SAMP_RATE;
	want.format = AUDIO_S16;
	want.channels = 1;
	want.samples = 8;
	want.callback = audio_callback;
	
	SDL_AudioDeviceID rec_dev = SDL_OpenAudioDevice(NULL, 1, &want, &have, 0);
	SDL_PauseAudioDevice(rec_dev, 0);
	
	bool running = true;
	u32 target_trame_time = (u32)(1.0f/60.0f*1000.0f);
	u32 frame_begin = 0; 
	SDL_Event e;
	while(running){
		frame_begin = SDL_GetTicks();
		while(SDL_PollEvent(&e)){
			switch(e.type){
				case SDL_QUIT:
				{
					running = false;
				}
				break;
				default:
				break;
			}
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);	
		rx_process(&rx);
		gui_render(renderer);
		SDL_RenderPresent(renderer);
	
		u32 frame_time = SDL_GetTicks() - frame_begin;
		if(frame_time < target_trame_time){
			SDL_Delay(target_trame_time-frame_time);
		}

	}
}