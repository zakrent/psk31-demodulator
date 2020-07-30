#pragma once

//Filter design
void filter_normalize(r32 *taps, u32 ntaps);
void filter_cosine(r32 *taps, u32 ntaps, u32 sps); //ntaps must be 2*sps+1

//Filter work
typedef struct{
	u32 ntaps;
	u32 idx;
	r32 *taps;
	c32 *history;
} filter_t;

filter_t filter_init(r32 *taps, u32 ntaps);
void filter_free(filter_t f);
void filter_work(c32 *in, c32 *out, u32 count, filter_t f);

//Timing recovery
typedef struct{
	u32 samples_till_symbol;
} timing_recovery_t;

void timing_recovery_work(c32 *in, c32 *out, u32 samples, u32 sps, u32 *out_size, timing_recovery_t *t);

//AGC
void agc_work(c32 *in, c32 *out, u32 samples);

//Demod
typedef struct{
	c32 last_symbol;
} demodulator_t;

void dpsk_demod_work(c32 *in, i8 *out, u32 samples, demodulator_t *d);

//Decoder
typedef struct{
	bool decoding;
	i8 last_bit;
	i8 history[16];
	u32 count;
} decoder_t;

void psk31_decoder_work(i8 *in, u32 bits, decoder_t *d);
