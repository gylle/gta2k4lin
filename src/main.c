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

#include "sound.h"
#include "network.h"
#include "hud.h"

int width = 640;
int height = 480;
int bpp = 32; // Vi gillar 32 här dåva
float bsize=5.0f;

#define bool  int
#define false 0
#define true  1

#define nrgubbar 100

bool keys[350];			// Array Used For The Keyboard Routine

bool NoBlend=true;

// Iallafall så är stommen för nätverket laggd...
// Hmmm, det tar sig...

// Enable:as bara ifall man ska försöka få igång nätverket...
bool Network=false;			// Nätverk eller singelplayer...
bool Server=false;			// Om Server, annars klient.

char *server_addr = NULL;
unsigned server_port = 9378;
char *nick = NULL;
int proto_only = 0;


int broms_in_progress = 0;

// Skaffa FPS räknare... hur ska man gööra?


bool debugBlend=false;
float blendcolor;






// Fina spel grejjer!
struct cube {
	// Vilket plan den är på. 0.0f är det man går/åker på,
	float z;
	// Vilken textur som ska mappas till kuben...
	int texturenr;
	// Ett namn på stället man är.
	const char* beskrivning;
};

struct car {
	// Storleken:
	float x,y,z;
	// Positionen:
	float posx,posy,posz;
	// Texturer, nr1: tak. nr2: sidor. nr3: fram. nr4: bak:
	int t1,t2,t3,t4;
	// Fartsaker
	float maxspeed,maxbspeed, curspeed, accspeed, bromsspeed;
	int turnspeed;
	// Hur fort bilen stannar om man inte gasar
	float speeddown;
	// Hur många grader bilen är vriden...
	int angle;
	// Hur hel bilen är(%). 100 är helhel, 0 är heltrasig.
	int helhet;
	// Poäng. I Multiplayer spel hur många "frags" man har...
	int Points;

};


struct gubbe {
	// Storleken:
	float x,y,z;
	// Positionen:
	float posx,posy,posz;
	// Texturer. ltexture2=huvudet. ltexture=resten. dtexturer=texture då gubben dött...
	int ltexture, ltexture2,  dtexture;
	// Fartsaker...
	float maxspeed,maxbspeed,curspeed,accspeed;
	// Hur många grader gubben är vriden...
	int angle;
	// Lever?
	bool alive;
	// Tid tills han lever igen...
	int atimer;
	// 2 temporära grejjer... Används framförallt av förflyttnings funktionen...
	float tmpx,tmpy;

};
const int gubbtid=300;		// Hur lång tid en gubbe är död... Räknas i frames :)
GLuint	GubbeDispList = 0;

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

struct world {
	int nrcubex;
	int nrcubey;
	struct cube *map;
	const char **texture_filenames;
	GLuint *texIDs;
	int ntextures;
};

#define TEXTURE_PATH "data/"
#define map_cube(world, x, y) world.map[(x) * (world).nrcubey + (y)]
struct world world;


struct car bil;
struct car mbil;		 // andra bilen i multiplayer


static void init_gubbe(struct gubbe *g) {
    g->alive=true;

    g->curspeed=0.0f;
    g->maxspeed=0.3f;
    g->accspeed=0.15f;
    g->maxbspeed=0.2f;

    //g->angle=0;

    g->x=1.0f;
    g->y=1.0f;
    g->z=1.9f;

    g->posx=(float)((rand() % world.nrcubex*bsize*2)*100)/100.0f;
    g->posy=(float)((rand() % world.nrcubey*bsize*2)*100)/100.0f;
    g->angle=rand() % 360;

    g->posz=bsize;

    g->ltexture=11;
    g->ltexture2=13;
    g->dtexture=12;

    g->atimer = 0;
}

static void gubbe_move(struct gubbe *g) {
    float tmpangle;

    // den här funktionen som bestämmer vad gubbarna ska göra måste skrivas om,
    // Gubbarna är totalt urblåsta.

    if(g->alive) {
        int tmprand=rand() % 100; // Ejjj, det wooorkar...

        if(tmprand==0 && tmprand<3)  // gubben ska bara vrida sig fååå gånger..
            g->angle+=10;

        if(tmprand>=3 && tmprand<5)  // Ge även möjligheten att vända åt andra hållet...
            g->angle-=10;

        if(tmprand>=5 && tmprand<=100)
            g->curspeed=g->curspeed+g->accspeed;



    } else {
        g->atimer++;

        if(g->atimer>=gubbtid) {   // Jag antar att man borde randomiza ut platsen igen...
            g->atimer=0;
            g->alive=true;

            g->posx=(float)((rand() % world.nrcubex*bsize*2)*100)/100;
            g->posy=(float)((rand() % world.nrcubey*bsize*2)*100)/100;
            g->angle=rand() % 360;
        }
    }

    if(g->curspeed<0) {
        if(g->curspeed<g->maxbspeed)
            g->curspeed=g->maxbspeed;
    } else {
        if(g->curspeed>g->maxspeed)
            g->curspeed=g->maxspeed;
    }

    if(g->angle<0)
        g->angle=g->angle+360;
    if(g->angle>359)
        g->angle=g->angle-360;


    tmpangle=(float)g->angle;

    if(g->angle>=0 && g->angle<=90) {
        g->tmpx=-((tmpangle/90.0f)*g->curspeed);
        g->tmpy=g->curspeed+g->tmpx;
    }

    if(g->angle>=270 && g->angle<=360) {
        tmpangle-=270.0f;
        g->tmpy=(tmpangle/90.0f)*g->curspeed;
        g->tmpx=g->curspeed-g->tmpy;
    }

    if(g->angle>90 && g->angle<=180) {
        tmpangle-=90.0f;
        g->tmpy=-((tmpangle/90.0f)*g->curspeed);
        g->tmpx=-(g->curspeed+g->tmpy);
    }

    if(g->angle>180 && g->angle<270) {
        tmpangle-=180.0f;
        g->tmpx=(tmpangle/90.0f)*g->curspeed;
        g->tmpy=-(g->curspeed-g->tmpx);
    }

    if(!g->alive) {
        g->tmpx=0;
        g->tmpy=0;
    }
}

