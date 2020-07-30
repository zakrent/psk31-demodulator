#pragma once

typedef struct{
	u32 sample_rate;
	r32 baud_rate;
	u32 sps;
	
	buffer_t input_buffer;
	filter_t matched_filter;
	timing_recovery_t timing_recovery;
	demodulator_t demodulator;
	decoder_t decoder;
	
	r32 lo_phase;
	r32 *matched_taps;
} rx_t;

rx_t rx_init(u32 sample_rate, r32 baud_rate);
void rx_add_samples(i16 *buffer, u32 count, rx_t *rx);
void rx_process(rx_t *rx);