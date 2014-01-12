/*
 * gta2k4lin
 *
 * Copyright David Hedberg  2001,2012
 * Copyright Jonas Eriksson 2012
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


/* Some code originally from nehe.gamedev.com,
 * original notice as follows:
 * -------------------------------------------------------
 * This Code Was Created By Jeff Molofee 2000
 * A HUGE Thanks To Fredric Echols For Cleaning Up
 * And Optimizing This Code, Making It More Flexible!
 * If You've Found This Code Useful, Please Let Me Know.
 * Visit My Site At nehe.gamedev.net
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <GL/gl.h>			// Header File For The OpenGL32 Library
#include <GL/glu.h>			// Header File For The GLu32 Library
#include <SDL.h>
#include <sys/time.h>
#include <sys/param.h>
#include <getopt.h>

#include "SDL_image.h"

#include "sound.h"
#include "network.h"
#include "hud.h"
#include "object.h"
#include "models.h"
#include "stl.h"

int initial_width = 640;
int initial_height = 480;
const int sdl_video_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
const int sdl_bpp = 32; // Vi gillar 32 här dåva

#define bool  int
#define false 0
#define true  1

#define nrgubbar 100

bool keys[SDL_NUM_SCANCODES];			// Array Used For The Keyboard Routine

bool NoBlend=true;

bool sound = true;
int sound_music = 1;

// Enable:as bara ifall man ska försöka få igång nätverket...
bool Network=false;			// Nätverk eller singelplayer...

char *server_addr = NULL;
unsigned server_port = 9378;
char *nick = NULL;
int proto_only = 0;

bool debugBlend=false;
float blendcolor;



struct cube {
	// Vilken textur som ska mappas till kuben...
	int texturenr;
	int in_use;
	// Ett namn på stället man är.
	const char* beskrivning;

	struct object o;
};

struct car {
	// Texturer, nr1: tak. nr2: sidor. nr3: fram. nr4: bak:
	int t1,t2,t3,t4;
	// Fartsaker
	float maxspeed,maxbspeed, accspeed, bromsspeed;
	int turnspeed;
	// Hur fort bilen stannar om man inte gasar
	float speeddown;
	// Hur hel bilen är(%). 100 är helhel, 0 är heltrasig.
	int helhet;
	// Poäng. I Multiplayer spel hur många "frags" man har...
	int Points;

	struct object o;
	struct stl_model *model;
};

struct camera {
    GLfloat x, y, z;
    GLfloat SpeedVar;
};

struct camera camera;

static void camera_init() {
    camera.x = 0.0f;
    camera.y = 0.0f;
    camera.z = -30.0f;
}

static void camera_move_for_car(struct car *car) {
    camera.x = car->o.x;
    camera.y = car->o.y;

    // Få kameran att höjas och sänkas beroende på hastigheten...
    GLfloat tmpSpeedVar=car->o.speed*5;
    if(tmpSpeedVar>0)
        tmpSpeedVar=-tmpSpeedVar;

    if(tmpSpeedVar>camera.SpeedVar)
        camera.SpeedVar+=0.4f;

    if(tmpSpeedVar<camera.SpeedVar)
        camera.SpeedVar-=0.4f;
}

struct gubbe {
	int ltexture, ltexture2,  dtexture;
	float maxspeed,maxbspeed,accspeed;
	bool alive;

	// Time to respawn
	int atimer;

	struct object o;
};
const int gubbe_respawn_frames=300;		// Hur lång tid en gubbe är död... Räknas i frames :)

GLuint	GubbeDispList = 0;

struct spelare {
	int points;
	int krockar;
	int runovers;
};

struct gubbe *gubbar; //[nrgubbar];

struct spelare player;

struct world {
	int nrcubex;
	int nrcubey;
	int nrcubez;
	struct cube *map;
	const char **texture_filenames;
	GLuint *texIDs;
	int ntextures;
};

#define TEXTURE_PATH "data/"
#define map_cube(world, x, y, z) world.map[((x) * (world).nrcubey + (y)) * (world).nrcubez + (z)]
struct world world;

static void draw_quads(float vertices[], int count) {
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

static void draw_stl_model(struct stl_model *m) {
    static const int stride = 3 + 3 + 2;

    glBegin(GL_TRIANGLES);
        int i;
        for(i = 0; i < m->nr_of_vertices; i++) {
            int offset = i*stride;
            glTexCoord2f(m->data[offset+6], m->data[offset+7]);
            glNormal3f(m->data[offset+3], m->data[offset+4], m->data[offset+5]);
            glVertex3f(m->data[offset], m->data[offset+1], m->data[offset+2]);
        }
    glEnd();
}

struct car bil;
struct car opponent_cars[NETWORK_MAX_OPPONENTS];
struct opponent opponents[NETWORK_MAX_OPPONENTS];

static void init_gubbe(struct gubbe *g) {
    g->alive=true;

    g->o.speed=0.0f;
    g->maxspeed=0.3f;
    g->accspeed=0.15f;
    g->maxbspeed=0.2f;

    g->o.size_x=GUBBSIZE_X;
    g->o.size_y=GUBBSIZE_Y;
    g->o.size_z=GUBBSIZE_Z;
    object_update_circle(&(g->o));

    g->o.x=(float)((rand() % world.nrcubex*BSIZE*2)*100)/100.0f;
    g->o.y=(float)((rand() % world.nrcubey*BSIZE*2)*100)/100.0f;
    g->o.angle=rand() % 360;

    g->o.z=BSIZE + GUBBSIZE_Z / 2;

    g->ltexture=11;
    g->ltexture2=13;
    g->dtexture=12;

    g->atimer = 0;
}

static void gubbe_move(struct gubbe *g) {
    if(g->alive) {
        int tmprand=rand() % 100;

        if(tmprand==0 && tmprand<3)
            g->o.angle+=10;

        if(tmprand>=3 && tmprand<5)
            g->o.angle-=10;

        if(tmprand>=5 && tmprand<=100)
            g->o.speed=g->o.speed+g->accspeed;



    } else {
        g->atimer++;

        if(g->atimer>=gubbe_respawn_frames) {
            g->atimer=0;
            g->alive=true;

            g->o.x=(float)((rand() % world.nrcubex*BSIZE*2)*100)/100;
            g->o.y=(float)((rand() % world.nrcubey*BSIZE*2)*100)/100;
            g->o.angle=rand() % 360;
        }
    }

    if(g->o.speed<0) {
        if(g->o.speed<g->maxbspeed)
            g->o.speed=g->maxbspeed;
    } else {
        if(g->o.speed>g->maxspeed)
            g->o.speed=g->maxspeed;
    }

    if(g->o.angle<0)
        g->o.angle=g->o.angle+360;
    if(g->o.angle>359)
        g->o.angle=g->o.angle-360;

    if (g->alive)
	object_forward(&(g->o));
}

static void gubbe_render(struct gubbe *g) {
    glPushMatrix();

    glTranslatef(g->o.x,g->o.y,0);
    glRotatef((float)g->o.angle,0.0f,0.0f,1.0f);

    if(g->alive) {
        if(!NoBlend)
            glEnable(GL_BLEND);

        glColor4f(1.0f,1.0f,1.0f,255);
        glCallList(GubbeDispList);

        if(blendcolor==0.0f)
            glDisable(GL_BLEND);
    } else {
        glBindTexture(GL_TEXTURE_2D,world.texIDs[g->dtexture]);

        glTranslatef(0.0f, 0.0f, -GUBBSIZE_Z+0.01f);
        draw_quads(gubbe_top_vertices, sizeof(gubbe_top_vertices)/(20*sizeof(float)));
    }
    glPopMatrix();
}

static void init_gubbe_displaylist() {
    GubbeDispList=glGenLists(1);

    glNewList(GubbeDispList,GL_COMPILE);
        glBindTexture(GL_TEXTURE_2D,world.texIDs[gubbar[0].ltexture2]);
        draw_quads(gubbe_top_vertices, sizeof(gubbe_top_vertices)/(20*sizeof(float)));

        glBindTexture(GL_TEXTURE_2D,world.texIDs[gubbar[0].ltexture]);
        draw_quads(gubbe_body_vertices, sizeof(gubbe_body_vertices)/(20*sizeof(float)));
    glEndList();
}

static void car_set_model(struct car *bil) {
    static struct stl_model *car_model;

    if(car_model == NULL) {
        car_model = load_stl_model("data/gta2kcar.stl");
    }
    bil->model = car_model;
}

static void init_car(struct car *bil) {
    car_set_model(bil);

    /* FIXME: CARSIZE can (should?) theoretically be replaced by values
     *  calculated from the model's min/max values.
     */
    bil->o.size_x=CARSIZE_X;
    bil->o.size_y=CARSIZE_Y;
    bil->o.size_z=CARSIZE_Z;
    object_update_circle(&(bil->o));

    bil->helhet=100;

    bil->o.x=10;
    bil->o.y=10;
    bil->o.z=BSIZE + CARSIZE_Z / 2; // Ska nog inte initialiseras här..

    bil->t1=1;
    bil->t2=1;
    bil->t3=1;
    bil->t4=1;

    bil->maxspeed=2.0f;
    bil->o.speed=0.0f;
    bil->accspeed=0.20f;
    bil->maxbspeed=-1.0f;
    bil->bromsspeed=0.3f;
    bil->speeddown=0.10f;
    bil->turnspeed=8;

    bil->o.angle=0;

    bil->Points=0;

}

