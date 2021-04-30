#include "sorting.h"
#include "record_io.h"
#include <stdlib.h>
#include <string.h>

#pragma warning (disable:4996)
#pragma warning (disable:6031)


bool copyNextRun(rec_file_t *from, rec_file_t *to) {
	static record_t cur, prev;

	if (!record_read(&cur, from->file))
		return false;
	do {
		record_write(&cur, to->file);
		prev = cur;
		if (!record_read(&cur, from->file))
			return true;
	} while (record_gt(&cur, &prev));

	to->lastRec = prev;
	stepBack(from->file);
	return true;
}

bool copyNextTrueRun(rec_file_t *from, rec_file_t *to) {
	record_t rec;

	if (!record_read(&rec, from->file))
		return false;

	stepBack(from->file);
	if (to->lastRec.student_no != 0 && record_gt(&rec, &to->lastRec))
		copyNextRun(from, to);
	return copyNextRun(from, to);
}

bool copyMultipleRuns(rec_file_t *from, rec_file_t *to, int n) {
	int num = n;
	while (n--) {
		if (from->realRuns != 0)
			--from->realRuns;
		if (copyNextTrueRun(from, to))
			++to->realRuns;
		else
			break;
	}
	if (n + 1 != num)
		to->dummyRuns += (n + 1);

	return (n == -1);
}

void distribute(rec_file_t *in, rec_file_t out[2]) {
	uchar index = 1;
	uint x = 2, y = 1;

	copyMultipleRuns(in, out, 1);
	copyMultipleRuns(in, out + 1, 2);
	while (copyMultipleRuns(in, &out[index ^= 1], x)) {
		x = x + y;
		y = x - y;
	}
}

void dummyMerge(rec_file_t *t1, rec_file_t *t2, rec_file_t *out) {
	if (t1->dummyRuns != 0) {
		copyMultipleRuns(t2, out, t1->dummyRuns);
		t1->dummyRuns = 0;
	}
	else if (t2->dummyRuns != 0) {
		copyMultipleRuns(t1, out, t2->dummyRuns);
		t2->dummyRuns = 0;
	}
}

void bufferedPush(record_t record, record_stack *stack, rec_file_t *out) {
	if (stack->top == B_FACT) {
		record_write_page(stack->arr, out->file);
		stack->top = 0;
	}
	stack->arr[stack->top++] = record;
}

void merge(rec_file_t *t0, rec_file_t *t1, rec_file_t *out) {
	record_stack readBuf[2], mergeBuf;
	record_t prev[2] = { 0 };
	short eof = 0;
	ushort stepdown, readPtr[2] = { 0 };
	uint mergeCnt = min(t0->realRuns, t1->realRuns);

	readBuf[0].top = readBuf[1].top = mergeBuf.top = 0;

	while (mergeCnt) {
		if (eof != 1 && readBuf[0].top == readPtr[0]) {
			if (readBuf[0].top = (ushort)record_read_page(readBuf[0].arr, t0->file))
				eof &= (~1U);
			else
				eof |= 1U;
			readPtr[0] = 0;
		}
		if (eof != 2 && readBuf[1].top == readPtr[1]) {
			if (readBuf[1].top = (ushort)record_read_page(readBuf[1].arr, t1->file))
				eof &= (~2U);
			else
				eof |= 2U;
			readPtr[1] = 0;
		}

		stepdown = 0;
		for (ushort i = 0; i < 2; ++i)
			if ((eof & (i + 1)) || record_gt(prev + i, readBuf[i].arr + readPtr[i]))
				stepdown += i + 1;
		if (stepdown == 3) {
			memset(prev, 0, sizeof(prev));
			--mergeCnt;
			--t0->realRuns;
			--t1->realRuns;
			++out->realRuns;
		}
		else if (eof) {
			const ushort ind = (eof ^ 3) - 1;
			bufferedPush(prev[ind] = readBuf[ind].arr[readPtr[ind]++], &mergeBuf, out);
		}
		else if (stepdown == 0 || stepdown == 3) {
			if (record_gt(readBuf[0].arr + readPtr[0], readBuf[1].arr + readPtr[1]))
				bufferedPush(prev[1] = readBuf[1].arr[readPtr[1]++], &mergeBuf, out);
			else
				bufferedPush(prev[0] = readBuf[0].arr[readPtr[0]++], &mergeBuf, out);
		}
		else {
			const ushort ind = (stepdown ^ 3) - 1;
			bufferedPush(prev[ind] = readBuf[ind].arr[readPtr[ind]++], &mergeBuf, out);
		}
	}

	fseek(t0->file, ftell(t0->file) - (readBuf[0].top - readPtr[0]) * sizeof(record_t), SEEK_SET);
	fseek(t1->file, ftell(t1->file) - (readBuf[1].top - readPtr[1]) * sizeof(record_t), SEEK_SET);
	record_write_bulk(mergeBuf.arr, out->file, mergeBuf.top);
}

uint polyphaseSort(FILE *inF, FILE *outF, FILE *tapeF[3], bool logPhases) {
	uint phase = 0;
	short writeInd = 2, emptyInd = 2, partialInd = 0;
	const char *tapeNames[3] = { "tape0", "tape1", "tape2" };
	rec_file_t in = file_wrap(inF), out = file_wrap(outF);
	rec_file_t tape[3] = { file_wrap(tapeF[0]), file_wrap(tapeF[1]), file_wrap(tapeF[2]) };

	distribute(&in, (rec_file_t *)tape);

	freopen("tape0", "rb", tape[0].file);
	freopen("tape1", "rb", tape[1].file);

	if (logPhases)
		logDistribution(tape);

	dummyMerge(tape + 0, tape + 1, tape + 2);

	do {
		++phase;
		writeInd = emptyInd;
		short ind[] = { (writeInd == 0), 1 + (writeInd != 2), writeInd };

		merge(tape + ind[0], tape + ind[1], tape + ind[2]);
		rewind(tape[writeInd].file);

		emptyInd = getEmptyTapeIndex(tape);
		partialInd = (emptyInd | writeInd) ^ 3;
		freopen(tapeNames[writeInd], "rb", tape[writeInd].file);
		freopen(tapeNames[emptyInd], "wb", tape[emptyInd].file);
	   
		if (logPhases)
			logMerge(phase, &tape[partialInd], &tape[writeInd]);

	} while ((tape[0].realRuns == 0) + (tape[1].realRuns == 0) + (tape[2].realRuns == 0) < 2);

	if (tape[writeInd].realRuns != 0)
		record_copy_file(tape[writeInd].file, out.file);
	else
		record_copy_file(tape[partialInd].file, out.file);

	return phase;
}