static void gubbe_render(struct gubbe *g) {
    glPushMatrix();

    // HAHA!!! Det gick till slut! :)
    glTranslatef(g->posx,g->posy,0);//Distance+SpeedVar);
    glRotatef((float)g->angle,0.0f,0.0f,1.0f);

    if(g->alive) {
        if(!NoBlend)
            glEnable(GL_BLEND);

        glColor4f(1.0f,1.0f,1.0f,255);
        glCallList(GubbeDispList);

        if(blendcolor==0.0f)
            glDisable(GL_BLEND);
    } else {
        // Är man överkörd står man nog inte upp längre... :) Det här blir bättre...
        glBindTexture(GL_TEXTURE_2D,world.texIDs[g->dtexture]);

        glBegin(GL_QUADS);

        // Ovanifrån...

        glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(g->x/2),0.0f+(g->y/2),bsize);// X-----------
        glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(g->x/2),0.0f+(g->y/2),bsize);// -----------X
        glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(g->x/2),0.0f-(g->y/2),bsize);// -----------X
        glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(g->x/2),0.0f-(g->y/2),bsize);// X-----------
        glEnd();

    }
    glPopMatrix();
}

static void init_gubbe_displaylist() {
    /* (Bygger på att den första gubben i gubbar är initialiserad */

    // Vi bygger en Display List!!! EJJJJ!!!(som i öj) :)

    GubbeDispList=glGenLists(1);

    glNewList(GubbeDispList,GL_COMPILE);

    glBindTexture(GL_TEXTURE_2D,world.texIDs[gubbar[0].ltexture2]);

    glBegin(GL_QUADS);

    // Ovanifrån...

    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(gubbar[0].x/2),0.0f+(gubbar[0].y/2),gubbar[0].z+gubbar[0].posz);// X-----------
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(gubbar[0].x/2),0.0f+(gubbar[0].y/2),gubbar[0].z+gubbar[0].posz);// -----------X
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(gubbar[0].x/2),0.0f-(gubbar[0].y/2),gubbar[0].z+gubbar[0].posz);// -----------X
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(gubbar[0].x/2),0.0f-(gubbar[0].y/2),gubbar[0].z+gubbar[0].posz);// X-----------
    glEnd();

    // Börja en ny glBegin för att vi ska kunna texturemappa huvudet och resten seperat...
    glBindTexture(GL_TEXTURE_2D,world.texIDs[gubbar[0].ltexture]);

    glBegin(GL_QUADS);

    //Höger och vänster


    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(gubbar[0].x/2),0.0f+(gubbar[0].y/2),gubbar[0].z+gubbar[0].posz);// X-----------
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f-(gubbar[0].x/2),0.0f-(gubbar[0].y/2),gubbar[0].z+gubbar[0].posz);// X-----------
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f-(gubbar[0].x/2),0.0f-(gubbar[0].y/2),gubbar[0].posz);// X-----------
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(gubbar[0].x/2),0.0f+(gubbar[0].y/2),gubbar[0].posz);// X-----------

    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f+(gubbar[0].x/2),0.0f+(gubbar[0].y/2),gubbar[0].z+gubbar[0].posz);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(gubbar[0].x/2),0.0f-(gubbar[0].y/2),gubbar[0].z+gubbar[0].posz);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(gubbar[0].x/2),0.0f-(gubbar[0].y/2),gubbar[0].posz);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f+(gubbar[0].x/2),0.0f+(gubbar[0].y/2),gubbar[0].posz);

    // bak och fram

    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(gubbar[0].x/2),0.0f+(gubbar[0].y/2),gubbar[0].z+gubbar[0].posz);// X-----------
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(gubbar[0].x/2),0.0f+(gubbar[0].y/2),gubbar[0].z+gubbar[0].posz);// -----------X
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(gubbar[0].x/2),0.0f+(gubbar[0].y/2),gubbar[0].posz);// -----------X
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(gubbar[0].x/2),0.0f+(gubbar[0].y/2),gubbar[0].posz);// X-----------

    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(gubbar[0].x/2),0.0f-(gubbar[0].y/2),gubbar[0].z+gubbar[0].posz);// X-----------
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(gubbar[0].x/2),0.0f-(gubbar[0].y/2),gubbar[0].z+gubbar[0].posz);// -----------X
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(gubbar[0].x/2),0.0f-(gubbar[0].y/2),gubbar[0].posz);// -----------X
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(gubbar[0].x/2),0.0f-(gubbar[0].y/2),gubbar[0].posz);// X-----------


    glEnd();

    glEndList();
}

