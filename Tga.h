#ifndef __TGA_H__
#define __TGA_H__

#pragma comment(lib, "Opengl32.lib")					//Link to OpenGL32.lib so we can use OpenGL stuff

//#include <windows.h>									// Standard windows header
#include <stdio.h>										// Standard I/O header 
#include <GL/gl.h>										// Header for OpenGL32 library
//#include "texture.h"

typedef struct
{
	GLubyte* imageData;// Hold All The Color Values For The Image.
	GLuint  bpp;// Hold The Number Of Bits Per Pixel.
	GLuint width;// The Width Of The Entire Image.
	GLuint height;// The Height Of The Entire Image.
	GLuint texID;// Texture ID For Use With glBindTexture.
	GLuint type; // Data Stored In * ImageData (GL_RGB Or GL_RGBA)
} Texture;


bool LoadTGA(Texture * texture, char * filename);				// Load a TGA file
bool LoadUncompressedTGA(Texture *, char *, FILE *);	// Load an Uncompressed file
bool LoadCompressedTGA(Texture *, char *, FILE *);		// Load a Compressed file

#endif
