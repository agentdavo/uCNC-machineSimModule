#ifndef STLIO_ASCII_PARSER__TOKEN_BUFFER_SIZE
	#define STLIO_ASCII_PARSER__TOKEN_BUFFER_SIZE	64		/* This has to fit all the ASCII constants (the longest is 8 characters) and floating point numbers in IEEE representation */
#endif

/*
	Basic STL Input / Output Routines
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>
#include <float.h>
#include <string.h>
#include "../include/stlio.h"
#include "../include/serdeshelper.h"

/*
	Floating point helper (comparison)
*/
#ifdef STLIO_VERTEX_NORMAL_VALIDATION_ENABLED
	static int fltEquals(float a, float b) {
		/* To avoid corner cases with fabs we use another approach */
		float t1, t2;

		/* Make numbers positive */
		t1 = (a < 0) ? -1.0 * a : a;
		t2 = (b < 0) ? -1.0 * b : b;

		/* Calculate absolute difference |a-b| */
		t1 = (t1 > t2) ? t1 - t2 : t2 - t1;

		return (t1 > FLT_EPSILON) ? 0 : 1;
	}
#endif
static int dblEquals(double a, double b) {
	/* To avoid corner cases with fabs we use another approach */
	double t1, t2;

	/* Make numbers positive */
	t1 = (a < 0) ? -1.0 * a : a;
	t2 = (b < 0) ? -1.0 * b : b;

	/* Calculate absolute difference |a-b| */
	t1 = (t1 > t2) ? t1 - t2 : t2 - t1;

	return (t1 > DBL_EPSILON) ? 0 : 1;
}

/*
	Floating point helper: reading ASCII float
*/
enum fltReadASCII_State {
	fltReadASCII_State__First,
	fltReadASCII_State__Number,
	fltReadASCII_State__Fraction,
	fltReadASCII_State__Exponent
};
static enum stlioError fltReadASCII(unsigned char* lpData, unsigned long int dwDataLength, float* lpOut) {
	unsigned long int i;
	double result 					= 0;
	unsigned long int dwExponent 	= 0;
	double dFractionDivisor			= 1.0;
	double dSign 					= 1.0;
	double dExponentSign 			= 0.0;
	enum fltReadASCII_State state 	= fltReadASCII_State__First;

	for(i = 0; i < dwDataLength; i=i+1) {
		switch(state) {
			case fltReadASCII_State__First:
				if(lpData[i] == '+') { state = fltReadASCII_State__Number; break; /* Ignore a positive sign */ }
				if(lpData[i] == '-') { dSign = -1.0; state = fltReadASCII_State__Number; break; /* Remember sign */ }
				if((lpData[i] >= 0x30) && (lpData[i] <= 0x39)) { /* Simple number */ state = fltReadASCII_State__Number; result = (result * 10.0) + (double)(lpData[i] - 0x30); break; }
				if((lpData[i] == 'e') || (lpData[i] == 'E')) { /* 0 * anything stays 0 ... but continue parsing anyways to detect errors */ state = fltReadASCII_State__Exponent; break; }
				if(lpData[i] == '.') { /* Fractional part 0.xxxx */ state = fltReadASCII_State__Fraction; break; }
				return stlioE_InvalidFormat_NotAFloat;
			case fltReadASCII_State__Number:
				if((lpData[i] >= 0x30) && (lpData[i] <= 0x39)) { /* Simple number */ result = (result * 10.0) + (double)(lpData[i] - 0x30); break; }
				if((lpData[i] == 'e') || (lpData[i] == 'E')) { state = fltReadASCII_State__Exponent; break; }
				if(lpData[i] == '.') { state = fltReadASCII_State__Fraction; break; }
				return stlioE_InvalidFormat_NotAFloat;
			case fltReadASCII_State__Fraction:
				if((lpData[i] >= 0x30) && (lpData[i] <= 0x39)) { /* Simple number */ result = (result * 10.0) + (double)(lpData[i] - 0x30); dFractionDivisor = dFractionDivisor * 10.0; break; }
				if((lpData[i] == 'e') || (lpData[i] == 'E')) { state = fltReadASCII_State__Exponent; break; }
				return stlioE_InvalidFormat_NotAFloat;
			case fltReadASCII_State__Exponent:
				if((lpData[i] == '+') && (dwExponent == 0) && (dExponentSign == 0.0)) { dExponentSign = 1.0; break; /* Ignore a positive sign */ }
				if((lpData[i] == '-') && (dwExponent == 0) && (dExponentSign == 0.0)) { dExponentSign = -1.0; break; /* Remember sign */ }
				if((lpData[i] >= 0x30) && (lpData[i] <= 0x39)) { dwExponent = (dwExponent * 10) + (unsigned long int)(lpData[i] - 0x30); break; }
				return stlioE_InvalidFormat_NotAFloat;
			default:
				return stlioE_ImplementationError;
		}
	}

	/* Calculate number */
	result = result / dFractionDivisor;
	if((dExponentSign == 1.0) || (dExponentSign == 0.0)) {
		while(dwExponent > 0) { result = result * 10.0; dwExponent = dwExponent - 1; }
	} else if(dExponentSign == -1.0) {
		while(dwExponent > 0) { result = result / 10.0; dwExponent = dwExponent - 1; }
	}

	(*lpOut) = (dSign > 0) ? result : -1.0*result;
	return stlioE_Ok;
}


/*
	ASCII constants
*/
static char* strSolid 		= "solid";
static char* strFacet 		= "facet";
static char* strNormal		= "normal";
static char* strOuter	 	= "outer";
static char* strLoop		= "loop";
static char* strVertex 		= "vertex";
static char* strEndloop 	= "endloop";
static char* strEndfacet 	= "endfacet";
static char* strEndsolid 	= "endsolid";

static char* strFacetNormal	= " facet normal "; /* Redundant but more simple for ASCII writer */
static char* strOuterLoop	= "  outer loop"; /* Redundant but more simple for ASCII writer */
static char* strVertex2		= "   vertex "; /* Redundant but more simple for ASCII writer */
static char* strEndloop2	= "  endloop";
static char* strEndfacet2	= " endfacet";
static char* strEndsolid2 	= "endsolid ";

static char strCRLF[2]			= { 0x0D, 0x0A };

#define strSolidLEN		5
#define strFacetLEN		5
#define strNormalLEN	6
#define strOuterLEN		5
#define strLoopLEN		4
#define strVertexLEN	6
#define strEndloopLEN	7
#define strEndfacetLEN	8
#define strEndsolidLEN	8

/*
	Binary STL Parser
	=================
*/

enum STLParser_Binary__ReadSection {
	STLParser_Binary__ReadSection_Header,
	STLParser_Binary__ReadSection_Count,
	STLParser_Binary__ReadSection_Triangles,

	/* Two additional states */
	STLParser_Binary__ReadSection_Done,
	STLParser_Binary__ReadSection_Error
};


#define STLParser_Binary__Flag__HeaderIsNotASCII					0x80000000
struct STLParser_Binary {
	struct STLParser					base;

	/* Parser state */
	uint32_t							dwFlags;
	enum STLParser_Binary__ReadSection	readSection;
	unsigned long int					dwReadTriangles;			/* Statistics: Counts the number of triangles that have already been read */
	unsigned long int					dwErrorCount;				/* Number of triangles that had an error (and have been repaired, dropped or used without repair according to the application) */
	unsigned long int					dwFileOffset;				/* Current (byte) offset inside the file */
	unsigned long int					dwReadBytesCurrentElement;	/* Number of bytes from the current element (header, count, triagne) that has been "read" from the input */

	unsigned long int					dwTriangleCount;			/* Number of triangles (0 till "count" has been decoded) */
	struct stlTriangle					currentTriangle;			/* Current triagle WITHOUT attribute data */
	struct stlTriangle_Attributes*		currentTriangle_Attributes;	/* If attribute data is present an appropriate structure will be allocated */

	unsigned char						bDataBuffer[12*4+2+30];

	/* Callbacks */
	lpfnSTLCallback_Triangle			callbackTriangle;
	lpfnSTLCallback_Error				callbackError;
	lpfnSTLCallback_EOF					callbackEOF;
	void*								callbackTriangleParam;
	void*								callbackErrorParam;
	void*								callbackEOFParam;
};

static const char stlConstHeaderBinary_ForbiddenBytes[] = "solid ";

#ifdef STLIO_VERTEX_NORMAL_VALIDATION_ENABLED
	static enum stlioError stlParserBinary__ValidateTriangle(
		struct STLParser_Binary* lpThis,
		struct stlTriangle* lpTriangle
	) {
		/* Validate that the surface normal points in the correct direction / the points are ordered counter clockwise */
		double dCalculatedNormal[3]; 	double dCalculatedNormalLen;
		double dPassedNormal[3]; 		double dPassedNormalLen;

		/* Cross product */
		dCalculatedNormal[0] = ( lpTriangle->vertices[1][1]-lpTriangle->vertices[0][1])*(lpTriangle->vertices[2][2]-lpTriangle->vertices[0][2])-(lpTriangle->vertices[1][2]-lpTriangle->vertices[0][2])*(lpTriangle->vertices[2][1]-lpTriangle->vertices[0][1]);
		dCalculatedNormal[1] = -(lpTriangle->vertices[1][0]-lpTriangle->vertices[0][0])*(lpTriangle->vertices[2][2]-lpTriangle->vertices[0][2])+(lpTriangle->vertices[1][2]-lpTriangle->vertices[0][2])*(lpTriangle->vertices[2][0]-lpTriangle->vertices[0][0]);
		dCalculatedNormal[2] = ( lpTriangle->vertices[1][0]-lpTriangle->vertices[0][0])*(lpTriangle->vertices[2][1]-lpTriangle->vertices[0][1])-(lpTriangle->vertices[1][1]-lpTriangle->vertices[0][1])*(lpTriangle->vertices[2][0]-lpTriangle->vertices[0][0]);

		if((dCalculatedNormal[0] == 0) && (dCalculatedNormal[1] == 0) && (dCalculatedNormal[2] == 0)) {
			return stlioE_InvalidFormat_NotATriangle;
		}
		/* Normalize */
		dCalculatedNormalLen = sqrt(dCalculatedNormal[0]*dCalculatedNormal[0]+dCalculatedNormal[1]*dCalculatedNormal[1]+dCalculatedNormal[2]*dCalculatedNormal[2]);
		dCalculatedNormal[0] = dCalculatedNormal[0] / dCalculatedNormalLen;
		dCalculatedNormal[1] = dCalculatedNormal[1] / dCalculatedNormalLen;
		dCalculatedNormal[2] = dCalculatedNormal[2] / dCalculatedNormalLen;

		dPassedNormal[0] = lpTriangle->surfaceNormal[0];
		dPassedNormal[1] = lpTriangle->surfaceNormal[1];
		dPassedNormal[2] = lpTriangle->surfaceNormal[2];

		dPassedNormalLen = sqrt(dPassedNormal[0]*dPassedNormal[0]+dPassedNormal[1]*dPassedNormal[1]+dPassedNormal[2]*dPassedNormal[2]);
		dPassedNormal[0] = dPassedNormal[0] / dPassedNormalLen;
		dPassedNormal[1] = dPassedNormal[1] / dPassedNormalLen;
		dPassedNormal[2] = dPassedNormal[2] / dPassedNormalLen;

		/* if((dPassedNormal[0] == dCalculatedNormal[0]) && (dPassedNormal[1] == dCalculatedNormal[1]) && (dPassedNormal[2] == dCalculatedNormal[2])) { */
		if(fltEquals(dPassedNormal[0], dCalculatedNormal[0]) && fltEquals(dPassedNormal[1], dCalculatedNormal[1]) && fltEquals(dPassedNormal[2], dCalculatedNormal[2])) {
			return stlioE_Ok;
		} else {
			return stlioE_InvalidFormat_VertexNormalMismatch;
		}
	}
#endif