void init_car(struct car *bil) {
    // Ladda en standard bil...

    bil->x=3;
    bil->y=5;
    bil->z=2;

    bil->helhet=100;

    bil->posx=10;
    bil->posy=10;
    bil->posz=bsize; // Ska nog inte initialiseras här..

    bil->t1=1;
    bil->t2=1;
    bil->t3=1;
    bil->t4=1;

    bil->maxspeed=2.0f;
    bil->curspeed=0.0f;
    bil->accspeed=0.20f;
    bil->maxbspeed=-1.0f;
    bil->bromsspeed=0.3f;
    bil->speeddown=0.10f;
    // Orginal värdet
    // bil->turnspeed=6;
    // Nytt värde, den svänger trotsallt lite segt...
    bil->turnspeed=8;

    bil->angle=0;

    bil->Points=0;

}

void car_render(struct car *bil) {
    glPushMatrix();

    glTranslatef(bil->posx, bil->posy, bil->posz);
    glRotatef((float)bil->angle,0.0f,0.0f,1.0f);

    glBindTexture(GL_TEXTURE_2D,world.texIDs[bil->t1]);

    glBegin(GL_QUADS);

        // Tak...
        glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(bil->x/2),0.0f+(bil->y/2),bil->z);// X-----------
        glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(bil->x/2),0.0f+(bil->y/2),bil->z);// -----------X
        glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(bil->x/2),0.0f-(bil->y/2),bil->z);// -----------X
        glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(bil->x/2),0.0f-(bil->y/2),bil->z);// X-----------

        // Inget golv, för det kommer inte att synas... tror jag.

        //Höger och vänster
        glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(bil->x/2),0.0f+(bil->y/2),bil->z);// X-----------
        glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f-(bil->x/2),0.0f-(bil->y/2),bil->z);// X-----------
        glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f-(bil->x/2),0.0f-(bil->y/2),0.0f);// X-----------
        glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(bil->x/2),0.0f+(bil->y/2),0.0f);// X-----------

        glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f+(bil->x/2),0.0f+(bil->y/2),bil->z);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(bil->x/2),0.0f-(bil->y/2),bil->z);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(bil->x/2),0.0f-(bil->y/2),0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f+(bil->x/2),0.0f+(bil->y/2),0.0f);

        // bak och fram
        glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(bil->x/2),0.0f+(bil->y/2),bil->z);// X-----------
        glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(bil->x/2),0.0f+(bil->y/2),bil->z);// -----------X
        glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(bil->x/2),0.0f+(bil->y/2),0.0f);// -----------X
        glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(bil->x/2),0.0f+(bil->y/2),0.0f);// X-----------

        glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(bil->x/2),0.0f-(bil->y/2),bil->z);// X-----------
        glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(bil->x/2),0.0f-(bil->y/2),bil->z);// -----------X
        glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(bil->x/2),0.0f-(bil->y/2),0.0f);// -----------X
        glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(bil->x/2),0.0f-(bil->y/2),0.0f);// X-----------
    glEnd();

    glPopMatrix();
}

/* Kamera */
GLfloat transx=0.0f, transy=0.0f;
GLfloat Distance=-30.0f, SpeedVar, tmpSpeedVar;

float CalcMapPlace(int cx,int cy,bool xy)
{

	// bool xy: FALSE= returnera x. TRUE= returnera y

	float Answer;

	if(!xy) {
		Answer=(float)cx*bsize*2;
	} else {
		Answer=(float)cy*bsize*2;

	}
	return Answer;
}


int LoadGLTextures()								// Load Bitmaps And Convert To Textures
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

void ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

