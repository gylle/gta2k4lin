/* x, y, z, texcoordx, texcoordy */

/* FIXME */
#define CARSIZE_X 3.0f
#define CARSIZE_Y 5.0f
#define CARSIZE_Z 2.0f

#define GUBBSIZE_X 1.0f
#define GUBBSIZE_Y 1.0f
#define GUBBSIZE_Z 1.9f

#define BSIZE 5.0f

/* TODO: They are already centered in x/y, do the same for z. */

float gubbe_top_vertices[] = {
    -GUBBSIZE_X/2, -GUBBSIZE_Y/2, BSIZE+GUBBSIZE_Z, 0.0f, 0.0f,
    -GUBBSIZE_X/2, GUBBSIZE_Y/2, BSIZE+GUBBSIZE_Z, 0.0f, 1.0f,
    GUBBSIZE_X/2, GUBBSIZE_Y/2, BSIZE+GUBBSIZE_Z, 1.0f, 1.0f,
    GUBBSIZE_X/2, -GUBBSIZE_Y/2, BSIZE+GUBBSIZE_Z, 1.0f, 0.0f
};

float gubbe_body_vertices[] = {
    /* Left */
    -GUBBSIZE_X/2, GUBBSIZE_Y/2, BSIZE, 0.0f, 0.0f,
    -GUBBSIZE_X/2, GUBBSIZE_Y/2, BSIZE+GUBBSIZE_Z, 0.0f, 1.0f,
    -GUBBSIZE_X/2, -GUBBSIZE_Y/2, BSIZE+GUBBSIZE_Z,1.0f, 1.0f,
    -GUBBSIZE_X/2, -GUBBSIZE_Y/2, BSIZE,1.0f, 0.0f,
    /* Right */
    GUBBSIZE_X/2, -GUBBSIZE_Y/2, BSIZE, 0.0f, 0.0f,
    GUBBSIZE_X/2, -GUBBSIZE_Y/2, BSIZE+GUBBSIZE_Z, 0.0f, 1.0f,
    GUBBSIZE_X/2, GUBBSIZE_Y/2, BSIZE+GUBBSIZE_Z, 1.0f, 1.0f,
    GUBBSIZE_X/2, GUBBSIZE_Y/2, BSIZE, 1.0f, 0.0f,
    /* Front */
    -GUBBSIZE_X/2, -GUBBSIZE_Y/2, BSIZE, 0.0f, 0.0f,
    -GUBBSIZE_X/2, -GUBBSIZE_Y/2, BSIZE+GUBBSIZE_Z, 0.0f, 1.0f,
    GUBBSIZE_X/2, -GUBBSIZE_Y/2, BSIZE+GUBBSIZE_Z, 1.0f, 1.0f,
    GUBBSIZE_X/2, -GUBBSIZE_Y/2, BSIZE, 1.0f, 0.0f,
    /* Back */
    GUBBSIZE_X/2, GUBBSIZE_Y/2, BSIZE, 0.0f, 0.0f,
    GUBBSIZE_X/2, GUBBSIZE_Y/2, BSIZE+GUBBSIZE_Z, 0.0f, 1.0f,
    -GUBBSIZE_X/2, GUBBSIZE_Y/2, BSIZE+GUBBSIZE_Z, 1.0f, 1.0f,
    -GUBBSIZE_X/2, GUBBSIZE_Y/2, BSIZE, 1.0f, 0.0f
};

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