static enum stlioError stlParserBinary__STLParser_ProcessByte(
	struct STLParser* lpSelf,
	unsigned char bData
) {
	enum stlioError e;
	struct STLParser_Binary* lpThis;
	#ifdef STLIO_VERTEX_NORMAL_VALIDATION_ENABLED
		double dTemp;
		double dvTemp[3];
	#endif

	/* Input data validation */
	if(lpSelf == NULL) { return stlioE_InvalidParam; }

	/* Recover this pointer */
	lpThis = (struct STLParser_Binary*)(lpSelf->lpReserved);

	/* Check our current state */
	lpThis->dwFileOffset = lpThis->dwFileOffset + 1;
	if(lpThis->readSection == STLParser_Binary__ReadSection_Header) {
		/* We read the first 80 bytes section header */
		lpThis->bDataBuffer[lpThis->dwReadBytesCurrentElement] = bData;

		/*
			Check continuously if we have an ASCII file instead
		*/
		if((lpThis->dwReadBytesCurrentElement = lpThis->dwReadBytesCurrentElement + 1) < 80) {
			if(lpThis->dwReadBytesCurrentElement < sizeof(stlConstHeaderBinary_ForbiddenBytes)) {
				if(stlConstHeaderBinary_ForbiddenBytes[lpThis->dwReadBytesCurrentElement] != bData) { lpThis->dwFlags = lpThis->dwFlags | STLParser_Binary__Flag__HeaderIsNotASCII; }
			}
			return stlioE_Ok;
		}

		/*
			Or we have finished the header.

			In this case check if it is an ASCII file instead ...
		*/
		if((lpThis->dwFlags & STLParser_Binary__Flag__HeaderIsNotASCII) == 0) {
			lpThis->readSection = STLParser_Binary__ReadSection_Error;
			if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat, 0, 0, 0, lpThis->callbackErrorParam); /* We ignore that return value */ }
			return stlioE_InvalidFormat; /* This is not recoverable */
		}
		lpThis->dwReadBytesCurrentElement = 0;
		lpThis->readSection = STLParser_Binary__ReadSection_Count;
		return stlioE_Ok;
	} else if(lpThis->readSection == STLParser_Binary__ReadSection_Count) {
		lpThis->bDataBuffer[lpThis->dwReadBytesCurrentElement] = bData;
		if((lpThis->dwReadBytesCurrentElement = lpThis->dwReadBytesCurrentElement + 1) < 4) {
			return stlioE_Ok;
		}

		/* Deserialize */
		lpThis->dwTriangleCount =
			(((unsigned long int)(lpThis->bDataBuffer[0])) & 0xFF) |
			((((unsigned long int)(lpThis->bDataBuffer[1])) << 8) & 0xFF00) |
			((((unsigned long int)(lpThis->bDataBuffer[2])) << 16) & 0xFF0000) |
			((((unsigned long int)(lpThis->bDataBuffer[3])) << 24) & 0xFF000000);
		if(lpThis->dwTriangleCount == 0) {
			lpThis->readSection = STLParser_Binary__ReadSection_Done;
			if(lpThis->callbackEOF != NULL) { lpThis->callbackEOF(lpThis->callbackEOFParam); }
			return stlioE_Done;
		}
		lpThis->dwReadBytesCurrentElement = 0;
		lpThis->readSection = STLParser_Binary__ReadSection_Triangles;
		return stlioE_Ok;
	} else if(lpThis->readSection == STLParser_Binary__ReadSection_Triangles) {
		if(lpThis->currentTriangle_Attributes != NULL) {
			/* We are already reading attributes ... */
			lpThis->currentTriangle_Attributes->attributeBytes[lpThis->dwReadBytesCurrentElement-50] = bData;
			if((lpThis->dwReadBytesCurrentElement = lpThis->dwReadBytesCurrentElement + 1) == (50 + lpThis->currentTriangle_Attributes->base.attributeByteCount)) {
				/* Done Triangle */

				/* Validate */
				#ifdef STLIO_VERTEX_NORMAL_VALIDATION_ENABLED
					e = stlParserBinary__ValidateTriangle(lpThis, (struct stlTriangle*)(lpThis->currentTriangle_Attributes));
					if(e != stlioE_Ok) {
						if(lpThis->callbackError != NULL) {
							if(e == stlioE_InvalidFormat_NotATriangle) {
								e = lpThis->callbackError(stlioE_InvalidFormat_NotATriangle, 0, 0, lpThis->dwFileOffset-1, lpThis->callbackErrorParam);
								if(e != stlioE_Keep) {
									lpThis->readSection = STLParser_Binary__ReadSection_Error;
									return stlioE_InvalidFormat_NotATriangle;
								}
							} else {
								switch(e = lpThis->callbackError(stlioE_InvalidFormat_VertexNormalMismatch, 0, 0, lpThis->dwFileOffset-1, lpThis->callbackErrorParam)) {
									case stlioE_RepairOrder:
										lpThis->currentTriangle.vertices[1][0] = lpThis->currentTriangle_Attributes->base.vertices[2][0];
										lpThis->currentTriangle.vertices[1][1] = lpThis->currentTriangle_Attributes->base.vertices[2][1];
										lpThis->currentTriangle.vertices[1][2] = lpThis->currentTriangle_Attributes->base.vertices[2][2];

										lpThis->currentTriangle_Attributes->base.vertices[2][0] = lpThis->currentTriangle_Attributes->base.vertices[1][0];
										lpThis->currentTriangle_Attributes->base.vertices[2][1] = lpThis->currentTriangle_Attributes->base.vertices[1][1];
										lpThis->currentTriangle_Attributes->base.vertices[2][2] = lpThis->currentTriangle_Attributes->base.vertices[1][2];

										lpThis->currentTriangle_Attributes->base.vertices[1][0] = lpThis->currentTriangle.vertices[1][0];
										lpThis->currentTriangle_Attributes->base.vertices[1][1] = lpThis->currentTriangle.vertices[1][1];
										lpThis->currentTriangle_Attributes->base.vertices[1][2] = lpThis->currentTriangle.vertices[1][2];
										break;
									case stlioE_RepairNormal:
										lpThis->currentTriangle_Attributes->base.surfaceNormal[0] =  (lpThis->currentTriangle_Attributes->base.vertices[1][1]-lpThis->currentTriangle_Attributes->base.vertices[0][1])*(lpThis->currentTriangle_Attributes->base.vertices[2][2]-lpThis->currentTriangle_Attributes->base.vertices[0][2])
																							   -(lpThis->currentTriangle_Attributes->base.vertices[1][2]-lpThis->currentTriangle_Attributes->base.vertices[0][2])*(lpThis->currentTriangle_Attributes->base.vertices[2][1]-lpThis->currentTriangle_Attributes->base.vertices[0][1]);
										lpThis->currentTriangle_Attributes->base.surfaceNormal[1] = -(lpThis->currentTriangle_Attributes->base.vertices[1][0]-lpThis->currentTriangle_Attributes->base.vertices[0][0])*(lpThis->currentTriangle_Attributes->base.vertices[2][2]-lpThis->currentTriangle_Attributes->base.vertices[0][2])
																							   +(lpThis->currentTriangle_Attributes->base.vertices[1][2]-lpThis->currentTriangle_Attributes->base.vertices[0][2])*(lpThis->currentTriangle_Attributes->base.vertices[2][0]-lpThis->currentTriangle_Attributes->base.vertices[0][0]);
										lpThis->currentTriangle_Attributes->base.surfaceNormal[2] =  (lpThis->currentTriangle_Attributes->base.vertices[1][0]-lpThis->currentTriangle_Attributes->base.vertices[0][0])*(lpThis->currentTriangle_Attributes->base.vertices[2][1]-lpThis->currentTriangle_Attributes->base.vertices[0][1])
																							   -(lpThis->currentTriangle_Attributes->base.vertices[1][1]-lpThis->currentTriangle_Attributes->base.vertices[0][1])*(lpThis->currentTriangle_Attributes->base.vertices[2][0]-lpThis->currentTriangle_Attributes->base.vertices[0][0]);

										dTemp = sqrt(lpThis->currentTriangle_Attributes->base.surfaceNormal[0]*lpThis->currentTriangle_Attributes->base.surfaceNormal[0]+lpThis->currentTriangle_Attributes->base.surfaceNormal[1]*lpThis->currentTriangle_Attributes->base.surfaceNormal[1]+lpThis->currentTriangle_Attributes->base.surfaceNormal[2]*lpThis->currentTriangle_Attributes->base.surfaceNormal[2]);

										lpThis->currentTriangle_Attributes->base.surfaceNormal[0] = lpThis->currentTriangle_Attributes->base.surfaceNormal[0] / dTemp;
										lpThis->currentTriangle_Attributes->base.surfaceNormal[1] = lpThis->currentTriangle_Attributes->base.surfaceNormal[1] / dTemp;
										lpThis->currentTriangle_Attributes->base.surfaceNormal[2] = lpThis->currentTriangle_Attributes->base.surfaceNormal[2] / dTemp;
										break;
									case stlioE_Keep:
										break;
									default:
										lpThis->readSection = STLParser_Binary__ReadSection_Error;
										return stlioE_InvalidFormat_VertexNormalMismatch;
								}
							}
						} else {
							/* No error callback so we will abort */
							lpThis->readSection = STLParser_Binary__ReadSection_Error;
							return stlioE_InvalidFormat_VertexNormalMismatch;
						}
					}
				#endif

				/* Pass to application */
				if(lpThis->callbackTriangle != NULL) {
					if((e = lpThis->callbackTriangle((struct stlTriangle*)(lpThis->currentTriangle_Attributes), lpThis->callbackTriangleParam)) != stlioE_Ok) {
						lpThis->readSection = STLParser_Binary__ReadSection_Error;
						return e;
					}
				}

				free(lpThis->currentTriangle_Attributes);
				lpThis->currentTriangle_Attributes = NULL;
				lpThis->dwReadBytesCurrentElement = 0;
				if((lpThis->dwReadTriangles = lpThis->dwReadTriangles + 1) == lpThis->dwTriangleCount) {
					/* Done reading (EOF) */
					if(lpThis->callbackEOF != NULL) { lpThis->callbackEOF(lpThis->callbackEOFParam); }
					lpThis->readSection = STLParser_Binary__ReadSection_Done;
					return stlioE_Done;
				}
				return stlioE_Ok;
			}
			return stlioE_Ok;
		}
		/* Gather data inside data buffer and deserialzie */
		lpThis->bDataBuffer[lpThis->dwReadBytesCurrentElement] = bData;

		if((lpThis->dwReadBytesCurrentElement = lpThis->dwReadBytesCurrentElement + 1) < 50) { return stlioE_Ok; }

		lpThis->currentTriangle.surfaceNormal[0] = deserializeIEEE754SingleFloat_4Bytes(&(lpThis->bDataBuffer[0*4]));
		lpThis->currentTriangle.surfaceNormal[1] = deserializeIEEE754SingleFloat_4Bytes(&(lpThis->bDataBuffer[1*4]));
		lpThis->currentTriangle.surfaceNormal[2] = deserializeIEEE754SingleFloat_4Bytes(&(lpThis->bDataBuffer[2*4]));

		lpThis->currentTriangle.vertices[0][0] = deserializeIEEE754SingleFloat_4Bytes(&(lpThis->bDataBuffer[3*4]));
		lpThis->currentTriangle.vertices[0][1] = deserializeIEEE754SingleFloat_4Bytes(&(lpThis->bDataBuffer[4*4]));
		lpThis->currentTriangle.vertices[0][2] = deserializeIEEE754SingleFloat_4Bytes(&(lpThis->bDataBuffer[5*4]));

		lpThis->currentTriangle.vertices[1][0] = deserializeIEEE754SingleFloat_4Bytes(&(lpThis->bDataBuffer[6*4]));
		lpThis->currentTriangle.vertices[1][1] = deserializeIEEE754SingleFloat_4Bytes(&(lpThis->bDataBuffer[7*4]));
		lpThis->currentTriangle.vertices[1][2] = deserializeIEEE754SingleFloat_4Bytes(&(lpThis->bDataBuffer[8*4]));

		lpThis->currentTriangle.vertices[2][0] = deserializeIEEE754SingleFloat_4Bytes(&(lpThis->bDataBuffer[9*4]));
		lpThis->currentTriangle.vertices[2][1] = deserializeIEEE754SingleFloat_4Bytes(&(lpThis->bDataBuffer[10*4]));
		lpThis->currentTriangle.vertices[2][2] = deserializeIEEE754SingleFloat_4Bytes(&(lpThis->bDataBuffer[11*4]));

		lpThis->currentTriangle.attributeByteCount =  ((((unsigned long int)(lpThis->bDataBuffer[12*4+0]))) & 0xFF) | ((((unsigned long int)(lpThis->bDataBuffer[12*4+1])) << 8) & 0xFF00);

		if(lpThis->currentTriangle.attributeByteCount == 0) {
			/* Done Triangle */
			/* Validate */
			#ifdef STLIO_VERTEX_NORMAL_VALIDATION_ENABLED
				e = stlParserBinary__ValidateTriangle(lpThis, &(lpThis->currentTriangle));
				if(e != stlioE_Ok) {
					if(lpThis->callbackError != NULL) {
						if(e == stlioE_InvalidFormat_NotATriangle) {
							e = lpThis->callbackError(stlioE_InvalidFormat_NotATriangle, 0, 0, lpThis->dwFileOffset-1, lpThis->callbackErrorParam);
							if(e != stlioE_Keep) {
								lpThis->readSection = STLParser_Binary__ReadSection_Error;
								return stlioE_InvalidFormat_NotATriangle;
							}
						} else {
							switch(e = lpThis->callbackError(stlioE_InvalidFormat_VertexNormalMismatch, 0, 0, lpThis->dwFileOffset-1, lpThis->callbackErrorParam)) {
								case stlioE_RepairOrder:
									dvTemp[0] = lpThis->currentTriangle.vertices[2][0];
									dvTemp[1] = lpThis->currentTriangle.vertices[2][1];
									dvTemp[2] = lpThis->currentTriangle.vertices[2][2];

									lpThis->currentTriangle.vertices[2][0] = lpThis->currentTriangle.vertices[1][0];
									lpThis->currentTriangle.vertices[2][1] = lpThis->currentTriangle.vertices[1][1];
									lpThis->currentTriangle.vertices[2][2] = lpThis->currentTriangle.vertices[1][2];

									lpThis->currentTriangle.vertices[1][0] = dvTemp[0];
									lpThis->currentTriangle.vertices[1][1] = dvTemp[1];
									lpThis->currentTriangle.vertices[1][2] = dvTemp[2];
									break;
								case stlioE_RepairNormal:
									lpThis->currentTriangle.surfaceNormal[0] =  (lpThis->currentTriangle.vertices[1][1]-lpThis->currentTriangle.vertices[0][1])*(lpThis->currentTriangle.vertices[2][2]-lpThis->currentTriangle.vertices[0][2])
																						   -(lpThis->currentTriangle.vertices[1][2]-lpThis->currentTriangle.vertices[0][2])*(lpThis->currentTriangle.vertices[2][1]-lpThis->currentTriangle.vertices[0][1]);
									lpThis->currentTriangle.surfaceNormal[1] = -(lpThis->currentTriangle.vertices[1][0]-lpThis->currentTriangle.vertices[0][0])*(lpThis->currentTriangle.vertices[2][2]-lpThis->currentTriangle.vertices[0][2])
																						   +(lpThis->currentTriangle.vertices[1][2]-lpThis->currentTriangle.vertices[0][2])*(lpThis->currentTriangle.vertices[2][0]-lpThis->currentTriangle.vertices[0][0]);
									lpThis->currentTriangle.surfaceNormal[2] =  (lpThis->currentTriangle.vertices[1][0]-lpThis->currentTriangle.vertices[0][0])*(lpThis->currentTriangle.vertices[2][1]-lpThis->currentTriangle.vertices[0][1])
																						   -(lpThis->currentTriangle.vertices[1][1]-lpThis->currentTriangle.vertices[0][1])*(lpThis->currentTriangle.vertices[2][0]-lpThis->currentTriangle.vertices[0][0]);

									dTemp = sqrt(lpThis->currentTriangle.surfaceNormal[0]*lpThis->currentTriangle.surfaceNormal[0]+lpThis->currentTriangle.surfaceNormal[1]*lpThis->currentTriangle.surfaceNormal[1]+lpThis->currentTriangle.surfaceNormal[2]*lpThis->currentTriangle.surfaceNormal[2]);

									lpThis->currentTriangle.surfaceNormal[0] = lpThis->currentTriangle.surfaceNormal[0] / dTemp;
									lpThis->currentTriangle.surfaceNormal[1] = lpThis->currentTriangle.surfaceNormal[1] / dTemp;
									lpThis->currentTriangle.surfaceNormal[2] = lpThis->currentTriangle.surfaceNormal[2] / dTemp;
									break;
								case stlioE_Keep:
									break;
								default:
									lpThis->readSection = STLParser_Binary__ReadSection_Error;
									return stlioE_InvalidFormat_VertexNormalMismatch;
							}
						}
					} else {
						/* No error callback so we will abort */
						lpThis->readSection = STLParser_Binary__ReadSection_Error;
						return stlioE_InvalidFormat_VertexNormalMismatch;
					}
				}
			#endif

			/* Pass to application */
			if(lpThis->callbackTriangle != NULL) {
				if((e = lpThis->callbackTriangle(&(lpThis->currentTriangle), lpThis->callbackTriangleParam)) != stlioE_Ok) {
					lpThis->readSection = STLParser_Binary__ReadSection_Error;
					return e;
				}
			}

			lpThis->dwReadBytesCurrentElement = 0;
			if((lpThis->dwReadTriangles = lpThis->dwReadTriangles + 1) == lpThis->dwTriangleCount) {
				/* Done reading (EOF) */
				if(lpThis->callbackEOF != NULL) { lpThis->callbackEOF(lpThis->callbackEOFParam); }
				lpThis->readSection = STLParser_Binary__ReadSection_Done;
				return stlioE_Done;
			}
			return stlioE_Ok;
		}

		/* Else continue reading attribute bytes */
		for(;;) {
			if((lpThis->currentTriangle_Attributes = (struct stlTriangle_Attributes*)malloc(sizeof(struct stlTriangle_Attributes)+lpThis->currentTriangle.attributeByteCount)) == NULL) {
				if(lpThis->callbackError != NULL) {
					if((e = lpThis->callbackError(stlioE_OutOfMemory, 0, 0, lpThis->dwFileOffset-1, lpThis->callbackErrorParam)) != stlioE_Ok) {
						lpThis->readSection = STLParser_Binary__ReadSection_Error;
						return stlioE_OutOfMemory;
					}
					continue; /* Retry allocation */
				} else {
					lpThis->readSection = STLParser_Binary__ReadSection_Error;
					return stlioE_OutOfMemory;
				}
			}
			break;
		}
		memcpy((void*)(lpThis->currentTriangle_Attributes), (void*)(&(lpThis->currentTriangle)), sizeof(lpThis->currentTriangle));
		return stlioE_Ok;
	} else if(lpThis->readSection == STLParser_Binary__ReadSection_Done) {
		return stlioE_InvalidState;
	} else if(lpThis->readSection == STLParser_Binary__ReadSection_Error) {
		return stlioE_InvalidState;
	} else {
		return stlioE_ImplementationError;
	}
}
static enum stlioError stlParserBinary__STLParser_ProcessBytes(
	struct STLParser* lpSelf,
	const unsigned char* lpData,
	unsigned long int dwBlockSize,
	unsigned long int* lpBytesProcessedOut
) {
	enum stlioError e;
	unsigned long int dwProcessed;

	/* Input data validation */
	if(lpSelf == NULL) { return stlioE_InvalidParam; }
	if(lpData == NULL) { return stlioE_InvalidParam; }
	if(lpBytesProcessedOut != NULL) { (*lpBytesProcessedOut) = 0; }

	dwProcessed = dwBlockSize;
	while(dwProcessed > 0) {
		e = stlParserBinary__STLParser_ProcessByte(lpSelf, lpData[dwBlockSize-dwProcessed]);
		if(e != stlioE_Ok) {
			if(lpBytesProcessedOut != NULL) { (*lpBytesProcessedOut) = dwBlockSize-dwProcessed; }
			return e;
		}
		dwProcessed = dwProcessed - 1;
	}
	if(lpBytesProcessedOut != NULL) { (*lpBytesProcessedOut) = dwBlockSize-dwProcessed; }
	return stlioE_Ok;
}
static enum stlioError stlParserBinary__STLParser_ProcessMappedFile(
	struct STLParser* lpSelf,
	unsigned char* lpData,
	unsigned long int dwDataLength
) {
	/*
		Currently we iterate bytewise but of course this is not required because
		we can simply jump from one position to the other without having to
		restart (no state machine required any more) ...

		TODO: Write a more efficient implementation if required anytime in
			the future.
	*/
	return stlParserBinary__STLParser_ProcessBytes(lpSelf, lpData, dwDataLength, NULL);
}

static enum stlioError stlParserBinary__STLParser_SetCallback_Triangle(
	struct STLParser* lpSelf,
	lpfnSTLCallback_Triangle callbackTriangle,
	void* lpFreeParam
) {
	struct STLParser_Binary* lpThis;

	/* Input data validation */
	if(lpSelf == NULL) { return stlioE_InvalidParam; }

	/* Recover this pointer */
	lpThis = (struct STLParser_Binary*)(lpSelf->lpReserved);

	lpThis->callbackTriangle = callbackTriangle;
	lpThis->callbackTriangleParam = lpFreeParam;
	return stlioE_Ok;
}
static enum stlioError stlParserBinary__STLParser_SetCallback_Error(
	struct STLParser* lpSelf,
	lpfnSTLCallback_Error callbackError,
	void* lpFreeParam
) {
	struct STLParser_Binary* lpThis;

	/* Input data validation */
	if(lpSelf == NULL) { return stlioE_InvalidParam; }

	/* Recover this pointer */
	lpThis = (struct STLParser_Binary*)(lpSelf->lpReserved);

	lpThis->callbackError = callbackError;
	lpThis->callbackErrorParam = lpFreeParam;
	return stlioE_Ok;
}
static enum stlioError stlParserBinary__STLParser_SetCallback_EOF(
	struct STLParser* lpSelf,
	lpfnSTLCallback_EOF callbackEOF,
	void* lpFreeParam
) {
	struct STLParser_Binary* lpThis;

	/* Input data validation */
	if(lpSelf == NULL) { return stlioE_InvalidParam; }

	/* Recover this pointer */
	lpThis = (struct STLParser_Binary*)(lpSelf->lpReserved);

	lpThis->callbackEOF = callbackEOF;
	lpThis->callbackEOFParam = lpFreeParam;
	return stlioE_Ok;
}

static enum stlioError stlParserBinary__STLParser_Release(
	struct STLParser* lpSelf
) {
	struct STLParser_Binary* lpThis;

	/* Input data validation */
	if(lpSelf == NULL) { return stlioE_InvalidParam; }

	/* Recover this pointer */
	lpThis = (struct STLParser_Binary*)(lpSelf->lpReserved);

	if(lpThis->currentTriangle_Attributes != NULL) {
		free(lpThis->currentTriangle_Attributes);
		lpThis->currentTriangle_Attributes = NULL;
	}
	free(lpThis);

	return stlioE_Ok;
}

static struct STLParser_Vtbl STLparser_Binary_vtbl = {
	&stlParserBinary__STLParser_Release,

	&stlParserBinary__STLParser_ProcessByte,
	&stlParserBinary__STLParser_ProcessBytes,
	&stlParserBinary__STLParser_ProcessMappedFile,

	&stlParserBinary__STLParser_SetCallback_Triangle,
	&stlParserBinary__STLParser_SetCallback_Error,
	&stlParserBinary__STLParser_SetCallback_EOF
};

/* Factory */
#ifdef __cplusplus
	extern "C" {
#endif

enum stlioError stlioParserBinary(struct STLParser** lpOut) {
	struct STLParser_Binary* lpNew;

	if(lpOut == NULL) {
		return stlioE_InvalidParam;
	}
	(*lpOut) = NULL;

	/* Allocate */
	if((lpNew = (struct STLParser_Binary*)malloc(sizeof(struct STLParser_Binary))) == NULL) { return stlioE_OutOfMemory; }

	lpNew->base.vtbl 					= &STLparser_Binary_vtbl;
	lpNew->base.lpReserved 				= (void*)lpNew;

	lpNew->dwFlags 						= 0;
	lpNew->readSection 					= STLParser_Binary__ReadSection_Header;
	lpNew->dwReadTriangles 				= 0;
	lpNew->dwErrorCount 				= 0;
	lpNew->dwFileOffset 				= 0;
	lpNew->dwReadBytesCurrentElement 	= 0;
	lpNew->dwTriangleCount 				= 0;
	lpNew->currentTriangle_Attributes 	= NULL;
	lpNew->callbackTriangle 			= NULL; lpNew->callbackTriangleParam 	= NULL;
	lpNew->callbackError 				= NULL; lpNew->callbackErrorParam 		= NULL;
	lpNew->callbackEOF 					= NULL; lpNew->callbackEOFParam 			= NULL;

	(*lpOut) = (struct STLParser*)lpNew;
	return stlioE_Ok;
}

#ifdef __cplusplus
	} /* extern "C" { */