int LoadLevel()
{

	// Används inte, borttaget för att spara plats...
	// Humm... jag kom på att det visst används. :)
	// Bara att kopiera tillbaks...

	// LADDA IN!!!!
	
	// Allocate them cubes!
	world.map = (struct cube *)calloc(world.nrcubex * world.nrcubey, sizeof(struct cube));
	if (world.map == NULL)  {
		return false;
	}

	int loop1 = 0, loop2 = 0;

	for(loop1=0;loop1<world.nrcubex;loop1++)
		for(loop2=0;loop2<world.nrcubey;loop2++) {
			map_cube(world, loop1, loop2).z=0.0f;
			map_cube(world, loop1, loop2).texturenr=0;
			map_cube(world, loop1, loop2).beskrivning="Testbeskrivning";
		}

	map_cube(world, 0, 0).z=0.0f;
	map_cube(world, 0, 0).texturenr=1;

	map_cube(world, 0, 1).z=0.0f;
	map_cube(world, 0, 1).texturenr=1;

	// Vägen -------------------------------
	for(loop1=0;loop1<world.nrcubey;loop1++)
		map_cube(world, 1, loop1).texturenr=2;

	for(loop1=0;loop1<world.nrcubey;loop1++)
		map_cube(world, 2, loop1).texturenr=3;

	for(loop1=0;loop1<world.nrcubex;loop1++)
		map_cube(world, loop1, world.nrcubey-2).texturenr=8;

	for(loop1=2;loop1<world.nrcubex;loop1++)
		map_cube(world, loop1, world.nrcubey-3).texturenr=9;

	map_cube(world, 1, world.nrcubey-2).texturenr=5;
	map_cube(world, 2, world.nrcubey-2).texturenr=6;
	map_cube(world, 2, world.nrcubey-3).texturenr=7;


	// "Väggen" runtomkring
	for(loop1=0;loop1<world.nrcubey;loop1++) {
		map_cube(world, 0, loop1).texturenr=4;
		map_cube(world, 0, loop1).z=1.0f;
	}

	for(loop1=0;loop1<world.nrcubey;loop1++) {
		map_cube(world, world.nrcubex-1, loop1).texturenr=4;
		map_cube(world, world.nrcubex-1, loop1).z=1.0f;
	}

	for(loop1=0;loop1<world.nrcubex;loop1++) {
		map_cube(world, loop1, 0).texturenr=4;
		map_cube(world, loop1, 0).z=1.0f;
	}
	for(loop1=0;loop1<world.nrcubex;loop1++) {
		map_cube(world, loop1, world.nrcubey-1).texturenr=4;
		map_cube(world, loop1, world.nrcubey-1).z=1.0f;
	}

	// Vi lägger in lite buskar
	for(loop1=1;loop1<(world.nrcubey/2-1);loop1+=2) {
		map_cube(world, world.nrcubex/2, loop1).texturenr=15;
		map_cube(world, world.nrcubex/2, loop1).z=1.0f;
	}

	// Vägen in till mitten och den fina credits saken där.
	for(loop1=3;loop1<world.nrcubex/2;loop1++)
		map_cube(world, loop1, world.nrcubey/2).texturenr=7;

	map_cube(world, world.nrcubex/2, world.nrcubey/2).texturenr=10;
	map_cube(world, world.nrcubex/2, world.nrcubey/2).z=2.0f;

	return true;
}

int LoadCars()   // och gubbar.
{

	init_car(&bil);
	init_car(&mbil); // "Nätverks"-bilen

	if(Network) {
		if(Server) {
			bil.posx=10;
			bil.posy=10;
			mbil.posx=50;
			mbil.posy=50;
		} else {
			mbil.posx=10;
			mbil.posy=10;
			bil.posx=50;
			bil.posy=50;
		}
        }


	// Kicka igång alla gubbar
	gubbar = malloc(sizeof(struct gubbe)*nrgubbar);

	int i;
	for(i = 0; i < nrgubbar; i++) {
            init_gubbe(&gubbar[i]);
	}

        init_gubbe_displaylist();

	return true;
}

int SetupNet()
{
	// Förnärvarande inget stöd för nätverk i linux versionen--
	return 0;
}

int InitGL()								//		 All Setup For OpenGL Goes Here
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


	float ratio = (float) width / (float) height;

	/* Our shading model--Gouraud (smooth). */
	glShadeModel( GL_SMOOTH );

	/* Culling. */
	//glCullFace( GL_BACK );
	//glFrontFace( GL_CCW );
	//glEnable( GL_CULL_FACE );

	/* Set the clear color. */
	glClearColor( 0, 0, 1, 0 );

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
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearDepth(1.0f);									// Depth Buffer Setup
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations


	return true;										// Initialization Went OK
}

