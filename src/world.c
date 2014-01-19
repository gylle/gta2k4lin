#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <GL/glew.h>

#include "Bullet-C-Api.h"
#include "btwrap.h"

#include "gl.h"
#include "models.h"
#include "world.h"

struct world world;

int map_load() {
    world.map.cubes = (struct cube *)calloc(world.map.nrcubex * world.map.nrcubey * world.map.nrcubez, sizeof(struct cube));
    if (world.map.cubes == NULL)  {
        return 0;
    }
    memset(world.map.cubes, 0, world.map.nrcubex * world.map.nrcubey * world.map.nrcubez *
           sizeof(struct cube));

    int loop1, loop2, loop3;

    for(loop1=0;loop1<world.map.nrcubex;loop1++) {
        for(loop2=0;loop2<world.map.nrcubey;loop2++) {
            for(loop3=0;loop3<world.map.nrcubez;loop3++) {
                map_cube(world, loop1, loop2, loop3).o.size_x=BSIZE * 2;
                map_cube(world, loop1, loop2, loop3).o.size_y=BSIZE * 2;
                map_cube(world, loop1, loop2, loop3).o.size_z=BSIZE * 2;
                map_cube(world, loop1, loop2, loop3).o.x=loop1 * BSIZE * 2;
                map_cube(world, loop1, loop2, loop3).o.y=loop2 * BSIZE * 2;
                map_cube(world, loop1, loop2, loop3).o.z= -1 * loop3 * BSIZE * 2;
                map_cube(world, loop1, loop2, loop3).texturenr=0;

                object_update_circle(&(map_cube(world, loop1, loop2, loop3).o));
            }
        }
    }

    map_cube(world, 0, 0, 0).o.z = BSIZE;
    map_cube(world, 0, 0, 0).texturenr=1;

    map_cube(world, 0, 1, 0).o.z = BSIZE * 2;
    map_cube(world, 0, 1, 0).texturenr=1;

    // Vägen -------------------------------
    for(loop1=0;loop1<world.map.nrcubey;loop1++)
        map_cube(world, 1, loop1, 0).texturenr=2;

    for(loop1=0;loop1<world.map.nrcubey;loop1++)
        map_cube(world, 2, loop1, 0).texturenr=3;

    for(loop1=0;loop1<world.map.nrcubex;loop1++)
        map_cube(world, loop1, world.map.nrcubey-2, 0).texturenr=8;

    for(loop1=2;loop1<world.map.nrcubex;loop1++)
        map_cube(world, loop1, world.map.nrcubey-3, 0).texturenr=9;

    map_cube(world, 1, world.map.nrcubey-2, 0).texturenr=5;
    map_cube(world, 2, world.map.nrcubey-2, 0).texturenr=6;
    map_cube(world, 2, world.map.nrcubey-3, 0).texturenr=7;


    // "Väggen" runtomkring
    for(loop1=0;loop1<world.map.nrcubey;loop1++) {
        map_cube(world, 0, loop1, 0).texturenr=4;
        map_cube(world, 0, loop1, 0).o.z = BSIZE * 2;
    }

    for(loop1=0;loop1<world.map.nrcubey;loop1++) {
        map_cube(world, world.map.nrcubex-1, loop1, 0).texturenr=4;
        map_cube(world, world.map.nrcubex-1, loop1, 0).o.z = BSIZE * 2;
    }

    for(loop1=0;loop1<world.map.nrcubex;loop1++) {
        map_cube(world, loop1, 0, 0).texturenr=4;
        map_cube(world, loop1, 0, 0).o.z = BSIZE * 2;
    }
    for(loop1=0;loop1<world.map.nrcubex;loop1++) {
        map_cube(world, loop1, world.map.nrcubey-1, 0).texturenr=4;
        map_cube(world, loop1, world.map.nrcubey-1, 0).o.z = BSIZE * 2;
    }

    // Vi lägger in lite buskar
    for(loop1=1;loop1<(world.map.nrcubey/2-1);loop1+=2) {
        map_cube(world, world.map.nrcubex/2, loop1, 0).texturenr=15;
        map_cube(world, world.map.nrcubex/2, loop1, 0).o.z = BSIZE * 2;
    }

    // Vägen in till mitten och den fina credits saken där.
    for(loop1=3;loop1<=world.map.nrcubex/2;loop1++)
        map_cube(world, loop1, world.map.nrcubey/2, 0).texturenr=7;

    map_cube(world, world.map.nrcubex/2, world.map.nrcubey/2, 1).texturenr=10;
    map_cube(world, world.map.nrcubex/2, world.map.nrcubey/2, 1).o.z = BSIZE * 2 * 2;

    /* FIXME: Fult */
    for(loop1=0;loop1<world.map.nrcubex;loop1++) {
        for(loop2=0;loop2<world.map.nrcubey;loop2++) {
            for(loop3=0;loop3<world.map.nrcubez;loop3++) {
                /* Bullet */
                plCollisionShapeHandle cube_shape;
                plRigidBodyHandle cube_rbody;
                void *user_data = NULL;

                cube_shape = plNewBoxShape(BSIZE, BSIZE, BSIZE);
                cube_rbody = plCreateRigidBody(user_data, 0.0f, cube_shape);

                struct object *cube_object = &map_cube(world, loop1, loop2, loop3).o;

                float cube_pos[3];
                cube_pos[0] = cube_object->x;
                cube_pos[1] = cube_object->y;
                cube_pos[2] = cube_object->z;

                /* plVector3 == float[3] */
                plSetPosition(cube_rbody, cube_pos);

                plAddRigidBody(world.dynamics_world, cube_rbody);

                /* Update the model matrix */
                mat4x4_identity(cube_object->m_rotation);
                mat4x4_translate(cube_object->m_translation, cube_object->x, cube_object->y, cube_object->z);
                mat4x4_mul(cube_object->m_translationRotation, cube_object->m_translation, cube_object->m_rotation);
            }
        }
    }

    /* Load some graphics stuff */
    GLuint vbo = -1;
    GLuint vbo_indices;
    GLuint shader;
    GLuint nr_of_indices;

    if(vbo == -1) {
        GLuint stride = 5*sizeof(GLfloat);
        GLuint nr_of_vertices = sizeof(map_cube_vertices)/stride;

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

        vbo = gl_new_buffer_object(GL_ARRAY_BUFFER, nr_of_vertices*sizeof(GLfloat)*5, map_cube_vertices);
        vbo_indices = gl_new_buffer_object(GL_ELEMENT_ARRAY_BUFFER, nr_of_indices*sizeof(GLuint), indices);

        shader = gl_new_program_from_files("data/shaders/default.vert", "data/shaders/default.frag");

        free(indices);
    }
    world.map.r_o.vbo = vbo;
    world.map.r_o.vbo_indices = vbo_indices;
    world.map.r_o.shader = shader;
    world.map.r_o.nr_of_indices = nr_of_indices;

    return 1;
}

