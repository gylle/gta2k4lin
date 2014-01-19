#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "gubbe.h"
#include "main.h"
#include "world.h"
#include "gl.h"
#include "linmath.h"

static const int gubbtid=300;		// Hur lång tid en gubbe är död... Räknas i frames :)

static GLfloat gubbe_vertices[] = {
    -GUBBSIZE_X/2, -GUBBSIZE_Y/2, GUBBSIZE_Z/2.0f, 0.0f, 0.0f, /* Top */
    -GUBBSIZE_X/2, GUBBSIZE_Y/2, GUBBSIZE_Z/2.0f, 0.0f, 1.0f,
    GUBBSIZE_X/2, GUBBSIZE_Y/2, GUBBSIZE_Z/2.0f, 1.0f, 1.0f,
    GUBBSIZE_X/2, -GUBBSIZE_Y/2, GUBBSIZE_Z/2.0f, 1.0f, 0.0f,
    GUBBSIZE_X/2, -GUBBSIZE_Y/2, -GUBBSIZE_Z/2.0f, 1.0f, 0.0f, /* Bottom */
    GUBBSIZE_X/2, GUBBSIZE_Y/2, -GUBBSIZE_Z/2.0f, 1.0f, 1.0f,
    -GUBBSIZE_X/2, GUBBSIZE_Y/2, -GUBBSIZE_Z/2.0f, 0.0f, 1.0f,
    -GUBBSIZE_X/2, -GUBBSIZE_Y/2, -GUBBSIZE_Z/2.0f, 0.0f, 0.0f,
    /* Left */
    -GUBBSIZE_X/2, GUBBSIZE_Y/2, -GUBBSIZE_Z/2.0f, 0.0f, 0.0f, /* Body */
    -GUBBSIZE_X/2, GUBBSIZE_Y/2, GUBBSIZE_Z/2.0f, 0.0f, 1.0f,
    -GUBBSIZE_X/2, -GUBBSIZE_Y/2, GUBBSIZE_Z/2.0f,1.0f, 1.0f,
    -GUBBSIZE_X/2, -GUBBSIZE_Y/2, -GUBBSIZE_Z/2.0f,1.0f, 0.0f,
    /* Right */
    GUBBSIZE_X/2, -GUBBSIZE_Y/2, -GUBBSIZE_Z/2.0f, 0.0f, 0.0f,
    GUBBSIZE_X/2, -GUBBSIZE_Y/2, GUBBSIZE_Z/2.0f, 0.0f, 1.0f,
    GUBBSIZE_X/2, GUBBSIZE_Y/2, GUBBSIZE_Z/2.0f, 1.0f, 1.0f,
    GUBBSIZE_X/2, GUBBSIZE_Y/2, -GUBBSIZE_Z/2.0f, 1.0f, 0.0f,
    /* Front */
    -GUBBSIZE_X/2, -GUBBSIZE_Y/2, -GUBBSIZE_Z/2.0f, 0.0f, 0.0f,
    -GUBBSIZE_X/2, -GUBBSIZE_Y/2, GUBBSIZE_Z/2.0f, 0.0f, 1.0f,
    GUBBSIZE_X/2, -GUBBSIZE_Y/2, GUBBSIZE_Z/2.0f, 1.0f, 1.0f,
    GUBBSIZE_X/2, -GUBBSIZE_Y/2, -GUBBSIZE_Z/2.0f, 1.0f, 0.0f,
    /* Back */
    GUBBSIZE_X/2, GUBBSIZE_Y/2, -GUBBSIZE_Z/2.0f, 0.0f, 0.0f,
    GUBBSIZE_X/2, GUBBSIZE_Y/2, GUBBSIZE_Z/2.0f, 0.0f, 1.0f,
    -GUBBSIZE_X/2, GUBBSIZE_Y/2, GUBBSIZE_Z/2.0f, 1.0f, 1.0f,
    -GUBBSIZE_X/2, GUBBSIZE_Y/2, -GUBBSIZE_Z/2.0f, 1.0f, 0.0f
};

static void gubbe_get_world_transform(void *data, plVector3 v, plReal *m) {
    /* printf("get_world_transform (%p, %p, %p)\n", data, v, m); */

    struct gubbe *g = (struct gubbe*)data;

    v[0] = g->o.x;
    v[1] = g->o.y;
    v[2] = g->o.z;

    memcpy(m, g->o.m_rotation, sizeof(float)*16);
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

    memcpy(g->o.m_rotation, m, sizeof(float)*16);

    /* FIXME? */
    g->o.m_rotation[3][3] = 1.0f;
}

