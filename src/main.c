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

// gta 2000... :) (Inte officielt namn)
// Detta ska föreställa linux porten...



//////////////////////////////////////////////////////////////////////////
//		KATASTROF::		Koden blir mer och mer ostrukturerad			//
//						Oklart hur länge det går att fatta nåt av den :)//
//////////////////////////////////////////////////////////////////////////



/*		This program is made by skatteola. It uses
 *		alot of code from the site nehe.gamedev.com
 *		That is because I am still learning, and I
 *		think that this code is really good to start with.
 *		The original "disclamer" follows:
 *	--------------------------------------------------------
 *
 *		This Code Was Created By Jeff Molofee 2000
 *		A HUGE Thanks To Fredric Echols For Cleaning Up
 *		And Optimizing This Code, Making It More Flexible!
 *		If You've Found This Code Useful, Please Let Me Know.
 *		Visit My Site At nehe.gamedev.net
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

#include "SDL.h"
#include "SDL_image.h"

#include "Bullet-C-Api.h"
#include "btwrap.h"

#include "main.h"
#include "sound.h"
#include "network.h"
#include "hud.h"
#include "object.h"
#include "models.h"
#include "stl.h"
#include "car.h"
#include "gubbe.h"

int initial_width = 640;
int initial_height = 480;
const int sdl_video_flags = SDL_OPENGL | SDL_RESIZABLE;
const int sdl_bpp = 32; // Vi gillar 32 här dåva

#define bool  int
#define false 0
#define true  1

#define nrgubbar 100

/* Bullet */
plPhysicsSdkHandle physics_sdk = 0;
plDynamicsWorldHandle dynamics_world = 0;

bool keys[350];			// Array Used For The Keyboard Routine

bool NoBlend=true;

bool sound = true;
int sound_music = 1;

// Iallafall så är stommen för nätverket laggd...
// Hmmm, det tar sig...

// Enable:as bara ifall man ska försöka få igång nätverket...
bool Network=false;			// Nätverk eller singelplayer...

char *server_addr = NULL;
unsigned server_port = 9378;
char *nick = NULL;
int proto_only = 0;



// Skaffa FPS räknare... hur ska man gööra?


bool debugBlend=false;
float blendcolor;






// Fina spel grejjer!
struct cube {
	// Vilken textur som ska mappas till kuben...
	int texturenr;
	int in_use;
	// Ett namn på stället man är.
	const char* beskrivning;

	struct object o;
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

struct spelare {
	// Poäng
	int points;
	// Krockar
	int krockar;
	// Överkörda människor
	int runovers;
};

struct gubbe *gubbar; //[nrgubbar];
struct spelare player;



// Storleken på banan skulle behövas laddas in från en fil, men för tillfället vet jag inte riktigt hur det skulle gå till...

#define TEXTURE_PATH "data/"
#define map_cube(world, x, y, z) world.map[((x) * (world).nrcubey + (y)) * (world).nrcubez + (z)]
struct world world;

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

struct car bil;
struct car opponent_cars[NETWORK_MAX_OPPONENTS];
struct opponent opponents[NETWORK_MAX_OPPONENTS];

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

	// Används inte, borttaget för att spara plats...
	// Humm... jag kom på att det visst används. :)
	// Bara att kopiera tillbaks...

	// LADDA IN!!!!
	
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

        /* FIXME: Fult */
        for(loop1=0;loop1<world.nrcubex;loop1++) {
            for(loop2=0;loop2<world.nrcubey;loop2++) {
                for(loop3=0;loop3<world.nrcubez;loop3++) {
                    /* Bullet */
                    plCollisionShapeHandle cube_shape;
                    plRigidBodyHandle cube_rbody;
                    void *user_data = NULL;

                    cube_shape = plNewBoxShape(BSIZE, BSIZE, BSIZE);
                    cube_rbody = plCreateRigidBody(user_data, 0.0f, cube_shape);

                    float cube_pos[3];
                    cube_pos[0] = map_cube(world, loop1, loop2, loop3).o.x;
                    cube_pos[1] = map_cube(world, loop1, loop2, loop3).o.y;
                    cube_pos[2] = map_cube(world, loop1, loop2, loop3).o.z;

                    /* plVector3 == float[3] */
                    plSetPosition(cube_rbody, cube_pos);

                    plAddRigidBody(dynamics_world, cube_rbody);
                }
            }
        }

	return true;
}

