/* x, y, z, texcoordx, texcoordy */

#include "car.h"
#include "gubbe.h"
#include "main.h"

/* TODO: They are already centered in x/y, do the same for z. */

float map_cube_vertices[] = {
    /* Bottom */
    -BSIZE,BSIZE,-BSIZE, 0.0f, 0.0f,
    -BSIZE,-BSIZE,-BSIZE, 0.0f, 1.0f,
    BSIZE,-BSIZE,-BSIZE, 1.0f, 1.0f,
    BSIZE,BSIZE,-BSIZE, 1.0f, 0.0f,

    /* Top */
    -BSIZE,-BSIZE,BSIZE, 0.0f, 0.0f,
    -BSIZE,BSIZE,BSIZE, 0.0f, 1.0f,
    BSIZE,BSIZE,BSIZE, 1.0f, 1.0f,
    BSIZE,-BSIZE,BSIZE, 1.0f, 0.0f,

    /* Left */
    -BSIZE,BSIZE,-BSIZE, 0.0f, 0.0f,
    -BSIZE,BSIZE,BSIZE, 0.0f, 1.0f,
    -BSIZE,-BSIZE,BSIZE, 1.0f, 1.0f,
    -BSIZE,-BSIZE,-BSIZE, 1.0f, 0.0f,

    /* Right */
    BSIZE,-BSIZE,-BSIZE, 0.0f, 0.0f,
    BSIZE,-BSIZE,BSIZE, 0.0f, 1.0f,
    BSIZE,BSIZE,BSIZE, 1.0f, 1.0f,
    BSIZE,BSIZE,-BSIZE, 1.0f, 0.0f,

    /* Front */
    -BSIZE,-BSIZE,-BSIZE, 0.0f, 0.0f,
    -BSIZE,-BSIZE,BSIZE, 0.0f, 1.0f,
    BSIZE,-BSIZE,BSIZE, 1.0f, 1.0f,
    BSIZE,-BSIZE,-BSIZE, 1.0f, 0.0f,

    /* Back */
    BSIZE,BSIZE,-BSIZE, 0.0f, 0.0f,
    BSIZE,BSIZE,BSIZE, 0.0f, 1.0f,
    -BSIZE,BSIZE,BSIZE, 1.0f, 1.0f,
    -BSIZE,BSIZE,-BSIZE, 1.0f, 0.0f
};