void gubbe_set_model(struct gubbe *g);
void init_gubbe(struct gubbe *g) {
    gubbe_set_model(g);

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

    g->o.x=(float)((rand() % world.map.nrcubex*BSIZE*2)*100)/100.0f;
    g->o.y=(float)((rand() % world.map.nrcubey*BSIZE*2)*100)/100.0f;
    g->o.angle=rand() % 360;

    g->o.z = BSIZE + GUBBSIZE_Z/2.0f + 0.1f;

    g->ltexture=11;
    g->ltexture2=13;
    g->dtexture=12;

    g->atimer = 0;

    /* Initialize bullet stuff */
    float bt_weight = 1.0f;
    void *user_data = NULL; /* 1.0f */
    g->bt_shape = plNewBoxShape(GUBBSIZE_X/2.0f, GUBBSIZE_Y/2.0f, GUBBSIZE_Z/2.0f);

    mat4x4_identity((vec4*)g->o.m_rotation);

    g->bt_rbody = plCreateRigidBodyWithCallback(user_data, bt_weight, g->bt_shape,
                                                &gubbe_get_world_transform,
                                                &gubbe_set_world_transform,
                                                g);

    plAddRigidBody(world.dynamics_world, g->bt_rbody);
    /* printf("Added gubbe-rbody %p\n", g->bt_rbody); */
}

void gubbe_render(struct gubbe *g) {
    glPushMatrix();

    glTranslatef(g->o.x,g->o.y,g->o.z);

    glMultMatrixf(g->o.m_rotation);

    glUseProgram(g->o.shader);

    glBindBuffer(GL_ARRAY_BUFFER, g->o.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g->o.vbo_indices);

    GLint a_position = glGetAttribLocation(g->o.shader, "a_position");
    //GLint a_normal = glGetAttribLocation(g->o.shader, "a_normal");
    GLint a_texcoord = glGetAttribLocation(g->o.shader, "a_texcoord");

    GLint u_texture1 = glGetUniformLocation(g->o.shader, "texture1");

    GLuint stride = 5*sizeof(GLfloat);
    glVertexAttribPointer(a_position, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(a_position);
    /* glVertexAttribPointer(a_normal, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(GLfloat))); */
    /* glEnableVertexAttribArray(a_normal); */
    glVertexAttribPointer(a_texcoord, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(a_texcoord);

    glActiveTexture(GL_TEXTURE0);
    /* glBindTexture(GL_TEXTURE_2D, world.texIDs[g->ltexture2]); */
    glBindTexture(GL_TEXTURE_2D, world.texIDs[13]);
    glUniform1i(u_texture1, 0);

    glDrawElements(GL_TRIANGLES, g->o.nr_of_indices, GL_UNSIGNED_INT, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glUseProgram(0);

    glPopMatrix();
}

void gubbe_set_model(struct gubbe *g) {
    static GLuint vbo = -1;
    static GLuint vbo_indices;
    static GLuint shader;
    static GLuint nr_of_indices;

    if(vbo == -1) {
        GLuint stride = 5*sizeof(GLfloat);
        GLuint nr_of_vertices = sizeof(gubbe_vertices)/stride;

        nr_of_indices = (nr_of_vertices/4)*6;
        GLuint *indices = malloc(nr_of_indices * sizeof(GLuint));

        /* We make a couple of triangles from each quad */
        unsigned int i, j;
        for(i = 0, j = 0; i < nr_of_indices; i += 6, j += 4) {
            indices[i] = j;
            indices[i+1] = j+1;
            indices[i+2] = j+2;

            indices[i+3] = j;
            indices[i+4] = j+2;
            indices[i+5] = j+3;

            printf("i/j: [%d, %d] -> (%d, %d, %d),(%d, %d, %d)", i,j, j, j+1, j+2, j, j+2, j+3);
        }

        vbo = gl_new_buffer_object(GL_ARRAY_BUFFER, nr_of_vertices*sizeof(GLfloat)*5, gubbe_vertices);
        vbo_indices = gl_new_buffer_object(GL_ELEMENT_ARRAY_BUFFER, nr_of_indices*sizeof(GLuint), indices);

        shader = gl_new_program_from_files("data/shaders/default.vert", "data/shaders/default.frag");

        free(indices);
    }
    g->o.vbo = vbo;
    g->o.vbo_indices = vbo_indices;
    g->o.shader = shader;
    g->o.nr_of_indices = nr_of_indices;
}
