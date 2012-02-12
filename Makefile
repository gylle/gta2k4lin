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
	rm -f *.o $(TARGETS) *.tar.bz2

dist:
	git archive --prefix=gta2k4lin/ HEAD | bzip2 > gta2k4lin-$(shell git describe --tags --always HEAD).tar.bz2

gta2k4lin: main.o TGALoader.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LIBS)

Makefile.depend: $(CXX_SOURCES)
	$(CXX) -MM $(CXXFLAGS) $^ > Makefile.depend

include Makefile.depend
