#include <stdio.h>

#include <linux/limits.h>
#include <SDL.h>
#include <SDL_image.h>
#include <GL/glew.h>
#include <GL/glu.h>

#include "gl.h"
#include "main.h"
#include "network.h"
#include "hud.h"
#include "gubbe.h"
#include "car.h"
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

    map_draw();

    car_render(&world.bil);
    if (Network) {
        for (i = 0; i < NETWORK_MAX_OPPONENTS; i++) {
            if (world.opponents[i].in_use)
                car_render(&(world.opponent_cars[i]));
        }
    }

    if(!Network) {
        int i;
        for(i=0; i<nrgubbar; i++) {
            gubbe_render(&world.gubbar[i]);
        }

    }

    hud_render();

    return 1;
}

static GLuint gl_new_shader(GLenum type, const char **source)
{
    GLuint name;

    name = glCreateShader(type);

    glShaderSource(name, 1, source, NULL);
    glCompileShader(name);

    GLint shader_ok;

    glGetShaderiv(name, GL_COMPILE_STATUS, &shader_ok);

    if(!shader_ok)
    {
        GLint logLength;
        glGetShaderiv(name, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0)
        {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetShaderInfoLog(name, logLength, &logLength, log);
            printf("Error in Shader Creation:\n%s\n",log);
            free(log);
        }

        glDeleteShader(name);

        return 0;
    }

    return name;
}

GLuint gl_new_program(GLuint vertexShader, GLuint fragmentShader)
{
    GLuint name;

    name = glCreateProgram();

    glAttachShader(name, vertexShader);
    glAttachShader(name, fragmentShader);

    glLinkProgram(name);

    GLint link_ok = 0;

    glGetProgramiv(name, GL_LINK_STATUS, &link_ok);

    if(!link_ok)
    {
        GLint logLength;
        glGetProgramiv(name, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0)
        {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetProgramInfoLog(name, logLength, &logLength, log);
            printf("Error when linking program:\n%s\n",log);
            free(log);
        }

        glDeleteProgram(name);

        return 0;
    }

    return name;
}

GLchar *load_file_contents(const char *filename)
{
    FILE *file;
    int sz;
    size_t done;
    char *data;

    if( !(file = fopen(filename, "rb")) ) {
        printf("Failed to open %s\n", filename);
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    sz = ftell(file);
    fseek(file, 0L, SEEK_SET);

    if( !(data = malloc(sizeof(char) * (sz+1))) ) {
        printf("Out of memory.\n");
        fclose(file);
        return NULL;
    }

    done = fread(data, sizeof(char), sz, file);

    fclose(file);

    if( done != sz ) {
        printf("Failed to load file %s (read %zu, expected %d)\n", filename, done, sz);
        free(data);
        return NULL;
    }

    data[sz] = '\0';

    return data;
}

GLuint gl_new_program_from_files(const char *vfile, const char *ffile)
{
    char *vsrc = load_file_contents(vfile);
    char *fsrc = load_file_contents(ffile);
    GLuint vsh, fsh, program;

    printf("Creating program from %s and %s...\n", vfile, ffile);

    vsh = gl_new_shader(GL_VERTEX_SHADER, (const char**)&vsrc);
    fsh = gl_new_shader(GL_FRAGMENT_SHADER, (const char**)&fsrc);

    free(vsrc);
    free(fsrc);

    program = gl_new_program(vsh, fsh);

    glDeleteShader(vsh);
    glDeleteShader(fsh);

    return program;
}

GLuint gl_new_buffer_object(GLenum type, GLsizeiptr size, const GLvoid *data)
{
    GLuint buffer;

    glGenBuffers(1, &buffer);

    glBindBuffer(type, buffer);
    glBufferData(type, size, data, GL_STATIC_DRAW);

    return buffer;
}


int gl_init(SDL_Window *window, int width, int height)
{
    SDL_GL_CreateContext(window);

    glewInit();

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