#endif

/*
	ASCII STL Parser
	================
*/

enum STLParser_ASCII__TokenizerState {
	STLParser_ASCII__Tokenizer__SkipSpace,	/* The last element was not significant */
	STLParser_ASCII__Tokenizer__ReadLine,	/* Will be used to read the whole remaining line into the buffer (for solid name) */
	STLParser_ASCII__Tokenizer__SkipLine,	/* Will be used to skip the whole remaining line (for solid name). Will lead to an empty token on end of line! */
	STLParser_ASCII__Tokenizer__ReadData,	/* We already started to read a token, the next whitespace terminates the token */
	STLParser_ASCII__Tokenizer__Done,
	STLParser_ASCII__Tokenizer__Error
};

enum STLParser_ASCII__EOLStyle {
	STLParser_ASCII__EOLStyle__Undecided,
	STLParser_ASCII__EOLStyle__CR,
	STLParser_ASCII__EOLStyle__LF
};

enum STLParser_ASCII__ParserState {
	STLParser_ASCII__ParserState__Solid,
	STLParser_ASCII__ParserState__SolidName,
	STLParser_ASCII__ParserState__Facet,
	STLParser_ASCII__ParserState__Normal,
	STLParser_ASCII__ParserState__N,
	STLParser_ASCII__ParserState__Outer,
	STLParser_ASCII__ParserState__Loop,
	STLParser_ASCII__ParserState__Vertex,
	STLParser_ASCII__ParserState__V,
	STLParser_ASCII__ParserState__EndLoop,
	STLParser_ASCII__ParserState__EndFacet,
	STLParser_ASCII__ParserState__EndOrFacet,
	STLParser_ASCII__ParserState__EndsolidName,
	STLParser_ASCII__ParserState__Done,
	STLParser_ASCII__ParserState__Error
};

struct STLParser_ASCII {
	struct STLParser							base;

	/* Tokenizer state */
	enum STLParser_ASCII__TokenizerState		stateToken;
	unsigned char								bTokenBuffer[STLIO_ASCII_PARSER__TOKEN_BUFFER_SIZE];
	unsigned long int							dwBufferedChars;
	unsigned long int							dwLineNumber;
	unsigned long int							dwColNumber;
	enum STLParser_ASCII__EOLStyle				eolStyle;

	/* Parser state */
	enum STLParser_ASCII__ParserState			stateParser;
	unsigned long int							dwReadFloats;		/* { 0,1,2 } Normals, {3,4,5} {6,7,8} {9,10,11} Vertices */
	struct stlTriangle							currentTriangle;	/* They are NEVER attributed! */
	#ifdef STLIO_VALIDATION_SOLIDNAME_MATCH_ENABLED
		unsigned char							bSolidNameBuffer[STLIO_ASCII_PARSER__TOKEN_BUFFER_SIZE+1];
	#endif

	/* Callbacks */
	lpfnSTLCallback_Triangle					callbackTriangle;
	lpfnSTLCallback_Error						callbackError;
	lpfnSTLCallback_EOF							callbackEOF;
	void*										callbackTriangleParam;
	void*										callbackErrorParam;
	void*										callbackEOFParam;
};

static enum stlioError stlParserASCII__STLParser_ProcessByte(
	struct STLParser* lpSelf,
	unsigned char bData
) {
	enum stlioError e;
	struct STLParser_ASCII* lpThis;
	float fTemp;
	#ifdef STLIO_VERTEX_NORMAL_VALIDATION_ENABLED
		double dCalculatedNormal[3];	double dCalculatedNormalLen;
		double dPassedNormal[3];		double dPassedNormalLen;
		double dTemp;
		double dvTemp[3];
	#endif
	#ifdef STLIO_VALIDATION_SOLIDNAME_MATCH_ENABLED
		unsigned long int i;
	#endif

	if(lpSelf == NULL) { return stlioE_InvalidParam; }
	lpThis = (struct STLParser_ASCII*)(lpSelf->lpReserved);

	/* Check that this is ANY of the allowed characters */
	if((bData & 0x80) != 0) {
		/* Call error callback here ... no recovery solution! */
		if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_InvalidCharacter, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
		return stlioE_InvalidFormat_InvalidCharacter;
	}
	for(;;) {
		/* One check per allowed class */
		if((bData >= 0x61) && (bData <= 0x7A)) { break; } /* 'a'-'z' */
		if((bData >= 0x41) && (bData <= 0x5A)) { break; } /* 'A'-'Z' */
		if((bData >= 0x30) && (bData <= 0x39)) { break; } /* '0'-'9' */
		if(
			(bData == '.') ||
			(bData == '-') ||
			(bData == '+') ||
			(bData == '_') ||
			(bData == 0x20) || /* Space */
			(bData == 0x0A) || /* Line Feed */
			(bData == 0x0D) /* Carriage Return */
		) { break; }

		/* Call error callback here ... no recovery solution! */
		if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_InvalidCharacter, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
		return stlioE_InvalidFormat_InvalidCharacter;
	}

	/* Tokenizer ... */
	lpThis->dwColNumber = lpThis->dwColNumber + 1;
	if((bData == 0x0A) || (bData == 0x0D)) {
		if(lpThis->eolStyle == STLParser_ASCII__EOLStyle__Undecided) {
			lpThis->eolStyle = (bData == 0x0A) ? STLParser_ASCII__EOLStyle__LF : STLParser_ASCII__EOLStyle__CR;
			lpThis->dwColNumber = 0;
			lpThis->dwLineNumber = lpThis->dwLineNumber + 1;
		} else if((lpThis->eolStyle == STLParser_ASCII__EOLStyle__LF) && (bData == 0x0A)) {
			lpThis->dwColNumber = 0;
			lpThis->dwLineNumber = lpThis->dwLineNumber + 1;
		} else if((lpThis->eolStyle == STLParser_ASCII__EOLStyle__CR) && (bData == 0x0D)) {
			lpThis->dwColNumber = 0;
			lpThis->dwLineNumber = lpThis->dwLineNumber + 1;
		} else {
			lpThis->dwColNumber = lpThis->dwColNumber - 1;
			return stlioE_Ok; /* We ignore different line ending symbols than the first that we have encountered */
		}
	}

	switch(lpThis->stateToken) {
		case STLParser_ASCII__Tokenizer__SkipSpace:
			if((bData == 0x0A) || (bData == 0x0D) || (bData == 0x20)) { return stlioE_Ok; } /* We ignore whitespace */
			/* If our data is NOT whitespace we start reading */
			lpThis->dwBufferedChars = 1;
			lpThis->bTokenBuffer[0] = bData;
			lpThis->stateToken = STLParser_ASCII__Tokenizer__ReadData;
			return stlioE_Ok;
		case STLParser_ASCII__Tokenizer__ReadLine:
			if(!(((bData == 0x0A) && (lpThis->eolStyle == STLParser_ASCII__EOLStyle__LF)) || ((bData == 0x0D) && (lpThis->eolStyle == STLParser_ASCII__EOLStyle__CR)))) {
				/* We collect that data */
				if(lpThis->dwBufferedChars >= STLIO_ASCII_PARSER__TOKEN_BUFFER_SIZE) {
					if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_TokenTooLong, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
					return stlioE_InvalidFormat_TokenTooLong;
				}
				lpThis->bTokenBuffer[lpThis->dwBufferedChars] = bData;
				lpThis->dwBufferedChars = lpThis->dwBufferedChars + 1;
				return stlioE_Ok;
			}
			/*
				If we reach here we have reached the end of the line - and got a token (we will leave this switch into the parser state machine).

				On the next line we will start reading another token (the parser will reset our state machine!)
			*/
			break;
		case STLParser_ASCII__Tokenizer__SkipLine:
			if(!(((bData == 0x0A) && (lpThis->eolStyle == STLParser_ASCII__EOLStyle__LF)) || ((bData == 0x0D) && (lpThis->eolStyle == STLParser_ASCII__EOLStyle__CR)))) {
				/* We will simply ignore everything here ... */
				return stlioE_Ok;
			}
			/*
				We have finished an empty token ...
				On the next line we will start reading another token (the parser will reset our state machine!)
			*/
			lpThis->dwBufferedChars = 0;
			break;
		case STLParser_ASCII__Tokenizer__ReadData:
			if(!(((bData == 0x0A) && (lpThis->eolStyle == STLParser_ASCII__EOLStyle__LF)) || ((bData == 0x0D) && (lpThis->eolStyle == STLParser_ASCII__EOLStyle__CR)) || (bData == 0x0A) || (bData == 0x0D) || (bData == 0x20))) {
				/* We collect that data */
				if(lpThis->dwBufferedChars >= STLIO_ASCII_PARSER__TOKEN_BUFFER_SIZE) {
					if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_TokenTooLong, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
					return stlioE_InvalidFormat_TokenTooLong;
				}
				lpThis->bTokenBuffer[lpThis->dwBufferedChars] = bData;
				lpThis->dwBufferedChars = lpThis->dwBufferedChars + 1;
				return stlioE_Ok;
			}
			if(((bData == 0x0A) && (lpThis->eolStyle == STLParser_ASCII__EOLStyle__LF)) || ((bData == 0x0D) && (lpThis->eolStyle == STLParser_ASCII__EOLStyle__CR)) || (bData == 0x20)) {
				break; /* End of token */
			}
			return stlioE_Ok; /* An EOL type we ignore ... */
		case STLParser_ASCII__Tokenizer__Done:
			return stlioE_InvalidState;
		case STLParser_ASCII__Tokenizer__Error:
			return stlioE_InvalidState;
		default:
			return stlioE_ImplementationError;
	}

	/*
		If we reached here the tokenizer has collected an token. Dependent on our state
		we parse that token
	*/
	switch(lpThis->stateParser) {
		case STLParser_ASCII__ParserState__Solid:
			if(lpThis->dwBufferedChars != strSolidLEN) {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_Solid, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_Solid;
			}
			if(strncmp((char*)(lpThis->bTokenBuffer), (char*)strSolid, strSolidLEN) != 0) {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_Solid, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_Solid;
			}
			lpThis->stateParser = STLParser_ASCII__ParserState__SolidName;
			#ifdef STLIO_VALIDATION_SOLIDNAME_MATCH_ENABLED
				lpThis->stateToken = STLParser_ASCII__Tokenizer__ReadLine;
			#else
				lpThis->stateToken = STLParser_ASCII__Tokenizer__SkipLine;
			#endif
			lpThis->dwBufferedChars = 0;
			return stlioE_Ok;
		case STLParser_ASCII__ParserState__SolidName:
			/* If we use validation we keep a copy - else ignore that name */
			#ifdef STLIO_VALIDATION_SOLIDNAME_MATCH_ENABLED
				for(i = 0; i < lpThis->dwBufferedChars; i=i+1) {
					lpThis->bSolidNameBuffer[i] = lpThis->bTokenBuffer[i];
				}
				lpThis->bSolidNameBuffer[i] = 0x00;
			#endif
			lpThis->stateParser = STLParser_ASCII__ParserState__Facet;
			break; /* We return into skip space mode ... */
		case STLParser_ASCII__ParserState__Facet:
			if(lpThis->dwBufferedChars != strFacetLEN) {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_Facet, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_Facet;
			}
			if(strncmp((char*)(lpThis->bTokenBuffer), (char*)strFacet, strFacetLEN) != 0) {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_Facet, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_Facet;
			}
			lpThis->stateParser = STLParser_ASCII__ParserState__Normal;
			break; /* We return into skip space mode ... */
		case STLParser_ASCII__ParserState__Normal:
			if(lpThis->dwBufferedChars != strNormalLEN) {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_Normal, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_Normal;
			}
			if(strncmp((char*)(lpThis->bTokenBuffer), (char*)strNormal, strNormalLEN) != 0) {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_Normal, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_Normal;
			}
			lpThis->dwReadFloats = 0;
			lpThis->stateParser = STLParser_ASCII__ParserState__N;
			break; /* We return into skip space mode ... */
		case STLParser_ASCII__ParserState__N:
			/* Parse the potentially read float ... */
			e = fltReadASCII(lpThis->bTokenBuffer, lpThis->dwBufferedChars, &fTemp);
			if(e != stlioE_Ok) {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_FloatNumber, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_FloatNumber;
			}
			lpThis->currentTriangle.surfaceNormal[lpThis->dwReadFloats] = fTemp;
			if((lpThis->dwReadFloats = lpThis->dwReadFloats + 1) == 3) {
				/* Done reading all normals ... */
				lpThis->stateParser = STLParser_ASCII__ParserState__Outer;
			}
			break; /* return into skip space mode and read next float .... */

		case STLParser_ASCII__ParserState__Outer:
			if(lpThis->dwBufferedChars != strOuterLEN) {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_Outer, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_Outer;
			}
			if(strncmp((char*)(lpThis->bTokenBuffer), (char*)strOuter, strOuterLEN) != 0) {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_Outer, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_Outer;
			}
			lpThis->stateParser = STLParser_ASCII__ParserState__Loop;
			break; /* We return into skip space mode ... */
		case STLParser_ASCII__ParserState__Loop:
			if(lpThis->dwBufferedChars != strLoopLEN) {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_Loop, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_Loop;
			}
			if(strncmp((char*)(lpThis->bTokenBuffer), (char*)strLoop, strLoopLEN) != 0) {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_Loop, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_Loop;
			}
			lpThis->stateParser = STLParser_ASCII__ParserState__Vertex;
			break; /* We return into skip space mode ... */
		case STLParser_ASCII__ParserState__Vertex:
			if(lpThis->dwBufferedChars != strVertexLEN) {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_Vertex, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_Vertex;
			}
			if(strncmp((char*)(lpThis->bTokenBuffer), (char*)strVertex, strVertexLEN) != 0) {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_Vertex, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_Vertex;
			}
			lpThis->stateParser = STLParser_ASCII__ParserState__V;
			break; /* We return into skip space mode ... */

		case STLParser_ASCII__ParserState__V:
			/* Parse the potentially read float ... */
			e = fltReadASCII(lpThis->bTokenBuffer, lpThis->dwBufferedChars, &fTemp);
			if(e != stlioE_Ok) {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_FloatNumber, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_FloatNumber;
			}
			lpThis->currentTriangle.vertices[(lpThis->dwReadFloats / 3)-1][lpThis->dwReadFloats % 3] = fTemp;
			if((lpThis->dwReadFloats = lpThis->dwReadFloats + 1) == 4*3) {
				/* Done reading ALL vertices ... */
				lpThis->stateParser = STLParser_ASCII__ParserState__EndLoop;
			} else if((lpThis->dwReadFloats % 3) == 0) {
				/* Done reading THESE vertices floats */
				lpThis->stateParser = STLParser_ASCII__ParserState__Vertex;
			}
			break; /* return into skip space mode and read next float .... */
		case STLParser_ASCII__ParserState__EndLoop:
			if(lpThis->dwBufferedChars != strEndloopLEN) {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_Endloop, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_Endloop;
			}
			if(strncmp((char*)(lpThis->bTokenBuffer), (char*)strEndloop, strEndloopLEN) != 0) {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_Endloop, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_Endloop;
			}
			lpThis->stateParser = STLParser_ASCII__ParserState__EndFacet;
			break; /* We return into skip space mode ... */
		case STLParser_ASCII__ParserState__EndFacet:
			if(lpThis->dwBufferedChars != strEndfacetLEN) {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_Endfacet, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_Endfacet;
			}
			if(strncmp((char*)(lpThis->bTokenBuffer), (char*)strEndfacet, strEndfacetLEN) != 0) {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_Endfacet, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_Endfacet;
			}

			/* Validate triangle */
			#ifdef STLIO_VERTEX_NORMAL_VALIDATION_ENABLED
				/* Cross product */
				dCalculatedNormal[0] = ( lpThis->currentTriangle.vertices[1][1]-lpThis->currentTriangle.vertices[0][1])*(lpThis->currentTriangle.vertices[2][2]-lpThis->currentTriangle.vertices[0][2])-(lpThis->currentTriangle.vertices[1][2]-lpThis->currentTriangle.vertices[0][2])*(lpThis->currentTriangle.vertices[2][1]-lpThis->currentTriangle.vertices[0][1]);
				dCalculatedNormal[1] = -(lpThis->currentTriangle.vertices[1][0]-lpThis->currentTriangle.vertices[0][0])*(lpThis->currentTriangle.vertices[2][2]-lpThis->currentTriangle.vertices[0][2])+(lpThis->currentTriangle.vertices[1][2]-lpThis->currentTriangle.vertices[0][2])*(lpThis->currentTriangle.vertices[2][0]-lpThis->currentTriangle.vertices[0][0]);
				dCalculatedNormal[2] = ( lpThis->currentTriangle.vertices[1][0]-lpThis->currentTriangle.vertices[0][0])*(lpThis->currentTriangle.vertices[2][1]-lpThis->currentTriangle.vertices[0][1])-(lpThis->currentTriangle.vertices[1][1]-lpThis->currentTriangle.vertices[0][1])*(lpThis->currentTriangle.vertices[2][0]-lpThis->currentTriangle.vertices[0][0]);

				/* Check for denatured triangle */
				if((dCalculatedNormal[0] == 0.0) && (dCalculatedNormal[1] == 0.0) && (dCalculatedNormal[2] == 0.0)) {
					if(lpThis->callbackError != NULL) {
						e = lpThis->callbackError(stlioE_InvalidFormat_NotATriangle, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam);
						if(e == stlioE_Keep) {
							/* We do nothing ... */
						} else if(e == stlioE_Skip) {
							lpThis->stateParser = STLParser_ASCII__ParserState__EndOrFacet;
							break; /* We return into skip space mode ... */
						} else {
							/* Abort */
							lpThis->stateParser = STLParser_ASCII__ParserState__Error;
							lpThis->stateToken = STLParser_ASCII__Tokenizer__Error;
							return stlioE_InvalidFormat_NotATriangle;
						}
					} else {
						lpThis->stateParser = STLParser_ASCII__ParserState__Error;
						lpThis->stateToken = STLParser_ASCII__Tokenizer__Error;
						return stlioE_InvalidFormat_NotATriangle;
					}
				} else {
					/* Normalize */
					dCalculatedNormalLen = sqrt(dCalculatedNormal[0]*dCalculatedNormal[0]+dCalculatedNormal[1]*dCalculatedNormal[1]+dCalculatedNormal[2]*dCalculatedNormal[2]);
					dCalculatedNormal[0] = dCalculatedNormal[0] / dCalculatedNormalLen;
					dCalculatedNormal[1] = dCalculatedNormal[1] / dCalculatedNormalLen;
					dCalculatedNormal[2] = dCalculatedNormal[2] / dCalculatedNormalLen;

					dPassedNormal[0] = lpThis->currentTriangle.surfaceNormal[0];
					dPassedNormal[1] = lpThis->currentTriangle.surfaceNormal[1];
					dPassedNormal[2] = lpThis->currentTriangle.surfaceNormal[2];

					dPassedNormalLen = sqrt(dPassedNormal[0]*dPassedNormal[0]+dPassedNormal[1]*dPassedNormal[1]+dPassedNormal[2]*dPassedNormal[2]);
					dPassedNormal[0] = dPassedNormal[0] / dPassedNormalLen;
					dPassedNormal[1] = dPassedNormal[1] / dPassedNormalLen;
					dPassedNormal[2] = dPassedNormal[2] / dPassedNormalLen;
					/* Check they are equal */
					if(!(fltEquals(dPassedNormal[0], dCalculatedNormal[0]) && fltEquals(dPassedNormal[1], dCalculatedNormal[1]) && fltEquals(dPassedNormal[2], dCalculatedNormal[2]))) {
						if(lpThis->callbackError != NULL) {
							e = lpThis->callbackError(stlioE_InvalidFormat_VertexNormalMismatch, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam);
							if(e == stlioE_Keep) {
								/* We do nothing ... */
							} else if(e == stlioE_Skip) {
								lpThis->stateParser = STLParser_ASCII__ParserState__EndOrFacet;
								break; /* We return into skip space mode ... */
							} else if(e == stlioE_RepairOrder) {
								dvTemp[0] = lpThis->currentTriangle.vertices[2][0];
								dvTemp[1] = lpThis->currentTriangle.vertices[2][1];
								dvTemp[2] = lpThis->currentTriangle.vertices[2][2];

								lpThis->currentTriangle.vertices[2][0] = lpThis->currentTriangle.vertices[1][0];
								lpThis->currentTriangle.vertices[2][1] = lpThis->currentTriangle.vertices[1][1];
								lpThis->currentTriangle.vertices[2][2] = lpThis->currentTriangle.vertices[1][2];

								lpThis->currentTriangle.vertices[1][0] = dvTemp[0];
								lpThis->currentTriangle.vertices[1][1] = dvTemp[1];
								lpThis->currentTriangle.vertices[1][2] = dvTemp[2];
							} else if(e == stlioE_RepairNormal) {
								lpThis->currentTriangle.surfaceNormal[0] =  (lpThis->currentTriangle.vertices[1][1]-lpThis->currentTriangle.vertices[0][1])*(lpThis->currentTriangle.vertices[2][2]-lpThis->currentTriangle.vertices[0][2])
													   					   -(lpThis->currentTriangle.vertices[1][2]-lpThis->currentTriangle.vertices[0][2])*(lpThis->currentTriangle.vertices[2][1]-lpThis->currentTriangle.vertices[0][1]);
								lpThis->currentTriangle.surfaceNormal[1] = -(lpThis->currentTriangle.vertices[1][0]-lpThis->currentTriangle.vertices[0][0])*(lpThis->currentTriangle.vertices[2][2]-lpThis->currentTriangle.vertices[0][2])
																		   +(lpThis->currentTriangle.vertices[1][2]-lpThis->currentTriangle.vertices[0][2])*(lpThis->currentTriangle.vertices[2][0]-lpThis->currentTriangle.vertices[0][0]);
								lpThis->currentTriangle.surfaceNormal[2] =  (lpThis->currentTriangle.vertices[1][0]-lpThis->currentTriangle.vertices[0][0])*(lpThis->currentTriangle.vertices[2][1]-lpThis->currentTriangle.vertices[0][1])
																		   -(lpThis->currentTriangle.vertices[1][1]-lpThis->currentTriangle.vertices[0][1])*(lpThis->currentTriangle.vertices[2][0]-lpThis->currentTriangle.vertices[0][0]);

								dTemp = sqrt(lpThis->currentTriangle.surfaceNormal[0]*lpThis->currentTriangle.surfaceNormal[0]+lpThis->currentTriangle.surfaceNormal[1]*lpThis->currentTriangle.surfaceNormal[1]+lpThis->currentTriangle.surfaceNormal[2]*lpThis->currentTriangle.surfaceNormal[2]);

								lpThis->currentTriangle.surfaceNormal[0] = lpThis->currentTriangle.surfaceNormal[0] / dTemp;
								lpThis->currentTriangle.surfaceNormal[1] = lpThis->currentTriangle.surfaceNormal[1] / dTemp;
								lpThis->currentTriangle.surfaceNormal[2] = lpThis->currentTriangle.surfaceNormal[2] / dTemp;
							} else {
								/* Abort */
								lpThis->stateParser = STLParser_ASCII__ParserState__Error;
								lpThis->stateToken = STLParser_ASCII__Tokenizer__Error;
								return stlioE_InvalidFormat_VertexNormalMismatch;
							}
						} else {
							lpThis->stateParser = STLParser_ASCII__ParserState__Error;
							lpThis->stateToken = STLParser_ASCII__Tokenizer__Error;
							return stlioE_InvalidFormat_VertexNormalMismatch;
						}
					}
				}
			#endif

			/* Pass triangle to application */
			lpThis->currentTriangle.attributeByteCount = 0; /* Always */
			if(lpThis->callbackTriangle != NULL) {
				if((e = lpThis->callbackTriangle(&(lpThis->currentTriangle), lpThis->callbackTriangleParam)) != stlioE_Ok) {
					lpThis->stateParser = STLParser_ASCII__ParserState__Error;
					lpThis->stateToken = STLParser_ASCII__Tokenizer__Error;
					return e;
				}
			}

			lpThis->stateParser = STLParser_ASCII__ParserState__EndOrFacet;
			break; /* We return into skip space mode ... */
		case STLParser_ASCII__ParserState__EndOrFacet:
			if(lpThis->dwBufferedChars == strFacetLEN) {
				if(strncmp((char*)(lpThis->bTokenBuffer), (char*)strFacet, strFacetLEN) != 0) {
					if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_FacetOrEndloop, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
					return stlioE_InvalidFormat_Expect_FacetOrEndloop;
				}
				lpThis->stateParser = STLParser_ASCII__ParserState__Normal;
				break;
			} else if(lpThis->dwBufferedChars == strEndsolidLEN) {
				if(strncmp((char*)(lpThis->bTokenBuffer), (char*)strEndsolid, strEndsolidLEN) != 0) {
					if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_FacetOrEndloop, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
					return stlioE_InvalidFormat_Expect_FacetOrEndloop;
				}
				#ifdef STLIO_VALIDATION_SOLIDNAME_MATCH_ENABLED
					lpThis->stateToken = STLParser_ASCII__Tokenizer__ReadLine;
				#else
					lpThis->stateToken = STLParser_ASCII__Tokenizer__SkipLine;
				#endif
				lpThis->stateParser = STLParser_ASCII__ParserState__EndsolidName;
				break;
			} else {
				if(lpThis->callbackError != NULL) { lpThis->callbackError(stlioE_InvalidFormat_Expect_FacetOrEndloop, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam); }
				return stlioE_InvalidFormat_Expect_FacetOrEndloop;
			}
		case STLParser_ASCII__ParserState__EndsolidName:
			#ifdef STLIO_VALIDATION_SOLIDNAME_MATCH_ENABLED
				if(lpThis->dwBufferedChars != strlen((char*)(lpThis->bSolidNameBuffer))) {
					if(lpThis->callbackError != NULL) {
						e = lpThis->callbackError(stlioE_InvalidFormat_SolidNameMismatch, lpThis->dwLineNumber, lpThis->dwColNumber, 0, lpThis->callbackErrorParam);
						if((e != stlioE_Skip) && (e != stlioE_Keep)) {
							return stlioE_InvalidFormat_SolidNameMismatch;
						}
					} else {
						return stlioE_InvalidFormat_SolidNameMismatch;
					}
				}
			#endif
			lpThis->stateParser = STLParser_ASCII__ParserState__Done;
			lpThis->stateToken = STLParser_ASCII__Tokenizer__Done;
			return stlioE_Done;
		case STLParser_ASCII__ParserState__Done:
			return stlioE_InvalidState;
		case STLParser_ASCII__ParserState__Error:
			return stlioE_InvalidState;
		default:
			return stlioE_ImplementationError;
	}

	lpThis->dwBufferedChars = 0;
	lpThis->stateToken = STLParser_ASCII__Tokenizer__SkipSpace;
	return stlioE_Ok;
}
static enum stlioError stlParserASCII__STLParser_ProcessBytes(
	struct STLParser* lpSelf,
	const unsigned char* lpData,
	unsigned long int dwBlockSize,
	unsigned long int* lpBytesProcessedOut
) {
	enum stlioError e;
	unsigned long int i;
	/*
		This is an ASCII file. We simply have to process that byte by byte.
		In case of ASCII this is equivalent to parsing character by character
	*/

	if(lpBytesProcessedOut != NULL) { (*lpBytesProcessedOut) = 0; }

	for(i = 0; i < dwBlockSize; i=i+1) {
		e = stlParserASCII__STLParser_ProcessByte(lpSelf, lpData[i]);
		if((e != stlioE_Ok) && (e != stlioE_Done)) { return e; }
		if(e == stlioE_Done) {
			(*lpBytesProcessedOut) = i;
			return e;
		}
	}
	(*lpBytesProcessedOut) = i;
	return stlioE_Ok;
}
static enum stlioError stlParserASCII__STLParser_ProcessMappedFile(
	struct STLParser* lpSelf,
	unsigned char* lpData,
	unsigned long int dwDataLength
) {
	enum stlioError e;
	unsigned long int dwBytesProcessedOut;
	e = stlParserASCII__STLParser_ProcessBytes(lpSelf, lpData, dwDataLength, &dwBytesProcessedOut);
	if(((e == stlioE_Ok) || (e == stlioE_Done)) && (dwDataLength != dwBytesProcessedOut)) { return stlioE_IOError; }
	return e;
}
static enum stlioError stlParserASCII__STLParser_SetCallback_Triangle(
	struct STLParser* lpSelf,
	lpfnSTLCallback_Triangle callbackTriangle,
	void* lpFreeParam
) {
	struct STLParser_ASCII* lpThis;

	if(lpSelf == NULL) { return stlioE_InvalidParam; }
	lpThis = (struct STLParser_ASCII*)(lpSelf->lpReserved);

	lpThis->callbackTriangle = callbackTriangle;
	lpThis->callbackTriangleParam = lpFreeParam;
	return stlioE_Ok;
}
static enum stlioError stlParserASCII__STLParser_SetCallback_Error(
	struct STLParser* lpSelf,
	lpfnSTLCallback_Error callbackError,
	void* lpFreeParam
) {
	struct STLParser_ASCII* lpThis;

	if(lpSelf == NULL) { return stlioE_InvalidParam; }
	lpThis = (struct STLParser_ASCII*)(lpSelf->lpReserved);

	lpThis->callbackError = callbackError;
	lpThis->callbackErrorParam = lpFreeParam;
	return stlioE_Ok;
}
static enum stlioError stlParserASCII__STLParser_SetCallback_EOF(
	struct STLParser* lpSelf,
	lpfnSTLCallback_EOF callbackEOF,
	void* lpFreeParam
) {
	struct STLParser_ASCII* lpThis;

	if(lpSelf == NULL) { return stlioE_InvalidParam; }
	lpThis = (struct STLParser_ASCII*)(lpSelf->lpReserved);

	lpThis->callbackEOF = callbackEOF;
	lpThis->callbackEOFParam = lpFreeParam;
	return stlioE_Ok;
}
static enum stlioError stlParserASCII__STLParser_Release(
	struct STLParser* lpSelf
) {
	if(lpSelf == NULL) { return stlioE_InvalidParam; }

	struct STLParser_ASCII* lpThis;
	lpThis = (struct STLParser_ASCII*)(lpSelf->lpReserved);

	free(lpThis);
	return stlioE_Ok;
}

