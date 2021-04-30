#include "modes.h"
#include "record_io.h"
#include "sorting.h"
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#pragma warning (disable:4996)
#pragma warning (disable:6031)

void manualInput(char *fileName) {
	FILE *out;
	record_t temp;
	bool alloc = validateFileName(&fileName);

	if (out = fopen(fileName, "wb")) {
		puts("Writing records from stdin to file; type '0' to finish.");
		while (1) {
			scanf("%u", &temp.student_no);
			if (temp.student_no == 0)
				break;
			scanf("%hu %hu %hu", temp.grades, temp.grades + 1, temp.grades + 2);
			record_write(&temp, out);
		}
		fclose(out);
	}
	else
		fprintf(stderr, "The file '%s' failed to open.\n", fileName);

	if (alloc)
		free(fileName);
}

void randomizeRecords(char *fileName, long long n) {
	uint num = 1;
	FILE *out;
	bool alloc = validateFileName(&fileName);

	uint seed = (uint)time(NULL);
	printf("seed = %u\n", seed);
	srand(seed);

	if (out = fopen(fileName, "wb")) {
		while (n < 0) {
			printf("Record count invalid or not specified, choose number of records to randomize: ");
			scanf("%lld", &n);
		}
		while (n--) {
			record_t temp = { num++, { 2 + rand() % 4, 2 + rand() % 4, 2 + rand() % 4 } };
			record_write(&temp, out);
		}
		fclose(out);
	}
	else
		fprintf(stderr, "The file '%s' failed to open.\n", fileName);

	if (alloc)
		free(fileName);
}

void sortFile(const char *fileNameIn, const char *fileNameOut, bool logPhases, bool noLog) {
	extern uint page_reads, page_writes;
	uint phases, initialRuns;
	FILE *in, *out, *tape[3];

	if (fileNameIn && fileNameOut) {
		if ((in = fopen(fileNameIn, "rb")) &&
			(out = fopen(fileNameOut, "wb")) &&
			(tape[0] = fopen("tape0", "wb")) &&
			(tape[1] = fopen("tape1", "wb")) &&
			(tape[2] = fopen("tape2", "wb")))
		{
			if (!noLog) {
				puts("\nINPUT FILE:");
				record_print_file(in);
				initialRuns = countRuns(in);
				rewind(in);
			}

			phases = polyphaseSort(in, out, tape, logPhases);

			fclose(in);
			fclose(tape[0]);
			fclose(tape[1]);
			fclose(tape[2]);

			unlink("tape0");
			unlink("tape1");
			unlink("tape2");

			if (!noLog) {
				puts("\nSORTED FILE:");
				freopen(fileNameOut, "rb", out);
				record_print_file(out);
				printf("\ninitial runs: %u\nsorting phases: %u\ndisk page reads: %u\ndisk page writes: %u\ntotal disk accesses: %u\n",
					initialRuns, phases, page_reads, page_writes, page_reads + page_writes);
			}

			fclose(out);
		}
		else
			fputs("One or more files failed to open.\n", stderr);
	}
	else
		fputs("Specify input and output file names with '-i' and '-o'.\n", stderr);
}

void printFile(char *fileName) {
	FILE *in;
	bool alloc = validateFileName(&fileName);

	if (in = fopen(fileName, "rb")) {
		record_print_file(in);
		fclose(in);
	}
	else
		fprintf(stderr, "The file '%s' failed to open.\n", fileName);

	if (alloc)
		free(fileName);
}
