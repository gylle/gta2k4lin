#ifndef __STL_H__
#define __STL_H__

struct stl_model {
    int nr_of_vertices;
    float min[3], max[3];
    float *data;
};

struct stl_model *load_stl_model(const char *filename);

#endif /* __STL_H__ */