static struct STLParser_Vtbl STLparser_ASCII_vtbl = {
	&stlParserASCII__STLParser_Release,

	&stlParserASCII__STLParser_ProcessByte,
	&stlParserASCII__STLParser_ProcessBytes,
	&stlParserASCII__STLParser_ProcessMappedFile,

	&stlParserASCII__STLParser_SetCallback_Triangle,
	&stlParserASCII__STLParser_SetCallback_Error,
	&stlParserASCII__STLParser_SetCallback_EOF
};

/* Factory */
#ifdef __cplusplus
	extern "C" {
#endif

enum stlioError stlioParserASCII(struct STLParser** lpOut) {
	struct STLParser_ASCII* lpNew;

	if(lpOut == NULL) { return stlioE_InvalidParam; }
	(*lpOut) = NULL;

	if((lpNew = (struct STLParser_ASCII*)malloc(sizeof(struct STLParser_ASCII))) == NULL) {
		return stlioE_OutOfMemory;
	}

	lpNew->base.lpReserved 			= (void*)lpNew;
	lpNew->base.vtbl 				= &STLparser_ASCII_vtbl;
	lpNew->stateToken 				= STLParser_ASCII__Tokenizer__SkipSpace;
	lpNew->dwBufferedChars 			= 0;
	lpNew->dwLineNumber 			= 0;
	lpNew->dwColNumber 				= 0;
	lpNew->eolStyle 				= STLParser_ASCII__EOLStyle__Undecided;
	lpNew->stateParser 				= STLParser_ASCII__ParserState__Solid;
	lpNew->dwReadFloats 			= 0;
	lpNew->callbackTriangle 		= NULL;
	lpNew->callbackError 			= NULL;
	lpNew->callbackEOF 				= NULL;
	lpNew->callbackTriangleParam 	= NULL;
	lpNew->callbackErrorParam 		= NULL;
	lpNew->callbackEOFParam 		= NULL;

	(*lpOut) = (struct STLParser*)lpNew;
	return stlioE_Ok;
}

#ifdef __cplusplus
	} /* extern "C" { */
#endif

/*
	Binary STL Writer
	=================
*/

static unsigned char stlBinaryHeaderTemplate[80+4] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00 /* Include a zeroed length field. Will be filled during finalize! (using a seek operation) */
};

enum STLWriter_Binary_State {
	STLWriter_Binary_State__Header,			/* Note that this state does NOT include the count of triangles which is updated at the end! */
	STLWriter_Binary_State__Triangle,		/* We accept the next triangle */
	STLWriter_Binary_State__TriangleFlush,	/* We are currently flushing the byte representation of the next triangle */
	STLWriter_Binary_State__FlushCount,		/* We are in the process of writing the "count" bytes into the header after seeking back */

	STLWriter_Binary_State__Done,			/* We have finished writing an STL file */
	STLWriter_Binary_State__Error			/* There has been an unrecoverable error */
};

struct STLWriter_Binary {
	struct STLWriter						base;

	struct STLStreamSink*					lpStreamSink;
	enum stlWriter_NormalMode				normalMode;

	enum STLWriter_Binary_State				state;
	unsigned long int						dwPendingBytes;

	unsigned char							bSerializedDataBuffer[4*3*4+2]; /* Buffer for normal, 3 vertices and attribute length */
	struct stlTriangle_Attributes*			lpAttributedTriangle;
	unsigned long int						dwWrittenTriangles;

	lpfnSTLWriter_Callback_Error			callbackError;
	void*									callbackErrorParam;
};

