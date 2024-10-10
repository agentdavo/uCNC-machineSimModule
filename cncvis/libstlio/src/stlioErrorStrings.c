#include "../include/stlio.h"

#ifdef STLIO_ERROR_STRINGS_ENABLED
	struct stlioErrorMessageC {
		enum stlioError e;			/* Code, should be ordered */
		char* lpMessage;			/* Message as C string */
	};

	static struct stlioErrorMessageC stlioErrorMessagesC[] = {
		{ stlioE_Ok,										"Ok" },
		{ stlioE_Done,										"Done" },
		{ stlioE_Continues,									"Continues" },
		{ stlioE_Failed, 									"Failed" },

		{ stlioE_FileNotfound,								"File not found" },
		{ stlioE_PermissionDenied,							"Permission denied" },
		{ stlioE_OutOfMemory,								"Out of memory" },
		{ stlioE_IOError,									"IO error" },

		{ stlioE_InvalidParam,								"Invalid parameter" },
		{ stlioE_InvalidState,								"Invalid state" },

		{ stlioE_InvalidFormat,								"Invalid format" },
		{ stlioE_InvalidFormat_InvalidCharacter,			"Invalid character" },
		{ stlioE_InvalidFormat_NotAFloat,					"Not a floating point number" },
		{ stlioE_InvalidFormat_SolidNameMismatch,			"Solid name mismatch" },
		{ stlioE_InvalidFormat_TokenTooLong,				"Token too long" },
		{ stlioE_InvalidFormat_Expect_Solid,				"Expecting solid" },
		{ stlioE_InvalidFormat_Expect_Facet,				"Expecting facet" },
		{ stlioE_InvalidFormat_Expect_Normal,				"Expecting normal" },
		{ stlioE_InvalidFormat_Expect_FloatNumber,			"Expecting floating pointer number" },
		{ stlioE_InvalidFormat_Expect_Outer,				"Expecting outer" },
		{ stlioE_InvalidFormat_Expect_Loop,					"Expecting loop" },
		{ stlioE_InvalidFormat_Expect_Vertex,				"Expecting vertex" },
		{ stlioE_InvalidFormat_Expect_Endloop,				"Expecting endloop" },
		{ stlioE_InvalidFormat_Expect_Endfacet,				"Expecting endfacet" },
		{ stlioE_InvalidFormat_Expect_FacetOrEndloop,		"Expecting facet or endloop" },
		{ stlioE_InvalidFormat_VertexNormalMismatch,		"Invalid format: Vertex normal mismatch" },
		{ stlioE_InvalidFormat_NotATriangle,				"Invalid format: Not a triangle (degenerated)" },
		{ stlioE_InvalidFormat_AttributesNotSupported,		"Invalid format: Attributes not supported" },
		{ stlioE_InvalidFormat_UnexpectedEnd,				"Invalid format: Unexpected end of file or stream" },

		{ stlioE_Abort,										"Abort" },
		{ stlioE_ImplementationError,						"Implementation error" },

		{ stlioE_ValidationError_UnsharedVertices,			"Validation Error: Vertex not shared by other triangle"		},
		{ stlioE_ValidationError_VertexEdgeIntersection,	"Validation Error: Vertex intersects edge of other polygon" },

		{ ~0,												"Unknown error" }
	};

	#ifdef __cplusplus
		extern "C" {
	#endif

	const char* stlioErrorStringC(enum stlioError e) {
		unsigned long int i;
		/* Do a linear scan ... later we should do a BST */

		for(i = 0; i < sizeof(stlioErrorMessagesC)/sizeof(struct stlioErrorMessageC); i=i+1) {
			if(stlioErrorMessagesC[i].e == e) {
				return stlioErrorMessagesC[i].lpMessage;
			}
		}
		return stlioErrorMessagesC[i-1].lpMessage; /* The last message is the default message */
	}

	#ifdef __cplusplus
		} /* extern "C" { */
	#endif
#else
	#ifdef __cplusplus
		extern "C" {
	#endif

	const char* stlioErrorStringC(enum stlioError e) {
		return "Unknown error code";
	}

	#ifdef __cplusplus
		} /* extern "C" { */
	#endif
#endif
