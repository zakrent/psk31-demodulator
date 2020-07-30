#pragma once

typedef struct {
	u32 write_cursor;
	u32 read_cursor;
	float *data;
} buffer_t;