static enum stlioError stlWriter_Binary__WriteTriangle(
	struct STLWriter* 				lpSelf,
	struct stlTriangle* 			lpTriangle
) {
	enum stlioError e;
	struct STLWriter_Binary *lpThis;
	unsigned long int dwBytesWritten;
	double dCalculatedNormal[3];
	double dCalculatedNormalLen;
	double dPassedNormal[3];
	double dPassedNormalLen;

	if((lpSelf == NULL) || (lpTriangle == NULL)) { return stlioE_InvalidParam; }
	lpThis = (struct STLWriter_Binary*)(lpSelf->lpReserved);

	/* Check if we are currently accepting ... */
	if(lpThis->state != STLWriter_Binary_State__Triangle) { return stlioE_InvalidState; }

	/* Validate triangle */
	switch(lpThis->normalMode) {
		case stlWriter_NormalMode__CheckNormals:
			/* Calclate normals and check they are identical after being normalized */
			dCalculatedNormal[0] = ( lpTriangle->vertices[1][1]-lpTriangle->vertices[0][1])*(lpTriangle->vertices[2][2]-lpTriangle->vertices[0][2])-(lpTriangle->vertices[1][2]-lpTriangle->vertices[0][2])*(lpTriangle->vertices[2][1]-lpTriangle->vertices[0][1]);
			dCalculatedNormal[1] = -(lpTriangle->vertices[1][0]-lpTriangle->vertices[0][0])*(lpTriangle->vertices[2][2]-lpTriangle->vertices[0][2])+(lpTriangle->vertices[1][2]-lpTriangle->vertices[0][2])*(lpTriangle->vertices[2][0]-lpTriangle->vertices[0][0]);
			dCalculatedNormal[2] = ( lpTriangle->vertices[1][0]-lpTriangle->vertices[0][0])*(lpTriangle->vertices[2][1]-lpTriangle->vertices[0][1])-(lpTriangle->vertices[1][1]-lpTriangle->vertices[0][1])*(lpTriangle->vertices[2][0]-lpTriangle->vertices[0][0]);
			if((dCalculatedNormal[0] == 0) && (dCalculatedNormal[1] == 0) && (dCalculatedNormal[2] == 0)) {
				if(lpThis->callbackError == NULL) {
					/* We don't have an error callback registered that can take an decision */
					return stlioE_InvalidFormat_NotATriangle;
				}
				/*
					Check what the application wants to do with denaturated triangles

					stlioE_Keep: Write that denaturated triangle into the file
					stlioE_Skip: Skip that triangle
					anything else: Abort this write
				*/
				e = lpThis->callbackError(stlioE_InvalidFormat_NotATriangle, lpTriangle, lpThis->callbackErrorParam);
				if(e == stlioE_Skip) { return stlioE_Ok; }		/* Do not process THAT triangle */
				if(e == stlioE_Keep) { break; }					/* Simply accept that triangle and use later on ... */
				return e;
			}
			dCalculatedNormalLen = sqrt(dCalculatedNormal[0]*dCalculatedNormal[0]+dCalculatedNormal[1]*dCalculatedNormal[1]+dCalculatedNormal[2]*dCalculatedNormal[2]);
			dCalculatedNormal[0] = dCalculatedNormal[0] / dCalculatedNormalLen;
			dCalculatedNormal[1] = dCalculatedNormal[1] / dCalculatedNormalLen;
			dCalculatedNormal[2] = dCalculatedNormal[2] / dCalculatedNormalLen;

			dPassedNormal[0] = lpTriangle->surfaceNormal[0];
			dPassedNormal[1] = lpTriangle->surfaceNormal[1];
			dPassedNormal[2] = lpTriangle->surfaceNormal[2];

			dPassedNormalLen = sqrt(dPassedNormal[0]*dPassedNormal[0]+dPassedNormal[1]*dPassedNormal[1]+dPassedNormal[2]*dPassedNormal[2]);
			dPassedNormal[0] = dPassedNormal[0] / dPassedNormalLen;
			dPassedNormal[1] = dPassedNormal[1] / dPassedNormalLen;
			dPassedNormal[2] = dPassedNormal[2] / dPassedNormalLen;

			// if((dPassedNormal[0] != dCalculatedNormal[0]) || (dPassedNormal[1] != dCalculatedNormal[1]) || (dPassedNormal[2] != dCalculatedNormal[2])) {
			if(dblEquals(dPassedNormal[0], dCalculatedNormal[0]) && dblEquals(dPassedNormal[1], dCalculatedNormal[1]) && dblEquals(dPassedNormal[2], dCalculatedNormal[2])) {
				if(lpThis->callbackError == NULL) {
					/* We don't have an error callback registered that can take an decision */
					return stlioE_InvalidFormat_VertexNormalMismatch;
				}
				/*
					Check what the application wants to do with denaturated triangles

					stlioE_Keep: Write that mismatched normal into the file
					stlioE_Skip: Skip that triangle
					stlioE_RepairNormal: Repair the normal (use the calculated one)
					anything else: Abort this write
				*/
				e = lpThis->callbackError(stlioE_InvalidFormat_NotATriangle, lpTriangle, lpThis->callbackErrorParam);
				if(e == stlioE_Skip) { return stlioE_Ok; }		/* Do not process THAT triangle */
				if(e == stlioE_Keep) { break; }					/* Simply accept that triangle and use later on ... */
				if(e == stlioE_RepairNormal) {
					lpTriangle->surfaceNormal[0] = dCalculatedNormal[0];
					lpTriangle->surfaceNormal[1] = dCalculatedNormal[1];
					lpTriangle->surfaceNormal[2] = dCalculatedNormal[2];
					break;
				}
				return e;
			}
			break;
		case stlWriter_NormalMode__CalculateNormals:
			/* Simply overwrite the normals contained inside the triangle */
			dCalculatedNormal[0] = ( lpTriangle->vertices[1][1]-lpTriangle->vertices[0][1])*(lpTriangle->vertices[2][2]-lpTriangle->vertices[0][2])-(lpTriangle->vertices[1][2]-lpTriangle->vertices[0][2])*(lpTriangle->vertices[2][1]-lpTriangle->vertices[0][1]);
			dCalculatedNormal[1] = -(lpTriangle->vertices[1][0]-lpTriangle->vertices[0][0])*(lpTriangle->vertices[2][2]-lpTriangle->vertices[0][2])+(lpTriangle->vertices[1][2]-lpTriangle->vertices[0][2])*(lpTriangle->vertices[2][0]-lpTriangle->vertices[0][0]);
			dCalculatedNormal[2] = ( lpTriangle->vertices[1][0]-lpTriangle->vertices[0][0])*(lpTriangle->vertices[2][1]-lpTriangle->vertices[0][1])-(lpTriangle->vertices[1][1]-lpTriangle->vertices[0][1])*(lpTriangle->vertices[2][0]-lpTriangle->vertices[0][0]);
			if((dCalculatedNormal[0] == 0) && (dCalculatedNormal[1] == 0) && (dCalculatedNormal[2] == 0)) {
				if(lpThis->callbackError == NULL) {
					/* We don't have an error callback registered that can take an decision */
					return stlioE_InvalidFormat_NotATriangle;
				}
				/*
					Check what the application wants to do with denaturated triangles

					stlioE_Keep: Write that denaturated triangle into the file
					stlioE_Skip: Skip that triangle
					anything else: Abort this write
				*/
				e = lpThis->callbackError(stlioE_InvalidFormat_NotATriangle, lpTriangle, lpThis->callbackErrorParam);
				if(e == stlioE_Skip) { return stlioE_Ok; }		/* Do not process THAT triangle */
				if(e == stlioE_Keep) { break; }					/* Simply accept that triangle and use later on ... */
				return e;
			}
			dCalculatedNormalLen = sqrt(dCalculatedNormal[0]*dCalculatedNormal[0]+dCalculatedNormal[1]*dCalculatedNormal[1]+dCalculatedNormal[2]*dCalculatedNormal[2]);
			dCalculatedNormal[0] = dCalculatedNormal[0] / dCalculatedNormalLen;
			dCalculatedNormal[1] = dCalculatedNormal[1] / dCalculatedNormalLen;
			dCalculatedNormal[2] = dCalculatedNormal[2] / dCalculatedNormalLen;

			lpTriangle->surfaceNormal[0] = dCalculatedNormal[0];
			lpTriangle->surfaceNormal[1] = dCalculatedNormal[1];
			lpTriangle->surfaceNormal[2] = dCalculatedNormal[2];
			break;
		case stlWriter_NormalMode__CopyNormals:
			break;
		default:
			return stlioE_ImplementationError;
	}

	/* Serialize the input triangle (and if there is attribute data keep a reference) */
	serializeIEEE754SingleFloat_4Bytes(lpTriangle->surfaceNormal[0], &(lpThis->bSerializedDataBuffer[ 0*4]));
	serializeIEEE754SingleFloat_4Bytes(lpTriangle->surfaceNormal[1], &(lpThis->bSerializedDataBuffer[ 1*4]));
	serializeIEEE754SingleFloat_4Bytes(lpTriangle->surfaceNormal[2], &(lpThis->bSerializedDataBuffer[ 2*4]));

	serializeIEEE754SingleFloat_4Bytes(lpTriangle->vertices[0][0], &(lpThis->bSerializedDataBuffer[ 3*4]));
	serializeIEEE754SingleFloat_4Bytes(lpTriangle->vertices[0][1], &(lpThis->bSerializedDataBuffer[ 4*4]));
	serializeIEEE754SingleFloat_4Bytes(lpTriangle->vertices[0][2], &(lpThis->bSerializedDataBuffer[ 5*4]));

	serializeIEEE754SingleFloat_4Bytes(lpTriangle->vertices[1][0], &(lpThis->bSerializedDataBuffer[ 6*4]));
	serializeIEEE754SingleFloat_4Bytes(lpTriangle->vertices[1][1], &(lpThis->bSerializedDataBuffer[ 7*4]));
	serializeIEEE754SingleFloat_4Bytes(lpTriangle->vertices[1][2], &(lpThis->bSerializedDataBuffer[ 8*4]));

	serializeIEEE754SingleFloat_4Bytes(lpTriangle->vertices[2][0], &(lpThis->bSerializedDataBuffer[ 9*4]));
	serializeIEEE754SingleFloat_4Bytes(lpTriangle->vertices[2][1], &(lpThis->bSerializedDataBuffer[10*4]));
	serializeIEEE754SingleFloat_4Bytes(lpTriangle->vertices[2][2], &(lpThis->bSerializedDataBuffer[11*4]));

	lpThis->bSerializedDataBuffer[12*4+0] = (uint8_t)((lpTriangle->attributeByteCount) & 0xFF);
	lpThis->bSerializedDataBuffer[12*4+1] = (uint8_t)((lpTriangle->attributeByteCount >> 8) & 0xFF);

	if(lpTriangle->attributeByteCount != 0) { lpThis->lpAttributedTriangle = (struct stlTriangle_Attributes*)lpTriangle; }
	lpThis->dwPendingBytes = 0;
	lpThis->dwWrittenTriangles = lpThis->dwWrittenTriangles + 1;
	lpThis->state = STLWriter_Binary_State__TriangleFlush;

	/* Try to write everything */
	e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, lpThis->bSerializedDataBuffer, 4*3*4+2, &dwBytesWritten);
	lpThis->dwPendingBytes = lpThis->dwPendingBytes + dwBytesWritten;
	if(e != stlioE_Ok) { return e; } /* We pass any errors back to the application - they may use continuation then later on */
	/* If there are attribute bytes flush them too */
	if(lpThis->lpAttributedTriangle != NULL) {
		e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, lpThis->lpAttributedTriangle->attributeBytes, lpThis->lpAttributedTriangle->base.attributeByteCount, &dwBytesWritten);
		lpThis->dwPendingBytes = lpThis->dwPendingBytes + dwBytesWritten;
		if(e != stlioE_Ok) { return e; } /* We pass any errors back to the application - they may use continuation then later on */
	}

	if(lpThis->dwPendingBytes == lpTriangle->attributeByteCount + 4*3*4 + 2) {
		lpThis->dwPendingBytes = 0;
		lpThis->state = STLWriter_Binary_State__Triangle;
		return stlioE_Ok;
	} else {
		return stlioE_Continues;
	}
}
static enum stlioError stlWriter_Binary__SetNormalMode(
	struct STLWriter* 				lpSelf,
	enum stlWriter_NormalMode		normalMode
) {
	struct STLWriter_Binary* lpThis;

	if(lpSelf == NULL) { return stlioE_InvalidParam; }
	lpThis = (struct STLWriter_Binary*)(lpSelf->lpReserved);

	switch(normalMode) {
		case stlWriter_NormalMode__CheckNormals:		break;
		case stlWriter_NormalMode__CalculateNormals:	break;
		case stlWriter_NormalMode__CopyNormals:			break;
		default:										return stlioE_InvalidParam;
	}

	lpThis->normalMode = normalMode;
	return stlioE_Ok;
}
static enum stlioError stlWriter_Binary__WriteTriangles(
	struct STLWriter* 				lpSelf,
	struct stlTriangle* 			lpTriangles,
	unsigned long int 				dwTriangleCount,
	unsigned long int				dwSizePerTriangle,			/* Can be set to != 0x00 then size is equal (independent of attribute size) */
	unsigned long int* 				lpTrianglesWritten
) {
	enum stlioError e;
	unsigned long int i;
	union {
		unsigned char* lpByte;
		struct stlTriangle* lpTriangle;
		struct stlTriangle_Attributes* lpTriangleAttrib;
		uintptr_t n;
	} ptr;

	if(lpTrianglesWritten != NULL) { (*lpTrianglesWritten) = 0; }

	ptr.lpTriangle = lpTriangles;

	if(dwSizePerTriangle > 0) {
		for(i = 0; i < dwTriangleCount; i=i+1) {
			e = stlWriter_Binary__WriteTriangle(lpSelf, ptr.lpTriangle);
			if(e != stlioE_Ok) { return e; }
			ptr.n = ptr.n + dwSizePerTriangle;
			if(lpTrianglesWritten != NULL) {
				(*lpTrianglesWritten) = (*lpTrianglesWritten) + 1;
			}
		}
	} else {
		for(i = 0; i < dwTriangleCount; i=i+1) {
			e = stlWriter_Binary__WriteTriangle(lpSelf, ptr.lpTriangle);
			if(e != stlioE_Ok) { return e; }
			ptr.n = ptr.n + ptr.lpTriangle->attributeByteCount + 4*3*4 + 2;
			if(lpTrianglesWritten != NULL) {
				(*lpTrianglesWritten) = (*lpTrianglesWritten) + 1;
			}
		}
	}

	return stlioE_Ok;
}
static enum stlioError stlWriter_Binary__Finalize(
	struct STLWriter*				lpSelf
) {
	struct STLWriter_Binary *lpThis;
	unsigned long int dwBytesWritten;
	enum stlioError e;

	if(lpSelf == NULL) { return stlioE_InvalidParam; }
	lpThis = (struct STLWriter_Binary*)(lpSelf->lpReserved);

	/* Check if we are currently accepting ... */
	if(lpThis->state != STLWriter_Binary_State__Triangle) { return stlioE_InvalidState; }

	lpThis->state = STLWriter_Binary_State__FlushCount;
	lpThis->dwPendingBytes = 4;
	lpThis->bSerializedDataBuffer[0] = (uint8_t)((lpThis->dwWrittenTriangles >>  0) & 0xFF);
	lpThis->bSerializedDataBuffer[1] = (uint8_t)((lpThis->dwWrittenTriangles >>  8) & 0xFF);
	lpThis->bSerializedDataBuffer[2] = (uint8_t)((lpThis->dwWrittenTriangles >> 16) & 0xFF);
	lpThis->bSerializedDataBuffer[3] = (uint8_t)((lpThis->dwWrittenTriangles >> 24) & 0xFF);

	/* We simply have to seek back, update the triangle count and then close the output stream */
	e = lpThis->lpStreamSink->vtbl->seekAbsolute(lpThis->lpStreamSink, 80);
	if(e != stlioE_Ok) { return e; } /* Pass errors outside, the application can use continue later on */

	e = lpThis->lpStreamSink->vtbl->writeBytes(
		lpThis->lpStreamSink,
		&(lpThis->bSerializedDataBuffer[4-lpThis->dwPendingBytes]),
		lpThis->dwPendingBytes,
		&dwBytesWritten
	);
	lpThis->dwPendingBytes = lpThis->dwPendingBytes - dwBytesWritten;
	if(e != stlioE_Ok) {
		return e;
	}
	if(lpThis->dwPendingBytes == 0) {
		/* Release our stream sink */
		lpThis->lpStreamSink->vtbl->release(lpThis->lpStreamSink);
		lpThis->lpStreamSink = NULL;

		lpThis->state = STLWriter_Binary_State__Done;
		return stlioE_Done;
	}
	return stlioE_Continues;
}
static enum stlioError stlWriter_Binary__SetCallbackError(
	struct STLWriter* 				lpSelf,
	lpfnSTLWriter_Callback_Error 	callbackError,
	void* 							lpFreeParam
) {
	struct STLWriter_Binary* lpThis;

	if(lpSelf == NULL) { return stlioE_InvalidParam; }
	lpThis = (struct STLWriter_Binary*)(lpSelf->lpReserved);

	lpThis->callbackError = callbackError;
	lpThis->callbackErrorParam = lpFreeParam;
	return stlioE_Ok;
}
static enum stlioError stlWriter_Binary__Release(
	struct STLWriter*				lpSelf
) {
	struct STLWriter_Binary* lpThis;

	if(lpSelf == NULL) { return stlioE_InvalidParam; }
	lpThis = (struct STLWriter_Binary*)(lpSelf->lpReserved);

	/* Release our stream sink if it's lingering around (not done) */
	if(lpThis->lpStreamSink != NULL) {
		lpThis->lpStreamSink->vtbl->release(lpThis->lpStreamSink);
		lpThis->lpStreamSink = NULL;
	}
	free((void*)lpThis);
	return stlioE_Ok;
}
static enum stlioError stlWriter_Binary__ContinueOp(
	struct STLWriter*				lpSelf
) {
	enum stlioError e;
	unsigned long int dwBytesWritten;
	struct STLWriter_Binary* lpThis;

	if(lpSelf == NULL) { return stlioE_InvalidParam; }
	lpThis = (struct STLWriter_Binary*)(lpSelf->lpReserved);

	switch(lpThis->state) {
		case STLWriter_Binary_State__Header:
			e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, &(stlBinaryHeaderTemplate[sizeof(stlBinaryHeaderTemplate)-lpThis->dwPendingBytes]), lpThis->dwPendingBytes, &dwBytesWritten);
			if((lpThis->dwPendingBytes = lpThis->dwPendingBytes - dwBytesWritten) == 0) {
				lpThis->state = STLWriter_Binary_State__Triangle;
			}
			return e;
		case STLWriter_Binary_State__Triangle:
			return stlioE_Ok; /* We accept the next triangle, no buffered data */
		case STLWriter_Binary_State__TriangleFlush:
			if(lpThis->dwPendingBytes < (4*3*4+2)) { /* We use them as bytes written, NOT as pending bytes! */
				/* We are inside the fixed triangle buffer */
				e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, &(lpThis->bSerializedDataBuffer[lpThis->dwPendingBytes]), (4*3*4+2)-lpThis->dwPendingBytes, &dwBytesWritten);
				lpThis->dwPendingBytes = lpThis->dwPendingBytes + dwBytesWritten;
				if(e != stlioE_Ok) { return e; }

				if((lpThis->lpAttributedTriangle == NULL) && (lpThis->dwPendingBytes == 4*3*4+2)) {
					lpThis->state = STLWriter_Binary_State__Triangle;
					return stlioE_Ok;
				} else if(lpThis->lpAttributedTriangle == NULL) {
					return stlioE_Continues;
				}
			}
			/* We are already writing attribute bytes ... */
			e = lpThis->lpStreamSink->vtbl->writeBytes(
				lpThis->lpStreamSink,
				&(lpThis->lpAttributedTriangle->attributeBytes[lpThis->dwPendingBytes-(4*3*4+2)]),
				lpThis->lpAttributedTriangle->base.attributeByteCount - (lpThis->dwPendingBytes-(4*3*4+2)),
				&dwBytesWritten
			);
			lpThis->dwPendingBytes = lpThis->dwPendingBytes + dwBytesWritten;
			if(e != stlioE_Ok) { return e; }
			if(lpThis->dwPendingBytes > 0) { return stlioE_Continues; }
			lpThis->state = STLWriter_Binary_State__Triangle;
			return stlioE_Ok;
		case STLWriter_Binary_State__FlushCount:
			if(lpThis->dwPendingBytes == 4) {
				/* We may have had a problem during seek */
				e = lpThis->lpStreamSink->vtbl->seekAbsolute(lpThis->lpStreamSink, 80);
				if(e != stlioE_Ok) { return e; }
			}
			e = lpThis->lpStreamSink->vtbl->writeBytes(
				lpThis->lpStreamSink,
				&(lpThis->bSerializedDataBuffer[4-lpThis->dwPendingBytes]),
				lpThis->dwPendingBytes,
				&dwBytesWritten
			);
			lpThis->dwPendingBytes = lpThis->dwPendingBytes - dwBytesWritten;
			if(e != stlioE_Ok) {
				return e;
			}
			if(lpThis->dwPendingBytes == 0) {
				if(lpThis->lpStreamSink != NULL) {
					lpThis->lpStreamSink->vtbl->release(lpThis->lpStreamSink);
					lpThis->lpStreamSink = NULL;
				}
				lpThis->state = STLWriter_Binary_State__Done;
				return stlioE_Done;
			}
			return stlioE_Continues;
		case STLWriter_Binary_State__Done:
			return stlioE_Done;
		case STLWriter_Binary_State__Error:
			return stlioE_InvalidState;
		default:
			return stlioE_ImplementationError;
	}
}

