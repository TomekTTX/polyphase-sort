#pragma once
#include <stdio.h>
#include <stdbool.h>

void manualInput(char *fileName);
void randomizeRecords(char *fileName, long long n);
void sortFile(const char *fileNameIn, const char *fileNameOut, bool logPhases, bool noLog);
void printFile(char *fileNameIn);
