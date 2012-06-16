TARGETS=gta2k4lin

BULLETSRC=bullet/src

C_SOURCES=src/main.c src/sound.c src/network.c src/hud.c src/lmq.c src/object.c src/stl.c src/car.c src/gubbe.c
CXX_SOURCES=src/btwrap.cpp

CC=gcc
CXX=g++

LIBRARIES=sdl glu x11 SDL_image

ifeq ($(shell sh -c 'pkg-config --libs SDL_mixer 2>/dev/null'),)
	LIBS_EXTRA+=-lSDL_mixer
else
	LIBRARIES+= SDL_mixer
endif

ifeq ($(shell sh -c 'pkg-config --libs SDL_ttf 2>/dev/null'),)
	LIBS_EXTRA+=-lSDL_ttf
else
	LIBRARIES+= SDL_ttf
endif

# LIBS_EXTRA+=$(BULLETSRC)/.libs/libBulletDynamics.so $(BULLETSRC)/.libs/libBulletCollision.so $(BULLETSRC)/.libs/libLinearMath.so
LIBS_EXTRA+=$(BULLETSRC)/.libs/libBulletDynamics.a $(BULLETSRC)/.libs/libBulletCollision.a $(BULLETSRC)/.libs/libLinearMath.a

LIBS=$(shell pkg-config --libs $(LIBRARIES)) $(LIBS_EXTRA)

CFLAGS=-O2 -Wall -I./src -I$(BULLETSRC) $(shell pkg-config --cflags $(LIBRARIES)) -lm -g

CXXFLAGS=$(CFLAGS) -fno-exceptions -fno-rtti -fno-check-new -Wwrite-strings -fpermissive

.PHONY: default clean dist depend

default: $(TARGETS)

clean:
	rm -f src/*.o $(TARGETS) *.tar.bz2

dist:
	git archive --prefix=gta2k4lin/ HEAD | bzip2 > gta2k4lin-$(shell git describe --tags --always HEAD).tar.bz2

depend:
	make -B Makefile.depend

src/btwrap.o:
	$(CXX) -c -o src/btwrap.o src/btwrap.cpp $(CFLAGS)

gta2k4lin: src/main.o src/sound.o src/network.o src/hud.o src/lmq.o src/object.o src/stl.o src/car.o src/gubbe.o src/btwrap.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

Makefile.depend: $(CXX_SOURCES) $(C_SOURCES)
	$(CC) -MM $(CFLAGS) $^ > Makefile.depend

include Makefile.depend
