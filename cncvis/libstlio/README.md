# STL Input/Output library

This is a small ANSI C99 library that provides basic input/output primitives
for STL (Stereo lithography) files. It has been built around the specification
at http://www.fabbers.com/tech/STL_Format

The library (libstlio) provides functions to

* Read STL files into memory
* Write a list of triangles from memory to STL files
* Read an STL data stream with an iterator callback
* Write a stream of triangles into a datastream

It supports binary and ASCII STL files. It also tries to validate some
basic properties of the file - for example to check if an outer loop
really describes an triangle or if all vertices are the element of
one line or if the direction of the surface normal matches the counter
clockwise orientation of the vertices. If these conditions are violated
the application can use an error callback to decide what to do (keep invalid
data, discard the triangle or in case of surface normals reorder vertices
or recalculate the surface normal).

The library also supports reading binary files with attributes.

## Reading an STL file into memory

The simplest way to read an STL file into memory is usage of the
_stlioReadFileMem_ function. It handles all internal object allocations
and destructions - and uses the libc's fopen, fgetc, fseek and fclose
functions to access the file.

```
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
	unsigned long int dwTriCount;
	unsigned long int dwStride;
	enum stlioError e;
	enum stlFileType fType;

	if(argc < 2) {
		printf("Missing filename\n");
		return -1;
	}

	e = stlioReadFileMem(
		argv[1],
		&(buf.lpTri),
		&dwTriCount,
		&dwStride,
		&callbackError,
		NULL, /* Free parameter */
		&fType
	);
	printf("Read returned %s (%u)\n", stlioErrorStringC(e), e);
	switch(fType) {
		case stlFileType_ASCII:		printf("File type: ASCII\n"); break;
		case stlFileType_Binary:	printf("File type: binary\n"); break;
		default:					break;
	}
	printf("Read %lu triangles with stride size %lu\n", dwTriCount, dwStride);

	/* Dumping triangles */
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
```

## Writing an STL file from memory

This is the counterparts to reading a file into memory. It requires a different
error callback

```
static enum stlioError callbackWriterError(
	enum stlioError eCode,
	struct stlTriangle* lpTriangle,
	void* lpFreeParam
) {
	/*
		Note that the callback is allowed to modify
		the triangle - the triangle is NOT checked
		again after the error callback has made
		it's decision.
	*/
	printf("%s:%u Writer encountered error %u: %s\n", __FILE__, __LINE__, eCode, stlioErrorStringC(eCode));
	return stlioE_Keep;
}
```

The triangles have to be either contained in an array with fixed
stride size (then dwStride should be not zero) or they could be
a concatenation of _stlTriangle_Attributes_ structures with variable
attribute byte length. The _fType_ variable selects the type
of the STL file (_stlFileType_ASCII_ or _stlFileType_Binary_).
Note that only binary files support attribute bytes - and most
applications don't understand files with an attribute byte count
different from zero.

```
e = stlioWriteFileMem(
	argv[2],
	buf.lpTri,
	dwTriCount,
	dwStride,
	&callbackWriterError,
	NULL,
	fType
);
```

## Reading an STL stream with callback functions

Reading using the callback functions supports three optional callbacks:

- The triangle callback that is called for each decoded triangle
- The error callback that's always called whenever an decoding error
  happens. In this case the error callback may be capable of correcting
  the error (in case of mismatch of vertex normals or loops which describe
  a line), deciding to keep invalid data or to skip triangles.
- An end of file callback that allows releasing resources, etc.

```
static enum stlioError callbackTriangle(
	struct stlTriangle* lpTriangle,
	void* lpFreeParam
) {
	/* Do whatever you want with that triangle */
	return stlioE_Ok;
}
```

```
static enum stlioError callbackError(
	enum stlioError eCode,
	unsigned long int dwLineNumber,
	unsigned long int dwLineChar,
	unsigned long int dwByteOffset,
	void* lpFreeParam
) {
	printf("Parser encountered error %u at line %lu:%lu or offset %lu\n", eCode, dwLineNumber, dwLineChar, dwByteOffset);
	return stlioE_Keep; /* We keep  the invalid data, no repair */
}
```

```
static enum stlioError callbackEOF(
	void* lpFreeParam
) {
	printf("End of file reached (EOF callback)\n");
	return stlioE_Ok;
}
```

To decode a file one has to create a parser object first:

```
struct STLParser* lpParser;
e = stlioParserBinary(&lpParser);
```

or

```
struct STLParser* lpParser;
e = stlioParserASCII(&lpParser);
```

Then one has to setup all callbacks:

