
TARGETS=gta2k4lin

CXX_SOURCES=main.cpp TGALoader.cpp

OBJS=$(subst,$(CXX_SOURCES),.cpp,.o)

LIBRARIES=sdl glu x11 SDL_mixer

CXXFLAGS=-O2 -fno-exceptions -fno-rtti -fno-check-new -Wwrite-strings -DSOUND -fpermissive \
				 $(shell pkg-config --cflags $(LIBRARIES))

LIBS=$(shell pkg-config --libs $(LIBRARIES))

.PHONY: default

default: $(TARGETS)

gta2k4lin: main.o TGALoader.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f *.o $(TARGETS)

