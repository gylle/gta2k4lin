/*
 * gta2k4lin
 *
 * Copyright David Hedberg  2001,2012
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

#include <GL/gl.h>

#include "stl.h"
#include "linmath.h"

struct stlb_header {
    unsigned char header[80];
    uint32_t nr_of_triangles;
};

struct stlb_triangle {
    GLfloat normal[3];
    GLfloat v1[3];
    GLfloat v2[3];
    GLfloat v3[3];
    unsigned char ac[2];
};

static const int szheader = sizeof(char)*80 + sizeof(uint32_t);
static const int sztriangle = sizeof(GLfloat)*12 + sizeof(char)*2;

static void *loadFile(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if(!file) {
        printf("Failed to open %s\n", filename);
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    int sz = ftell(file);
    fseek(file, 0L, SEEK_SET);

    char *data = malloc(sizeof(char) * sz);
    if(!data) {
        printf("Out of memory.\n");
        fclose(file);
        return NULL;
    }

    size_t done = fread(data, sizeof(char), sz, file);
    fclose(file);

    if(done != sz) {
        printf("Failed to load file %s (read %zu, expected %d)\n", filename, done, sz);
        free(data);
        return NULL;
    }

    return data;
}

#define max(a,b) ((a)>(b) ? (a) : (b))
#define min(a,b) ((a)<(b) ? (a) : (b))

struct stl_model *load_stl_model(const char *filename)
{
    /* vertex[3], normal[3], texcoord[2] */
    static const int stride = 3 + 3 + 2;

    char *filedata = loadFile(filename);
    if(!filedata) {
        printf("Failed to open stl file %s\n", filename);
        return NULL;
    }

    struct stlb_header *header = (struct stlb_header*)filedata;

    struct stl_model *model = malloc(sizeof(struct stl_model));
    model->data = malloc(sizeof(GLfloat)*header->nr_of_triangles*3*stride);
    model->nr_of_vertices = header->nr_of_triangles*3;

    int i;
    for(i = 0; i < 3; i++) {
        model->min[i] = FLT_MAX;
        model->max[i] = FLT_MIN;
    }

    for(i = 0; i < header->nr_of_triangles; i++)
    {
        struct stlb_triangle *t =
            (struct stlb_triangle*)(filedata + szheader + sztriangle*i);

        vec3 vector1, vector2, normal;
        vec3_sub(vector1, t->v1, t->v2);
        vec3_sub(vector2, t->v3, t->v2);
        vec3_mul_cross(normal, vector1, vector2);
        vec3_norm(normal, normal);

        int j;
        for(j = 0; j < 3; j++)
        {
            model->data[i*3*stride+j] = t->v1[j];
            model->data[i*3*stride+j+3] = normal[j];
            /* 2 texcoord */
            model->data[i*3*stride+j+8] = t->v2[j];
            model->data[i*3*stride+j+11] = normal[j];
            /* 2 texcoord */
            model->data[i*3*stride+j+16] = t->v3[j];
            model->data[i*3*stride+j+19] = normal[j];
            /* 2 texcoord */

            model->min[j] = min(model->min[j], t->v1[j]);
            model->max[j] = max(model->max[j], t->v1[j]);
            model->min[j] = min(model->min[j], t->v2[j]);
            model->max[j] = max(model->max[j], t->v2[j]);
            model->min[j] = min(model->min[j], t->v3[j]);
            model->max[j] = max(model->max[j], t->v3[j]);
        }
    }
    free(filedata);

    /* Calculate texture coordinates.
     * We just drag it out across the x/y-plane for now.
     */
    float m_x = 1.0f/(model->max[0] - model->min[0]);
    float m_y = 1.0f/(model->max[1] - model->min[1]);
    float k_x = -(m_x*model->min[0]);
    float k_y = -(m_y*model->min[1]);
    for(i = 0; i < model->nr_of_vertices; i++)
    {
        /* For each vertex.. */
        float *v = &model->data[i*stride];
        float texcoord_x = v[0]*m_x + k_x;
        float texcoord_y = v[1]*m_y + k_y;
        v[6] = texcoord_x;
        v[7] = texcoord_y;
    }
    return model;
}