static struct STLWriter_Vtbl STLWriter_Binary__DefaultVTBL = {
	&stlWriter_Binary__Release,
	&stlWriter_Binary__Finalize,
	&stlWriter_Binary__ContinueOp,

	&stlWriter_Binary__WriteTriangle,
	&stlWriter_Binary__WriteTriangles,

	&stlWriter_Binary__SetNormalMode,
	&stlWriter_Binary__SetCallbackError
};

#ifdef __cplusplus
	extern "C" {
#endif

enum stlioError stlioWriterBinary(struct STLWriter** lpOut, struct STLStreamSink* lpStreamSink) {
	enum stlioError e;
	struct STLWriter_Binary* lpNew;
	unsigned long int dwWritten;

	if(lpOut == NULL) { return stlioE_InvalidParam; }
	(*lpOut) = NULL;

	if(lpStreamSink == NULL) { return stlioE_InvalidParam; }
	if(lpStreamSink->vtbl == NULL) { return stlioE_InvalidParam; }
	if((lpStreamSink->vtbl->writeBytes == NULL) || (lpStreamSink->vtbl->seekAbsolute == NULL) || (lpStreamSink->vtbl->release == NULL)) { return stlioE_InvalidParam; }

	if((lpNew = (struct STLWriter_Binary*)malloc(sizeof(struct STLWriter_Binary))) == NULL) { return stlioE_OutOfMemory; }

	lpNew->base.vtbl 				= &STLWriter_Binary__DefaultVTBL;
	lpNew->base.lpReserved 			= (void*)lpNew;
	lpNew->lpStreamSink 			= lpStreamSink;
	lpNew->normalMode 				= stlWriter_NormalMode__CheckNormals; /* By default we only validate */
	lpNew->state					= STLWriter_Binary_State__Header;
	lpNew->dwPendingBytes			= sizeof(stlBinaryHeaderTemplate); /* We start without writing the header ... */
	lpNew->callbackError			= NULL;
	lpNew->callbackErrorParam		= NULL;
	lpNew->lpAttributedTriangle		= NULL;
	lpNew->dwWrittenTriangles		= 0;

	/* Immediately write a simple STL "header" */
	dwWritten = 0;
	e = lpStreamSink->vtbl->writeBytes(lpStreamSink, stlBinaryHeaderTemplate, lpNew->dwPendingBytes, &dwWritten);
	lpNew->dwPendingBytes = lpNew->dwPendingBytes - dwWritten;
	if((e != stlioE_Ok) && (e != stlioE_Continues)) {
		lpStreamSink->vtbl->release(lpStreamSink);
		free((void*)lpNew);
		return e;
	}
	if(lpNew->dwPendingBytes == 0) { lpNew->state = STLWriter_Binary_State__Triangle; /* We will start accepting triangles */ }

	(*lpOut) = (struct STLWriter*)lpNew;
	return e;
}

#ifdef __cplusplus
	} /* extern "C" { */
#endif

/*
	ASCII STL Writer
	================
*/

static char* stlioWriterASCII_SolidName_Default = "libstlio.stl_v1";

enum STLWriter_ASCII_State {
	STLWriter_ASCII_State__Solid,

	STLWriter_ASCII_State__AcceptTriangle,
	STLWriter_ASCII_State__FacetNormal,
	STLWriter_ASCII_State__Normal,
	STLWriter_ASCII_State__OuterLoop,
	STLWriter_ASCII_State__Vertex,
	STLWriter_ASCII_State__VertexCoordinate,
	STLWriter_ASCII_State__Endloop,
	STLWriter_ASCII_State__Endfacet,

	STLWriter_ASCII_State__Endsolid,
	STLWriter_ASCII_State__EndsolidName,

	STLWriter_ASCII_State__Done,
	STLWriter_ASCII_State__Error
};


struct STLWriter_ASCII {
	struct STLWriter						base;

	struct STLStreamSink*					lpStreamSink;
	enum stlWriter_NormalMode				normalMode;
	enum stlioEndOfLineMarker				eolMarkerType;

	enum STLWriter_ASCII_State				state;
	unsigned long int						dwBytesWritten;
	unsigned long int						dwVerticesWritten;

	struct stlTriangle						currentTriangle; /* Note that ASCII never can include Attributes ... */

	char*									lpSolidName;
	unsigned long int						dwSolidNameLen;

	lpfnSTLWriter_Callback_Error			callbackError;
	void*									callbackErrorParam;
};

static enum stlioError stlWriter_ASCII__ContinueOp(
	struct STLWriter*				lpSelf
) {
	enum stlioError e;
	struct STLWriter_ASCII* lpThis;
	unsigned long int dwWritten;
	unsigned long int i,j;
	uint8_t bDataBuffer[32]; /* Used for sprintf */
	uint8_t bUsedEOL[3];
	uint8_t bTemp;
	int rSize;

	if(lpSelf == NULL) { return stlioE_InvalidParam; }
	lpThis = (struct STLWriter_ASCII*)(lpSelf->lpReserved);

	switch(lpThis->state) {
		case STLWriter_ASCII_State__Solid:
			while(lpThis->dwBytesWritten < strlen(strSolid)) {
				e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strSolid[lpThis->dwBytesWritten]), strlen(strSolid)-lpThis->dwBytesWritten, &dwWritten);
				lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
				if(e != stlioE_Ok) { return e; }
			}
			if(lpThis->dwBytesWritten < strlen(strSolid)+1) {
				bTemp = 0x20; /* ASCII space */
				e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, &bTemp, 1, &dwWritten);
				lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
				if(e != stlioE_Ok) { return e; }
			}
			while(lpThis->dwBytesWritten < strlen(strSolid)+1+lpThis->dwSolidNameLen) {
				e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(lpThis->lpSolidName[lpThis->dwBytesWritten-strlen(strSolid)-1]), lpThis->dwSolidNameLen-(lpThis->dwBytesWritten-strlen(strSolid)-1), &dwWritten);
				lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
				if(e != stlioE_Ok) { return e; }
			}
			while(lpThis->dwBytesWritten < strlen(strSolid)+1+lpThis->dwSolidNameLen+(lpThis->eolMarkerType == stlioEndOfLineMarker_CRLF ? 2 : 1)) {
				/*
					Bytes in front of marker: strlen(strSolid)+1+lpThis->dwSolidNameLen
				*/
				switch(lpThis->eolMarkerType) {
					case stlioEndOfLineMarker_CRLF:
						e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strCRLF[lpThis->dwBytesWritten-(strlen(strSolid)+1+lpThis->dwSolidNameLen)]), (strlen(strSolid)+1+lpThis->dwSolidNameLen+2)-lpThis->dwBytesWritten, &dwWritten);
						lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
						if(e != stlioE_Ok) { return e; }
						break;
					case stlioEndOfLineMarker_LF:
						e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strCRLF[1]), 1, &dwWritten);
						lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
						if(e != stlioE_Ok) { return e; }
						break;
					case stlioEndOfLineMarker_CR:
						e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strCRLF[0]), 1, &dwWritten);
						lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
						if(e != stlioE_Ok) { return e; }
						break;
					default:
						return stlioE_ImplementationError;
				}
			}
			/*
				If we reach here we have already written the whole "solid SOLIDNAME\n" line
				We are now ready to accept triangles
			*/
			lpThis->state = STLWriter_ASCII_State__AcceptTriangle;
			lpThis->dwBytesWritten = 0;
			return stlioE_Ok;

		case STLWriter_ASCII_State__AcceptTriangle:
			return stlioE_Ok;

		case STLWriter_ASCII_State__FacetNormal:
			/*
				output string "facet normal ". After that we switch into "normal"
				state that writes the three normals
			*/
			while(lpThis->dwBytesWritten < strlen(strFacetNormal)) {
				e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strFacetNormal[lpThis->dwBytesWritten]), strlen(strFacetNormal)-lpThis->dwBytesWritten, &dwWritten);
				lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
				if(e != stlioE_Ok) { return e; }
			}
			lpThis->state = STLWriter_ASCII_State__Normal;
			lpThis->dwBytesWritten = 0;
			/* Fall into next state if we ever reach here ... */
		case STLWriter_ASCII_State__Normal:
			/*
				Write one normal (including the trailing space) after each other into our databuffer ...
				j counts the bytes since the beginning of the first vertex normal that we have already written
				to be capable to skip writes that have already been done
			*/
			switch(lpThis->eolMarkerType) {
				case stlioEndOfLineMarker_LF:		bUsedEOL[0] = 0x0A; bUsedEOL[1] = 0x00; break;
				case stlioEndOfLineMarker_CR:		bUsedEOL[0] = 0x0D; bUsedEOL[1] = 0x00; break;
				case stlioEndOfLineMarker_CRLF:		bUsedEOL[0] = 0x0D; bUsedEOL[1] = 0x0A; bUsedEOL[2] = 0x00; break;
				default:							return stlioE_ImplementationError;
			}

			j = 0;
			for(i = 0; i < sizeof(lpThis->currentTriangle.surfaceNormal)/sizeof(double); i=i+1) {
				/* Convert given float to string using snprintf */
				if(i != (sizeof(lpThis->currentTriangle.surfaceNormal)/sizeof(double))-1) {
					rSize = snprintf((char*)bDataBuffer, sizeof(bDataBuffer), "%g ", lpThis->currentTriangle.surfaceNormal[i]);
				} else {
					rSize = snprintf((char*)bDataBuffer, sizeof(bDataBuffer), "%g%s", lpThis->currentTriangle.surfaceNormal[i], bUsedEOL);
				}
				if((rSize < 0) || (rSize == sizeof(bDataBuffer))) { return stlioE_ImplementationError; }

				/* We want to write rSize bytes into our output buffer ... check if we HAVE already written them */
				if(lpThis->dwBytesWritten < (j+rSize)) {
					/* We have to write additional bytes from this normal */
					e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(bDataBuffer[lpThis->dwBytesWritten-j]), rSize - (lpThis->dwBytesWritten-j), &dwWritten);
					lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
					if(e != stlioE_Ok) { return e; }
				}
				j = j + rSize;
			}
			lpThis->state = STLWriter_ASCII_State__OuterLoop;
			lpThis->dwBytesWritten = 0;
			/* Fall into next state if we ever reach here ... */
		case STLWriter_ASCII_State__OuterLoop:
			/*
				output string "outer loop"
			*/
			while(lpThis->dwBytesWritten < strlen(strOuterLoop)) {
				e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strOuterLoop[lpThis->dwBytesWritten]), strlen(strOuterLoop)-lpThis->dwBytesWritten, &dwWritten);
				lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
				if(e != stlioE_Ok) { return e; }
			}

			while(lpThis->dwBytesWritten < strlen(strOuterLoop)+(lpThis->eolMarkerType == stlioEndOfLineMarker_CRLF ? 2 : 1)) {
				/*
					Bytes in front of marker: strlen(strSolid)+1+lpThis->dwSolidNameLen
				*/
				switch(lpThis->eolMarkerType) {
					case stlioEndOfLineMarker_CRLF:
						e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strCRLF[lpThis->dwBytesWritten-strlen(strOuterLoop)]), (strlen(strOuterLoop)+2)-lpThis->dwBytesWritten, &dwWritten);
						lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
						if(e != stlioE_Ok) { return e; }
						break;
					case stlioEndOfLineMarker_LF:
						e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strCRLF[1]), 1, &dwWritten);
						lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
						if(e != stlioE_Ok) { return e; }
						break;
					case stlioEndOfLineMarker_CR:
						e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strCRLF[0]), 1, &dwWritten);
						lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
						if(e != stlioE_Ok) { return e; }
						break;
					default:
						return stlioE_ImplementationError;
				}
			}

			lpThis->state = STLWriter_ASCII_State__Vertex;
			lpThis->dwBytesWritten = 0;
			lpThis->dwVerticesWritten = 0;
			/* Fall into next state if we ever reach here ... */
		STLWriter_ASCII_State__Vertex:	/* Used for goto from VertexCoordinate state up */
		case STLWriter_ASCII_State__Vertex:
			/*
				output string "vertex ". After that we switch into "vertex"
				state that writes the three normals
			*/
			while(lpThis->dwBytesWritten < strlen(strVertex2)) {
				e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strVertex2[lpThis->dwBytesWritten]), strlen(strVertex2)-lpThis->dwBytesWritten, &dwWritten);
				lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
				if(e != stlioE_Ok) { return e; }
			}
			lpThis->state = STLWriter_ASCII_State__VertexCoordinate;
			lpThis->dwBytesWritten = 0;
			/* Fall into next state if we ever reach here ... */
		case STLWriter_ASCII_State__VertexCoordinate:
			/*
				Write one vertex component (including the trailing space) after each other into our databuffer ...
				j counts the bytes since the beginning of the first vertex normal that we have already written
				to be capable to skip writes that have already been done
			*/
			switch(lpThis->eolMarkerType) {
				case stlioEndOfLineMarker_LF:		bUsedEOL[0] = 0x0A; bUsedEOL[1] = 0x00; break;
				case stlioEndOfLineMarker_CR:		bUsedEOL[0] = 0x0D; bUsedEOL[1] = 0x00; break;
				case stlioEndOfLineMarker_CRLF:		bUsedEOL[0] = 0x0D; bUsedEOL[1] = 0x0A; bUsedEOL[2] = 0x00; break;
				default:							return stlioE_ImplementationError;
			}

			j = 0;
			for(i = 3*lpThis->dwVerticesWritten; i < 3*lpThis->dwVerticesWritten+3; i=i+1) {
				/* Convert given float to string using snprintf */
				if(i != (3*lpThis->dwVerticesWritten+3)-1) {
					rSize = snprintf((char*)bDataBuffer, sizeof(bDataBuffer), "%g ", lpThis->currentTriangle.vertices[i / 3][i % 3]);
				} else {
					rSize = snprintf((char*)bDataBuffer, sizeof(bDataBuffer), "%g%s", lpThis->currentTriangle.vertices[i / 3][i % 3], bUsedEOL);
				}
				if((rSize < 0) || (rSize == sizeof(bDataBuffer))) { return stlioE_ImplementationError; }

				/* We want to write rSize bytes into our output buffer ... check if we HAVE already written them */
				if(lpThis->dwBytesWritten < (j+rSize)) {
					/* We have to write additional bytes from this normal */
					e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(bDataBuffer[lpThis->dwBytesWritten-j]), rSize - (lpThis->dwBytesWritten-j), &dwWritten);
					lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
					if(e != stlioE_Ok) { return e; }
				}
				j = j + rSize;
			}
			if((lpThis->dwVerticesWritten = lpThis->dwVerticesWritten + 1) != 3) {
				lpThis->state = STLWriter_ASCII_State__Vertex;
				lpThis->dwBytesWritten = 0;
				goto STLWriter_ASCII_State__Vertex; /* Yes, we know that goto is bad and one should a loop instead ... */
			}
			lpThis->state = STLWriter_ASCII_State__Endloop;
			lpThis->dwBytesWritten = 0;
			/* Fall into next state if we ever reach here ... */
		case STLWriter_ASCII_State__Endloop:
			/*
				output string "vertex ". After that we switch into "vertex"
				state that writes the three normals
			*/
			while(lpThis->dwBytesWritten < strlen(strEndloop2)) {
				e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strEndloop2[lpThis->dwBytesWritten]), strlen(strEndloop2)-lpThis->dwBytesWritten, &dwWritten);
				lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
				if(e != stlioE_Ok) { return e; }
			}

			/* Fall into next state if we ever reach here ... */
			while(lpThis->dwBytesWritten < strlen(strEndloop2)+(lpThis->eolMarkerType == stlioEndOfLineMarker_CRLF ? 2 : 1)) {
				/*
					Bytes in front of marker: strlen(strSolid)+1+lpThis->dwSolidNameLen
				*/
				switch(lpThis->eolMarkerType) {
					case stlioEndOfLineMarker_CRLF:
						e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strCRLF[lpThis->dwBytesWritten-strlen(strEndloop2)]), 2, &dwWritten);
						lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
						if(e != stlioE_Ok) { return e; }
						break;
					case stlioEndOfLineMarker_LF:
						e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strCRLF[1]), 1, &dwWritten);
						lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
						if(e != stlioE_Ok) { return e; }
						break;
					case stlioEndOfLineMarker_CR:
						e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strCRLF[0]), 1, &dwWritten);
						lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
						if(e != stlioE_Ok) { return e; }
						break;
					default:
						return stlioE_ImplementationError;
				}
			}

			lpThis->state = STLWriter_ASCII_State__Endfacet;
			lpThis->dwBytesWritten = 0;
			/* Fall into next state if we ever reach here ... */
		case STLWriter_ASCII_State__Endfacet:
			/*
				output string "vertex ". After that we switch into "vertex"
				state that writes the three normals
			*/
			while(lpThis->dwBytesWritten < strlen(strEndfacet2)) {
				e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strEndfacet2[lpThis->dwBytesWritten]), strlen(strEndfacet2)-lpThis->dwBytesWritten, &dwWritten);
				lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
				if(e != stlioE_Ok) { return e; }
			}

			/* Fall into next state if we ever reach here ... */
			while(lpThis->dwBytesWritten < strlen(strEndloop2)+(lpThis->eolMarkerType == stlioEndOfLineMarker_CRLF ? 2 : 1)) {
				/*
					Bytes in front of marker: strlen(strSolid)+1+lpThis->dwSolidNameLen
				*/
				switch(lpThis->eolMarkerType) {
					case stlioEndOfLineMarker_CRLF:
						e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strCRLF[lpThis->dwBytesWritten-strlen(strEndloop2)]), 2, &dwWritten);
						lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
						if(e != stlioE_Ok) { return e; }
						break;
					case stlioEndOfLineMarker_LF:
						e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strCRLF[1]), 1, &dwWritten);
						lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
						if(e != stlioE_Ok) { return e; }
						break;
					case stlioEndOfLineMarker_CR:
						e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strCRLF[0]), 1, &dwWritten);
						lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
						if(e != stlioE_Ok) { return e; }
						break;
					default:
						return stlioE_ImplementationError;
				}
			}

			lpThis->state = STLWriter_ASCII_State__AcceptTriangle;
			lpThis->dwBytesWritten = 0;
			return stlioE_Ok;
		case STLWriter_ASCII_State__Endsolid:
			/*
				output string "endsolid ". After that we switch into "endsolidname"
				state that writes the solid name again
			*/
			while(lpThis->dwBytesWritten < strlen(strEndsolid2)) {
				e = lpThis->lpStreamSink->vtbl->writeBytes(lpThis->lpStreamSink, (unsigned char*)&(strEndsolid2[lpThis->dwBytesWritten]), strlen(strEndsolid2)-lpThis->dwBytesWritten, &dwWritten);
				lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
				if(e != stlioE_Ok) { return e; }
			}
			lpThis->state = STLWriter_ASCII_State__EndsolidName;
			lpThis->dwBytesWritten = 0;
			/* Fall into next state */
		case STLWriter_ASCII_State__EndsolidName:
			while(lpThis->dwBytesWritten < lpThis->dwSolidNameLen) {
				e = lpThis->lpStreamSink->vtbl->writeBytes(
					lpThis->lpStreamSink,
					(unsigned char*)&(lpThis->lpSolidName[lpThis->dwBytesWritten]),
					lpThis->dwSolidNameLen-lpThis->dwBytesWritten,
					&dwWritten
				);
				lpThis->dwBytesWritten = lpThis->dwBytesWritten + dwWritten;
				if(e != stlioE_Ok) { return e; }
			}
			/* If we reach here we are done ... */
			lpThis->lpStreamSink->vtbl->release(lpThis->lpStreamSink);
			lpThis->lpStreamSink = NULL;
			return stlioE_Done;

		case STLWriter_ASCII_State__Done:
			return stlioE_Done;

		case STLWriter_ASCII_State__Error:
			return stlioE_InvalidState;

		default:
			return stlioE_ImplementationError;
	}
}