static void car_render(struct car *bil) {

    glPushMatrix();

    glTranslatef(bil->o.x, bil->o.y, bil->o.z);
    glRotatef((float)bil->o.angle,0.0f,0.0f,1.0f);

    glBindTexture(GL_TEXTURE_2D,world.texIDs[bil->t1]);
    draw_stl_model(bil->model);

    glPopMatrix();
}

static int LoadGLTextures()								// Load Bitmaps And Convert To Textures
{
	/* Load textures from file names in world */
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

	return 1;							// Return The Status
}

static int LoadLevel()
{
	// Allocate them cubes!
	world.map = (struct cube *)calloc(world.nrcubex * world.nrcubey * world.nrcubez, sizeof(struct cube));
	if (world.map == NULL)  {
		return false;
	}
	memset(world.map, 0, world.nrcubex * world.nrcubey * world.nrcubez *
			sizeof(struct cube));

	int loop1, loop2, loop3;

	for(loop1=0;loop1<world.nrcubex;loop1++) {
		for(loop2=0;loop2<world.nrcubey;loop2++) {
			for(loop3=0;loop3<world.nrcubez;loop3++) {
				map_cube(world, loop1, loop2, loop3).o.size_x=BSIZE * 2;
				map_cube(world, loop1, loop2, loop3).o.size_y=BSIZE * 2;
				map_cube(world, loop1, loop2, loop3).o.size_z=BSIZE * 2;
				map_cube(world, loop1, loop2, loop3).o.x=loop1 * BSIZE * 2;
				map_cube(world, loop1, loop2, loop3).o.y=loop2 * BSIZE * 2;
				map_cube(world, loop1, loop2, loop3).o.z= -1 * loop3 * BSIZE * 2;
				map_cube(world, loop1, loop2, loop3).texturenr=0;
				map_cube(world, loop1, loop2, loop3).beskrivning="Testbeskrivning";
				object_update_circle(&(map_cube(world, loop1, loop2, loop3).o));
			}
		}
	}

	map_cube(world, 0, 0, 0).o.z = BSIZE;
	map_cube(world, 0, 0, 0).texturenr=1;

	map_cube(world, 0, 1, 0).o.z = BSIZE * 2;
	map_cube(world, 0, 1, 0).texturenr=1;

	// Vägen -------------------------------
	for(loop1=0;loop1<world.nrcubey;loop1++)
		map_cube(world, 1, loop1, 0).texturenr=2;

	for(loop1=0;loop1<world.nrcubey;loop1++)
		map_cube(world, 2, loop1, 0).texturenr=3;

	for(loop1=0;loop1<world.nrcubex;loop1++)
		map_cube(world, loop1, world.nrcubey-2, 0).texturenr=8;

	for(loop1=2;loop1<world.nrcubex;loop1++)
		map_cube(world, loop1, world.nrcubey-3, 0).texturenr=9;

	map_cube(world, 1, world.nrcubey-2, 0).texturenr=5;
	map_cube(world, 2, world.nrcubey-2, 0).texturenr=6;
	map_cube(world, 2, world.nrcubey-3, 0).texturenr=7;


	// "Väggen" runtomkring
	for(loop1=0;loop1<world.nrcubey;loop1++) {
		map_cube(world, 0, loop1, 0).texturenr=4;
		map_cube(world, 0, loop1, 0).o.z = BSIZE * 2;
	}

	for(loop1=0;loop1<world.nrcubey;loop1++) {
		map_cube(world, world.nrcubex-1, loop1, 0).texturenr=4;
		map_cube(world, world.nrcubex-1, loop1, 0).o.z = BSIZE * 2;
	}

	for(loop1=0;loop1<world.nrcubex;loop1++) {
		map_cube(world, loop1, 0, 0).texturenr=4;
		map_cube(world, loop1, 0, 0).o.z = BSIZE * 2;
	}
	for(loop1=0;loop1<world.nrcubex;loop1++) {
		map_cube(world, loop1, world.nrcubey-1, 0).texturenr=4;
		map_cube(world, loop1, world.nrcubey-1, 0).o.z = BSIZE * 2;
	}

	// Vi lägger in lite buskar
	for(loop1=1;loop1<(world.nrcubey/2-1);loop1+=2) {
		map_cube(world, world.nrcubex/2, loop1, 0).texturenr=15;
		map_cube(world, world.nrcubex/2, loop1, 0).o.z = BSIZE * 2;
	}

	// Vägen in till mitten och den fina credits saken där.
	for(loop1=3;loop1<=world.nrcubex/2;loop1++)
		map_cube(world, loop1, world.nrcubey/2, 0).texturenr=7;

	map_cube(world, world.nrcubex/2, world.nrcubey/2, 1).texturenr=10;
	map_cube(world, world.nrcubex/2, world.nrcubey/2, 1).o.z = BSIZE * 2 * 2;

	return true;
}

