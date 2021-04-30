#pragma once
#include "typedefs.h"
#include <stdio.h>
#include <stdbool.h>

size_t record_write(const record_t *record, FILE *file);
size_t record_read(record_t *record, FILE *file);

size_t record_write_bulk(const record_t *record, FILE *file, uint n);

size_t record_write_page(const record_t record[B_FACT], FILE *file);
size_t record_read_page(record_t record[B_FACT], FILE *file);


void record_print(const record_t *rec);
void record_print_file(FILE *file);
void record_log_file(FILE *file, FILE *log);

void record_copy_file(FILE *from, FILE *to);
