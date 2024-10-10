#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

int main(int argc, char* argv[]) {
	union {
		struct stlTriangle* lpTri;
		unsigned char* lpBuff;
	} buf;
	union {
		struct stlTriangle* lpT;
		struct stlTriangle_Attributes* lpTA;
	} lpTriangle;
	unsigned long int dwTriCount;
	unsigned long int dwStride;
	unsigned long int i;
	enum stlioError e;
	enum stlFileType fType;

	if(argc < 2) {
		printf("Missing filename\n");
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

	printf("Triangles:\n");
	for(i = 0; i < dwTriCount; i=i+1) {
		/* Calculate buffer position (from stride size) */
		lpTriangle.lpT = (struct stlTriangle*)((uintptr_t)(buf.lpBuff) + dwStride*i);

		printf("Triangle: (%lf %lf %lf) (%lf %lf %lf) (%lf %lf %lf), normal (%lf %lf %lf), attributes: %lu\n",
			lpTriangle.lpT->vertices[0][0],
			lpTriangle.lpT->vertices[0][1],
			lpTriangle.lpT->vertices[0][2],

			lpTriangle.lpT->vertices[1][0],
			lpTriangle.lpT->vertices[1][1],
			lpTriangle.lpT->vertices[1][2],

			lpTriangle.lpT->vertices[2][0],
			lpTriangle.lpT->vertices[2][1],
			lpTriangle.lpT->vertices[2][2],

			lpTriangle.lpT->surfaceNormal[0],
			lpTriangle.lpT->surfaceNormal[1],
			lpTriangle.lpT->surfaceNormal[2],

			lpTriangle.lpT->attributeByteCount
		);
	}

	return 0;
}
