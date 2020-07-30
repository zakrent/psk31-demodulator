#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "common.h"
#include "constants.h"
#include "complex.h"

c32 cosc(r32 x){
	return (c32){
		.r = cosf(x),
		.i = sinf(x),
	};
}

c32 conj(c32 x){
	return (c32){
		.r = x.r,
		.i = -x.i,
	};
}

r32 magc(c32 x){
	return sqrt(x.r*x.r+x.i*x.i);
}

c32 mulc(c32 a, c32 b){
	return (c32){
		.r = a.r*b.r-b.i*a.i,
		.i = a.r*b.i+b.r*a.i,
	};
}

