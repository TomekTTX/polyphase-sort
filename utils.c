#include "utils.h"
#include "record_io.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#pragma warning (disable:4996)

float avg3(const ushort arr[3]) {
	return ((float)arr[0] + arr[1] + arr[2]) / 3;
}

bool record_gt(const record_t *a, const record_t *b) {
	const bool ret = (avg3(a->grades) > avg3(b->grades));
#ifdef DEBUG
	record_print(a);
	printf(" %c ", ret ? '>' : '<');
	record_print(b);
	putchar('\n');
#endif
	return ret;
}

bool validateFileName(char **fileName) {
	if (*fileName == NULL && (*fileName = malloc(64))) {

		printf("Choose file name: ");
		fgets(*fileName, 63, stdin);
		(*fileName)[strlen(*fileName) - 1] = '\0';

		return true;
	}
	return false;
}

rec_file_t file_wrap(FILE *f) {
	rec_file_t ret = { f, { 0 }, 0, 0 };
	return ret;
}

void stepBack(FILE *f) {
	const long pos = ftell(f) - sizeof(record_t);
	fseek(f, pos, SEEK_SET);
}

short getEmptyTapeIndex(const rec_file_t tape[3]) {
	for (short i = 0; i < 3; ++i)
		if (tape[i].realRuns == 0)
			return i;
	return -1;
}


void logDistribution(const rec_file_t *tape) {
	FILE *log;

	if (log = fopen("Distribution.log", "w")) {
		fputs("DISTRIBUTION:\n", log);

		record_log_file(tape[0].file, log);
		fprintf(log, "%u real runs\n", tape[0].realRuns);
		fprintf(log, "%u dummy runs\n", tape[0].dummyRuns);
		fputc('\n', log);

		record_log_file(tape[1].file, log);
		fprintf(log, "%u real runs\n", tape[1].realRuns);
		fprintf(log, "%u dummy runs\n", tape[1].dummyRuns);

		fclose(log);
	}
}

void logMerge(uint phase, const rec_file_t *inTape, const rec_file_t *outTape) {
	FILE *log;
	char phaseLogName[32];

	sprintf(phaseLogName, "Phase_%u.log", phase);
	if (log = fopen(phaseLogName, "w")) {

		fprintf(log, "PHASE %u MERGE RESULT:\n", phase);
		fputs("\nINPUT TAPE:\n", log);
		record_log_file(inTape->file, log);
		fprintf(log, "%u runs\n", inTape->realRuns);

		fputs("\nOUTPUT TAPE:\n", log);
		record_log_file(outTape->file, log);
		fprintf(log, "%u runs\n", outTape->realRuns);

		fclose(log);
	}
}

uint countRuns(FILE *f) {
	uint ret = 1;
	record_t cur = { 0 }, prev = { 0 };
	const long pos = ftell(f);

	while (record_read(&cur, f)) {
		ret += record_gt(&prev, &cur);
		prev = cur;
	}

	fseek(f, pos, SEEK_SET);
	return ret;
}
