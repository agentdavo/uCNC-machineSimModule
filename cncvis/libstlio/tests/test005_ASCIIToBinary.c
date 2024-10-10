#include <stdio.h>
#include <stdlib.h>

#include "../include/stlio.h"

/* Reader callbacks */
static enum stlioError callbackTriangle(
	struct stlTriangle* lpTriangle,
	void* lpFreeParam
) {
	enum stlioError e;
	struct STLWriter* lpWriter;

	lpWriter = (struct STLWriter*)lpFreeParam;
	if(lpWriter == NULL) {
		printf("Failed to fetch writer via free parameter\n");
		return stlioE_InvalidParam;
	}

	/* printf("Triangle: (%lf %lf %lf) (%lf %lf %lf) (%lf %lf %lf), normal (%lf %lf %lf), attributes: %lu\n",
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


	e = lpWriter->vtbl->writeTriangle(
		lpWriter,
		lpTriangle
	);
	if(e != stlioE_Ok) {
		printf("%s:%u Failed to write triangle, code %u\n", __FILE__, __LINE__, e);
		return e;
	}

	/* Write triangle into writer ... */
	// printf(".");
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

/* Writer Error Callback - TODO */

/* First the stream sink */
static FILE* fHandleOut = NULL;

static enum stlioError STLStreamSink_writeBytes(
	struct STLStreamSink* 		lpSelf,
	unsigned char* 				lpData,
	unsigned long int 			dwByteCount,
	unsigned long int* 			lpBytesWritten
) {
	size_t res;

	if(fHandleOut == NULL) { return stlioE_InvalidState; }

	if(lpBytesWritten != NULL) { (*lpBytesWritten) = 0; }

	res = fwrite(lpData, 1, dwByteCount, fHandleOut);
	if(res <= 0) { return stlioE_IOError; }
	if(lpBytesWritten != NULL) {
		(*lpBytesWritten) = res;
		return (dwByteCount == res) ? stlioE_Ok : stlioE_Continues;
	} else {
		return (dwByteCount == res) ? stlioE_Ok : stlioE_IOError;
	}
}
static enum stlioError STLStreamSink_seekAbsolute(
	struct STLStreamSink* 		lpSelf,
	unsigned long int			dwOffset
) {
	if(fHandleOut == NULL) { return stlioE_InvalidState; }

	if(fseek(fHandleOut, (signed long int)dwOffset, SEEK_SET) < 0) {
		return stlioE_IOError;
	} else {
		return stlioE_Ok;
	}
}
static enum stlioError STLStreamSink_seekRelative(
	struct STLStreamSink* 		lpSelf,
	signed long int				dwOffset
) {
	if(fHandleOut == NULL) { return stlioE_InvalidState; }

	if(fseek(fHandleOut, dwOffset, SEEK_CUR) < 0) {
		return stlioE_IOError;
	} else {
		return stlioE_Ok;
	}
}
static enum stlioError STLStreamSink_release(
	struct STLStreamSink* 		lpSelf
) {
	if(fHandleOut == NULL) { return stlioE_InvalidState; }
	fclose(fHandleOut);
	fHandleOut = NULL;
	return stlioE_Ok;
}

static struct STLStreamSink_vtbl streamSinkVTBL = {
	&STLStreamSink_release,

	&STLStreamSink_writeBytes,
	&STLStreamSink_seekAbsolute,
	&STLStreamSink_seekRelative
};
static struct STLStreamSink streamSink = {
	&streamSinkVTBL,
	NULL
};


static enum stlioError STLWriter_Callback_Error(
	enum stlioError eCode,
	struct stlTriangle* lpTriangle,
	void* lpFreeParam
) {
	if(lpFreeParam != (void*)0xDEADDEAD) {
		printf("%s:%u FAILED: Error callback received invalid free data\n", __FILE__, __LINE__);
		return stlioE_Abort;
	}
	printf("%s:%u Writer signaled an error %u via callback\n", __FILE__, __LINE__, eCode);
	printf("\tTriangle: (%lf %lf %lf) (%lf %lf %lf) (%lf %lf %lf), normal (%lf %lf %lf), attrib: %lu\n",
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
	return stlioE_Keep; /* We keep invalid data */
}



int main(int argc, char* argv[]) {
	enum stlioError e;
	FILE* fHandle;
	unsigned char bByte;

	struct STLParser* lpParser;
	struct STLWriter* lpWriter;

	/* Check command line arguments */
	if(argc < 3) { return 0; }

	/* Create parser and writer ... */
	printf("Opening ASCII STL input file %s\n", argv[1]);
	fHandle = fopen(argv[1], "rb");
	if(!fHandle) { printf("%s:%u Failed to open file %s\n", __FILE__, __LINE__, argv[1]); return 1; }

	printf("Opening binary STL output file %s\n", argv[2]);
	fHandleOut = fopen(argv[2], "w+b");
	if(!fHandleOut) { printf("%s:%u Failed to open file %s\n", __FILE__, __LINE__, argv[2]); fclose(fHandleOut); return 1; }

	/* Create writer */
	if((e = stlioWriterBinary(&lpWriter, &streamSink)) != stlioE_Ok) {
		printf("%s:%u Failed to create binary STL writer. Code %u\n", __FILE__, __LINE__, e);
		return 1;
	}
	/* Disable vertex nomralization in case some test data contains denaturated triangles. TODO Move this to error callback ... */
	// lpWriter->vtbl->setNormalMode(lpWriter, stlWriter_NormalMode__CopyNormals);
	lpWriter->vtbl->setCallbackError(lpWriter, &STLWriter_Callback_Error, (void*)0xDEADDEAD);

	/* Create binary STL reader */
	if((e = stlioParserASCII(&lpParser)) != stlioE_Ok) { printf("%s:%u Failed to create ASCII STL parser. Code %u\n", __FILE__, __LINE__, e); return 1; }
	/* Set reader callbacks */
	lpParser->vtbl->setCallbackTriangle(lpParser, &callbackTriangle, 	(void*)lpWriter);
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

	/* Finalize writer */
	printf("%s:%u Finalizing writer ... ", __FILE__, __LINE__);
	e = lpWriter->vtbl->finalize(lpWriter);
	if(e != stlioE_Done) {
		printf("failed (code %u)\n", e);
	} else {
		printf("ok (Done)\n");
	}

	fclose(fHandle);
	printf("%s:%u Releasing reader ... ", __FILE__, __LINE__);
	if((e = lpParser->vtbl->release(lpParser)) == stlioE_Ok) { printf("Ok\n"); } else { printf("failed (%u)\n", e); return 1; }

	printf("%s:%u Releasing writer ... ", __FILE__, __LINE__);
	if((e = lpWriter->vtbl->release(lpWriter)) == stlioE_Ok) { printf("Ok\n"); } else { printf("failed (%u)\n", e); return 1; }

	printf("Reading file done");
	return 0;
}
