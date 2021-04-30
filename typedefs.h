#pragma once

#define B_FACT (4096 / sizeof(record_t))
//#define DEBUG

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

typedef struct record_t {
	uint student_no;
	ushort grades[3];
} record_t;

typedef struct record_stack {
	record_t arr[B_FACT];
	ushort top;
} record_stack;