```
/* Set callbacks */
lpParser->vtbl->setCallbackTriangle(lpParser, &callbackTriangle, NULL);
lpParser->vtbl->setCallbackError(lpParser, &callbackError, NULL);
lpParser->vtbl->setCallbackEOF(lpParser, &callbackEOF, NULL);
```

After that one can push bytes into the parser object that have been
read from any data source (file, network, etc.):

```
e = lpParser->vtbl->processByte(lpParser, bByte);
```

This function should either return _stlioE_Ok_ in case it can accept
more data or _stlioE_Done_ whenever the processing of the STL file has
finished.

At the end the parser has to be released:

```
lpParser->vtbl->release(lpParser)
```

## Writing an triangle stream into a stream sink

Writing via the STLWriter object is done by using a StreamSink object to
allow writing into various data sinks like files, network streams, etc.

The stream sink has to:

- Support writing bytes via _writeBytes_
- Support seeking to an absolute position via _seekAbsolute_
- Relative seeking via _seekRelative_
- Releasing itself via _release_

An example implementation using the libc's function can be seen below:

```
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
```

The sink also supports an error callback:

```
static enum stlioError STLWriter_Callback_Error(
	enum stlioError eCode,
	struct stlTriangle* lpTriangle,
	void* lpFreeParam
) {
	return stlioE_Keep;
}
```

One can then instantiate a writer for binary or ASCII file and
set the appropriate optional error callback:

```
struct STLWriter* lpWriter;
e = stlioWriterBinary(&lpWriter, &streamSink);
lpWriter->vtbl->setCallbackError(lpWriter, &STLWriter_Callback_Error, NULL);
```

```
struct STLWriter* lpWriter;
e = stlioWriterASCII(&lpWriter, &streamSink);
lpWriter->vtbl->setCallbackError(lpWriter, &STLWriter_Callback_Error, NULL);
```

After that one can output an abitrary number of triangles into the
writer object:

```
union {
	struct stlTriangle_Attributes* lpTriangle;
	struct stlTriangle_Attributes* lpAttributedTriangle;
} lpTriangle;
e = lpWriter->vtbl->writeTriangle(lpWriter, lpTriangle.lpTriangle);
```

At the end one has to call _finalize_. This either writes the ASCII
_endsolid_ statement or seeks to the front and writes the total number
of triangles contained in an binary file.

```
lpWriter->vtbl->finalize(lpWriter);
```

At the end one has to release the resources again:

```
lpParser->vtbl->release(lpParser);
```

## The object oriented pattern used in this project

Throughout the whole project an object oriented pattern is used that
also supporty polymorphism. Objects are described by data structures
and function pointer tables (_vtbl_). This resembles the way most
current C++ compilers implement objects internally. The first argument
of each member function also points to the structure that defines
the object's type. This structure always consists of two pointers,
one to an _vtbl_ that has functions specific to the given object type
and an reserved pointer used internally.

```
struct Object {
	struct Object_VTBL* vtbl;
	void* lpReserved;
};
```

The internal representation may be different but is never leaked into
code outside the particular module - an internal representation can
implement multiple different interfaces / object types (i.e. supports
polymorphism).

```
struct ObjectInternal {
	...

	struct Object		objObject;

	...
};
```

The _lpReserved_ pointer always points to the base of the internal
structure that describes the object. If an object only implements one
interface or one set of functions it can point to the base of
the _struct Object_. _vtbl_ points to a table of function pointers.

```
typedef int (*lpfnFunctionA)(
	struct Object* lpSelf,
	...
);
typedef int (*lpfnFunctionB)(
	struct Object* lpSelf,
	...
);
struct Object_VTBL {
	lpfnFunctionA functionA;
	lpfnFunctionB functionB;
};
```

This allows extending an object type by simply subclassing the function
table:

```
typedef ObjectB_VTBL {
	struct Object_VTBL base;
	lpfnFunctionC functionC;
	lpfnFunctionC functionD;
}
```

Because ANSI C allows downcasting a structure to it's first member
this allows the typical downcast to a lower object type.

The member functions typically first recover the pointer to the object's
internal structure:

```
static int implementationFunctionA(
	struct Object* lpSelf,
	...
) {
	struct ObjectInternal* lpThis;

	if(lpSelf == NULL) {
		/* Handle that error */
	}

	lpThis = (struct ObjectInternal*)(lpSelf->lpReserved);

	...
}
```

## Tools included

There are currently two tools included that allow conversion from
STL files into binary or ASCII format. They are contained in the
tools directory and do not support any options yet.

One can use these tool to convert from any supported STL file format
into the ASCII or binary STL format:

```
stl2ascii source.stl ascii.stl
stl2bin source.stl ascii.stl
```
