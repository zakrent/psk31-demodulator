#pragma once

typedef struct{
	r32 r;
	r32 i;
} c32;

c32 cosc(r32 x);

c32 conj(c32 x);
r32 magc(c32 x);

c32 mulc(c32 a, c32 b);

