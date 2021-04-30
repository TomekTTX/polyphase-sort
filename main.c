#include "modes.h"
#include "record_io.h"
#include "argparser/posargprsr.h"

int main(int argc, char **argv) {
	const args_t args = parseArgv(argv, '-', defaultBuf, argc, false);
	char *modeOpt = (char *)getArgString("m", &args), *fileName, *fileName2;
	char mode = modeOpt ? modeOpt[0] : '\0';
	long long count = 0;

	switch (mode) {
	case 'm':
		fileName = (char *)getArgString("f", &args);
		manualInput(fileName);
		break;
	case 'r':
		fileName = (char *)getArgString("f", &args);
		count = getArgLLong("c", &args);
		randomizeRecords(fileName, count);
		break;
	case 's':
		fileName = (char *)getArgString("i", &args);
		fileName2 = (char *)getArgString("o", &args);
		sortFile(fileName, fileName2, getArgBool("l", &args), getArgBool("no-log", &args));
		break;
	case 'p':
		fileName = (char *)getArgString("f", &args);
		printFile(fileName);
		break;
	default:
		puts("Specify operation mode after '-m'.");
		break;
	}

	puts("Finished.");
	return 0;
}
