#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>

#include "gubbe.h"
#include "main.h"

static const int gubbtid=300;		// Hur lång tid en gubbe är död... Räknas i frames :)
static GLuint GubbeDispList = 0;

static float gubbe_top_vertices[] = {
    -GUBBSIZE_X/2, -GUBBSIZE_Y/2, BSIZE+GUBBSIZE_Z, 0.0f, 0.0f,
    -GUBBSIZE_X/2, GUBBSIZE_Y/2, BSIZE+GUBBSIZE_Z, 0.0f, 1.0f,
    GUBBSIZE_X/2, GUBBSIZE_Y/2, BSIZE+GUBBSIZE_Z, 1.0f, 1.0f,
    GUBBSIZE_X/2, -GUBBSIZE_Y/2, BSIZE+GUBBSIZE_Z, 1.0f, 0.0f
};

static float gubbe_body_vertices[] = {
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

static void gubbe_get_world_transform(void *data, plVector3 v, plReal *m) {
    /* printf("get_world_transform (%p, %p, %p)\n", data, v, m); */

    struct gubbe *g = (struct gubbe*)data;

    v[0] = g->o.x;
    v[1] = g->o.y;
    v[2] = g->o.z;

    memcpy(m, g->rotation, sizeof(float)*16);
}

static void gubbe_set_world_transform(void *data, plVector3 v, plReal *m) {
    /* printf("set_world_transform (%p, %p, %p)\n", data, v, m); */

    /* printf("v: %f, %f, %f\n", v[0], v[1], v[2]); */
    /* printf("m: %f, %f, %f %f\n %f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n", */
    /*        m[0], m[1], m[2], m[3], */
    /*        m[4], m[5], m[6], m[7], */
    /*        m[8], m[9], m[10], m[11], */
    /*        m[12], m[13], m[14], m[15]); */

    struct gubbe *g = (struct gubbe*)data;
    g->o.x = v[0];
    g->o.y = v[1];
    g->o.z = v[2];

    memcpy(g->rotation, m, sizeof(float)*16);

    /* FIXME? */
    g->rotation[15] = 1.0f;
}

void init_gubbe(struct gubbe *g) {
    g->alive=1;

    g->o.speed=0.0f;
    g->maxspeed=0.3f;
    g->accspeed=0.15f;
    g->maxbspeed=0.2f;

    //g->o.angle=0;

    g->o.size_x=GUBBSIZE_X;
    g->o.size_y=GUBBSIZE_Y;
    g->o.size_z=GUBBSIZE_Z;
    /* object_update_circle(&(g->o)); */

    g->o.x=(float)((rand() % world.nrcubex*BSIZE*2)*100)/100.0f;
    g->o.y=(float)((rand() % world.nrcubey*BSIZE*2)*100)/100.0f;
    g->o.angle=rand() % 360;

    g->o.z=BSIZE + GUBBSIZE_Z / 2 + 1.0f;

    g->ltexture=11;
    g->ltexture2=13;
    g->dtexture=12;

    g->atimer = 0;

    /* Initialize bullet stuff */
    float bt_weight = 1.0f;
    void *user_data = NULL; /* 1.0f */
    g->bt_shape = plNewBoxShape(GUBBSIZE_X/2.0f, GUBBSIZE_Y/2.0f, GUBBSIZE_Z/2.0f);

    g->rotation[0] = 1.0f; g->rotation[1] = 0.0f; g->rotation[2] = 0.0f; g->rotation[3] = 0.0f;
    g->rotation[4] = 0.0f; g->rotation[5] = 1.0f; g->rotation[6] = 0.0f; g->rotation[7] = 0.0f;
    g->rotation[8] = 0.0f; g->rotation[9] = 0.0f; g->rotation[10] = 1.0f; g->rotation[11] = 0.0f;
    g->rotation[12] = 0.0f; g->rotation[13] = 0.0f; g->rotation[14] = 0.0f; g->rotation[15] = 1.0f;

    g->bt_rbody = plCreateRigidBodyWithCallback(user_data, bt_weight, g->bt_shape,
                                                &gubbe_get_world_transform,
                                                &gubbe_set_world_transform,
                                                g);

    plAddRigidBody(dynamics_world, g->bt_rbody);
    /* printf("Added gubbe-rbody %p\n", g->bt_rbody); */

    if(GubbeDispList == 0) {
        printf("Initializing gubbe-displaylist");

        // Vi bygger en Display List!!! EJJJJ!!!(som i öj) :)
        GubbeDispList = glGenLists(1);

        glNewList(GubbeDispList,GL_COMPILE);
            glBindTexture(GL_TEXTURE_2D,world.texIDs[g->ltexture2]);
            draw_quads(gubbe_top_vertices, sizeof(gubbe_top_vertices)/(20*sizeof(float)));

            glBindTexture(GL_TEXTURE_2D,world.texIDs[g->ltexture]);
            draw_quads(gubbe_body_vertices, sizeof(gubbe_body_vertices)/(20*sizeof(float)));
        glEndList();
    }
}

void gubbe_render(struct gubbe *g) {
    glPushMatrix();

    // HAHA!!! Det gick till slut! :)
    glTranslatef(g->o.x,g->o.y,0);
    /* glRotatef((float)g->o.angle,0.0f,0.0f,1.0f); */
    glMultMatrixf(g->rotation);

    if(g->alive) {
        /* if(!NoBlend) */
        /*     glEnable(GL_BLEND); */

        glColor4f(1.0f,1.0f,1.0f,1.0f);
        glCallList(GubbeDispList);

        /* if(blendcolor==0.0f) */
        /*     glDisable(GL_BLEND); */
    } else {
        // Är man överkörd står man nog inte upp längre... :) Det här blir bättre...
        glBindTexture(GL_TEXTURE_2D,world.texIDs[g->dtexture]);

        glTranslatef(0.0f, 0.0f, -GUBBSIZE_Z+0.01f);
        draw_quads(gubbe_top_vertices, sizeof(gubbe_top_vertices)/(20*sizeof(float)));
    }
    glPopMatrix();
}
