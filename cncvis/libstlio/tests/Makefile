include ../Makefile.SUPPORTED

ifeq (,$(findstring $(OS),$(SUPPORTEDPLATFORMS)))

all:

	@echo The OS environment variable is currently set to [$(OS)]
	@echo Please set the OS environment variable to one of the following:
	@echo $(SUPPORTEDPLATFORMS)

.PHONY: all

else

include ../Makefile.$(OS)

LIBSRCFILES=../src/stlio.c ../src/stlioErrorStrings.c

OPTIONS=-DSTLIO_VERTEX_NORMAL_VALIDATION_ENABLED -DSTLIO_ERROR_STRINGS_ENABLED -DSTLIO_VALIDATION_SOLIDNAME_MATCH_ENABLED

all: ../bin/test001_serdeshelper$(EXESUFFIX) \
	../bin/test002_LoadSTL$(EXESUFFIX) \
	../bin/test003_LoadStoreSTL$(EXESUFFIX) \
	../bin/test004_LoadSTLASCII$(EXESUFFIX) \
	../bin/test005_ASCIIToBinary$(EXESUFFIX) \
	../bin/test006_BinaryToASCII$(EXESUFFIX) \
	../bin/test007_ReadWrapperCallback$(EXESUFFIX) \
	../bin/test008_ReadWrapperMem$(EXESUFFIX) \
	../bin/test009_WriteFileMem$(EXESUFFIX)

../bin/test001_serdeshelper$(EXESUFFIX): test001_serdeshelper.c $(LIBSRCFILES)

	$(CC) $(OPTIONS) -o ../bin/test001_serdeshelper$(EXESUFFIX) test001_serdeshelper.c $(LIBSRCFILES)

../bin/test002_LoadSTL$(EXESUFFIX): test002_LoadSTL.c $(LIBSRCFILES)

	$(CC) $(OPTIONS) -o ../bin/test002_LoadSTL$(EXESUFFIX) test002_LoadSTL.c $(LIBSRCFILES)

../bin/test003_LoadStoreSTL$(EXESUFFIX): test003_LoadStoreSTL.c $(LIBSRCFILES)

	$(CC) $(OPTIONS) -o ../bin/test003_LoadStoreSTL$(EXESUFFIX) test003_LoadStoreSTL.c $(LIBSRCFILES)

../bin/test004_LoadSTLASCII$(EXESUFFIX): test004_LoadSTLASCII.c $(LIBSRCFILES)

	$(CC) $(OPTIONS) -o ../bin/test004_LoadSTLASCII$(EXESUFFIX) test004_LoadSTLASCII.c $(LIBSRCFILES)

../bin/test005_ASCIIToBinary$(EXESUFFIX): test005_ASCIIToBinary.c $(LIBSRCFILES)

	$(CC) $(OPTIONS) -o ../bin/test005_ASCIIToBinary$(EXESUFFIX) test005_ASCIIToBinary.c $(LIBSRCFILES)

../bin/test006_BinaryToASCII$(EXESUFFIX): test006_BinaryToASCII.c $(LIBSRCFILES)

	$(CC) $(OPTIONS) -o ../bin/test006_BinaryToASCII$(EXESUFFIX) test006_BinaryToASCII.c $(LIBSRCFILES)

../bin/test007_ReadWrapperCallback$(EXESUFFIX): test007_ReadWrapperCallback.c $(LIBSRCFILES)

	$(CC) $(OPTIONS) -o ../bin/test007_ReadWrapperCallback$(EXESUFFIX) test007_ReadWrapperCallback.c $(LIBSRCFILES)

../bin/test008_ReadWrapperMem$(EXESUFFIX): test008_ReadWrapperMem.c $(LIBSRCFILES)

	$(CC) $(OPTIONS) -o ../bin/test008_ReadWrapperMem$(EXESUFFIX) test008_ReadWrapperMem.c $(LIBSRCFILES)

../bin/test009_WriteFileMem$(EXESUFFIX): test009_WriteFileMem.c $(LIBSRCFILES)

	$(CC) $(OPTIONS) -o ../bin/test009_WriteFileMem$(EXESUFFIX) test009_WriteFileMem.c $(LIBSRCFILES)

endif
