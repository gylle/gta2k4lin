TARGETS=gta2k4lin

CXX_SOURCES=main.cpp TGALoader.cpp

OBJS=$(subst,$(CXX_SOURCES),.cpp,.o)

LIBRARIES=sdl glu x11 SDL_mixer

CXXFLAGS=-O2 -fno-exceptions -fno-rtti -fno-check-new -Wwrite-strings -fpermissive \
				 $(shell pkg-config --cflags $(LIBRARIES))

LIBS=$(shell pkg-config --libs $(LIBRARIES))

.PHONY: default clean dist

default: Makefile.depend $(TARGETS)

clean:
	rm -f *.o $(TARGETS)

dist: gta2k4lin.tar.bz2

gta2k4lin: main.o TGALoader.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LIBS)

# "git describe --tags --always HEAD" would be nice to use in the file name,
# but how to solve outside of git (i.e. in tar.bz2 source dist)?
gta2k4lin.tar.bz2: $(CXX_SOURCES)
	git archive --prefix=gta2k4lin/ HEAD | bzip2 > $@

Makefile.depend: $(CXX_SOURCES)
	$(CXX) -MM $(CXXFLAGS) $^ > Makefile.depend

include Makefile.depend
