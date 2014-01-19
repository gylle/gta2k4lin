#ifndef __LOCAL_GL_H__
#define __LOCAL_GL_H__

#include <SDL.h>

void draw_quads(float[], int);
void gl_resize(int, int);
int gl_init(SDL_Window *window, int, int);
int gl_drawscene();

GLuint gl_new_buffer_object(GLenum type, GLsizeiptr size, const GLvoid *data);
GLuint gl_new_program_from_files(const char *vfile, const char *ffile);

#endif
