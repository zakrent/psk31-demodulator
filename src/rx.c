#include <SDL2/SDL.h>
#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "common.h"
#include "complex.h"
#include "buffer.h"
#include "constants.h"
#include "dsp.h"
#include "rx.h"

#define BUNCH_SIZE 4096
#define RX_IBUFFER_SIZE BUNCH_SIZE*8

rx_t rx_init(u32 sample_rate, r32 baud_rate){
	rx_t ret = {0};
	
	ret.input_buffer.data = malloc(RX_IBUFFER_SIZE*sizeof(r32));
	
	ret.sample_rate = sample_rate;
	ret.baud_rate = baud_rate;
	ret.sps = sample_rate/baud_rate;
	
	u32 matched_taps_size = 2*sample_rate/baud_rate+1;
	
	ret.matched_taps = malloc(matched_taps_size*sizeof(r32));
	
	filter_cosine(ret.matched_taps, matched_taps_size, ret.sps);
	
	ASSERT(ret.matched_taps[0] < 0.5);
	
	ret.matched_filter = filter_init(ret.matched_taps, matched_taps_size);
	return ret;
}

void rx_add_samples(i16 *buffer, u32 count, rx_t *rx){
	for(int i = 0; i < count; i++){
		rx->input_buffer.data[rx->input_buffer.write_cursor++] = buffer[i]/32767.0;
		if(rx->input_buffer.write_cursor >= RX_IBUFFER_SIZE){
			rx->input_buffer.write_cursor = 0;
		}
	}
}

void rx_process(rx_t *rx){
	
	u32 distance;
	if(rx->input_buffer.write_cursor >= rx->input_buffer.read_cursor){
		distance = rx->input_buffer.write_cursor-rx->input_buffer.read_cursor;
	}
	else{
		distance = rx->input_buffer.write_cursor+RX_IBUFFER_SIZE-rx->input_buffer.read_cursor;
	}
	
	if(distance > BUNCH_SIZE){
		r32 input_bunch[BUNCH_SIZE];
		for(int i = 0; i < BUNCH_SIZE; i++){
			input_bunch[i] = rx->input_buffer.data[rx->input_buffer.read_cursor++];
		}
		if(rx->input_buffer.read_cursor >= RX_IBUFFER_SIZE){
			rx->input_buffer.read_cursor = 0;
		}
		
		c32 input_iq[BUNCH_SIZE];
		for(int i = 0; i < BUNCH_SIZE; i++){
			input_iq[i] = mulc(cosc(rx->lo_phase), (c32){.r = input_bunch[i], .i = 0.0f});
			rx->lo_phase += 2*M_PI*BASE_FREQ/rx->sample_rate;
			if(rx->lo_phase >= 2*M_PI){
				rx->lo_phase -= 2*M_PI;
			}
		}
		
		c32 filtered_iq[BUNCH_SIZE];
		filter_work(input_iq, filtered_iq, BUNCH_SIZE, rx->matched_filter);
		
		agc_work(filtered_iq, filtered_iq, BUNCH_SIZE);
		
		u32 symbols_size = 0;
		c32 *symbols_iq = malloc((BUNCH_SIZE/rx->sps+1)*sizeof(c32)); //TODO:
		timing_recovery_work(filtered_iq, symbols_iq, BUNCH_SIZE, rx->sps, &symbols_size, &rx->timing_recovery);
		
		i8 *bits = malloc((BUNCH_SIZE/rx->sps+1)*sizeof(i8));
		dpsk_demod_work(symbols_iq, bits, symbols_size, &rx->demodulator);
			
		psk31_decoder_work(bits, symbols_size, &rx->decoder);

		gui_draw_IQ(symbols_iq, symbols_size);
		gui_draw_IQ_2(filtered_iq, BUNCH_SIZE);
		gui_draw_graph(input_bunch, BUNCH_SIZE);

		free(symbols_iq);
		free(bits);
	}
}
