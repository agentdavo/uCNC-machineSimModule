#include <stdio.h>
#include <stdlib.h>

#include "stlio.h"

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
	/* printf("%s:%u Writer encountered error %u: %s\n", __FILE__, __LINE__, eCode, stlioErrorStringC(eCode)); */
	return stlioE_Keep;
}

static void printUsage(int argc, char* argv[]) {
	printf("Usage: %s sourceFile targetFile\n\n", argv[0]);
	printf("\tConverts sourceFile STL file into an ASCII STL file that\n\tis written into targetFile.\n");
	return;
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

	if(argc != 3) {
		printUsage(argc, argv);
		return -1;
	}

	e = stlioReadFileMem(
		argv[1],
		&(buf.lpTri),
		&dwTriCount,
		&dwStride,
		&callbackError,
		NULL,
		&fType
	);
	if(e != stlioE_Ok) {
		printf("Reading from %s failed: %s (code %u)\n", argv[1], stlioErrorStringC(e), e);
		return -1;
	}

	#if 0
		switch(fType) {
			case stlFileType_ASCII:		printf("Source file type: ASCII, "); break;
			case stlFileType_Binary:	printf("Source file type: binary, "); break;
			default:					break;
		}
		printf("read %lu triangles\n", dwTriCount);
	#endif

	e = stlioWriteFileMem(
		argv[2],
		buf.lpTri,
		dwTriCount,
		dwStride,
		&callbackWriterError,
		NULL,
		stlFileType_ASCII
	);
	if(e != stlioE_Ok) {
		printf("Writing into %s failed: %s (code %u)\n", argv[2], stlioErrorStringC(e), e);
		return -1;
	}

	return 0;
}