static int LoadCars()   // och gubbar.
{
	camera_init(&camera);

	init_car(&bil);

	// Kicka igång alla gubbar
	gubbar = malloc(sizeof(struct gubbe)*nrgubbar);
        memset(gubbar, 0, sizeof(struct gubbe)*nrgubbar);

        printf("sizeof(struct gubbe): %ld\n", sizeof(struct gubbe));

	int i;
	for(i = 0; i < nrgubbar; i++) {
            init_gubbe(&gubbar[i]);
            printf("init_gubbe: %p:\n", &gubbar[i]);
	}

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

static int InitGL(int width, int height) //		 All Setup For OpenGL Goes Here
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
        static int broms_in_progress = 0;

	if(keys[SDLK_SPACE]) {
		bil.brakeForce = 200.0f;

		if (!broms_in_progress) {
			sound_play(broms);
			broms_in_progress = 1;
		}
	}
	else {
		broms_in_progress = 0;
		bil.brakeForce = 0.0f;
	}

        if(keys[SDLK_RIGHT] || keys[SDLK_LEFT]) {
            if(keys[SDLK_RIGHT]) {
                bil.steering -= 0.2f;
            } else if(keys[SDLK_LEFT]) {
                bil.steering += 0.2f;
            }

            if(bil.steering > 0.6f)
                bil.steering = 0.6f;
            else if(bil.steering < -0.6f)
                bil.steering = -0.6f;

        } else {

            /* Go straight */
            bil.steering *= 0.05f;
        }


	if(bil.helhet==0) {
		if(keys[SDLK_RETURN]) {
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

        if(bil.helhet) {
            if(keys[SDLK_UP]) {
                bil.engineForce += 40.0f;
            } else if(keys[SDLK_DOWN]) {
                bil.engineForce -= 40.0f;
            } else {
                bil.engineForce *= 0.1f;
            }

            if(bil.engineForce > 1000.0f)
                bil.engineForce = 1000.0f;
        }

        /* FIXME */
        plRaycastVehicle_ApplyEngineForce(bil.bt_vehicle, bil.engineForce, 2);
        plRaycastVehicle_ApplyEngineForce(bil.bt_vehicle, bil.engineForce, 3);
        plRaycastVehicle_SetBrake(bil.bt_vehicle, bil.brakeForce, 2);
        plRaycastVehicle_SetBrake(bil.bt_vehicle, bil.brakeForce, 3);

        plRaycastVehicle_SetSteeringValue(bil.bt_vehicle, bil.steering, 0);
        plRaycastVehicle_SetSteeringValue(bil.bt_vehicle, bil.steering, 1);


        if(keys[SDLK_u]) {
            plVector3 force, rel_pos;
            force[0] = 0.0f; force[1] = 0.0f; force[2] = 50.0f;
            rel_pos[0] = 0.0f; rel_pos[1] = 0.0f; rel_pos[2] = 0.0f;

            plRigidBody_ApplyForce(bil.bt_rbody, force, rel_pos);
        }
        if(keys[SDLK_n]) {
            plVector3 force, rel_pos;
            force[0] = 0.0f; force[1] = 0.0f; force[2] = -50.0f;
            rel_pos[0] = 0.0f; rel_pos[1] = 0.0f; rel_pos[2] = 0.0f;

            plRigidBody_ApplyForce(bil.bt_rbody, force, rel_pos);
        }

	if(keys[SDLK_TAB]) {
		sound_cont_play(tut);
	}
	else {
		sound_cont_stop(tut, 0);
	}

	// Detta är debug grejjer/saker som inte ska vara kvar i "riktiga" versionen...
	// Styr kameran
	if(keys[SDLK_F5]) {
		camera.z-=0.5f;
	}
	if(keys[SDLK_F6]) {
		camera.z+=0.5f;
	}

	if(keys[SDLK_F8]) {
		camera.x+=0.9f;
	}
	if(keys[SDLK_F7]) {
		camera.x-=0.9f;
	}
	if(keys[SDLK_F3]) {
		camera.y+=0.9f;
	}
	if(keys[SDLK_F4]) {
		camera.y-=0.9f;
	}

	if(keys[SDLK_F9])
		NoBlend=!NoBlend;

	if(keys[SDLK_F2]) {
		debugBlend=true;
	} else {
		debugBlend=false;
	}
        if(keys[SDLK_t]) {
            hud_show_input_field(1);
        }

        if(keys[SDLK_ESCAPE])
        {
            printf("Escape tryckt, avslutar...\n");
            return false;
        }

	return true;
}

static int DrawGLScene()
{
	int i;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	/* Set camera position (by translating the world in the opposite direction */
	glLoadIdentity();
	glTranslatef(-camera.x,-camera.y,camera.z+camera.SpeedVar);

	// Spelplanen
	// Hmm, nu ska vi rita upp spelplanen... sen ska den optimeras så den bara ritar upp det nödvändiga...

	//glDisable(GL_COLOR_MATERIAL);

	// Debugging


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


	// Ritar upp bil(en/arna) --------------------------------

	// GAAAH!!!
	// öj, det fungerar... Men det fungerar nog inte om man ska ha fler bilar... <- Nuså. :p

        car_render(&bil);
	if (Network) {
		for (i = 0; i < NETWORK_MAX_OPPONENTS; i++) {
			if (opponents[i].in_use)
				car_render(&(opponent_cars[i]));
		}
	}

	// Rita upp gubbbananerna...
	// Hoho! De SNURRAR!!! :)))))

	if(!Network) {
		for(loop1=0;loop1<nrgubbar;loop1++) {
		    gubbe_render(&gubbar[loop1]);
		}

	} // !Network

        hud_set_damage(bil.helhet);

	if(!Network) {
            hud_set_score(player.runovers);
	} else {
            /* TODO */
	}

	return true;

	// TYp.

}

static void peer_send_line(const char *nick, const char *input) {
	hud_printf("%s> %s", nick, input);
}

static void input_send_line(const char *input) {
	if (Network)
		network_amsg_send((char*)input);
	hud_printf("Me> %s", input);
}

static int handle_input_field(SDL_keysym key, int type) {
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

    if(key.sym == SDLK_ESCAPE) {
        free(input_field);
        input_field = NULL;
        hud_show_input_field(0);
    } else if(key.sym == SDLK_BACKSPACE && input_length > 0) {
        input_field[--input_length] = '\0';
        hud_update_input_field(input_field);
    } else if(key.sym == SDLK_RETURN) {
        if(input_length > 0)
            input_send_line(input_field);

        free(input_field);
        input_field = NULL;
        hud_update_input_field("");
        hud_show_input_field(0);
    } else if(input_length < input_max) {
        /* TODO: We should just go UTF-8, I guess. */

        char c = 0;
        if((key.sym >= 'a' && key.sym <= 'z')) {
            c = key.sym;
            if(key.mod & KMOD_SHIFT)
                c -= 'a' - 'A';
        } else if((key.sym >= 0x20 && key.sym <= 0x7e) ||
                  (key.sym >= 0xc0 && key.sym <= 0xff)) {
                  c = key.sym;
        }

        if(c != 0) {
            input_field[input_length++] = c;
            input_field[input_length] = '\0';
            hud_update_input_field(input_field);
        }
    }

    return 1; /* We handled the key, stop processing it. */
}

static void event_handle_resize(SDL_ResizeEvent *resize) {
    SDL_SetVideoMode(resize->w, resize->h, sdl_bpp, sdl_video_flags);
    gl_resize(resize->w, resize->h);
}

static int CheckaEvents()
{
	SDL_Event event;

	while( SDL_PollEvent( &event ) ){
		switch( event.type ){
		case SDL_KEYDOWN:
			if(!handle_input_field(event.key.keysym, SDL_KEYDOWN))
			    keys[event.key.keysym.sym]=true;
			break;

		case SDL_KEYUP:
			handle_input_field(event.key.keysym, SDL_KEYUP);
			keys[event.key.keysym.sym]=false;
			break;

		case SDL_VIDEORESIZE:
			event_handle_resize(&event.resize);
			break;

			/* SDL_QUIT event (window close) */
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
	// Voila, en slumpgenerator... då var det bara collisiondetection grejjen kvar...
	// den förbannade doningen FUNGERAR INTE!
	srand(time(NULL));

	char mbuf[1024];

	parse_options(argc, argv);

        /* Bullet */
        physics_sdk = plNewBulletSdk();
        dynamics_world = plCreateDynamicsWorld(physics_sdk);
        plDynamicsWorld_SetGravity(dynamics_world, 0, 0, -30.0f);

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

	// Här är den första delen av porten....

	// Nollställ knapp-arrayen...
	int tmpk;
	for(tmpk=0;tmpk<350;tmpk++)
		keys[tmpk]=false;

	SDL_Surface *screen;

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

	screen = SDL_SetVideoMode(initial_width, initial_height, sdl_bpp, sdl_video_flags);
	if (screen == NULL)
	{
		printf("SetVideoMode failed (EInar)\n");
		exit(1);
	}

	InitGL(initial_width, initial_height);
	LoadCars();
	LoadLevel();

	// HUVUDLOOPEN!!! Detta är själva spelet!
	// TODO: Implementera frameskip...
	Uint32 TimerTicks;

        hud_init();
	if (sound)
		sound_init(sound_music);
	opponents_init();

	while(!done)
	{
		/* Bullet */
		plStepSimulation(dynamics_world, 1.0f/60.0f);

		TimerTicks=SDL_GetTicks();
		if(CheckaEvents()==1) {
			exit(0);
		}

                if(!hud_input_field_active()) {
                    if(!RespondToKeys())
                        done = 1;
                }

                camera_move_for_car(&bil);

		DrawGLScene();
                hud_render();
		SDL_GL_SwapBuffers();

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

	return 0; 	// SLYYT.
}
