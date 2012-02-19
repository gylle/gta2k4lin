TARGETS=gta2k4lin

C_SOURCES=src/sound.c src/network.c src/hud.c
CXX_SOURCES=src/main.cpp

CC=gcc
CXX=g++

OBJS=$(subst,$(CXX_SOURCES),.cpp,.o) $(subst,$(C_SOURCES),.c,.o)

LIBRARIES=sdl glu x11 SDL_image SDL_ttf

ifeq ($(shell sh -c 'pkg-config --libs SDL_mixer 2>/dev/null'),)
	LIBS_EXTRA=-lSDL_mixer
else
	LIBRARIES+= SDL_mixer
endif

LIBS=$(shell pkg-config --libs $(LIBRARIES)) $(LIBS_EXTRA)

CFLAGS=-O2 -Wall -I./src $(shell pkg-config --cflags $(LIBRARIES))

CXXFLAGS=$(CFLAGS) -fno-exceptions -fno-rtti -fno-check-new -Wwrite-strings -fpermissive

.PHONY: default clean dist depend

default: $(TARGETS)

clean:
	rm -f src/*.o $(TARGETS) *.tar.bz2

dist:
	git archive --prefix=gta2k4lin/ HEAD | bzip2 > gta2k4lin-$(shell git describe --tags --always HEAD).tar.bz2

depend:
	make -B Makefile.depend

gta2k4lin: src/main.o src/sound.o src/network.o src/hud.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LIBS)

Makefile.depend: $(CXX_SOURCES) $(C_SOURCES)
	$(CC) -MM $(CFLAGS) $^ > Makefile.depend
	$(CXX) -MM $(CXXFLAGS) $^ >> Makefile.depend

include Makefile.depend
