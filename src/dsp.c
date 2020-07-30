#include <stdlib.h>
#include <stdint.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "common.h"
#include "complex.h"
#include "constants.h"
#include "dsp.h"

void filter_normalize(r32 *taps, u32 ntaps){
	r32 sum = 0;
	for(int i = 0; i < ntaps; i++){
		sum += taps[i];
	}
	for(int i = 0; i < ntaps; i++){
		taps[i] /= sum;
	}
}


void filter_cosine(r32 *taps, u32 ntaps, u32 sps){
	ASSERT(ntaps == sps*2+1);
	for(int i = 0; i <= sps; i++){
		taps[ntaps/2+i]=taps[ntaps/2-i]=(1+cos(M_PI*i/(r32)sps))/2;
	}
	filter_normalize(taps, ntaps);
}

filter_t filter_init(r32 *taps, u32 ntaps){
	return (filter_t){
		.ntaps   = ntaps,
		.idx     = 0,
		.taps    = taps,
		.history = malloc(sizeof(c32)*ntaps),
	};
}

void filter_free(filter_t f){
	free(f.history);
}

void filter_work(c32 *in, c32 *out, u32 count, filter_t f){
	for(int i = 0; i < count; i++){
		f.history[f.idx++] = in[i];
		if(f.idx == f.ntaps){
			f.idx = 0;
		}
		
		out[i] = (c32){0};		
		u32 idx = f.idx;
		for(int j = 0; j < f.ntaps; j++){
			idx = idx != 0 ? idx-1 : f.ntaps-1;
			out[i].r += f.history[idx].r * f.taps[j];
			out[i].i += f.history[idx].i * f.taps[j];
		}
	}
}

void timing_recovery_work(c32 *in, c32 *out, u32 samples, u32 sps, u32 *out_size, timing_recovery_t *t){
	ASSERT((sps % 2 == 0));
	u32 out_cursor = 0;
	
	//First bit is received without doing timing
	//recovery because it doesn't have previous bit
	u32 current_symbol = t->samples_till_symbol;
	out[out_cursor++] = in[current_symbol];
	t->samples_till_symbol = sps;
	
	ASSERT(current_symbol+t->samples_till_symbol < samples);
	
	u32 sphs = sps/2;
	r32 error;
	while(1){
		current_symbol += t->samples_till_symbol;
		u32 prev_symbol = current_symbol-sps;
		u32 half_symbol = current_symbol-sphs;
		//Gardner timing error calculation
		error = (in[current_symbol].r - in[prev_symbol].r)*in[half_symbol].r +
				(in[current_symbol].i - in[prev_symbol].i)*in[half_symbol].i;
				
		out[out_cursor++] = in[current_symbol];
		
		t->samples_till_symbol = sps;
		
		if(error > 0){
			t->samples_till_symbol--;
		}
		else{
			t->samples_till_symbol++;
		}
		
		
		if(current_symbol+t->samples_till_symbol > samples){
			t->samples_till_symbol -= samples-current_symbol;
			break;
		}
	}
	
	*out_size = out_cursor;
}

void agc_work(c32 *in, c32 *out, u32 samples){
	r32 max_2 = 0.0;
	for(int i = 0; i < samples; i++){
		r32 amp_2 = in[i].r*in[i].r+in[i].i*in[i].i;
		if(amp_2 > max_2)
			max_2 = amp_2;
	}
	
	r32 max = sqrt(max_2);
	for(int i = 0; i < samples; i++){
		out[i] = mulc(in[i], (c32){.r = 1.0/max});
	}
}

void dpsk_demod_work(c32 *in, i8 *out, u32 samples, demodulator_t *d){
	for(int i = 0; i < samples; i++){
		c32 res = mulc(in[i], conj(d->last_symbol));
		d->last_symbol = in[i];
		out[i] = 127*res.r/ABS(res.r);
	}
}

