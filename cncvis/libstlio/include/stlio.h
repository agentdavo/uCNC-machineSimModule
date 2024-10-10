#ifdef __cplusplus
	extern "C" {
#endif

/*
	stlTriangle is the data structure that gets stored
	inside an STL file.

	The surfaceNormal is (normally) redundant information.
	Depending on the reader and writer configuration
	consistency between the surface normal and vertices
	is checked.

	Vertices have to be ordered in counter-clockwise
	order. This is validated via the right hand rule
	and the direction of the surface normal.

	Triangles can have additional attribute bytes (up
	to 65536). This is only supported in binary STL
	format - and most programs don't expect any attribute
	bytes attached and either fail to read the file,
	crash or output garbage. Note that the format and
	interpretation of attribute bytes is implementation
	specific.

	To allow easier access to attribute bytes the
	struct stlTriangle_Attributes structure is
	derived from struct stlTriangle (i.e. one can up-
	and downcast between them according to the C
	language specification without encountering aliasing
	errors).
*/
struct stlTriangle {
	double				surfaceNormal[3];
	double				vertices[3][3];

	unsigned long int	attributeByteCount;
};

struct stlTriangle_Attributes {
	struct stlTriangle	base;
	unsigned char		attributeBytes[];
};

enum stlioError {
	stlioE_Ok,										/* The operation has been successful */
	stlioE_Done,									/* The operation has been successful and has finished (for example a whole file has been encoded) */
	stlioE_Continues,								/* The operation has been interrupted and can be continued later on (by the continue member function of the specific object for example) */
	stlioE_Failed,									/* Generic failure code (no specific cause) */

	stlioE_FileNotfound,							/* A file thas should be opened has not been found */
	stlioE_PermissionDenied,						/* Access to an resource (like a file) has been denied because of policy */
	stlioE_OutOfMemory,								/* Memory allocation failed */
	stlioE_IOError,									/* An I/O resource (like an file) could not be written */

	stlioE_InvalidParam,							/* One of the functions parameters had been invalid */
	stlioE_InvalidState,							/* The object that has been called has been in an invalid state for the requested operation */

	stlioE_InvalidFormat,							/* The STL file or the passed triangle has an invalid format */
	stlioE_InvalidFormat_InvalidCharacter,			/* An invalid character has been encountered in the ASCII STL file */
	stlioE_InvalidFormat_NotAFloat,					/* A number inside an ASCII STL file has not been a valid floating point number */
	stlioE_InvalidFormat_SolidNameMismatch,			/* The names specified at "solid" and "endsolid" in an ASCII STL file did not match */
	stlioE_InvalidFormat_TokenTooLong,				/* An token that is currently being read has been too long (normally indicates an invalid format) */
	stlioE_InvalidFormat_Expect_Solid,				/* ASCII STL: The next expected token would be the literal "solid" */
	stlioE_InvalidFormat_Expect_Facet,				/* ASCII STL: The next expected token would be the literal "facet" */
	stlioE_InvalidFormat_Expect_Normal,				/* ASCII STL: The next expected token would be the literal "normal" */
	stlioE_InvalidFormat_Expect_FloatNumber,		/* ASCII STL: The next expected token would be an floating point number */
	stlioE_InvalidFormat_Expect_Outer,				/* ASCII STL: The next expected token would be "outer" */
	stlioE_InvalidFormat_Expect_Loop,				/* ASCII STL: The next expected token would be "loop" */
	stlioE_InvalidFormat_Expect_Vertex,				/* ASCII STL: The next expected token would be "vertex" */
	stlioE_InvalidFormat_Expect_Endloop,			/* ASCII STL: The next expected token would be "endloop" */
	stlioE_InvalidFormat_Expect_Endfacet,			/* ASCII STL: The next expected token would be "endfacet" */
	stlioE_InvalidFormat_Expect_FacetOrEndloop,		/* ASCII STL: The next expected token would be "facet" or "endloop" */
	stlioE_InvalidFormat_VertexNormalMismatch,		/* The direction of the supplied vertex normal does not match the plane defined by the vertices (in counter-clockwise order) */
	stlioE_InvalidFormat_NotATriangle,				/* The passed object is not a triangle - for example all 3 points are lying on the same line */
	stlioE_InvalidFormat_AttributesNotSupported,	/* Attributes have been passed to an ASCII writer. Attributes are not supported for an ASCII writer */
	stlioE_InvalidFormat_UnexpectedEnd,				/* Unexpected end of file */

	stlioE_Skip,									/* Used in error callbacks: Skip the errornous triangle */
	stlioE_Keep,									/* Used in error callbacks: Keep the triangle with invalid data */
	stlioE_RepairNormal,							/* Used in error callback if VertexNormalMismatch - the library should re-calculate the normal */
	stlioE_RepairOrder,								/* Used in error callback if VertexNormalMismatch - the library should reverse the order of all vertices */

	stlioE_Abort,									/* Requests abort of an operation */
	stlioE_ImplementationError,						/* Should never happen. If it happens there is an error in this library (for example an undefined or unhandeled state, etc.) */

	stlioE_ValidationError_UnsharedVertices,		/* Used by the STL validator (if enabled). There exists an vertex that is not shared by another triangle - that means that this STL is not a manifold */
	stlioE_ValidationError_VertexEdgeIntersection,	/* Used by the STL validator (if enabled). There exists an edge of a triangle that contains another vertex which is not allowed by the STL format specification */
};

enum stlFileType {
	stlFileType_ASCII,
	stlFileType_Binary
};

enum stlioEndOfLineMarker {
	stlioEndOfLineMarker_LF				= 0,		/* Default; Unix, Linux, Android, macOS, etc. */
	stlioEndOfLineMarker_CRLF			= 1,		/* Used on MS Windows, DOS, OS/2, CP/M, ... */
	stlioEndOfLineMarker_CR				= 2,		/* Used on macOS before version 10, C64, ... */
};

/*
	Error helper function

	This converts an enum stlioError error code into an
	human readable ASCII string (if STLIO_ERROR_STRINGS_ENABLED
	has been set during compile time; if not it always
	returns the same string)
*/
const char* stlioErrorStringC(enum stlioError e);

/*
	Forward declarations
*/
struct STLParser;
struct STLStreamSink;

/*
	===========================
	Reader Callback definitions
	===========================
*/

/*
	Triangle callback

	This is called by all readers whenever a triangle
	has been sucessully decoded. Note that the
	passed triangle is only valid until the end of the
	function.

	lpFreeParam is a parameter that can be freely choosen
	by the application during registration of the callback
*/
typedef enum stlioError (*lpfnSTLCallback_Triangle)(
	struct stlTriangle* lpTriangle,
	void* lpFreeParam
);
/*
	Error callback

	Called on all parsing- or format errors. The
	error code is passed in eCode and either
	the line number dwLineNumber and position dwLineChar
	(for ASCII files) or the binary offset (for binary files).

	To distinguish if linenumber and character or byte offset
	are set one can simply check if either offset is not
	zero (then it's the byte offset into the binary file),
	the offset is zero then one can use linenumber and linechar
	inside and ASCII file or all values are zero then the error
	happened at the beginning of the file.

	lpFreeParam is a parameter that can be freely choosen
	by the application during registration of the callback
*/
typedef enum stlioError (*lpfnSTLCallback_Error)(
	enum stlioError eCode,
	unsigned long int dwLineNumber,
	unsigned long int dwLineChar,
	unsigned long int dwByteOffset,
	void* lpFreeParam
);
/*
	End of file callback

	Called whenever the end of a source file has
	been reached (can be used to close the file
	in an event based file source for example)
*/
typedef enum stlioError (*lpfnSTLCallback_EOF)(
	void* lpFreeParam
);


/*
	====================
	STL Parser Interface
	====================

	The STL parser interface is universal for all kinds
	of STL files (binary or ASCII). It basically is an
	interface to an parser that can process input data
	bytewise - there are additional functions that allow
	multiple bytes or ranges to be passed (in arbitrary
	sizes) or allow possibly more efficient processing
	of a mapped file.

	Currently these routines do not provide performance
	gain but they allow to reduce complexity of calling
	code.
*/

/*
	Process byte

	Processes a single byte.
*/
typedef enum stlioError (*lpfnSTLParser_ProcessByte)(
	struct STLParser* lpSelf,
	unsigned char bData
);
/*
	Process bytes

	Processes a whole block of bytes. If lpBytesProcessedOut
	is not NULL the function returns the number of processed
	bytes and may also successfully abort in case not all
	data has been processed. If the parameter is NULL and
	an abort or blocking condition (stlioE_Continues) occurs
	the function fails (because restarting would not be possible
	without knowing the last processed position).

	The function does not take ownership of the passed
	data block and does not keep any references
*/
typedef enum stlioError (*lpfnSTLParser_ProcessBytes)(
	struct STLParser* lpSelf,
	const unsigned char* lpData,
	unsigned long int dwBlockSize,
	unsigned long int* lpBytesProcessedOut
);
/*
	Process a whole mapped file

	This function should only be called once for the complete
	memory mapped file. Depending on the parser implementation
	it allows the parser to run directly without caching
	any data in it's own buffers.
*/
typedef enum stlioError (*lpfnSTLParser_ProcessMappedFile)(
	struct STLParser* lpSelf,
	unsigned char* lpData,
	unsigned long int dwDataLength
);
/*
	Callback setting functions.

	These allow setting function pointers for all three
	supported callbacks (triangle, error and EOF). All callback
	can accept an "free parameter" that is opaque to the
	stlio library - it simply gets passed back to the
	application (for example to keep a reference to a structure
	that describes the current operation in an event based
	application).

	To unset an callback simply set it to NULL.

	All callbacks are optional.
*/
typedef enum stlioError (*lpfnSTLParser_SetCallback_Triangle)(
	struct STLParser* lpSelf,
	lpfnSTLCallback_Triangle callbackTriangle,
	void* lpFreeParam
);
typedef enum stlioError (*lpfnSTLParser_SetCallback_Error)(
	struct STLParser* lpSelf,
	lpfnSTLCallback_Error callbackError,
	void* lpFreeParam
);
typedef enum stlioError (*lpfnSTLParser_SetCallback_EOF)(
	struct STLParser* lpSelf,
	lpfnSTLCallback_EOF callbackEOF,
	void* lpFreeParam
);
/*
	Release the parser

	This releases all resources associated with the parser.
	All pointers to STLParser are invalid after this call.
*/
typedef enum stlioError (*lpfnSTLParser_Release)(
	struct STLParser* lpSelf
);

/*
	STL Parser object declarations. See description
	of the implementation of the object oriented model
	for details. lpReserved is used internally by the
	library to recover an reference to the internal
	state block even if an object is polymorphic.
*/
struct STLParser_Vtbl {
	lpfnSTLParser_Release					release;

	lpfnSTLParser_ProcessByte				processByte;
	lpfnSTLParser_ProcessBytes				processBytes;
	lpfnSTLParser_ProcessMappedFile			processMappedFile;

	lpfnSTLParser_SetCallback_Triangle		setCallbackTriangle;
	lpfnSTLParser_SetCallback_Error			setCallbackError;
	lpfnSTLParser_SetCallback_EOF			setCallbackEOF;
};
struct STLParser {
	struct STLParser_Vtbl*					vtbl;
	void*									lpReserved;
};

/*
	=======================
	Factories for STLParser
	=======================

	These two exported routines allow instantiation of ASCII
	and binary STL parsers.
*/

enum stlioError stlioParserASCII(struct STLParser** lpOut);
enum stlioError stlioParserBinary(struct STLParser** lpOut);

/*
	========================================
	Helper functions for simple file reading
	========================================

	The following functions allow applications to use the
	STL parsers without the little bit more complicated
	object oriented interface and without knowing the
	type (ASCII or binary) of the STL file. They use - in
	contrast to the object oriented functions - the libc
	file input and output functions to read data.

	There are two types of functions. On the one hand
	there is stlioReadFile which basically only wraps
	the data source - data and error conditions are
	still delivered via callback functions. The function
	returns after all bytes have been processed.

	Then there is stlioReadFileMem which reads an entire
	file into memory. The STL file should of course fit
	the available memory and address space - additionally
	this routine guarantees that all triangles occupy the
	same size in memory - if they have attribute bytes
	attached they are all expanded and zero padded  until
	they have the same size as the longest attribute
	block. To achieve this the file MAY be read at most
	two times (the first read is an optimistic read
	where the size is guessed by the attribute count
	of the first triangle; the second read happens in
	case an triangle with a different attribute byte count
	is encountered - then the currently allocated memory
	block is discarded and the read is finished to determine
	the maximum attribute block size; Then the read is
	restarted with this additional information). For ASCII
	files the triangles are first read into a linked list
	of internal buffers and then merged into a continuous
	memory buffer afterwards because the count is not known
	from the ASCII header. This leads to doubled memory
	usage.
*/
enum stlioError stlioReadFile(
	char* 						lpFilename,
	lpfnSTLCallback_Triangle 	callbackTriangle,
	void*						callbackTriangleParam,
	lpfnSTLCallback_Error	 	callbackError,
	void* 						callbackErrorParam,
	lpfnSTLCallback_EOF		 	callbackEOF,
	void* 						callbackEOFParam,
	enum stlFileType* 			lpDetectedType
);
enum stlioError stlioReadFileMem(
	char*						lpFilename,
	struct stlTriangle**		lpTriangles,
	unsigned long int*			lpTriangleCountOut,
	unsigned long int*			lpStrideSizeOut,
	lpfnSTLCallback_Error	 	callbackError,
	void* 						callbackErrorParam,
	enum stlFileType* 			lpDetectedType
);




/*
	=============
	STLStreamSink
	=============

	The STL stream sink is an interface used by the
	STL file writers to output their data independent of
	the destination (files, network, etc.) and independent
	of the libc implementation. The stream sink should
	be implemented by the application that uses the
	STL writer functions.
*/

/*
	Write bytes into the sink.

	Requests that the sink outputs the data block at
	offset lpData with dwByteCount bytes. The function
	has to write the number of bytes that have been
	successfully outputed into the lpBytesWritten variable.
	lpBytesWritten is always supplied.
*/
typedef enum stlioError (*lpfnSTLStreamSink_writeBytes)(
	struct STLStreamSink* 		lpSelf,
	unsigned char* 				lpData,
	unsigned long int 			dwByteCount,
	unsigned long int* 			lpBytesWritten
);
/*
	Seek inside the datastream

	This is required for writing binary files to update
	the triangle count on finalize. It is not used
	when writing ASCII files. The offset is always
	absolute.
*/
typedef enum stlioError (*lpfnSTLStreamSink_seekAbsolute)(
	struct STLStreamSink* 		lpSelf,
	unsigned long int			dwOffset
);
/*
	Seek inside the datastream

	This is required for writing binary files to update
	the triangle count on finalize. It is not used
	when writing ASCII files. The offset is always
	relative. A negative offset means moving towards
	the beginning of the file, a positive offset towards
	the end.

	Currently this function is not used.
*/
typedef enum stlioError (*lpfnSTLStreamSink_seekRelative)(
	struct STLStreamSink* 		lpSelf,
	signed long int				dwOffset
);
/*
	Release the stream sink

	This function should release the stream sink and
	all associated resources. The pointer to STLStreamSink
	is discarded after this function has been called.
*/
typedef enum stlioError (*lpfnSTLStreamSink_release)(
	struct STLStreamSink* 		lpSelf
);

/*
	STL stream sink object declarations. See description
	of the implementation of the object oriented model
	for details. lpReserved is used internally by the
	library to recover an reference to the internal
	state block even if an object is polymorphic.
*/
struct STLStreamSink_vtbl {
	lpfnSTLStreamSink_release		release;

	lpfnSTLStreamSink_writeBytes	writeBytes;
	lpfnSTLStreamSink_seekAbsolute	seekAbsolute;
	lpfnSTLStreamSink_seekRelative	seekRelative;
};
struct STLStreamSink {
	struct STLStreamSink_vtbl		*vtbl;
	void*							lpReserved;
};

/*
	===========================
	Writer callback definitions
	===========================
*/

/*
	Error callback. This is called whenever an error happens.
	If the error is caused by an invalid triangle the triangle
	is referenced at lpTriangle. The application can decide
	to skip (do not write) or keep (write invalid) the triangle
	or - in case of a surface normal mismatch - repair the
	order or repair the normal (the effects are equal to
	corrections applied by the STL parsers)
*/
typedef enum stlioError (*lpfnSTLWriter_Callback_Error)(
	enum stlioError eCode,
	struct stlTriangle* lpTriangle,
	void* lpFreeParam
);

/*
	=============
	Writer object
	=============
*/

struct STLWriter;

/*
	Normal writer mode

	Check normals:		All normaly are validated and the error callback is
						used in case there is a mismatch between calculated
						normals and the direction of the passed normal
	Calculate normals:	Normals passed into the writer are discarded and
						are calculated by the stlio library.
	Copy normals:		Whatever is passed into the normal fields of the
						triangles is written to the file (no validation!)
*/
enum stlWriter_NormalMode {
	stlWriter_NormalMode__CheckNormals,
	stlWriter_NormalMode__CalculateNormals,
	stlWriter_NormalMode__CopyNormals
};

/*
	Write a single triangle
*/
typedef enum stlioError (*lpfnSTLWriter_WriteTriangle)(
	struct STLWriter* 				lpSelf,
	struct stlTriangle* 			lpTriangle
);
/*
	Set normal mode

	Set one of the above normal modes. Node that the default
	mode is stlWriter_NormalMode__CheckNormals.
*/
typedef enum stlioError (*lpfnSTLWriter_SetNormalMode)(
	struct STLWriter* 				lpSelf,
	enum stlWriter_NormalMode		normalMode
);
/*
	Write a batch of triangles

	Parameter supplied is an array or list of triangles.
	If dwSizePerTriangle is zero the size of each entry (in bytes)
	is calculated by the attribute byte count (i.e. each triangle
	could have a different size because of different attribute
	byte count).
	If dwSizePerTriangle is not zero each entry inside the array
	is considered to have the same byte size - even if the attribute
	byte count (which has to fit into the size per triangle limit)
	varies the adresses are advanced always the same size. Output is
	written only for present attribute bytes.
*/
typedef enum stlioError (*lpfnSTLWriter_WriteTriangles)(
	struct STLWriter* 				lpSelf,
	struct stlTriangle* 			lpTriangles,
	unsigned long int 				dwTriangleCount,
	unsigned long int				dwSizePerTriangle,			/* Can be set to != 0x00 then size is equal (independent of attribute size) */
	unsigned long int* 				lpTrianglesWritten
);
/*
	Finalize write

	Has to be called after all triangles have been written and
	before releasing the writer. On ASCII this is a no-operation,
	on binary this writes the count of triangles into the header.
*/
typedef enum stlioError (*lpfnSTLWriter_Finalize)(
	struct STLWriter*				lpSelf
);
/*
	Continue

	Required whenever a previous operation has blocked (i.e.
	returned stlioE_Continues). New commands are only accepted
	after Continues returns stlioE_Ok - or the finalize has been
	finished after stlioE_Done.
*/
typedef enum stlioError (*lpfnSTLWriter_Continue)(
	struct STLWriter*				lpSelf
);
/*
	Set error callback
*/
typedef enum stlioError (*lpfnSTLWriter_SetCallbackError)(
	struct STLWriter* 				lpSelf,
	lpfnSTLWriter_Callback_Error 	callbackError,
	void* 							lpFreeParam
);
/*
	Release

	Releases all associated resources including the attached
	stream sink (if it hasn't been released up until now)
*/
typedef enum stlioError (*lpfnSTLWriter_Release)(
	struct STLWriter*				lpSelf
);



struct STLWriter_Vtbl {
	lpfnSTLWriter_Release					release;
	lpfnSTLWriter_Finalize					finalize;
	lpfnSTLWriter_Continue					continueOp;

	lpfnSTLWriter_WriteTriangle				writeTriangle;
	lpfnSTLWriter_WriteTriangles			writeTriangles;

	lpfnSTLWriter_SetNormalMode				setNormalMode;
	lpfnSTLWriter_SetCallbackError			setCallbackError;
};
struct STLWriter {
	struct STLWriter_Vtbl*					vtbl;
	void*									lpReserved;
};

/*
	=======================
	Factories for STLWriter
	=======================
*/

/*
	Create STL writer for ASCII files

	This function does not accept a "solid" filename.
*/
/*@
	assigns \result, *lpOut from \lpOut;
	behaviour invaldParamNullPtr:
		assumes lpOut == \null;
		assigns \result;
		ensures \result == stlioE_InvalidParam;
	behaviour successOrOutOfMemory:
		assumes lpOut != \null;
		requires \valid(lpOut);
		assigns \result, *lpOut \from lpOut;
		ensures ((\result == stlioE_Ok) && (*lpOut != \null) && \initialized(*lpOut)) || ((\result == stlioE_OutOfMemory) && (*lpOut == \null));
	complete behaviors;
	disjoint behaviours;
*/
enum stlioError stlioWriterASCII(struct STLWriter** lpOut, struct STLStreamSink* lpStreamSink);
/*
	Create a writer for binary files

	This function uses the default name for the "solid".
*/
/*@
	assigns \result, *lpOut from \lpOut;
	behaviour invaldParamNullPtr:
		assumes lpOut == \null;
		assigns \result;
		ensures \result == stlioE_InvalidParam;
	behaviour successOrOutOfMemory:
		assumes lpOut != \null;
		requires \valid(lpOut);
		assigns \result, *lpOut \from lpOut;
		ensures ((\result == stlioE_Ok) && (*lpOut != \null) && \initialized(*lpOut)) || ((\result == stlioE_OutOfMemory) && (*lpOut == \null));
	complete behaviors;
	disjoint behaviours;
*/
enum stlioError stlioWriterBinary(struct STLWriter** lpOut, struct STLStreamSink* lpStreamSink);

/*
	Write all triangles from a single array or list (note that
	the size of each entry has to match struct stlTriangle+attributeBytes
	if stride size is set to 0, else it has a constant stride size)
	into an STL file with the given name.

	Uses libc file I/O functions
*/
/*@
	assigns \result;
	behaviour invalidParam:
		assumes (lpFilename == \null) || (lpTriangles == \null) || (dwTriangleCount <= 0) || ((fileType != stlFileType_ASCII) && (fileType != stlFileType_Binary));
		assigns \result;
		ensures \result == stlioE_InvalidParam;
	behaviour ok:
		assumes lpFilename != \null;
		assumes lpTriangles != \null;
		assumes dwTriangleCount > 0;
		assumes (fileType == stlFileType_ASCII) || (fileType == stlFileType_Binary);
		requires \valid(lpTriangles)
		requires \valid_read(lpTriangles+(0 .. dwTriangleCount-1));
		assigns \result;
		ensures ((\result == stlioE_Ok) || (\result == stlioE_FileNotfound) || (\result == stlioE_PermissionDenied));
*/
enum stlioError stlioWriteFileMem(
	char*							lpFilename,
	struct stlTriangle*				lpTriangles,
	unsigned long int				dwTriangleCount,
	unsigned long int				dwStrideSize,
	lpfnSTLWriter_Callback_Error	callbackError,
	void* 							callbackErrorParam,
	enum stlFileType 				fileType
);

#ifdef __cplusplus
	} /* extern "C" { */
#endif
