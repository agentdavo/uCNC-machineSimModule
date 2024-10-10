include Makefile.SUPPORTED

ifeq (,$(findstring $(OS),$(SUPPORTEDPLATFORMS)))

all:

	@echo The OS environment variable is currently set to [$(OS)]
	@echo Please set the OS environment variable to one of the following:
	@echo $(SUPPORTEDPLATFORMS)

.PHONY: all

else

include Makefile.$(OS)

OPTIONS=-DSTLIO_VERTEX_NORMAL_VALIDATION_ENABLED -DSTLIO_ERROR_STRINGS_ENABLED -DSTLIO_VALIDATION_SOLIDNAME_MATCH_ENABLED
LIBSRCFILES=../src/stlio.c ../src/stlioErrorStrings.c

OBJFILES=tmp/stlio$(OBJSUFFIX) \
	tmp/stlioErrorStrings$(OBJSUFFIX)

all: staticLib dynamicLib tests tools

staticLib: $(OBJFILES)

	$(ARCMD) bin/libstlio$(SLIBSUFFIX) $(OBJFILES)

dynamicLib:  $(OBJFILES)

	$(SHAREDCC) $(OPTIONS) -o bin/libstlio$(DYNLIBSUFFIX) $(OBJFILES)

package: staticLib dynamicLib tests tools

	# Create staging hierarchy
	$(MKDIR) stage/lib
	$(MKDIR) stage/bin

	$(MKDIR) packages

	# Copy files
	$(CP) bin/libstlio$(SLIBSUFFIX) stage/lib/libstlio$(SLIBSUFFIX)
	$(CP) bin/libstlio$(DYNLIBSUFFIX) stage/lib/libstlio$(DYNLIBSUFFIX)
	$(CP) bin/tools/stl2bin$(EXESUFFIX) stage/bin/stl2bin$(EXESUFFIX)
	$(CP) bin/tools/stl2ascii$(EXESUFFIX) stage/bin/stl2ascii$(EXESUFFIX)

	# Create archive
	$(MKSTAGEARCHIVE)

	# Cleanup
	$(RMDIR) stage

tmp/stlio$(OBJSUFFIX): src/stlio.c include/serdeshelper.h include/stlio.h

	$(CCLIB) $(OPTIONS) -c -o tmp/stlio$(OBJSUFFIX) src/stlio.c

tmp/stlioErrorStrings$(OBJSUFFIX): src/stlioErrorStrings.c include/stlio.h

	$(CCLIB) $(OPTIONS) -c -o tmp/stlioErrorStrings$(OBJSUFFIX) src/stlioErrorStrings.c

tests:

	- @$(MAKE) -C ./tests

tools:

	@echo Building tools
	- @$(MAKE) -C ./tools

clean:

	- @$(RMFILE) bin/test*
	- @$(RMFILE) bin/lib*
	- @$(RMFILE) bin/tools/stl*
	- @$(RMFILE) tmp/*$(OBJSUFFIX)

.PHONY: staticLib dynamicLib tests tools clean

endif
