#ifndef __is_included__F0CBD5F3_AC0B_4F2F_9137_1D3A922083A7
#define __is_included__F0CBD5F3_AC0B_4F2F_9137_1D3A922083A7 1

/*
	Requires stdint.h and math.h
*/

/*
	IEEE 754 Floats (Single):
		All Bits		1+r+p		32 Bit
		Exponent 		r			8  Bit		-126 < e < 127 (Bias 127)		Bits 23-30
		Mantissa		p			23 Bit										Bits 0-22
		Sign																	Bit 31
*/
static void serializeIEEE754SingleFloat_4Bytes(float dataIn, unsigned char* dataOut) {
	double dExponent;
	double dMantissa;
	double dData;
	double denormalizedBoundary;
	uint8_t bSign;
	uint32_t bExponent;
	uint32_t bMantissa;

	denormalizedBoundary = 1;
	for(dExponent = 126; dExponent > 0; dExponent = dExponent - 1) { denormalizedBoundary = denormalizedBoundary / 2; }

	/* Special cases ... */
	if(isnan(dataIn)) {
		/* NaN */
		bSign = 0;
		bExponent = 0xFF;
		bMantissa = 1;
	} else if(isinf(dataIn)) {
		/* +- INFINIY */
		bSign = (dataIn > 0) ? 0 : 1;
		bExponent = 0xFF;
		bMantissa = 0;
	} else if((dataIn == 0.0) || (dataIn == -0.0)) {
		bMantissa = 0;
		bExponent = 0;
		bSign = (dataIn == -0.0) ? 1 : 0;
	} else if((dataIn <= denormalizedBoundary) && (dataIn >= -denormalizedBoundary)) {
		/* Subnormal / denormalized */

		/* Expand to double */
		dMantissa = dataIn;

		dExponent = 126+23;
		while(dExponent > 0) { dMantissa = dMantissa * 2.0; dExponent = dExponent - 1; }

		bSign = (dMantissa < 0) ? 1 : 0;
		bExponent = 0;
		bMantissa = (uint32_t)dMantissa;
	} else {
		/* Normalized number */

		/* Expand to double */
		dData = dataIn;

		/* Determine sign */
		bSign = (dData < 0) ? 1 : 0;

		/* We always work with positive data ... */
		if(dData < 0) {
			dData = dData * -1.0;
		}

		/* Determine exponent */
		dExponent = 0;

		/* If exponent is positive, count */
		while(dData >= 2.0) {
			dData = dData / 2.0;
			dExponent = dExponent + 1.0;
		}
		while(dData < 1) {
			dData = dData * 2.0;
			dExponent = dExponent - 1.0;
		}

		/* Bias exponent */
		dExponent = dExponent + 127.0;
		bExponent = (uint32_t)dExponent;

		/* Now create mantissa bits */
		dMantissa = dData - 1.0;
		bMantissa = (uint32_t)(dMantissa*8388608);

		/* Apply bitmasks */
		bMantissa = bMantissa & 0x007FFFFF;
		bExponent = bExponent & 0xFF;
		bSign = bSign & 0x01;
	}

	/* Write output */
	dataOut[0] = (uint8_t)((bMantissa >> 0) & 0xFF);
	dataOut[1] = (uint8_t)((bMantissa >> 8) & 0xFF);
	dataOut[2] = (uint8_t)((bMantissa >> 16) & 0x7F) | (uint8_t)((bExponent << 7) & 0x80);
	dataOut[3] = (uint8_t)((bExponent >> 1) & 0x7F) | (uint8_t)((bSign << 7) & 0x80);
}
static float deserializeIEEE754SingleFloat_4Bytes(unsigned char* dataIn) {
	uint8_t bSign;
	uint32_t bMantissa;
	uint32_t bExponent;

	double dMantissa;
	double dExponent;
	/*
		First fetch sign, mantissa and exponent from bytes
	*/

	bSign = ((dataIn[3] & 0x80) >> 7);
	bMantissa = ((((uint32_t)(dataIn[0])) << 0) & 0x000000FF) |
		((((uint32_t)(dataIn[1])) << 8) & 0x0000FF00) |
		((((uint32_t)(dataIn[2])) << 16) & 0x007F0000);
	bExponent = ((((uint32_t)(dataIn[2])) >> 7) & 0x00000001) |
		((((uint32_t)(dataIn[3])) << 1) & 0x000000FE);

	/* Convert bits to double */
	dMantissa = (double)bMantissa;
	dExponent = (double)bExponent;

	/* Case discrimination */
	if(bExponent == 0) {
		/* NULL or denom */
		if(bMantissa == 0) {
			/* NULL */
			if(bSign == 0) {
				return 0.0;
			} else {
				return -0.0;
			}
		}
		/* Denormalized form */
		dExponent = 126+23;
		while(dExponent > 0) { dMantissa = dMantissa / 2.0; dExponent = dExponent - 1; }
		return (bSign == 0) ? dMantissa : -1.0*dMantissa;
	}

	if(bExponent == 0xFF) {
		/* Infinite or NaN */
		if(bMantissa == 0) {
			/* +- infinite */
			if(bSign == 0) {
				#ifdef INFINITY
					return INFINITY;
				#else
					return 1.0f/0.0f;
				#endif
			} else {
				#ifdef INFINITY
					return -1.0*INFINITY;
				#else
					return -1.0/0.0;
				#endif
			}
		} else {
			/* NaN */
			#ifdef NAN
				return NAN;
			#else
				return 0.0 / 0.0; /* This is IND on some implementations and not NAN but there seems to be no other way to relieable produce NaNs */
			#endif
		}
	}

	/* Normalized number */
	dExponent = dExponent - 127; /* Bias */
	dMantissa = 1.0 + dMantissa / 8388608; /* 1 + M/2^p */
	if(dExponent > 0) {
		while(dExponent > 0) {
			dMantissa = dMantissa * 2.0;
			dExponent = dExponent - 1;
		}
		return (bSign == 0) ? dMantissa : -1.0*dMantissa;
	} else if(dExponent < 0) {
		while(dExponent < 0) {
			dMantissa = dMantissa / 2.0;
			dExponent = dExponent + 1;
		}
		return (bSign == 0) ? dMantissa : -1.0*dMantissa;
	} else {
		return (bSign == 0) ? dMantissa : -1.0*dMantissa;
	}
}


#endif
