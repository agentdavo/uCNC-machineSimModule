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

all: ../bin/tools/stl2bin$(EXESUFFIX) \
	../bin/tools/stl2ascii$(EXESUFFIX)


../bin/tools/stl2bin$(EXESUFFIX): ../bin/libstlio$(SLIBSUFFIX) ./stl2bin.c ../include/stlio.h

	$(CC) $(OPTIONS) -o ../bin/tools/stl2bin$(EXESUFFIX) -L../bin/ -I../include/ stl2bin.c ../bin/libstlio$(SLIBSUFFIX)

../bin/tools/stl2ascii$(EXESUFFIX): ../bin/libstlio$(SLIBSUFFIX) ./stl2ascii.c ../include/stlio.h

	$(CC) $(OPTIONS) -o ../bin/tools/stl2ascii$(EXESUFFIX) -L../bin/ -I../include/ stl2ascii.c ../bin/libstlio$(SLIBSUFFIX)


endif
