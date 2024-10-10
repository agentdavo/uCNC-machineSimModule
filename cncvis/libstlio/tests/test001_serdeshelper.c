#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include "../include/serdeshelper.h"

static float fTestValues[16] = {
	1.0f,
	-1.0f,
	12e3f,
	12e-3f,
	0.0f,
	13e-12f,
	1e-36f,
	1e-37f,
	1e-38f,
	1e-39f,
	0.1e-40f,
	1e10,
	INFINITY,
	0.0/0.0,
	1.0/0.0,
	-1.0/0.0
};

int main(int argc, char* argv[]) {
	uint8_t bFailed;
	unsigned long int i;
	uint8_t bData[4];
	float fDataOut;

	bFailed = 0;

	printf("Serializer and deserializer helper test\n");
	printf("Testing floating point serdes:\n");

	for(i = 0; i < sizeof(fTestValues)/sizeof(float); i=i+1) {
		printf("[%02lu] SerDes %s %f \t-> ", i, (isnormal(fTestValues[i]) == 0) ? "denorm" : "normal", fTestValues[i]);
		serializeIEEE754SingleFloat_4Bytes(fTestValues[i], bData);
		printf("%02x %02x %02x %02x \t-> ", bData[0], bData[1], bData[2], bData[3]);
		fDataOut = deserializeIEEE754SingleFloat_4Bytes(bData);
		printf("%f ", fDataOut);

		if(isnan(fDataOut) && isnan(fTestValues[i])) {
			printf("(ok) ");
		} else if(fDataOut != fTestValues[i]) {
			printf("(failed) ");
			bFailed = 1;
		} else {
			printf("(ok) ");
		}

		/* Compare with the "cast trick" result but do not include in success / failure */
		fDataOut = ((float*)(bData))[0];
		printf(" (cast deserializer: %f - %s)\n", fDataOut, ((fDataOut == fTestValues[i]) || (isnan(fDataOut) && (isnan(fTestValues[i])))) ? "ok" : "failed");
	}

	if(bFailed == 0) {
		printf("\nAll successful\n");
	} else {
		printf("\nSome tests failed\n");
	}

	return bFailed;
}