void map_draw() {
    int x, y, z;

    glUseProgram(world.map.r_o.shader);

    glBindBuffer(GL_ARRAY_BUFFER, world.map.r_o.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, world.map.r_o.vbo_indices);

    GLint a_position = glGetAttribLocation(world.map.r_o.shader, "a_position");
    //GLint a_normal = glGetAttribLocation(world.map.r_o.shader, "a_normal");
    GLint a_texcoord = glGetAttribLocation(world.map.r_o.shader, "a_texcoord");

    GLint u_modelView = glGetUniformLocation(world.map.r_o.shader, "u_modelView");
    GLint u_projection = glGetUniformLocation(world.map.r_o.shader, "u_projection");
    GLint u_texture1 = glGetUniformLocation(world.map.r_o.shader, "texture1");

    glUniformMatrix4fv(u_projection, 1, 0, (GLfloat*)world.camera.projection);

    GLuint stride = 5*sizeof(GLfloat);
    glVertexAttribPointer(a_position, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(a_position);
    /* glVertexAttribPointer(a_normal, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(GLfloat))); */
    /* glEnableVertexAttribArray(a_normal); */
    glVertexAttribPointer(a_texcoord, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(a_texcoord);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(u_texture1, 0);

    /* float x_pos, y_pos, z_pos; */
    for(x=0; x<world.map.nrcubex; x++) {
        for(y=0; y<world.map.nrcubey; y++) {
            for(z=0; z<world.map.nrcubez; z++) {

                struct object *cube_object = &map_cube(world, x, y, z).o;

                mat4x4 modelView;
                mat4x4_mul(modelView, world.camera.view, cube_object->m_translationRotation);
                glUniformMatrix4fv(u_modelView, 1, 0, (GLfloat*)modelView);

                glBindTexture(GL_TEXTURE_2D,world.texIDs[map_cube(world, x, y, z).texturenr]);
                glDrawElements(GL_TRIANGLES, world.map.r_o.nr_of_indices, GL_UNSIGNED_INT, (void*)0);
            }
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glUseProgram(0);
}

void camera_init() {
    world.camera.x = 0.0f;
    world.camera.y = 0.0f;
    world.camera.z = -30.0f;
}

void camera_move_for_car(struct car *car) {
    world.camera.x = car->o.x;
    world.camera.y = car->o.y;

    // Få kameran att höjas och sänkas beroende på hastigheten...
    GLfloat tmpSpeedVar=car->o.speed*5;
    if(tmpSpeedVar>0)
        tmpSpeedVar=-tmpSpeedVar;

    if(tmpSpeedVar>world.camera.SpeedVar)
        world.camera.SpeedVar+=0.4f;

    if(tmpSpeedVar<world.camera.SpeedVar)
        world.camera.SpeedVar-=0.4f;
}

void opponents_init() {
    int i;

    for (i = 0; i < NETWORK_MAX_OPPONENTS; i++) {
        world.opponents[i].o = &(world.opponent_cars[i].o);
        world.opponent_cars[i].t1=1;
        world.opponent_cars[i].t2=1;
        world.opponent_cars[i].t3=1;
        world.opponent_cars[i].t4=1;
        car_set_model(&world.opponent_cars[i]);
    }
}

int cars_init() {
    camera_init(&world.camera);

    init_car(&world.bil);

    return 1;
}

int gubbar_init() {
    world.gubbar = malloc(sizeof(struct gubbe)*nrgubbar);
    memset(world.gubbar, 0, sizeof(struct gubbe)*nrgubbar);

    int i;
    for(i = 0; i < nrgubbar; i++) {
        init_gubbe(&world.gubbar[i]);
        printf("init_gubbe: %p:\n", &world.gubbar[i]);
    }

    return 1;
}

int world_init() {
    /* Bullet */
    world.physics_sdk = plNewBulletSdk();
    world.dynamics_world = plCreateDynamicsWorld(world.physics_sdk);
    plDynamicsWorld_SetGravity(world.dynamics_world, 0, 0, -30.0f);

    world.map.nrcubex = 20;
    world.map.nrcubey = 20;
    world.map.nrcubez = 2;

    map_load();

    camera_init();
    opponents_init();
    cars_init();
    gubbar_init();

    return 1;
}
