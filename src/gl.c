#include <stdio.h>

#include <linux/limits.h>
#include <SDL.h>
#include <SDL_image.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "gl.h"
#include "main.h"
#include "network.h"
#include "hud.h"
#include "gubbe.h"
#include "car.h"
#include "models.h"
#include "world.h"

#define TEXTURE_PATH "data/"

/* FIXME: Move somewhere */
static const char *texture_filenames[] = {
    "test.tga",
    "carroof.tga",
    "road1.tga",
    "road2.tga",
    "building1.tga",
    "road3.tga",
    "road4.tga",
    "road5.tga",
    "road6.tga",
    "road7.tga",
    "dhcred.tga",
    "gubbel.tga",
    "gubbed.tga",
    "gubbel2.tga",
    "carroof2.tga",
    "buske.tga"
};

void draw_quads(float vertices[], int count) {
    /* Loosely modeled upon glDrawElements & co */

    glBegin(GL_QUADS);
    int i,j;
    for(i = 0; i < count; i++) {
        for(j = 0; j < 4; j++) {
            int offset = i*5*4 + j*5;
            glTexCoord2f(vertices[offset+3], vertices[offset+4]);
            glVertex3f(vertices[offset], vertices[offset+1], vertices[offset+2]);
        }
    }
    glEnd();
}

void gl_resize(int width, int height) {

    float ratio = (float) width / (float) height;

    /* Setup our viewport. */
    glViewport( 0, 0, width, height );

    /*
     * Change to the projection matrix and set
     * our viewing volume.
     */
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    gluPerspective( 60.0, ratio, 1.0, 1024.0 );

    glMatrixMode( GL_MODELVIEW );
}


static int gl_loadtextures()                                                            // Load Bitmaps And Convert To Textures
{
    /* Load textures from file names in world */
    world.texture_filenames = texture_filenames;
    world.ntextures = sizeof(texture_filenames) / sizeof(char *);
    world.texIDs = (GLuint *)calloc(world.ntextures, sizeof(GLuint));
    if (world.texIDs == NULL) {
        return 0;
    }

    char path_buf[PATH_MAX];

    int i;
    for (i = 0; i < world.ntextures; i++) {
        snprintf(path_buf, PATH_MAX, "%s%s", TEXTURE_PATH,
                 world.texture_filenames[i]);
        SDL_Surface *texture = IMG_Load(path_buf);
        if (texture == NULL) {
            return 0;
        }

        // Mmkay, så opengl räknar koordinater från nedre vänstra
        // hörnet. Undrar om den gode ola roterade alla texturer i
        // tgaloader-versionen :D
        // Aja, vi fixar.
        char tp[3];
        char *pixels = (char *) texture->pixels;
        int w = texture->w * 3;
        unsigned int j, k, size = texture->w * texture->h * 3;
        for (j = 0; j < size / 2; j += 3) {
            memcpy(tp, &pixels[j], 3);
            memcpy(&pixels[j], &pixels[size - j], 3);
            memcpy(&pixels[size - j], tp, 3);
        }
        // Vafan, spegelvänt också?!
        for (j = 0; j < texture->h * w; j += w) {
            for (k = 0; k < w / 2; k += 3) {
                memcpy(tp, &pixels[j + k], 3);
                memcpy(&pixels[j + k], &pixels[j + w - k - 3], 3);
                memcpy(&pixels[j + w - k - 3], tp, 3);
            }
        }

        glGenTextures(1, &world.texIDs[i]);
        glBindTexture(GL_TEXTURE_2D, world.texIDs[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, 3,
                     texture->w, texture->h, 0,
                     GL_BGR, GL_UNSIGNED_BYTE,
                     texture->pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                        GL_LINEAR);

        SDL_FreeSurface(texture);
    }

    return 1;                                                       // Return The Status
}

int gl_drawscene()
{
    int i;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    /* Set camera position (by translating the world in the opposite direction */
    glLoadIdentity();
    glTranslatef(-world.camera.x,-world.camera.y,world.camera.z+world.camera.SpeedVar);

    if(blendcolor>0.0f)
        glEnable(GL_BLEND);
    if(blendcolor==0.0f)
        glDisable(GL_BLEND);

    glColor4f(1.0f,1.0f,1.0f,blendcolor);

    int loop1, loop2, loop3;

    // Ritar upp banan -----------------------------
    float lp1bstmp,lp2bstmp,ztmp;
    for(loop1=0; loop1<world.map.nrcubex; loop1++) {
        for(loop2=0; loop2<world.map.nrcubey; loop2++) {
            for(loop3=0; loop3<world.map.nrcubez; loop3++) {


                lp1bstmp=(float)loop1*(BSIZE*2);
                lp2bstmp=(float)loop2*(BSIZE*2);
                ztmp=map_cube(world, loop1, loop2, loop3).o.z;

                glPushMatrix();
                glTranslatef(lp1bstmp, lp2bstmp, ztmp);

                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D,world.texIDs[map_cube(world, loop1, loop2, loop3).texturenr]);

                draw_quads(map_cube_vertices, sizeof(map_cube_vertices)/(20*sizeof(float)));

                glPopMatrix();
            }
        }
    }


    car_render(&world.bil);
    if (Network) {
        for (i = 0; i < NETWORK_MAX_OPPONENTS; i++) {
            if (world.opponents[i].in_use)
                car_render(&(world.opponent_cars[i]));
        }
    }

    if(!Network) {
        for(loop1=0;loop1<nrgubbar;loop1++) {
            gubbe_render(&world.gubbar[loop1]);
        }

    }

    hud_set_damage(world.bil.helhet);

    if(!Network) {
        hud_set_score(world.player.runovers);
    } else {
        /* TODO */
    }

    return 1;
}

int gl_init(SDL_Window *window, int width, int height)
{
    SDL_GL_CreateContext(window);

    if (!gl_loadtextures()) {
        printf("Bananeinar, det verkar inte som om den vill ladda texturerna.");
            exit(1);
        }

    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );


    /* Our shading model--Gouraud (smooth). */
    glShadeModel( GL_SMOOTH );

    /* Culling. */
    glFrontFace( GL_CW );
    glCullFace( GL_BACK );
    glEnable( GL_CULL_FACE );

    /* Set the clear color. */
    glClearColor( 0, 0, 1, 0 );

    glShadeModel(GL_SMOOTH);                                                        // Enable Smooth Shading
    glClearDepth(1.0f);                                                                     // Depth Buffer Setup
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);                                                        // Enables Depth Testing
    glDepthFunc(GL_LEQUAL);                                                         // The Type Of Depth Testing To Do
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);      // Really Nice Perspective Calculations

    gl_resize(width, height);

    return 1;                                                                               // Initialization Went OK
}