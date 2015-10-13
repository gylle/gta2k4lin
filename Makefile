TARGETS=gta2k4lin

C_SOURCES=src/main.c src/world.c src/gl.c src/sound.c src/network.c src/hud.c src/lmq.c src/object.c src/stl.c src/car.c src/gubbe.c src/console.c
CXX_SOURCES=src/btwrap.cpp

CC=gcc
CXX=g++

LIBRARIES=sdl2 glew glu x11 SDL2_image bullet
LIBS_EXTRA=-lm -pthread

ifeq ($(shell sh -c 'pkg-config --libs SDL2_mixer 2>/dev/null'),)
	LIBS_EXTRA+=-lSDL2_mixer
else
	LIBRARIES+= SDL2_mixer
endif

ifeq ($(shell sh -c 'pkg-config --libs SDL2_ttf 2>/dev/null'),)
	LIBS_EXTRA+=-lSDL2_ttf
else
	LIBRARIES+= SDL2_ttf
endif

LIBS=$(shell pkg-config --libs $(LIBRARIES)) $(LIBS_EXTRA)

CFLAGS=-O2 -Wall -I./src $(shell pkg-config --cflags $(LIBRARIES)) -g

CXXFLAGS=$(CFLAGS) -fno-exceptions -fno-rtti -fno-check-new -Wwrite-strings -fpermissive

.PHONY: default clean dist depend

default: $(TARGETS)

clean:
	rm -f src/*.o $(TARGETS) *.tar.bz2

dist:
	git archive --prefix=gta2k4lin/ HEAD | bzip2 > gta2k4lin-$(shell git describe --tags --always HEAD).tar.bz2

depend:
	make -B Makefile.depend

gta2k4lin: src/main.o src/gl.o src/world.o src/sound.o src/network.o src/hud.o src/lmq.o src/object.o src/stl.o src/car.o src/gubbe.o src/btwrap.o src/console.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LIBS)

Makefile.depend: $(CXX_SOURCES) $(C_SOURCES)
	$(CC) -MM $(CFLAGS) $^ > Makefile.depend
	$(CXX) -MM $(CXXFLAGS) $^ >> Makefile.depend

include Makefile.depend
