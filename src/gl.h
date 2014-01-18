#ifndef __GL_H__
#define __GL_H__

#include <SDL.h>

void draw_quads(float[], int);
void gl_resize(int, int);
int gl_init(SDL_Window *window, int, int);
int gl_drawscene();

#endif