//Copied from fldigi
static unsigned int varicodetab2[] = {
	0x2AB, 0x2DB, 0x2ED, 0x377, 0x2EB, 0x35F, 0x2EF, 0x2FD, 
	0x2FF, 0x0EF, 0x01D, 0x36F, 0x2DD, 0x01F, 0x375, 0x3AB, 
	0x2F7, 0x2F5, 0x3AD, 0x3AF, 0x35B, 0x36B, 0x36D, 0x357, 
	0x37B, 0x37D, 0x3B7, 0x355, 0x35D, 0x3BB, 0x2FB, 0x37F, 
	0x001, 0x1FF, 0x15F, 0x1F5, 0x1DB, 0x2D5, 0x2BB, 0x17F, 
	0x0FB, 0x0F7, 0x16F, 0x1DF, 0x075, 0x035, 0x057, 0x1AF, 
	0x0B7, 0x0BD, 0x0ED, 0x0FF, 0x177, 0x15B, 0x16B, 0x1AD, 
	0x1AB, 0x1B7, 0x0F5, 0x1BD, 0x1ED, 0x055, 0x1D7, 0x2AF, 
	0x2BD, 0x07D, 0x0EB, 0x0AD, 0x0B5, 0x077, 0x0DB, 0x0FD, 
	0x155, 0x07F, 0x1FD, 0x17D, 0x0D7, 0x0BB, 0x0DD, 0x0AB, 
	0x0D5, 0x1DD, 0x0AF, 0x06F, 0x06D, 0x157, 0x1B5, 0x15D, 
	0x175, 0x17B, 0x2AD, 0x1F7, 0x1EF, 0x1FB, 0x2BF, 0x16D, 
	0x2DF, 0x00B, 0x05F, 0x02F, 0x02D, 0x003, 0x03D, 0x05B, 
	0x02B, 0x00D, 0x1EB, 0x0BF, 0x01B, 0x03B, 0x00F, 0x007, 
	0x03F, 0x1BF, 0x015, 0x017, 0x005, 0x037, 0x07B, 0x06B, 
	0x0DF, 0x05D, 0x1D5, 0x2B7, 0x1BB, 0x2B5, 0x2D7, 0x3B5, 
	0x3BD, 0x3BF, 0x3D5, 0x3D7, 0x3DB, 0x3DD, 0x3DF, 0x3EB, 
	0x3ED, 0x3EF, 0x3F5, 0x3F7, 0x3FB, 0x3FD, 0x3FF, 0x555, 
	0x557, 0x55B, 0x55D, 0x55F, 0x56B, 0x56D, 0x56F, 0x575, 
	0x577, 0x57B, 0x57D, 0x57F, 0x5AB, 0x5AD, 0x5AF, 0x5B5, 
	0x5B7, 0x5BB, 0x5BD, 0x5BF, 0x5D5, 0x5D7, 0x5DB, 0x5DD, 
	0x5DF, 0x5EB, 0x5ED, 0x5EF, 0x5F5, 0x5F7, 0x5FB, 0x5FD, 
	0x5FF, 0x6AB, 0x6AD, 0x6AF, 0x6B5, 0x6B7, 0x6BB, 0x6BD, 
	0x6BF, 0x6D5, 0x6D7, 0x6DB, 0x6DD, 0x6DF, 0x6EB, 0x6ED, 
	0x6EF, 0x6F5, 0x6F7, 0x6FB, 0x6FD, 0x6FF, 0x755, 0x757, 
	0x75B, 0x75D, 0x75F, 0x76B, 0x76D, 0x76F, 0x775, 0x777, 
	0x77B, 0x77D, 0x77F, 0x7AB, 0x7AD, 0x7AF, 0x7B5, 0x7B7, 
	0x7BB, 0x7BD, 0x7BF, 0x7D5, 0x7D7, 0x7DB, 0x7DD, 0x7DF, 
	0x7EB, 0x7ED, 0x7EF, 0x7F5, 0x7F7, 0x7FB, 0x7FD, 0x7FF, 
	0xAAB, 0xAAD, 0xAAF, 0xAB5, 0xAB7, 0xABB, 0xABD, 0xABF, 
	0xAD5, 0xAD7, 0xADB, 0xADD, 0xADF, 0xAEB, 0xAED, 0xAEF, 
	0xAF5, 0xAF7, 0xAFB, 0xAFD, 0xAFF, 0xB55, 0xB57, 0xB5B
};

void psk31_decode_character(i8 *in, u32 bits){
	u32 symbol = 0;
	for(int i = 0; i < bits; i++){
		symbol = symbol << 1;
		symbol |= (in[i] > 0);
	}
	
	char character = 0;
	for(int i = 0; i < 256; i++){
		if(symbol == varicodetab2[i]){
			character = i;
			break;
		}
	}
			
	printf("%c", character);
}

void psk31_decoder_work(i8 *in, u32 bits, decoder_t *d){
	for(int i = 0; i < bits; i++){
		if(d->decoding){
			if(d->last_bit < 0 && in[i] < 0){
				psk31_decode_character(d->history, d->count);
				d->decoding = false;
				d->count = 0;
			}
			else{
				d->history[d->count++] = d->last_bit;
				d->last_bit = in[i];
				if(d->count == 16){
					d->decoding = false;
					d->count = 0;
				}
			}
		}
		else{
			if(d->last_bit < 0 && in[i] < 0){
				continue;
			}
			else if(in[i] > 0){
				d->decoding = true;
				d->last_bit = in[i];
			}
		}
	}
}