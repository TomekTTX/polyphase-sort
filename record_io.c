#include "record_io.h"
#include "utils.h"
#include <stdio.h>

#pragma warning (disable:4996)

uint page_reads = 0, page_writes = 0;

size_t record_write(const record_t *record, FILE *file) {
#ifdef DEBUG
	printf("Wrote {%u,{%hu,%hu,%hu}}.\n", record->student_no, record->grades[0], record->grades[1], record->grades[2]);
#endif
	return fwrite(record, sizeof(record_t), 1, file);
}

size_t record_read(record_t *record, FILE *file) {
	size_t ret = fread(record, sizeof(record_t), 1, file);
#ifdef DEBUG
	printf("Read {%u,{%hu,%hu,%hu}}.\n", record->student_no, record->grades[0], record->grades[1], record->grades[2]);
#endif
	return ret;
}

size_t record_write_bulk(const record_t *record, FILE *file, uint n) {
	size_t ret = fwrite(record, sizeof(record_t), n, file);
	if (ret)
		++page_writes;
	return ret;
}

size_t record_write_page(const record_t record[B_FACT], FILE *file) {
	size_t ret = fwrite(record, sizeof(record_t), B_FACT, file);
	if (ret)
		++page_writes;
	return ret;
}

size_t record_read_page(record_t record[B_FACT], FILE *file) {
	size_t ret = fread(record, sizeof(record_t), B_FACT, file);
	if (ret) 
		++page_reads;
	return ret;
}

void record_print(const record_t *rec) {
	printf("{stud=%06u, grad={%hu,%hu,%hu}, avg=%.2f}",
		rec->student_no, rec->grades[0], rec->grades[1], rec->grades[2], avg3(rec->grades));
}

void record_print_file(FILE *file) {
	if (file == NULL)
		return;

	record_t temp;
	uint cnt = 0;
	long pos = ftell(file);

	while (fread(&temp, sizeof(temp), 1, file) > 0) {
		printf("{stud=%06u, grad={%hu,%hu,%hu}, avg=%.2f}\n",
			temp.student_no, temp.grades[0], temp.grades[1], temp.grades[2], avg3(temp.grades));
		++cnt;
	}
	printf("%u records total\n", cnt);
	fseek(file, pos, SEEK_SET);
}

void record_log_file(FILE *file, FILE *log) {
	if (file == NULL)
		return;

	record_t temp;
	uint cnt = 0;
	long pos = ftell(file);

	while (fread(&temp, sizeof(temp), 1, file) > 0) {
		fprintf(log, "{stud=%06u, grad={%hu,%hu,%hu}, avg=%.2f}\n",
			temp.student_no, temp.grades[0], temp.grades[1], temp.grades[2], avg3(temp.grades));
		++cnt;
	}
	fprintf(log, "%u records total\n", cnt);
	fseek(file, pos, SEEK_SET);
}

void record_copy_file(FILE *from, FILE *to) {
	record_stack buf;
	
	while ((buf.top = (ushort)record_read_page(buf.arr, from)) == B_FACT)
		record_write_page(buf.arr, to);
	record_write_bulk(buf.arr, to, buf.top);
}