static int LoadCars()
{
	camera_init(&camera);

	init_car(&bil);

	// Kicka igång alla gubbar
	gubbar = malloc(sizeof(struct gubbe)*nrgubbar);

	int i;
	for(i = 0; i < nrgubbar; i++) {
            init_gubbe(&gubbar[i]);
	}

        init_gubbe_displaylist();

	return true;
}

static void gl_resize(int width, int height) {

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

static int InitGL(int width, int height)
{
	if (!LoadGLTextures())							// Jump To Texture Loading Routine ( NEW )
	{
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

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearDepth(1.0f);									// Depth Buffer Setup
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	gl_resize(width, height);

	return true;										// Initialization Went OK
}

static int RespondToKeys()
{

	float sttmp;
	bool brakepressed=false;
        static int broms_in_progress = 0;

	if(keys[SDL_SCANCODE_SPACE]) {
		brakepressed=true;
		if(bil.o.speed<0.0f && bil.o.speed>-bil.bromsspeed)
			bil.o.speed=0.0f;
		if(bil.o.speed>0.0f && bil.o.speed<bil.bromsspeed)
			bil.o.speed=0.0f;

		if (!broms_in_progress) {
			sound_play(broms);
			broms_in_progress = 1;
		}

		if(bil.o.speed<0.0f)
			bil.o.speed+=bil.bromsspeed;
		if(bil.o.speed>0.0f)
			bil.o.speed-=bil.bromsspeed;

	}
	else {
		broms_in_progress = 0;
	}

	if(keys[SDL_SCANCODE_LEFT]) {
		sttmp=bil.o.speed/bil.maxspeed;			// Omöjliggör styrning vid stillastående, och
		if(brakepressed)
			sttmp+=0.7f;

		if(bil.o.speed!=0.0f)
			bil.o.angle+=bil.turnspeed*sttmp;				// öka graden av styrmöjlighet ju snabbare det går.
	}

	if(keys[SDL_SCANCODE_RIGHT]) {
		sttmp=bil.o.speed/bil.maxspeed;
		if(brakepressed)
			sttmp+=0.7f;

		if(bil.o.speed!=0.0f)
			bil.o.angle-=bil.turnspeed*sttmp;				// öka graden av styrmöjlighet ju snabbare det går.
	}

	if(bil.helhet==0) {
		if(keys[SDL_SCANCODE_RETURN]) {
			bil.helhet=100;
			bil.t1=1;
			/* mbil.Points++; */

			sound_cont_stop(brinner, 1);
			sound_play(respawn);

		}
		else {
			sound_cont_play(brinner);
		}
	}


	if(!(bil.helhet==0)) {
		if(keys[SDL_SCANCODE_UP]) {
			bil.o.speed=bil.o.speed+bil.accspeed;
		}

		if(keys[SDL_SCANCODE_DOWN]) {
			bil.o.speed=bil.o.speed-bil.accspeed;
		}
	}

	if(keys[SDL_SCANCODE_TAB]) {
		sound_cont_play(tut);
	}
	else {
		sound_cont_stop(tut, 0);
	}

	// Styr kameran
	if(keys[SDL_SCANCODE_F5]) {
		camera.z-=0.5f;
	}
	if(keys[SDL_SCANCODE_F6]) {
		camera.z+=0.5f;
	}

	if(keys[SDL_SCANCODE_F8]) {
		camera.x+=0.9f;
	}
	if(keys[SDL_SCANCODE_F7]) {
		camera.x-=0.9f;
	}
	if(keys[SDL_SCANCODE_F3]) {
		camera.y+=0.9f;
	}
	if(keys[SDL_SCANCODE_F4]) {
		camera.y-=0.9f;
	}

	if(keys[SDL_SCANCODE_F9])
		NoBlend=!NoBlend;

	if(keys[SDL_SCANCODE_F2]) {
		debugBlend=true;
	} else {
		debugBlend=false;
	}
        if(keys[SDL_SCANCODE_T]) {
            hud_show_input_field(1);
        }

        if(keys[SDL_SCANCODE_ESCAPE])
        {
            printf("Escape tryckt, avslutar...\n");
            return false;
        }

	return true;
}

static int CalcGameVars()
{
	// Tar hand om hastigheten...
	if(bil.o.speed>bil.maxspeed)
		bil.o.speed=bil.maxspeed;

	if(bil.o.speed<bil.maxbspeed)
		bil.o.speed=bil.maxbspeed;

	if(bil.o.speed<0.0f && bil.o.speed>-bil.speeddown)
		bil.o.speed=0.0f;
	if(bil.o.speed>0.0f && bil.o.speed<bil.speeddown)
		bil.o.speed=0.0f;

	if(bil.o.speed>0)
		bil.o.speed=bil.o.speed-bil.speeddown;
	if(bil.o.speed<0)
		bil.o.speed=bil.o.speed+bil.speeddown;


	if(bil.o.angle<0)
		bil.o.angle=bil.o.angle+360;
	if(bil.o.angle>359)
		bil.o.angle=bil.o.angle-360;


	// Svänger bilen så att den åker åt rätt håll

	float tmpx=0.0f,tmpy=0.0f;

	object_forward(&bil.o);

	if(!Network) {
		int i;
		for(i=0;i<nrgubbar;i++) {
			gubbe_move(&gubbar[i]);
		}

	}	//!Network

	int loop1 = 0, loop2 = 0;

	for(loop1=0 ;loop1<world.nrcubex;loop1++) {
		for(loop2=0;loop2<world.nrcubey;loop2++) {
			if (object_colliding(&(map_cube(world, loop1, loop2, 0).o), &bil.o)) {
				player.krockar++;

				int damage = abs((int)(bil.o.speed*5));
				if (damage) {
					sound_play(krasch);

					bil.helhet -= damage;
					if (bil.helhet <= 0) {
						bil.helhet = 0;
						bil.t1=14;
					}
				}
				object_backward(&bil.o);
				bil.o.speed = 0;
				loop1=world.nrcubex; loop2=world.nrcubey; // XXX: ugly exit
			}
		}
	}

	if(!Network) {

		for(loop1=0; loop1<nrgubbar; loop1++) {
			if (!gubbar[loop1].alive)
				continue;

			if (object_colliding(&gubbar[loop1].o, &bil.o)) {
				if(bil.o.speed>0.4f || bil.o.speed<-0.4f) {
					gubbar[loop1].alive=false;

					struct timeval tv;
					gettimeofday(&tv, NULL);
					if(tv.tv_usec % 2)
						sound_play(aj0);
					else
						sound_play(aj1);
					player.runovers++;
				} else {
					object_backward(&(gubbar[loop1].o));
					gubbar[loop1].o.angle+=180;
					sound_play(move);
				}
			}

		}

                int loop2, loop3;
		for (loop3=0 ;loop3<nrgubbar; loop3++) {
			for (loop1=0 ;loop1<world.nrcubex;loop1++) {
				for (loop2=0;loop2<world.nrcubey;loop2++) {
					if (!gubbar[loop3].alive)
						continue;

					if (object_colliding(&(map_cube(world, loop1, loop2, 0).o), &gubbar[loop3].o)) {
						object_backward(&gubbar[loop3].o);
						gubbar[loop3].o.angle+=60;
					}
				}
			}
		}
	}

	bil.o.x+=tmpx;
	bil.o.y+=tmpy;

        camera_move_for_car(&bil);

	// Debugging grejjer!

	if(debugBlend)
		blendcolor+=0.1f;
	else
		blendcolor-=0.1f;

	if(blendcolor>0.4f)
		blendcolor=0.4f;
	if(blendcolor<0.0f)
		blendcolor=0.0f;


	return true;
}

static int DrawGLScene()
{
	int i;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	/* Set camera position (by translating the world in the opposite direction */
	glLoadIdentity();
	glTranslatef(-camera.x,-camera.y,camera.z+camera.SpeedVar);

	if(blendcolor>0.0f)
		glEnable(GL_BLEND);
	if(blendcolor==0.0f)
		glDisable(GL_BLEND);

	glColor4f(1.0f,1.0f,1.0f,blendcolor);

	int loop1, loop2, loop3;

	// Ritar upp banan -----------------------------
	float lp1bstmp,lp2bstmp,ztmp;
	for(loop1=0; loop1<world.nrcubex; loop1++) {
		for(loop2=0; loop2<world.nrcubey; loop2++) {
			for(loop3=0; loop3<world.nrcubez; loop3++) {


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


        car_render(&bil);
	if (Network) {
		for (i = 0; i < NETWORK_MAX_OPPONENTS; i++) {
			if (opponents[i].in_use)
				car_render(&(opponent_cars[i]));
		}
	}

	if(!Network) {
		for(loop1=0;loop1<nrgubbar;loop1++) {
		    gubbe_render(&gubbar[loop1]);
		}

	}

        hud_set_damage(bil.helhet);

	if(!Network) {
            hud_set_score(player.runovers);
	} else {
            /* TODO */
	}

	return true;
}

static void peer_send_line(const char *nick, const char *input) {
	hud_printf("%s> %s", nick, input);
}

static void input_send_line(const char *input) {
	if (Network)
		network_amsg_send((char*)input);
	hud_printf("Me> %s", input);
}

static int handle_input_field(SDL_Keysym key, int type) {
    if(!hud_input_field_active())
        return 0;

    static const int input_max = 80;
    static char *input_field = NULL;
    static int input_length = 0;

    if(type == SDL_KEYUP) {
        return 0;
    }

    if(input_field == NULL) {
        input_field = (char*)malloc(input_max+1);
        input_field[0] = '\0';
        input_length = 0;
    }

    if(key.scancode == SDL_SCANCODE_ESCAPE) {
        free(input_field);
        input_field = NULL;
        hud_show_input_field(0);
    } else if(key.scancode == SDL_SCANCODE_BACKSPACE && input_length > 0) {
        input_field[--input_length] = '\0';
        hud_update_input_field(input_field);
    } else if(key.scancode == SDL_SCANCODE_RETURN) {
        if(input_length > 0)
            input_send_line(input_field);

        free(input_field);
        input_field = NULL;
        hud_update_input_field("");
        hud_show_input_field(0);
    } else if(input_length < input_max) {
        /* TODO: We should just go UTF-8, I guess. */

        char c = 0;
        if((key.scancode >= 'a' && key.scancode <= 'z')) {
            c = key.scancode;
            if(key.mod & KMOD_SHIFT)
                c -= 'a' - 'A';
        } else if((key.scancode >= 0x20 && key.scancode <= 0x7e) ||
                  (key.scancode >= 0xc0 && key.scancode <= 0xff)) {
                  c = key.scancode;
        }

        if(c != 0) {
            input_field[input_length++] = c;
            input_field[input_length] = '\0';
            hud_update_input_field(input_field);
        }
    }

    return 1; /* We handled the key, stop processing it. */
}


static int CheckaEvents()
{
	SDL_Event event;

	while( SDL_PollEvent( &event ) ){
		switch( event.type ){
		case SDL_KEYDOWN:
			if(!handle_input_field(event.key.keysym, SDL_KEYDOWN))
			    keys[event.key.keysym.scancode]=true;
			break;

		case SDL_KEYUP:
			handle_input_field(event.key.keysym, SDL_KEYUP);
			keys[event.key.keysym.scancode]=false;
			break;

		case SDL_WINDOWEVENT:
			switch (event.window.event) {
			case SDL_WINDOWEVENT_RESIZED:
				gl_resize(event.window.data1, event.window.data2);
				break;
			}
			break;

		case SDL_QUIT:
			return 1;
			break;

		default:
			break;
		}

	}

	return 0;

}

static void print_help()
{
	fprintf(stderr, "Usage: gta2k [OPTIONS]\n\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -s, --server=SERVER   Connect to server\n");
	fprintf(stderr, "  -p, --port=PORT       Use port number PORT "
			"(default: %u)\n", server_port);
	fprintf(stderr, "  -n, --nick=NICK       Use NICK as nick name\n");
	fprintf(stderr, "  -4, --ipv4-only       Force usage of IPv4 when "
			"connecting to a server\n");
	fprintf(stderr, "  -6, --ipv6-only       Force usage of IPv6 when "
			"connecting to a server\n");
	fprintf(stderr, "  -h, --help            Show this help\n");
	fprintf(stderr, "  -M, --no-music        Disable music\n");
	fprintf(stderr, "  -S, --no-sound        Disable all sounds\n");
}

static int parse_options(int argc, char *argv[])
{
	char opt;
	int option_index = 0;
	int ipv4_only = 0;
	int ipv6_only = 0;
	static struct option long_options[] = {
		{"server",    required_argument, 0,  0 },
		{"port",      required_argument, 0,  0 },
		{"nick",      required_argument, 0,  0 },
		{"ipv4-only", no_argument,       0,  0 },
		{"ipv6-only", no_argument,       0,  0 },
		{"help",      no_argument,       0,  0 },
		{"no-sound",  no_argument,       0,  0 },
		{"no-music",  no_argument,       0,  0 },
		{0,           0,                 0,  0 }
	};
	const char *short_options = "s:p:n:46hSM";

	while (1) {
		opt = getopt_long(argc, argv, short_options,
				long_options, &option_index);
		if (opt == -1)
			break;

		/* Doneri, donera. Skyffla vidare longopts till
		 * shortopts-motsvarigheten. Går väl att stoppa in lite logik
		 * här för eventuella framtida longopts-only-saker. */
		if (opt == 0) {
			switch (option_index) {
			case 0:
				opt = 's';
				break;
			case 1:
				opt = 'p';
				break;
			case 2:
				opt = 'n';
				break;
			case 3:
				opt = '4';
				break;
			case 4:
				opt = '6';
				break;
			case 5:
				opt = 'h';
				break;
			case 6:
				opt = 'S';
				break;
			case 7:
				opt = 'M';
				break;
			}
		}
		switch (opt) {
		case 's':
			server_addr = optarg;
			break;
		case 'p':
			if (sscanf(optarg, "%u", &server_port) != 1) {
				fprintf(stderr, "Error: Illegal port number: '%s'\n\n",
						optarg);
				print_help();
				exit(42);
			}
			break;
		case 'n':
			nick = optarg;
			break;
		case '4':
			ipv4_only = 1;
			proto_only = 4;
			break;
		case '6':
			ipv6_only = 1;
			proto_only = 6;
			break;
		case 'h':
			print_help();
			exit(42);
			break;
		case 'S':
			sound = false;
			break;
		case 'M':
			sound_music = false;
			break;
		default:
			printf("?? getopt returned character code 0x%x ??\n", opt);
			break;
		}
	}

	/* Kolla så allt är sunt. Eller osunt. Eller nåt. Einar. */
	if (server_addr != NULL && nick == NULL) {
		fprintf(stderr, "Error: Network play requires yo to supply a "
				"nick name as well\n\n");
		print_help();
		exit(42);
	}

	if (ipv4_only && ipv6_only) {
		fprintf(stderr, "Error: You do realize that you cannot run "
				"'only IPv4' AND 'only IPv6', right?\n\n");
		print_help();
		exit(42);
	}

	return optind;
}

static void opponents_init() {
	int i;

	for (i = 0; i < NETWORK_MAX_OPPONENTS; i++) {
		opponents[i].o = &(opponent_cars[i].o);
		opponent_cars[i].t1=1;
		opponent_cars[i].t2=1;
		opponent_cars[i].t3=1;
		opponent_cars[i].t4=1;
		car_set_model(&opponent_cars[i]);
	}
}

int main(int argc, char *argv[])
{
	srand(time(NULL));

	char mbuf[1024];

	parse_options(argc, argv);

	// C++ SUGER SÅ MYCKET!!!1
	world.nrcubex = 20;
	world.nrcubey = 20;
	world.nrcubez = 2;
	// De texturer som på nåt sätt ska laddas är:
	const char *texture_filenames[] = {
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
	world.texture_filenames = texture_filenames; // Vi lämnar aldrig detta
						     // scope, så det funkar.
						     // Jag lovar!
	world.ntextures = sizeof(texture_filenames) / sizeof(char *);

	int tmpk;
	for(tmpk=0;tmpk<350;tmpk++)
		keys[tmpk]=false;

	SDL_Window *screen;

	bool done=false;

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)<0) {
		printf("Kunde inte initialisera SDL!\n");
		exit(1);
	}

	atexit(SDL_Quit);

	network_init();

	Network=false;

	if (server_addr != NULL) {
		if (network_connect(server_addr, server_port, nick, proto_only)) {
			fprintf(stderr, "Connect failed, exiting :(\n");
			return 28;
		}
		Network = true;
	}

	screen = SDL_CreateWindow("gta2k4lin",
				  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				  initial_width, initial_height,
				  sdl_video_flags);
	if (screen == NULL)
	{
		printf("SDL_CreateWindow failed\n");
		exit(1);
	}

	SDL_GLContext glcontext = SDL_GL_CreateContext(screen);

	InitGL(initial_width, initial_height);
	LoadCars();
	LoadLevel();

	// TODO: Implementera frameskip...
	Uint32 TimerTicks;

        hud_init();
	if (sound)
		sound_init(sound_music);
	opponents_init();

	while(!done)
	{

		TimerTicks=SDL_GetTicks();
		if(CheckaEvents()==1) {
			exit(0);
		}

                if(!hud_input_field_active()) {
                    if(!RespondToKeys())
                        done = 1;
                }

		CalcGameVars();

		DrawGLScene();
                hud_render();
		SDL_GL_SwapWindow(screen);

		if (Network) {
			network_put_position(&(bil.o));
			network_get_positions(opponents);

			unsigned long id;
			while (network_amsg_recv(mbuf, &id, 1024) > 0) {
				peer_send_line(network_lookup_id(id), mbuf);
			}
		}

		while(TimerTicks+30>SDL_GetTicks()) { usleep(1); }
	}

        SDL_Quit();

	return 0;
}
