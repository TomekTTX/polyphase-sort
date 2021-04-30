#pragma once
#include "typedefs.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct rec_file_t {
	FILE *file;
	record_t lastRec;
	ulong realRuns, dummyRuns;
} rec_file_t;

bool record_gt(const record_t *a, const record_t *b);
float avg3(const ushort arr[3]);

bool validateFileName(char **fileName);
void stepBack(FILE *f);
rec_file_t file_wrap(FILE *f);
uint countRuns(FILE *f);

short getEmptyTapeIndex(const rec_file_t tape[3]);

void logDistribution(const rec_file_t *tapes);
void logMerge(uint phase, const rec_file_t *inTape, const rec_file_t *outTape);
