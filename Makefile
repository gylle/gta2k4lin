TARGETS=gta2k4lin

CXX_SOURCES=main.cpp

OBJS=$(subst,$(CXX_SOURCES),.cpp,.o)

LIBRARIES=sdl glu x11 SDL_image

ifeq ($(shell sh -c 'pkg-config --libs SDL_mixer 2>/dev/null'),)
	LIBS_EXTRA=-lSDL_mixer
else
	LIBRARIES+= SDL_mixer
endif

LIBS=$(shell pkg-config --libs $(LIBRARIES)) $(LIBS_EXTRA)

CFLAGS=-O2 $(shell pkg-config --cflags $(LIBRARIES))

CXXFLAGS=$(CFLAGS) -fno-exceptions -fno-rtti -fno-check-new -Wwrite-strings -fpermissive

.PHONY: default clean dist depend

default: Makefile.depend $(TARGETS)

clean:
	rm -f *.o $(TARGETS) *.tar.bz2

dist:
	git archive --prefix=gta2k4lin/ HEAD | bzip2 > gta2k4lin-$(shell git describe --tags --always HEAD).tar.bz2

depend:
	make -B Makefile.depend

gta2k4lin: main.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

Makefile.depend: $(CXX_SOURCES)
	$(CXX) -MM $(CXXFLAGS) $^ > Makefile.depend

include Makefile.depend