static enum stlioError stlWriter_ASCII__WriteTriangle(
	struct STLWriter* 				lpSelf,
	struct stlTriangle* 			lpTriangle
) {
	enum stlioError e;
	struct STLWriter_ASCII* lpThis;
	double dCalculatedNormal[3];	double dCalculatedNormalLen;
	double dPassedNormal[3];		double dPassedNormalLen;

	if((lpSelf == NULL) || (lpTriangle == NULL)) { return stlioE_InvalidParam; }

	lpThis = (struct STLWriter_ASCII*)(lpSelf->lpReserved);

	/* Check if we are in accepting state */
	if(lpThis->state != STLWriter_ASCII_State__AcceptTriangle) { return stlioE_InvalidState; }

	/* Validate triangle (if required) */
	switch(lpThis->normalMode) {
		case stlWriter_NormalMode__CheckNormals:
			/* Calclate normals and check they are identical after being normalized */
			dCalculatedNormal[0] = ( lpTriangle->vertices[1][1]-lpTriangle->vertices[0][1])*(lpTriangle->vertices[2][2]-lpTriangle->vertices[0][2])-(lpTriangle->vertices[1][2]-lpTriangle->vertices[0][2])*(lpTriangle->vertices[2][1]-lpTriangle->vertices[0][1]);
			dCalculatedNormal[1] = -(lpTriangle->vertices[1][0]-lpTriangle->vertices[0][0])*(lpTriangle->vertices[2][2]-lpTriangle->vertices[0][2])+(lpTriangle->vertices[1][2]-lpTriangle->vertices[0][2])*(lpTriangle->vertices[2][0]-lpTriangle->vertices[0][0]);
			dCalculatedNormal[2] = ( lpTriangle->vertices[1][0]-lpTriangle->vertices[0][0])*(lpTriangle->vertices[2][1]-lpTriangle->vertices[0][1])-(lpTriangle->vertices[1][1]-lpTriangle->vertices[0][1])*(lpTriangle->vertices[2][0]-lpTriangle->vertices[0][0]);
			if((dCalculatedNormal[0] == 0) && (dCalculatedNormal[1] == 0) && (dCalculatedNormal[2] == 0)) {
				if(lpThis->callbackError == NULL) {
					/* We don't have an error callback registered that can take an decision */
					return stlioE_InvalidFormat_NotATriangle;
				}
				/*
					Check what the application wants to do with denaturated triangles

					stlioE_Keep: Write that denaturated triangle into the file
					stlioE_Skip: Skip that triangle
					anything else: Abort this write
				*/
				e = lpThis->callbackError(stlioE_InvalidFormat_NotATriangle, lpTriangle, lpThis->callbackErrorParam);
				if(e == stlioE_Skip) { return stlioE_Ok; }		/* Do not process THAT triangle */
				if(e == stlioE_Keep) { break; }					/* Simply accept that triangle and use later on ... */
				return e;
			}
			dCalculatedNormalLen = sqrt(dCalculatedNormal[0]*dCalculatedNormal[0]+dCalculatedNormal[1]*dCalculatedNormal[1]+dCalculatedNormal[2]*dCalculatedNormal[2]);
			dCalculatedNormal[0] = dCalculatedNormal[0] / dCalculatedNormalLen;
			dCalculatedNormal[1] = dCalculatedNormal[1] / dCalculatedNormalLen;
			dCalculatedNormal[2] = dCalculatedNormal[2] / dCalculatedNormalLen;

			dPassedNormal[0] = lpTriangle->surfaceNormal[0];
			dPassedNormal[1] = lpTriangle->surfaceNormal[1];
			dPassedNormal[2] = lpTriangle->surfaceNormal[2];

			dPassedNormalLen = sqrt(dPassedNormal[0]*dPassedNormal[0]+dPassedNormal[1]*dPassedNormal[1]+dPassedNormal[2]*dPassedNormal[2]);
			dPassedNormal[0] = dPassedNormal[0] / dPassedNormalLen;
			dPassedNormal[1] = dPassedNormal[1] / dPassedNormalLen;
			dPassedNormal[2] = dPassedNormal[2] / dPassedNormalLen;

			// if((dPassedNormal[0] != dCalculatedNormal[0]) || (dPassedNormal[1] != dCalculatedNormal[1]) || (dPassedNormal[2] != dCalculatedNormal[2])) {
			if(dblEquals(dPassedNormal[0], dCalculatedNormal[0]) && dblEquals(dPassedNormal[1], dCalculatedNormal[1]) && dblEquals(dPassedNormal[2], dCalculatedNormal[2])) {
				if(lpThis->callbackError == NULL) {
					/* We don't have an error callback registered that can take an decision */
					return stlioE_InvalidFormat_VertexNormalMismatch;
				}
				/*
					Check what the application wants to do with denaturated triangles

					stlioE_Keep: Write that mismatched normal into the file
					stlioE_Skip: Skip that triangle
					stlioE_RepairNormal: Repair the normal (use the calculated one)
					anything else: Abort this write
				*/
				e = lpThis->callbackError(stlioE_InvalidFormat_NotATriangle, lpTriangle, lpThis->callbackErrorParam);
				if(e == stlioE_Skip) { return stlioE_Ok; }		/* Do not process THAT triangle */
				if(e == stlioE_Keep) { break; }					/* Simply accept that triangle and use later on ... */
				if(e == stlioE_RepairNormal) {
					lpTriangle->surfaceNormal[0] = dCalculatedNormal[0];
					lpTriangle->surfaceNormal[1] = dCalculatedNormal[1];
					lpTriangle->surfaceNormal[2] = dCalculatedNormal[2];
					break;
				}
				return e;
			}
			break;
		case stlWriter_NormalMode__CalculateNormals:
			/* Simply overwrite the normals contained inside the triangle */
			dCalculatedNormal[0] = ( lpTriangle->vertices[1][1]-lpTriangle->vertices[0][1])*(lpTriangle->vertices[2][2]-lpTriangle->vertices[0][2])-(lpTriangle->vertices[1][2]-lpTriangle->vertices[0][2])*(lpTriangle->vertices[2][1]-lpTriangle->vertices[0][1]);
			dCalculatedNormal[1] = -(lpTriangle->vertices[1][0]-lpTriangle->vertices[0][0])*(lpTriangle->vertices[2][2]-lpTriangle->vertices[0][2])+(lpTriangle->vertices[1][2]-lpTriangle->vertices[0][2])*(lpTriangle->vertices[2][0]-lpTriangle->vertices[0][0]);
			dCalculatedNormal[2] = ( lpTriangle->vertices[1][0]-lpTriangle->vertices[0][0])*(lpTriangle->vertices[2][1]-lpTriangle->vertices[0][1])-(lpTriangle->vertices[1][1]-lpTriangle->vertices[0][1])*(lpTriangle->vertices[2][0]-lpTriangle->vertices[0][0]);
			if((dCalculatedNormal[0] == 0) && (dCalculatedNormal[1] == 0) && (dCalculatedNormal[2] == 0)) {
				if(lpThis->callbackError == NULL) {
					/* We don't have an error callback registered that can take an decision */
					return stlioE_InvalidFormat_NotATriangle;
				}
				/*
					Check what the application wants to do with denaturated triangles

					stlioE_Keep: Write that denaturated triangle into the file
					stlioE_Skip: Skip that triangle
					anything else: Abort this write
				*/
				e = lpThis->callbackError(stlioE_InvalidFormat_NotATriangle, lpTriangle, lpThis->callbackErrorParam);
				if(e == stlioE_Skip) { return stlioE_Ok; }		/* Do not process THAT triangle */
				if(e == stlioE_Keep) { break; }					/* Simply accept that triangle and use later on ... */
				return e;
			}
			dCalculatedNormalLen = sqrt(dCalculatedNormal[0]*dCalculatedNormal[0]+dCalculatedNormal[1]*dCalculatedNormal[1]+dCalculatedNormal[2]*dCalculatedNormal[2]);
			dCalculatedNormal[0] = dCalculatedNormal[0] / dCalculatedNormalLen;
			dCalculatedNormal[1] = dCalculatedNormal[1] / dCalculatedNormalLen;
			dCalculatedNormal[2] = dCalculatedNormal[2] / dCalculatedNormalLen;

			lpTriangle->surfaceNormal[0] = dCalculatedNormal[0];
			lpTriangle->surfaceNormal[1] = dCalculatedNormal[1];
			lpTriangle->surfaceNormal[2] = dCalculatedNormal[2];
			break;
		case stlWriter_NormalMode__CopyNormals:
			break;
		default:
			return stlioE_ImplementationError;
	}

	/* check that we don't have attributes (not supported in ASCII format) */
	if(lpTriangle->attributeByteCount != 0) {
		if(lpThis->callbackError != NULL) {
			e = lpThis->callbackError(stlioE_InvalidFormat_AttributesNotSupported, lpTriangle, lpThis->callbackErrorParam);
			if(e == stlioE_Skip) { /* We skip that triangle */ return stlioE_Ok; 					}
			if(e != stlioE_Keep) { 	return stlioE_InvalidFormat_AttributesNotSupported; 			}
			/* We keep the triangle but skip attribute data */
		} else {
			return stlioE_InvalidFormat_AttributesNotSupported;
		}
	}

	/* We now have an triangle. Store that inside our local buffer */
	lpThis->currentTriangle.surfaceNormal[0] = lpTriangle->surfaceNormal[0];
	lpThis->currentTriangle.surfaceNormal[1] = lpTriangle->surfaceNormal[1];
	lpThis->currentTriangle.surfaceNormal[2] = lpTriangle->surfaceNormal[2];

	lpThis->currentTriangle.vertices[0][0] = lpTriangle->vertices[0][0];
	lpThis->currentTriangle.vertices[0][1] = lpTriangle->vertices[0][1];
	lpThis->currentTriangle.vertices[0][2] = lpTriangle->vertices[0][2];

	lpThis->currentTriangle.vertices[1][0] = lpTriangle->vertices[1][0];
	lpThis->currentTriangle.vertices[1][1] = lpTriangle->vertices[1][1];
	lpThis->currentTriangle.vertices[1][2] = lpTriangle->vertices[1][2];

	lpThis->currentTriangle.vertices[2][0] = lpTriangle->vertices[2][0];
	lpThis->currentTriangle.vertices[2][1] = lpTriangle->vertices[2][1];
	lpThis->currentTriangle.vertices[2][2] = lpTriangle->vertices[2][2];

	lpThis->currentTriangle.attributeByteCount = 0;

	/* Switch to "write" stage */
	lpThis->state = STLWriter_ASCII_State__FacetNormal;
	lpThis->dwBytesWritten = 0;

	/* Use continuation function (that contains the whole writing logic) */
	return stlWriter_ASCII__ContinueOp(lpSelf);
}

static enum stlioError stlWriter_ASCII__SetNormalMode(
	struct STLWriter* 				lpSelf,
	enum stlWriter_NormalMode		normalMode
) {
	struct STLWriter_ASCII* lpThis;

	if(lpSelf == NULL) { return stlioE_InvalidParam; }
	lpThis = (struct STLWriter_ASCII*)(lpSelf->lpReserved);

	switch(normalMode) {
		case stlWriter_NormalMode__CheckNormals:			break;
		case stlWriter_NormalMode__CalculateNormals:		break;
		case stlWriter_NormalMode__CopyNormals:				break;
		default:											return stlioE_InvalidParam;
	}

	lpThis->normalMode = normalMode;
	return stlioE_Ok;
}

static enum stlioError stlWriter_ASCII__WriteTriangles(
	struct STLWriter* 				lpSelf,
	struct stlTriangle* 			lpTriangles,
	unsigned long int 				dwTriangleCount,
	unsigned long int				dwSizePerTriangle,			/* Can be set to != 0x00 then size is equal (independent of attribute size) */
	unsigned long int* 				lpTrianglesWritten
) {
	enum stlioError e;
	unsigned long int i;
	union {
		unsigned char* lpByte;
		struct stlTriangle* lpTriangle;
		struct stlTriangle_Attributes* lpTriangleAttrib;
		uintptr_t n;
	} ptr;

	if(lpTrianglesWritten != NULL) { (*lpTrianglesWritten) = 0; }

	ptr.lpTriangle = lpTriangles;

	if(dwSizePerTriangle > 0) {
		for(i = 0; i < dwTriangleCount; i=i+1) {
			e = stlWriter_ASCII__WriteTriangle(lpSelf, ptr.lpTriangle);
			if(e != stlioE_Ok) { return e; }
			ptr.n = ptr.n + dwSizePerTriangle;
			if(lpTrianglesWritten != NULL) {
				(*lpTrianglesWritten) = (*lpTrianglesWritten) + 1;
			}
		}
	} else {
		for(i = 0; i < dwTriangleCount; i=i+1) {
			e = stlWriter_ASCII__WriteTriangle(lpSelf, ptr.lpTriangle);
			if(e != stlioE_Ok) { return e; }
			ptr.n = ptr.n + ptr.lpTriangle->attributeByteCount + 4*3*4 + 2;
			if(lpTrianglesWritten != NULL) {
				(*lpTrianglesWritten) = (*lpTrianglesWritten) + 1;
			}
		}
	}

	return stlioE_Ok;
}
static enum stlioError stlWriter_ASCII__Finalize(
	struct STLWriter*				lpSelf
) {
	struct STLWriter_ASCII* lpThis;
	/*
		This writes endsolid after each triangle has been finished
	*/
	if(lpSelf == NULL) { return stlioE_InvalidParam; }
	lpThis = (struct STLWriter_ASCII*)(lpSelf->lpReserved);

	if(lpThis->state != STLWriter_ASCII_State__AcceptTriangle) { return stlioE_InvalidState; }

	lpThis->state = STLWriter_ASCII_State__Endsolid;
	lpThis->dwBytesWritten = 0;
	return stlWriter_ASCII__ContinueOp(lpSelf);
}
static enum stlioError stlWriter_ASCII__SetCallbackError(
	struct STLWriter* 				lpSelf,
	lpfnSTLWriter_Callback_Error 	callbackError,
	void* 							lpFreeParam
) {
	struct STLWriter_ASCII* lpThis;

	if(lpSelf == NULL) { return stlioE_InvalidParam; }
	lpThis = (struct STLWriter_ASCII*)(lpSelf->lpReserved);

	lpThis->callbackError = callbackError;
	lpThis->callbackErrorParam = lpFreeParam;
	return stlioE_Ok;
}
static enum stlioError stlWriter_ASCII__Release(
	struct STLWriter*				lpSelf
) {
	struct STLWriter_ASCII* lpThis;

	if(lpSelf == NULL) { return stlioE_InvalidParam; }
	lpThis = (struct STLWriter_ASCII*)(lpSelf->lpReserved);

	if(lpThis->lpStreamSink != NULL) {
		lpThis->lpStreamSink->vtbl->release(lpThis->lpStreamSink);
		lpThis->lpStreamSink = NULL;
	}

	free((void*)lpThis);
	return stlioE_Ok;
}

