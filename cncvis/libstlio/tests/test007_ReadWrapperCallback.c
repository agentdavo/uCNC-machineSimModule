#include <stdio.h>
#include <stdlib.h>

#include "../include/stlio.h"

/* Reader callbacks */
static enum stlioError callbackTriangle(
	struct stlTriangle* lpTriangle,
	void* lpFreeParam
) {
	printf("Triangle: (%lf %lf %lf) (%lf %lf %lf) (%lf %lf %lf), normal (%lf %lf %lf), attributes: %lu\n",
		lpTriangle->vertices[0][0],
		lpTriangle->vertices[0][1],
		lpTriangle->vertices[0][2],

		lpTriangle->vertices[1][0],
		lpTriangle->vertices[1][1],
		lpTriangle->vertices[1][2],

		lpTriangle->vertices[2][0],
		lpTriangle->vertices[2][1],
		lpTriangle->vertices[2][2],

		lpTriangle->surfaceNormal[0],
		lpTriangle->surfaceNormal[1],
		lpTriangle->surfaceNormal[2],

		lpTriangle->attributeByteCount
	);

	return stlioE_Ok;
}
static enum stlioError callbackError(
	enum stlioError eCode,
	unsigned long int dwLineNumber,
	unsigned long int dwLineChar,
	unsigned long int dwByteOffset,
	void* lpFreeParam
) {
	printf("Parser encountered error %u at line %lu:%lu or offset %lu\n", eCode, dwLineNumber, dwLineChar, dwByteOffset);
	return stlioE_Keep; /* Ignore errors ... */
}
static enum stlioError callbackEOF(
	void* lpFreeParam
) {
	printf("End of file reached (EOF callback)\n");
	return stlioE_Ok;
}

int main(int argc, char* argv[]) {
	enum stlioError e;
	enum stlFileType fType;

	if(argc < 2) {
		printf("Missing filename\n");
		return -1;
	}

	printf("Reading file %s\n", argv[1]);
	e = stlioReadFile(argv[1], &callbackTriangle, NULL, &callbackError, NULL, &callbackEOF, NULL, &fType);
	printf("Read returned %s (%u)\n", stlioErrorStringC(e), e);
	switch(fType) {
		case stlFileType_ASCII:		printf("File type: ASCII\n"); break;
		case stlFileType_Binary:	printf("File type: binary\n"); break;
		default:					break;
	}
	return 0;
}
