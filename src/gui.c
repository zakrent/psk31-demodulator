#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdint.h>
#include "common.h"
#include "constants.h"
#include "complex.h"
#include "gui.h"

static u32 graph_count;
static u32 IQ_count;
static u32 IQ_count_2;
static r32 *graph_data;
static c32 *IQ_data;
static c32 *IQ_data_2;

void gui_draw_graph(r32 *data, i32 count){
	free(graph_data);
	graph_data = malloc(count*sizeof(r32));
	graph_count = count;
	for(int i = 0; i < count; i++){
		graph_data[i] = data[i];
	}
}

void gui_draw_graph_c(c32 *data, i32 count){
	free(graph_data);
	graph_data = malloc(count*sizeof(r32));
	graph_count = count;
	for(int i = 0; i < count; i++){
		graph_data[i] = magc(data[i])*data[i].r/ABS(data[i].r);
	}
}

void gui_draw_IQ(c32 *iq, u32 count){
	free(IQ_data);
	IQ_data = malloc(count*sizeof(c32));
	IQ_count = count;
	for(int i = 0; i < count; i++){
		IQ_data[i] = iq[i];
	}
}

void gui_draw_IQ_2(c32 *iq, u32 count){
	free(IQ_data_2);
	IQ_data_2 = malloc(count*sizeof(c32));
	IQ_count_2 = count;
	for(int i = 0; i < count; i++){
		IQ_data_2[i] = iq[i];
	}
}

void draw_cross(SDL_Renderer *renderer, u32 x, u32 y){
	SDL_RenderDrawPoint(renderer, x, y);
	for(int x_o = 1; x_o < 5; x_o++){
		SDL_RenderDrawPoint(renderer, x+x_o, y);
		SDL_RenderDrawPoint(renderer, x-x_o, y);
	}
	for(int y_o = 1; y_o < 5; y_o++){
		SDL_RenderDrawPoint(renderer, x, y+y_o);
		SDL_RenderDrawPoint(renderer, x, y-y_o);
	}
}

void gui_render(SDL_Renderer *renderer){
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	for(int i = 0; i < graph_count; i++){
		u32 x = 0.5*1000.0/graph_count*i;
		u32 y = 0.5*(HEIGHT/2+graph_data[i]*250.0);
		SDL_RenderDrawPoint(renderer, x, y);
	}
	
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	for(int i = 0; i < IQ_count_2; i++){
		u32 x = WIDTH/2+IQ_data_2[i].r*250.0f;
		u32 y = HEIGHT/2+IQ_data_2[i].i*250.0f;
		SDL_RenderDrawPoint(renderer, x, y);
	}

	
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	for(int i = 0; i < IQ_count; i++){
		u32 x = WIDTH/2+IQ_data[i].r*250.0f;
		u32 y = HEIGHT/2+IQ_data[i].i*250.0f;
		draw_cross(renderer, x, y);
	}

}