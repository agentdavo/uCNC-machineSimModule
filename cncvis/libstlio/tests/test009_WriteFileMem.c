#include <stdio.h>
#include <stdlib.h>

#include "../include/stlio.h"

static enum stlioError callbackError(
	enum stlioError eCode,
	unsigned long int dwLineNumber,
	unsigned long int dwLineChar,
	unsigned long int dwByteOffset,
	void* lpFreeParam
) {
	printf("Parser encountered error %u:%s at line %lu:%lu or offset %lu\n", eCode, stlioErrorStringC(eCode), dwLineNumber, dwLineChar, dwByteOffset);
	return stlioE_Keep; /* Ignore errors ... */
}

static enum stlioError callbackWriterError(
	enum stlioError eCode,
	struct stlTriangle* lpTriangle,
	void* lpFreeParam
) {
	printf("%s:%u Writer encountered error %u: %s\n", __FILE__, __LINE__, eCode, stlioErrorStringC(eCode));
	return stlioE_Keep;
}

int main(int argc, char* argv[]) {
	union {
		struct stlTriangle* lpTri;
		unsigned char* lpBuff;
	} buf;
	unsigned long int dwTriCount;
	unsigned long int dwStride;
	enum stlioError e;
	enum stlFileType fType;

	if(argc < 3) {
		printf("Missing filenames (source, output)\n");
		return -1;
	}

	printf("Reading file %s\n", argv[1]);
	e = stlioReadFileMem(
		argv[1],
		&(buf.lpTri),
		&dwTriCount,
		&dwStride,
		&callbackError,
		NULL,
		&fType
	);
	printf("Read returned %s (%u)\n", stlioErrorStringC(e), e);
	switch(fType) {
		case stlFileType_ASCII:		printf("File type: ASCII\n"); break;
		case stlFileType_Binary:	printf("File type: binary\n"); break;
		default:					break;
	}
	printf("Read %lu triangles with stride size %lu\n", dwTriCount, dwStride);

	/* Now write again */
	if(fType == stlFileType_ASCII) {
		printf("Writing into file %s as binary STL\n", argv[2]);
		fType = stlFileType_Binary;
	} else {
		printf("Writing into file %s as ASCII STL\n", argv[2]);
		fType = stlFileType_ASCII;
	}
	e = stlioWriteFileMem(
		argv[2],
		buf.lpTri,
		dwTriCount,
		dwStride,
		&callbackWriterError,
		NULL,
		fType
	);
	printf("Write resulted in %u: %s\n", e, stlioErrorStringC(e));

	return (e == stlioE_Ok) ? 0 : -1;
}
