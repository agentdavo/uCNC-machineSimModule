#include <stdio.h>
#include <stdlib.h>

#include "../include/stlio.h"

static enum stlioError callbackTriangle(
	struct stlTriangle* lpTriangle,
	void* lpFreeParam
) {
	if(lpFreeParam != (void*)0x12345678) {
		printf("%s:%u ERROR: Invalid parameter passed\n", __FILE__, __LINE__);
		return stlioE_Abort;
	}

/*	printf("Triangle: (%lf %lf %lf) (%lf %lf %lf) (%lf %lf %lf), normal (%lf %lf %lf), attributes: %lu\n",
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
	); */

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
	if(lpFreeParam != (void*)(0xCAFEBACC)) {
		printf("%s:%u ERROR: Invalid parameter passed\n", __FILE__, __LINE__);
		return stlioE_Abort;
	}
	return stlioE_Keep;
}
static enum stlioError callbackEOF(
	void* lpFreeParam
) {
	printf("End of file reached (EOF callback)\n");
	if(lpFreeParam != (void*)(0xDEADBEAF)) {
		printf("%s:%u ERROR: Invalid parameter passed\n", __FILE__, __LINE__);
		return stlioE_Abort;
	}
	return stlioE_Ok;
}

int main(int argc, char* argv[]) {
	enum stlioError e;
	FILE* fHandle;
	unsigned char bByte;
/*	enum stlFileType stlFileType; */

	struct STLParser* lpParser;
	/* Check command line arguments */
	if(argc < 2) { return 0; }

	/* Try to load via triangle iterator */
/*	e = stlioReadFile(
		argv[1],
		&callbackTriangle,
		(void*)(0x12345678),
		&callbackError,
		(void*)(0xCAFEBACC),
		&callbackEOF,
		(void*)(0xDEADBEAF),
		&stlFileType
	);

	if(e != stlioE_Ok) {
		printf("Failed to read STL file %s\n", argv[1]);
		return -1;
	} */

	printf("Opening binary STL file %s\n", argv[1]);
	fHandle = fopen(argv[1], "rb");
	if(!fHandle) { printf("%s:%u Failed to open file %s\n", __FILE__, __LINE__, argv[1]); return 1; }

	/* Create binary STL reader */
	if((e = stlioParserBinary(&lpParser)) != stlioE_Ok) { printf("%s:%u Failed to create binary STL parser. Code %u\n", __FILE__, __LINE__, e); return 1; }

	/* Set callbacks */
	lpParser->vtbl->setCallbackTriangle(lpParser, &callbackTriangle, 	(void*)0x12345678);
	lpParser->vtbl->setCallbackError(lpParser, &callbackError, 			(void*)0xCAFEBACC);
	lpParser->vtbl->setCallbackEOF(lpParser, &callbackEOF, 				(void*)0xDEADBEAF);

	/* Read file byte-wise */
	for(;;) {
		bByte = (unsigned char)fgetc(fHandle);
		if(feof(fHandle)) { break; }

		e = lpParser->vtbl->processByte(lpParser, bByte);
		if(e == stlioE_Done) {
			printf("Done\n");
			break;
		}
		if(e != stlioE_Ok) {
			printf("%s:%u Process byte failed, code %u\n", __FILE__, __LINE__, e);
			break;
		}
	}


	fclose(fHandle);
	printf("%s:%u Releasing reader ... ", __FILE__, __LINE__);
	if((e = lpParser->vtbl->release(lpParser)) == stlioE_Ok) { printf("Ok\n"); } else { printf("failed (%u)\n", e); return 1; }

	printf("Reading file done");
	return 0;
}