int RespondToKeys()
{

	if(bil.helhet==0) {
		if(keys[SDLK_RETURN]) {
			bil.helhet=100;
			bil.t1=1;
			mbil.Points++;

			sound_cont_stop(brinner, 1);
			sound_play(respawn);

		}
		else {
			sound_cont_play(brinner);
		}
	}


	if(!(bil.helhet==0)) {
		if(keys[SDLK_UP]) {
			bil.curspeed=bil.curspeed+bil.accspeed;
		}

		if(keys[SDLK_DOWN]) {
			bil.curspeed=bil.curspeed-bil.accspeed;
		}
	}

	bool brakepressed=false;

	if(keys[SDLK_SPACE]) {
		brakepressed=true;
		if(bil.curspeed<0.0f && bil.curspeed>-bil.bromsspeed)
			bil.curspeed=0.0f;
		if(bil.curspeed>0.0f && bil.curspeed<bil.bromsspeed)
			bil.curspeed=0.0f;

		if (!broms_in_progress) {
			sound_play(broms);
			broms_in_progress = 1;
		}

		if(bil.curspeed<0.0f)
			bil.curspeed+=bil.bromsspeed;
		if(bil.curspeed>0.0f)
			bil.curspeed-=bil.bromsspeed;

	}
	else {
		broms_in_progress = 0;
	}

	if(keys[SDLK_TAB]) {
		sound_cont_play(tut);
	}
	else {
		sound_cont_stop(tut, 0);
	}

	float sttmp;



	if(keys[SDLK_LEFT]) {
		sttmp=bil.curspeed/bil.maxspeed;			// Omöjliggör styrning vid stillastående, och
		if(brakepressed)
			sttmp+=0.7f;

		if(bil.curspeed!=0.0f)
			bil.angle+=bil.turnspeed*sttmp;				// öka graden av styrmöjlighet ju snabbare det går.
	}

	if(keys[SDLK_RIGHT]) {
		sttmp=bil.curspeed/bil.maxspeed;
		//std::cout << "HOGER" << std::endl;
		if(brakepressed)
			sttmp+=0.7f;

		if(bil.curspeed!=0.0f)
			bil.angle-=bil.turnspeed*sttmp;				// öka graden av styrmöjlighet ju snabbare det går.
	}

	// Detta är debug grejjer/saker som inte ska vara kvar i "riktiga" versionen...
	// Styr kameran
	if(keys[SDLK_F5]) {
		Distance-=0.5f;
	}
	if(keys[SDLK_F6]) {
		Distance+=0.5f;
	}

	if(keys[SDLK_F8]) {
		transx+=0.9f;
	}
	if(keys[SDLK_F7]) {
		transx-=0.9f;
	}
	if(keys[SDLK_F3]) {
		transy+=0.9f;
	}
	if(keys[SDLK_F4]) {
		transy-=0.9f;
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

	// Nu vet vi precis vilka knappar som var pressade...
	// Då skickar vi strängen PressedB, och tar emot mPressedB
	// Hmm, det här var snabbare än mitt förra sätt, men jag tycker att det borde finnas ännu snabbare sätt att
	// Skicka data på... Undrar hur quake fungerar, tex... :)


	// NÄTVERKSSAK BORTTAGEN.

	///////////////////////////////////////////////////////////
	// Beräkna samma saker fast för den andra bilen!! ////////////////////////77
	/////////////////////////////////////////////////////////////////////////////


	// NÄTVERKSSAK BORTTAGEN

	return true;
}

int CalcGameVars()
{

	// Tar hand om hastigheten...
	if(bil.curspeed>bil.maxspeed)
		bil.curspeed=bil.maxspeed;

	if(bil.curspeed<bil.maxbspeed)
		bil.curspeed=bil.maxbspeed;

	if(bil.curspeed<0.0f && bil.curspeed>-bil.speeddown)
		bil.curspeed=0.0f;
	if(bil.curspeed>0.0f && bil.curspeed<bil.speeddown)
		bil.curspeed=0.0f;

	if(bil.curspeed>0)
		bil.curspeed=bil.curspeed-bil.speeddown;
	if(bil.curspeed<0)
		bil.curspeed=bil.curspeed+bil.speeddown;


	if(bil.angle<0)
		bil.angle=bil.angle+360;
	if(bil.angle>359)
		bil.angle=bil.angle-360;


	// Svänger bilen så att den åker åt rätt håll

	float tmpx=0.0f,tmpy=0.0f;
	float tmpangle;

	tmpangle=(float)bil.angle;

	// Det här fungerar, nu är det bara 3/4 kvar att fixa :D
	if(bil.angle>=0 && bil.angle<=90) {
		tmpx=-((tmpangle/90.0f)*bil.curspeed);
		tmpy=bil.curspeed+tmpx;
	}
	// EJ!! Den workar, bara 2/4 kvar! :)))
	if(bil.angle>=270 && bil.angle<=360) {
		tmpangle-=270.0f;
		tmpy=(tmpangle/90.0f)*bil.curspeed;
		tmpx=bil.curspeed-tmpy;
	}

	// WEHOOOOO!!! 1/4 KVAR!!!
	if(bil.angle>90 && bil.angle<=180) {
		tmpangle-=90.0f;
		tmpy=-((tmpangle/90.0f)*bil.curspeed);
		tmpx=-(bil.curspeed+tmpy);
	}

	// FÄRDIGT!!!
	if(bil.angle>180 && bil.angle<270) {
		tmpangle-=180.0f;
		tmpx=(tmpangle/90.0f)*bil.curspeed;
		tmpy=-(bil.curspeed-tmpx);
	}

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///			Styr även nätverksbilarna...						 //
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	float mtmpx=0.0f,mtmpy=0.0f;

	//NÄTVERKSSAK BORTTAGEN



	// Styr de datorkontrollerade gubbarna (och senare även bilar?)
	// ----------------------------------------------------
	// Nej, jag har bestämt mig. Vi stänger av gubbarna när vi kör med networch...

	if(!Network) {
		int i;
		for(i=0;i<nrgubbar;i++) {
			gubbe_move(&gubbar[i]);
		}

	}	//!Network





	// Nu är det kollisiondetection som gäller här...
	// För att inte göra det för svårt, gör vi bara kuber med ett z:a värde på 0.0f åkbara...

	// Kontrollera så att den inte krockar med en kuuub...
	/* -------------------------------------------------------------
	   HÄÄÄÄR ÄR DET VIIIKTIGAST JUST NUUUUUU!!!!!!!!!!!!!!!!!!
	   ------------------------------------------------------------ */

	float tmpbilx, tmpbily;

	/*if((bil.angle>=75 && bil.angle<=105) || (bil.angle>=255 && bil.angle<= 285)) {
	  tmpbilx=bil.y;  // Kompensera lite för att bilen är olika stor och bred....
	  tmpbily=bil.x;	// BANAN!! Detta fungerade, men risken är att bilen fastnar i väggen...
	  } else { */
	tmpbilx=bil.x;
	tmpbily=bil.y;
	//}

	/////////////////////////////////////////////////////////////////////////////////
	/// EINAR!!! Det här kommer ju att dra lika mycket CPU som... jag vet inte vad...
	/////////////////////////////////////////////////////////////////////////////////


	int loop1 = 0, loop2 = 0;


	// Det kan tyckas vara onödigt att kolla alla kuber på banan... fixa så att den kollar bara de närmaste...
	// kontrollera så att inte bilen krockar med en stor KUUB!
	for(loop1=0 ;loop1<world.nrcubex;loop1++)
		for(loop2=0;loop2<world.nrcubey;loop2++) {
			if(map_cube(world, loop1, loop2).z!=0.0f) {		// Om inte kuben är ett underlag...

				if(bil.posx+tmpx+tmpbilx/2>=CalcMapPlace(loop1,loop2,0)-bsize && bil.posx+tmpx-tmpbilx/2<=CalcMapPlace(loop1,loop2,0)+bsize)
					if(bil.posy+tmpy+tmpbily/2>=CalcMapPlace(loop1,loop2,1)-bsize && bil.posy+tmpy-tmpbily/2<=CalcMapPlace(loop1,loop2,1)+bsize) {
						// Vi stannar bilen under den här framen för att inte bilen ska åka in där iallafall...
						tmpx=0.0f; tmpy=0.0f;
						// EJJ, SÅ HÄR SKA DET JU VARA!!!!!
						bil.curspeed=-bil.curspeed;
						player.krockar++;
						//std::cout << "KROCK!";

                                                int damage = abs((int)(bil.curspeed*5));
                                                if(damage) {
                                                    sound_play(krasch);

                                                    bil.helhet -= damage;
                                                    if(bil.helhet <= 0) {
                                                        bil.helhet = 0;
                                                        bil.t1=14;
                                                    }
                                                }
					}
			}
		}


	// Om vi spelar nätverk, så ska vi även dra CPU på att kolla om andra bilen har krockat...
	/////////////////////////////////////////////////////////////////////////////////////////
	// Detta är alltså nätverksbilen som kollas emot krockar!!!!

	//NÄTVERKSSAK BORTTAGEN


	// Kolla så att inte de små bilbananerna krockar med varandra...

	//NÄTVERKSSAK BORTTAGEN




	// Kolla så att inte gubbarna krockar med bilen...
	// Vi lägger detta före de andra gubb grejjerna, för att jag kanske knuffar lite på gubbarna här...

	if(!Network) {

		if((bil.angle>=75 && bil.angle<=105) || (bil.angle>=255 && bil.angle<= 285)) {
			tmpbilx=bil.y;  // Kompensera lite för att bilen är olika lång och bred....
			tmpbily=bil.x;	// Detta fungerade inte så bra till väggarna, men jag tror det gör underverk för gubbarna...
		} else {
			tmpbilx=bil.x;
			tmpbily=bil.y;
		}

		for(loop1=0; loop1<nrgubbar; loop1++) {
			if(gubbar[loop1].alive)
				if(bil.posx+tmpx+tmpbilx/2>=gubbar[loop1].posx+gubbar[loop1].tmpx-(gubbar[loop1].x/2) && bil.posx+tmpx-tmpbilx/2<=gubbar[loop1].posx+gubbar[loop1].tmpx+(gubbar[loop1].x/2))
					if(bil.posy+tmpy+tmpbily/2>=gubbar[loop1].posy+gubbar[loop1].tmpy-(gubbar[loop1].y/2) && bil.posy+tmpy-tmpbily/2<=gubbar[loop1].posy+gubbar[loop1].tmpy+(gubbar[loop1].y/2)) {
						if(bil.curspeed>0.4f || bil.curspeed<-0.4f) {
							gubbar[loop1].alive=false;		 // Här ska man väl även få poäng, antar jag...

							struct timeval tv;
							gettimeofday(&tv, NULL);
							if(tv.tv_usec % 2)
								sound_play(aj0);
							else
								sound_play(aj1);
							player.runovers++;
							/*} else if(bil.curspeed<1.0f && bil.curspeed>-1.0f) { // det ska vara så att man skjuter dem framför om man kör på dem för sakta...
							  gubbar[loop1].tmpx=tmpx;
							  gubbar[loop1].tmpy=tmpy; */
					} else {
						//gubbar[loop1].angle=-gubbar[loop1].angle;
						gubbar[loop1].angle+=180;
						sound_play(move);
					}
					}

		}

		// Se till så att inte gubbarna krockar med sig själva
		// denna funktion kan optimeras tusenfalt!!
		// Men det värsta måste väl vara att den inte fungerar...
		// Orkar inte krångla med den nu, jag vill få igång lite roliga saker...

		//TRASIG GUBBSAK BORTTAGEN

		// Se till så att inte gubbarna krockar med nåt väggaktigt...
		// Fungerar, men vad som händer med gubbarna behöver absolut finjusteras...

		// Oj, oj, oj... precis när jag trodde att jag nått CPU toppen för en liten funktion...
                int loop2, loop3;
		for(loop3=0 ;loop3<nrgubbar; loop3++)
			for(loop1=0 ;loop1<world.nrcubex;loop1++)
				for(loop2=0;loop2<world.nrcubey;loop2++) {
					if(map_cube(world, loop1, loop2).z!=0.0f) {		// Om inte kuben är ett underlag...

						if(gubbar[loop3].posx+gubbar[loop3].tmpx+gubbar[loop3].x/2>=CalcMapPlace(loop1,loop2,0)-bsize && gubbar[loop3].posx+gubbar[loop3].tmpx-gubbar[loop3].x/2<=CalcMapPlace(loop1,loop2,0)+bsize)
							if(gubbar[loop3].posy+gubbar[loop3].tmpy+gubbar[loop3].y/2>=CalcMapPlace(loop1,loop2,1)-bsize && gubbar[loop3].posy+gubbar[loop3].tmpy-gubbar[loop3].y/2<=CalcMapPlace(loop1,loop2,1)+bsize) {

								gubbar[loop3].tmpx=0.0f; gubbar[loop3].tmpy=0.0f;
								// Einar...
								//	gubbar[loop3].angle=-gubbar[loop3].angle;
								gubbar[loop3].curspeed=-gubbar[loop3].curspeed;
								gubbar[loop3].angle+=60;
							}



					}
				}
	} // !Network
	/* -----------------------------------------------------------------*/

	bil.posx+=tmpx;
	bil.posy+=tmpy;

	if(Network) {
		mbil.posx+=mtmpx;
		mbil.posy+=mtmpy;
	}



	// Räkna ut i vilket väderstreck den andra bilen befinner sig...
	if(Network) {

		// NÄTVERKSSAK BORTTAGEN

	}

	if(!Network)
		for(loop1=0;loop1<nrgubbar;loop1++) {
			gubbar[loop1].posx+=gubbar[loop1].tmpx;
			gubbar[loop1].posy+=gubbar[loop1].tmpy;
		}


	// Få kameran att höjas och sänkas beroende på hastigheten...
	tmpSpeedVar=bil.curspeed*5;
	if(tmpSpeedVar>0)
		tmpSpeedVar=-tmpSpeedVar;

	if(tmpSpeedVar>SpeedVar)
		SpeedVar+=0.4f;

	if(tmpSpeedVar<SpeedVar)
		SpeedVar-=0.4f;


	transx=-bil.posx;
	transy=-bil.posy;


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

int DrawGLScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	/* Set camera position */
	glLoadIdentity();
	glTranslatef(transx,transy,Distance+SpeedVar);

	// Spelplanen
	// Hmm, nu ska vi rita upp spelplanen... sen ska den optimeras så den bara ritar upp det nödvändiga...

	//glDisable(GL_COLOR_MATERIAL);

	// Debugging


	if(blendcolor>0.0f)
		glEnable(GL_BLEND);
	if(blendcolor==0.0f)
		glDisable(GL_BLEND);

	glColor4f(1.0f,1.0f,1.0f,blendcolor);

	int loop1 = 0, loop2 = 0;

	// Ritar upp banan -----------------------------
	float lp1bstmp,lp2bstmp,ztmp;
	for(loop1=0; loop1<world.nrcubex; loop1++) {
		for(loop2=0; loop2<world.nrcubey; loop2++) {


			lp1bstmp=(float)loop1*(bsize*2);
			lp2bstmp=(float)loop2*(bsize*2);
			ztmp=map_cube(world, loop1, loop2).z*(bsize*2);

			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,world.texIDs[map_cube(world, loop1, loop2).texturenr]);

			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(lp1bstmp-bsize,lp2bstmp+bsize,ztmp-bsize);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(lp1bstmp+bsize,lp2bstmp+bsize,ztmp-bsize);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(lp1bstmp+bsize,lp2bstmp-bsize,ztmp-bsize);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(lp1bstmp-bsize,lp2bstmp-bsize,ztmp-bsize);

			glTexCoord2f(0.0f, 0.0f); glVertex3f(lp1bstmp-bsize,lp2bstmp+bsize,ztmp-bsize);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(lp1bstmp-bsize,lp2bstmp-bsize,ztmp-bsize);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(lp1bstmp-bsize,lp2bstmp-bsize,ztmp+bsize);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(lp1bstmp-bsize,lp2bstmp+bsize,ztmp+bsize);

			glTexCoord2f(0.0f, 0.0f); glVertex3f(lp1bstmp-bsize,lp2bstmp-bsize,ztmp+bsize);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(lp1bstmp+bsize,lp2bstmp-bsize,ztmp+bsize);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(lp1bstmp+bsize,lp2bstmp+bsize,ztmp+bsize);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(lp1bstmp-bsize,lp2bstmp+bsize,ztmp+bsize);


			// Vänster sida.
			glTexCoord2f(0.0f, 0.0f); glVertex3f(lp1bstmp+bsize,lp2bstmp-bsize,ztmp-bsize);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(lp1bstmp+bsize,lp2bstmp+bsize,ztmp-bsize);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(lp1bstmp+bsize,lp2bstmp+bsize,ztmp+bsize);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(lp1bstmp+bsize,lp2bstmp-bsize,ztmp+bsize);


			glTexCoord2f(0.0f, 0.0f); glVertex3f(lp1bstmp-bsize,lp2bstmp+bsize,ztmp-bsize);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(lp1bstmp+bsize,lp2bstmp+bsize,ztmp-bsize);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(lp1bstmp+bsize,lp2bstmp+bsize,ztmp+bsize);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(lp1bstmp-bsize,lp2bstmp+bsize,ztmp+bsize);

			glTexCoord2f(0.0f, 0.0f); glVertex3f(lp1bstmp-bsize,lp2bstmp-bsize,ztmp-bsize);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(lp1bstmp+bsize,lp2bstmp-bsize,ztmp-bsize);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(lp1bstmp+bsize,lp2bstmp-bsize,ztmp+bsize);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(lp1bstmp-bsize,lp2bstmp-bsize,ztmp+bsize);

			glEnd();

		}
	}


	// Ritar upp bil(en/arna) --------------------------------

	// GAAAH!!!
	// öj, det fungerar... Men det fungerar nog inte om man ska ha fler bilar... <- Nuså. :p

        car_render(&bil);

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

void KillGLWindow()
{
	SDL_Quit();

}

void peer_send_line(const char *nick, const char *input) {
	hud_printf("%s> %s", nick, input);
}

void input_send_line(const char *input) {
	if (Network)
		network_amsg_send((char*)input);
	hud_printf("Me> %s", input);
}

void handle_input_field(SDL_keysym key, int type) {
    if(!hud_input_field_active())
        return;

    static const int input_max = 80;
    static char *input_field = NULL;
    static int input_length = 0;

    if(type == SDL_KEYDOWN) {
        return;
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
    } else if(key.sym == SDLK_RETURN && input_length > 0) {
        input_send_line(input_field);
        free(input_field);
        input_field = NULL;
        hud_update_input_field("");
        hud_show_input_field(0);
    } else if(input_length < input_max) {
        if((key.sym >= 'a' && key.sym <= 'z') ||
           (key.sym >= '0' && key.sym <= '9') ||
            key.sym == ' ') {
            input_field[input_length++] = key.sym;
            input_field[input_length] = '\0';
            hud_update_input_field(input_field);
        }
    }
}

int CheckaEvents()
{
	SDL_Event event;

	while( SDL_PollEvent( &event ) ){
		switch( event.type ){
		case SDL_KEYDOWN:
			handle_input_field(event.key.keysym, SDL_KEYDOWN);
			keys[event.key.keysym.sym]=true;
			break;

		case SDL_KEYUP:
			handle_input_field(event.key.keysym, SDL_KEYUP);
			keys[event.key.keysym.sym]=false;
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

void print_help()
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
}

int parse_options(int argc, char *argv[])
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
		{0,           0,                 0,  0 }
	};
	const char *short_options = "s:p:n:46h";

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

int main(int argc, char *argv[])
{
	// Voila, en slumpgenerator... då var det bara collisiondetection grejjen kvar...
	// den förbannade doningen FUNGERAR INTE!
	srand(time(NULL));

	char mbuf[1024];

	parse_options(argc, argv);

	// C++ SUGER SÅ MYCKET!!!1
	world.nrcubex = 20;
	world.nrcubey = 20;
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

	screen = SDL_SetVideoMode(width, height, bpp, SDL_OPENGL);
	if (screen == NULL)
	{
		printf("SetVideoMode failed (EInar)\n");
		exit(1);
	}

	InitGL();
	LoadCars();
	LoadLevel();

	// HUVUDLOOPEN!!! Detta är själva spelet!
	// TODO: Implementera frameskip...
	Uint32 TimerTicks;

        init_hud();
	sound_init();

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
		SDL_GL_SwapBuffers();

		unsigned long id;
		while (network_amsg_recv(mbuf, &id, 1024) > 0) {
			peer_send_line(network_lookup_id(id), mbuf);
		}

		while(TimerTicks+30>SDL_GetTicks()) { usleep(1); }
	}

        SDL_Quit();

	return 0; 	// SLYYT.
}