static struct STLWriter_Vtbl STLWriter_ASCII__DefaultVTBL = {
	&stlWriter_ASCII__Release,
	&stlWriter_ASCII__Finalize,
	&stlWriter_ASCII__ContinueOp,

	&stlWriter_ASCII__WriteTriangle,
	&stlWriter_ASCII__WriteTriangles,

	&stlWriter_ASCII__SetNormalMode,
	&stlWriter_ASCII__SetCallbackError
};


#ifdef __cplusplus
	extern "C" {
#endif

enum stlioError stlioWriterASCII(struct STLWriter** lpOut, struct STLStreamSink* lpStreamSink) {
	struct STLWriter_ASCII* lpNew;
	enum stlioError e;

	/* This constructor does NOT accept an "solid name". We will use a default solid name as defined above */
	if(lpOut == NULL) { return stlioE_InvalidParam; }
	(*lpOut) = NULL;
	if(lpStreamSink == NULL) { return stlioE_InvalidParam; }

	lpNew = (struct STLWriter_ASCII*)malloc(sizeof(struct STLWriter_ASCII));
	if(lpNew == NULL) { return stlioE_OutOfMemory; }

	/* Initialize structure, then write headers (use the continuation function for actual file I/O so we only have that in one place ...) */
	lpNew->base.vtbl 				= &STLWriter_ASCII__DefaultVTBL;
	lpNew->base.lpReserved 			= (void*)lpNew;
	lpNew->lpStreamSink				= lpStreamSink;
	lpNew->normalMode				= stlWriter_NormalMode__CheckNormals; /* By default we only validate */
	lpNew->state 					= STLWriter_ASCII_State__Solid;
	lpNew->dwBytesWritten			= 0;
	lpNew->callbackError 			= NULL;
	lpNew->callbackErrorParam		= NULL;
	lpNew->lpSolidName				= stlioWriterASCII_SolidName_Default; /* We currently do not accept an solid name */
	lpNew->dwSolidNameLen			= strlen(stlioWriterASCII_SolidName_Default);
	#ifndef WIN32
		lpNew->eolMarkerType		= stlioEndOfLineMarker_LF;
	#else
		lpNew->eolMarkerType		= stlioEndOfLineMarker_CRLF;
	#endif

	(*lpOut) = (struct STLWriter*)lpNew;

	/* Call continuation function */
	e = stlWriter_ASCII__ContinueOp((struct STLWriter*)lpNew);
	if((e != stlioE_Ok) && (e != stlioE_Continues)) {
		lpStreamSink->vtbl->release(lpStreamSink);
		free((void*)lpNew);
		(*lpOut) = NULL;
		return e;
	}
	return e;
}

#ifdef __cplusplus
	} /* extern "C" { */
#endif

/*
	Input/output helper routines
*/

#ifdef __cplusplus
	extern "C" {
#endif

enum stlioError stlioReadFile(
	char* 						lpFilename,
	lpfnSTLCallback_Triangle 	callbackTriangle,
	void*						callbackTriangleParam,
	lpfnSTLCallback_Error	 	callbackError,
	void* 						callbackErrorParam,
	lpfnSTLCallback_EOF		 	callbackEOF,
	void* 						callbackEOFParam,
	enum stlFileType* 			lpDetectedType
) {
	FILE* fHandle;
	unsigned char bByte;
	enum stlioError e;
	enum stlFileType fType;
	struct STLParser* lpParser;
	char bTemp[strlen(strSolid)];
	int appendedLinebreaks;

	if(lpFilename == NULL) { return stlioE_InvalidParam; }

	/*
		Do signature detection
	*/
	fHandle = fopen(lpFilename, "rb");
	if(!fHandle) {
		switch(errno) {
			case EACCES:		return stlioE_PermissionDenied;
			case ENOMEM:		return stlioE_OutOfMemory;
			case ENOENT:		return stlioE_FileNotfound;
			default:			return stlioE_Failed;
		}
	}
	if(fread(bTemp, strlen(strSolid), 1, fHandle) != 1) { fclose(fHandle); return stlioE_InvalidFormat; }

	fclose(fHandle);
	if(memcmp(bTemp, strSolid, strlen(strSolid)) == 0) {
		fType = stlFileType_ASCII;
	} else {
		fType = stlFileType_Binary;
	}
	if(lpDetectedType != NULL) { (*lpDetectedType) = fType; }

	/*
		We always try ASCII first even if it's not the
		preferred type because it has a signature
		("solid " followed by the solid name)
	*/
	if(fType == stlFileType_ASCII){
		/* Open file for binary read */
		fHandle = fopen(lpFilename, "rb");
		if(!fHandle) {
			switch(errno) {
				case EACCES:		return stlioE_PermissionDenied;
				case ENOMEM:		return stlioE_OutOfMemory;
				case ENOENT:		return stlioE_FileNotfound;
				default:			return stlioE_Failed;
			}
		}

		/* Create and initialize parser */
		if((e = stlioParserASCII(&lpParser)) != stlioE_Ok) { return e; }
		lpParser->vtbl->setCallbackTriangle(lpParser, callbackTriangle, callbackTriangleParam);
		lpParser->vtbl->setCallbackError(lpParser, callbackError, callbackErrorParam);
		lpParser->vtbl->setCallbackEOF(lpParser, callbackEOF, callbackEOFParam);

		/* Now read the file (byte wise) */
		for(;;) {
			bByte = (unsigned char)fgetc(fHandle);
			if(feof(fHandle)) {
				/*
					Normally we would never reach end of file except
					"endsolid [SOLIDNAME]" is not followed by a linebreak
					(which is perfectly valid for an STL file). Because of this
					we append both possible linebreak symbols - if this doesn't
					complete the file parsing we have an truncated file.
				*/
				if(appendedLinebreaks == 0) {
					e = lpParser->vtbl->processByte(lpParser, 0x0A);
					appendedLinebreaks = appendedLinebreaks + 1;
				} else if(appendedLinebreaks == 1) {
					e = lpParser->vtbl->processByte(lpParser, 0x0D);
					appendedLinebreaks = appendedLinebreaks + 1;
				} else {
					e = stlioE_InvalidFormat_UnexpectedEnd;
					break;
				}
			} else {
				e = lpParser->vtbl->processByte(lpParser, bByte);
			}

			if(e == stlioE_Done) {
				e = stlioE_Ok;
				break;
			}
			if(e != stlioE_Ok) {
				if(e == stlioE_Continues) { e = stlioE_Failed; }
				break;
			}
		}

		/* Release reader, close file, done */
		lpParser->vtbl->release(lpParser);
		fclose(fHandle);
		return e;
		/* If we reach control here we haven't found the signature */
	} else if(fType == stlFileType_Binary) {
		/* Open file for binary read */
		fHandle = fopen(lpFilename, "rb");
		if(!fHandle) {
			switch(errno) {
				case EACCES:		return stlioE_PermissionDenied;
				case ENOMEM:		return stlioE_OutOfMemory;
				case ENOENT:		return stlioE_FileNotfound;
				default:			return stlioE_Failed;
			}
		}

		/* Create and initialize parser */
		if((e = stlioParserBinary(&lpParser)) != stlioE_Ok) { return e; }
		lpParser->vtbl->setCallbackTriangle(lpParser, callbackTriangle, callbackTriangleParam);
		lpParser->vtbl->setCallbackError(lpParser, callbackError, callbackErrorParam);
		lpParser->vtbl->setCallbackEOF(lpParser, callbackEOF, callbackEOFParam);

		/* Now read the file (byte wise) */
		for(;;) {
			bByte = (unsigned char)fgetc(fHandle);
			if(feof(fHandle)) { e = stlioE_InvalidFormat_UnexpectedEnd; break; }

			e = lpParser->vtbl->processByte(lpParser, bByte);
			if(e == stlioE_Done) { break; }
			if(e != stlioE_Ok) {
				lpParser->vtbl->release(lpParser);
				if(e == stlioE_Continues) { e = stlioE_Failed; }
				break;
			}
		}

		/* Release reader, close file, done */
		lpParser->vtbl->release(lpParser);
		fclose(fHandle);
		return e;
	} else {
		return stlioE_ImplementationError;
	}
}


struct stlioReadFileMem_State {
	unsigned char*				lpBuffer;

	unsigned long int			dwTriangleCount;
	unsigned long int			dwTrianglesRead;
	unsigned long int			dwStrideSize;
};

static enum stlioError stlioReadFileMem__Callback__TriangleCount(
	struct stlTriangle* lpTriangle,
	void* lpFreeParam
) {
	struct stlioReadFileMem_State* state = (struct stlioReadFileMem_State*)lpFreeParam;

	state->dwTriangleCount = state->dwTriangleCount + 1;
	if(state->dwStrideSize < sizeof(struct stlTriangle)+(lpTriangle->attributeByteCount)) {
		state->dwStrideSize = sizeof(struct stlTriangle)+(lpTriangle->attributeByteCount);
	}

	return stlioE_Ok;
}
static enum stlioError stlioReadFileMem__Callback__TriangleRead(
	struct stlTriangle* lpTriangle,
	void* lpFreeParam
) {
	union {
		unsigned char* 					lpOff;
		struct stlTriangle_Attributes* 	tri;
	} buf;
	struct stlioReadFileMem_State* state = (struct stlioReadFileMem_State*)lpFreeParam;

	buf.lpOff = (unsigned char*)(((uintptr_t)state->lpBuffer) + state->dwTrianglesRead*state->dwStrideSize);
	memcpy(buf.lpOff, lpTriangle, sizeof(struct stlTriangle)+lpTriangle->attributeByteCount);
	state->dwTrianglesRead = state->dwTrianglesRead + 1;
	return stlioE_Ok;
}

enum stlioError stlioReadFileMem(
	char*						lpFilename,
	struct stlTriangle**		lpTriangles,
	unsigned long int*			lpTriangleCountOut,
	unsigned long int*			lpStrideSizeOut,
	lpfnSTLCallback_Error	 	callbackError,
	void* 						callbackErrorParam,
	enum stlFileType* 			lpDetectedType
) {
	enum stlioError e;
	struct stlioReadFileMem_State	stateBlock;
	/*
		TODO: Currently this is the most inefficient implementation that is
			possible - the files are read TWICE. Once to calculate the
			required buffer size and once to read the data into memory
	*/
	if((lpFilename == NULL) || (lpTriangles == NULL) || (lpTriangleCountOut == NULL) || (lpStrideSizeOut == NULL)) { return stlioE_InvalidParam; }

	/*
		Pass one: Count triangles AND determine stride size
	*/
	stateBlock.lpBuffer 			= NULL;
	stateBlock.dwTriangleCount		= 0;
	stateBlock.dwTrianglesRead 		= 0;
	stateBlock.dwStrideSize 		= sizeof(struct stlTriangle);
	e = stlioReadFile(
		lpFilename,
		&stlioReadFileMem__Callback__TriangleCount,
		(void*)(&stateBlock),
		callbackError,
		callbackErrorParam,
		NULL,
		NULL,
		lpDetectedType
	);
	if((e !=  stlioE_Ok) && (e != stlioE_Done)) { return e; }

	/*
		Pass two: Really read triangles
	*/
	stateBlock.lpBuffer = (unsigned char*)malloc(stateBlock.dwStrideSize * stateBlock.dwTriangleCount);
	if(stateBlock.lpBuffer == NULL) { return stlioE_OutOfMemory; }

	e = stlioReadFile(
		lpFilename,
		&stlioReadFileMem__Callback__TriangleRead,
		(void*)(&stateBlock),
		callbackError,
		callbackErrorParam,
		NULL,
		NULL,
		lpDetectedType
	);
	if((e != stlioE_Ok) && (e != stlioE_Done)) { free((void*)(stateBlock.lpBuffer)); return e; }

	(*lpTriangles) = (struct stlTriangle*)(stateBlock.lpBuffer);
	(*lpTriangleCountOut) = stateBlock.dwTrianglesRead;
	(*lpStrideSizeOut) = stateBlock.dwStrideSize;
	return stlioE_Ok;
}


struct stlioWriteFileMem__State {
	struct STLStreamSink		sinkObject;

	FILE*						fHandle;
};

static enum stlioError stlioWriteFileMem__StreamSink__WriteBytes(
	struct STLStreamSink* 		lpSelf,
	unsigned char* 				lpData,
	unsigned long int 			dwByteCount,
	unsigned long int* 			lpBytesWritten
) {
	struct stlioWriteFileMem__State* lpThis;
	size_t res;

	if((lpSelf == NULL) || ((lpData == NULL) && (dwByteCount > 0))) { return stlioE_InvalidParam; }
	if(dwByteCount == 0) {
		if(lpBytesWritten != NULL) { (*lpBytesWritten) = 0; }
		return stlioE_Ok;
	}

	lpThis = (struct stlioWriteFileMem__State*)lpSelf->lpReserved;

	if(lpThis->fHandle == NULL) { return stlioE_InvalidState; }

	res = fwrite(lpData, 1, dwByteCount, lpThis->fHandle);

	if(res <= 0) {
		if(lpBytesWritten != NULL) { (*lpBytesWritten) = 0; }
		return stlioE_IOError;
	}

	if(lpBytesWritten != NULL) {
		(*lpBytesWritten) = res;
		return (dwByteCount == res) ? stlioE_Ok : stlioE_Continues;
	} else {
		return (dwByteCount == res) ? stlioE_Ok : stlioE_IOError;
	}
}
static enum stlioError stlioWriteFileMem__StreamSink__SeekAbsolute(
	struct STLStreamSink* 		lpSelf,
	unsigned long int			dwOffset
) {
	struct stlioWriteFileMem__State* lpThis;
	if(lpSelf == NULL) { return stlioE_InvalidParam; }
	lpThis = (struct stlioWriteFileMem__State*)lpSelf->lpReserved;

	if(lpThis->fHandle == NULL) { return stlioE_InvalidState; }

	if(fseek(lpThis->fHandle, (signed long int)dwOffset, SEEK_SET) < 0) { return stlioE_IOError; } else { return stlioE_Ok; }
}
static enum stlioError stlioWriteFileMem__StreamSink__SeekRelative(
	struct STLStreamSink* 		lpSelf,
	signed long int				dwOffset
) {
	struct stlioWriteFileMem__State* lpThis;
	if(lpSelf == NULL) { return stlioE_InvalidParam; }
	lpThis = (struct stlioWriteFileMem__State*)lpSelf->lpReserved;

	if(lpThis->fHandle == NULL) { return stlioE_InvalidState; }

	if(fseek(lpThis->fHandle, dwOffset, SEEK_CUR) < 0) { return stlioE_IOError; } else { return stlioE_Ok; }
}
static enum stlioError stlioWriteFileMem__StreamSink__Release(
	struct STLStreamSink* 		lpSelf
) {
	struct stlioWriteFileMem__State* lpThis;
	if(lpSelf == NULL) { return stlioE_InvalidParam; }
	lpThis = (struct stlioWriteFileMem__State*)lpSelf->lpReserved;

	if(lpThis->fHandle != NULL) { fclose(lpThis->fHandle); lpThis->fHandle = NULL; }
	/*
		If we WOULD be an structure allocated with malloc we would now free
		but this implementation uses stack-located objects ...
	*/

	return stlioE_Ok;
}
static struct STLStreamSink_vtbl stlioWriteFileMem__StreamSink__VTBL = {
	&stlioWriteFileMem__StreamSink__Release,

	&stlioWriteFileMem__StreamSink__WriteBytes,
	&stlioWriteFileMem__StreamSink__SeekAbsolute,
	&stlioWriteFileMem__StreamSink__SeekRelative
};

enum stlioError stlioWriteFileMem(
	char*							lpFilename,
	struct stlTriangle*				lpTriangles,
	unsigned long int				dwTriangleCount,
	unsigned long int				dwStrideSize,
	lpfnSTLWriter_Callback_Error	callbackError,
	void* 							callbackErrorParam,
	enum stlFileType 				fileType
) {
	struct stlioWriteFileMem__State state;
	struct STLWriter* lpWriter;
	enum stlioError e;

	if((lpFilename == NULL) || ((lpTriangles == NULL) && (dwTriangleCount > 0)) ||((fileType != stlFileType_ASCII) && (fileType != stlFileType_Binary))) { return stlioE_InvalidParam; }

	/* Open file */
	state.fHandle = fopen(lpFilename, "w+b");
	if(!state.fHandle) {
		switch(errno) {
			case EACCES:		return stlioE_PermissionDenied;
			case ENOMEM:		return stlioE_OutOfMemory;
			case ENOENT:		return stlioE_FileNotfound;
			default:			return stlioE_Failed;
		}
	}

	/* Initialize stream sink structure */
	state.sinkObject.lpReserved = (void*)(&state);
	state.sinkObject.vtbl = &stlioWriteFileMem__StreamSink__VTBL;

	/* Create writer */
	switch(fileType) {
		case stlFileType_ASCII:		e = stlioWriterASCII(&lpWriter, &(state.sinkObject)); break;
		case stlFileType_Binary:	e = stlioWriterBinary(&lpWriter, &(state.sinkObject)); break;
		default:					return stlioE_ImplementationError; /* In this case we would have made a mistake at input parameter validation ... */
	}

	for(;;) {
		if(e == stlioE_Ok) { break; }
		if(e == stlioE_Continues) { e = lpWriter->vtbl->continueOp(lpWriter); continue; }

		lpWriter->vtbl->release(lpWriter);
		if(state.fHandle != NULL) { fclose(state.fHandle); state.fHandle = NULL; }
		return e;
	}

	/* Setup error callback */
	lpWriter->vtbl->setCallbackError(lpWriter, callbackError, callbackErrorParam);

	/* Now write all triangles to output ... */
	e = lpWriter->vtbl->writeTriangles(
		lpWriter,
		lpTriangles,
		dwTriangleCount,
		dwStrideSize,
		NULL
	);

	if(e == stlioE_Ok) {
		e = lpWriter->vtbl->finalize(lpWriter);

		while(e == stlioE_Continues) { e = lpWriter->vtbl->continueOp(lpWriter); }
	}

	/* Cleanup */
	lpWriter->vtbl->release(lpWriter);
	if(state.fHandle != NULL) { fclose(state.fHandle); state.fHandle = NULL; }
	return (e == stlioE_Done) ? stlioE_Ok : e;
}


#ifdef __cplusplus
	/* extern "C" { */
#endif
