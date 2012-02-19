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
#include <math.h>
#include <GL/gl.h>			// Header File For The OpenGL32 Library
#include <GL/glu.h>			// Header File For The GLu32 Library
#include <fstream>
#include <iostream>
#include <SDL.h>
#include <sys/time.h>
#include <sys/param.h>
#include <getopt.h>

#include "SDL.h"
#include "SDL_image.h"

extern "C" {
#include "sound.h"
#include "network.h"
}

int width = 640;
int height = 480;
int bpp = 32; // Vi gillar 32 här dåva


// Hymmz, dessa verkar inte finnas i linux...
bool TRUE=true;
bool FALSE=false;

bool keys[350];			// Array Used For The Keyboard Routine
bool active=TRUE;		// Window Active Flag Set To TRUE By Default
bool fullscreen=TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default

bool NoMusic=FALSE;
bool NoSound=FALSE;
bool NoBlend=TRUE;

// Iallafall så är stommen för nätverket laggd...
// Hmmm, det tar sig...

// Enable:as bara ifall man ska försöka få igång nätverket...
bool Network=FALSE;			// Nätverk eller singelplayer...
bool Server=FALSE;			// Om Server, annars klient.

char *server_addr = NULL;
unsigned server_port = 9378;
char *nick = NULL;
int proto_only = 0;

// I vilkenservervillduanslutatill-rutan?
char TextEntered[256];

int broms_in_progress = 0;

// Skaffa FPS räknare... hur ska man gööra?


// Temporära grejjer!!!
bool dod=FALSE;
bool sant=FALSE;
int krocktimer=20;
int tmprand;
float debug1,debug2;	// Dessa kan jag nog ge lite olika uppgifter...

bool debugBlend=FALSE;
float blendcolor;

char mapFile[17]="data/default,map";



GLuint	GubbeDispList;



// Fina spel grejjer!
float speed=50.0f;		// Aj aj... det här var inte bra...

const int gubbtid=300;		// Hur lång tid en gubbe är död... Räknas i frames :)
const int nrgubbar=100;


// ej, det här är typ.nätwärch stuff. (Vilket språk jag har!)
/*
   WSADATA wsaData;
   SOCKET AnnanData;
   SOCKET listeningSocket;

   LPHOSTENT ServerInfo;

   SOCKADDR_IN saServer;
   int nRet;
   */

// Den här är för stor, tar mycket bandbredd. (tror jag)
//char BuFFer[256];
char BuFFer[16];

// En array bestående av 1:or och 0:or... alla knappar...
char PressedB[8];
// En till, fast för knapparna _mottagna_ från andra datorn...
char mPressedB[8];

int IBuf;
float FBuf;

// I vilken rikting den andra bilen befinner sig...
char Riktning[2];

struct cube {
	// Vilket plan den är på. 0.0f är det man går/åker på,
	float z;
	// Vilken textur som ska mappas till kuben...
	int texturenr;
	// Ett namn på stället man är.
	char* beskrivning;
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

struct spelare {
	// Poäng
	int points;
	// Krockar
	int krockar;
	// Överkörda människor
	int runovers;
};

/*struct FPSCOUNTER {
  float timer;
//
int fps;
};


FPSCOUNTER fps; */

gubbe gubbar[nrgubbar];

spelare player;


float bsize=5.0f;

// Storleken på banan skulle behövas laddas in från en fil, men för tillfället vet jag inte riktigt hur det skulle gå till...

struct world {
	int nrcubex;
	int nrcubey;
	struct cube *map;
	char **texture_filenames;
	GLuint *texIDs;
	int ntextures;
};

#define TEXTURE_PATH "data/"
#define map_cube(world, x, y) world.map[(x) * (world).nrcubey + (y)]
struct world world;


car bil;
car mbil;		 // andra bilen i multiplayer


// Bra att ha för att rotera och ha sig...
GLfloat	zrot=0,xrot=0,yrot=0;
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

char* ensiffrachar(int ensiffraint)
{
	// Ejj, kommer direkt från mitt chartest program :)
	// (Undrar om den verkligen fungerar, eller om den läcker minne 2000)

	char* temp= new char[20];

	//itoa(ensiffraint,temp,10);

	temp = "Fungerarej";

	return temp;
}







void glPrint(const char *fmt, ...)					// Custom GL "Print" Routine
{

}

int LoadGLTextures()								// Load Bitmaps And Convert To Textures
{

	/* Load textures from file names in world */
	world.texIDs = (GLuint *)calloc(world.ntextures, sizeof(GLuint));
	if (world.texIDs == NULL) {
		return 0;
	}

	char path_buf[PATH_MAX];

	for (int i = 0; i < world.ntextures; i++) {
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
		unsigned int size = texture->w * texture->h * 3;
		for (int j = 0; j < size / 2; j += 3) {
			memcpy(tp, &pixels[j], 3);
			memcpy(&pixels[j], &pixels[size - j], 3);
			memcpy(&pixels[size - j], tp, 3);
		}
		// Vafan, spegelvänt också?!
		for (int j = 0; j < texture->h * w; j += w) {
			for (int k = 0; k < w / 2; k += 3) {
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

void TimerInit()								// Initialize Our Timer (Get It Ready)
{
}

float TimerGetTime()								// Get Time In Milliseconds
{
	//return( (float) ( timeGetTime() - timer.mm_timer_start) * timer.resolution)*1000.0f;
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
	world.map = (cube *)calloc(world.nrcubex * world.nrcubey, sizeof(struct cube));
	if (world.map == NULL)  {
		return FALSE;
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

	return TRUE;
}

int LoadCars()   // och gubbar.
{
	// Ladda en standard bil...

	bil.x=3;
	bil.y=5;
	bil.z=2;

	bil.helhet=100;

	bil.posx=10;
	bil.posy=10;
	bil.posz=bsize; // Ska nog inte initialiseras här..

	bil.t1=1;
	bil.t2=1;
	bil.t3=1;
	bil.t4=1;

	bil.maxspeed=2.0f;
	bil.curspeed=0.0f;
	bil.accspeed=0.20f;
	bil.maxbspeed=-1.0f;
	bil.bromsspeed=0.3f;
	bil.speeddown=0.10f;
	// Orginal värdet
	// bil.turnspeed=6;
	// Nytt värde, den svänger trotsallt lite segt...
	bil.turnspeed=8;

	bil.angle=0;

	bil.Points=0;

	// "Nätverks"-bilen
	mbil.x=3;
	mbil.y=5;
	mbil.z=2;

	mbil.helhet=100;

	mbil.posx=10;
	mbil.posy=10;
	mbil.posz=bsize; // Ska nog inte initialiseras här..

	mbil.t1=1;
	mbil.t2=1;
	mbil.t3=1;
	mbil.t4=1;

	mbil.maxspeed=2.0f;
	mbil.curspeed=0.0f;
	mbil.accspeed=0.20f;
	mbil.maxbspeed=-1.0f;
	mbil.bromsspeed=0.3f;
	mbil.speeddown=0.10f;
	mbil.turnspeed=8;

	mbil.angle=0;

	mbil.Points=0;

	if(Network)
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




	// Kicka igång alla gubbar
	bool bra;

	int einar=0;
	float tmpcntdon;

	int loop1 = 0;

	for(loop1=0;loop1<nrgubbar;loop1++) {

		gubbar[loop1].alive=TRUE;

		gubbar[loop1].curspeed=0.0f;
		gubbar[loop1].maxspeed=0.3f;
		gubbar[loop1].accspeed=0.15f;
		gubbar[loop1].maxbspeed=0.2f;

		//gubbar[loop1].angle=0;

		gubbar[loop1].x=1.0f;
		gubbar[loop1].y=1.0f;
		gubbar[loop1].z=1.9f;


		bra=FALSE;
		// Voila, en slumpgenerator... då var det bara collisiondetection grejjen kvar...
		// den förbannade doningen FUNGERAR INTE!
		while(!bra) {
			srand(TimerGetTime()+einar);
			einar = einar +1;
			//std::cout << einar << std::endl;
			//std::cout << "J:" << (float)(rand() % world.nrcubex)*100.0f << ":J-" << std::endl;
			gubbar[loop1].posx=(float)((rand() % world.nrcubex*bsize*2)*100)/100.0f;
			gubbar[loop1].posy=(float)((rand() % world.nrcubey*bsize*2)*100)/100.0f;
			gubbar[loop1].angle=rand() % 360;
			//std::cout << "D: X:" << gubbar[loop1].posx << "Y:" << gubbar[loop1].posy << "-" << std::endl;

			bra=TRUE;
		}


		//gubbar[loop1].posx=10.0f;
		//gubbar[loop1].posy=10.0f+loop1*(gubbar[loop1].y+2.2f);

		gubbar[loop1].posz=bsize;

		gubbar[loop1].ltexture=11;
		gubbar[loop1].ltexture2=13;
		gubbar[loop1].dtexture=12;


	}


	// Vi bygger en Display List!!! EJJJJ!!!(som i öj) :)

	GubbeDispList=glGenLists(1);

	glNewList(GubbeDispList,GL_COMPILE);

	/* glBegin(GL_QUADS);

	// Huvudet...

	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// X-----------
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// X-----------
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].posz);// X-----------
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].posz);// X-----------

	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].posz);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].posz);

	// bak och fram


	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// X-----------
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// -----------X
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].posz);// -----------X
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].posz);// X-----------

	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// X-----------
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// -----------X
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].posz);// -----------X
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].posz);// X-----------




*/
	// För att det finns massa som refererar till detta:
	loop1=1;
	glBindTexture(GL_TEXTURE_2D,world.texIDs[gubbar[loop1].ltexture2]);

	glBegin(GL_QUADS);


	// Ovanifrån...

	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// X-----------
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// -----------X
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// -----------X
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// X-----------
	glEnd();

	// Börja en ny glBegin för att vi ska kunna texturemappa huvudet och resten seperat...
	glBindTexture(GL_TEXTURE_2D,world.texIDs[gubbar[loop1].ltexture]);

	glBegin(GL_QUADS);

	//Höger och vänster


	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// X-----------
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// X-----------
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].posz);// X-----------
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].posz);// X-----------

	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].posz);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].posz);

	// bak och fram

	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// X-----------
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// -----------X
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].posz);// -----------X
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].posz);// X-----------

	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// X-----------
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// -----------X
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].posz);// -----------X
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].posz);// X-----------


	glEnd();

	glEndList();


	return TRUE;
}

int SetupNet()
{
	// Förnärvarande inget stöd för nätverk i linux versionen--
	return 0;
}

int InitGL()								//		 All Setup For OpenGL Goes Here
{

	static bool Varitherebefore;

	if (!LoadGLTextures())							// Jump To Texture Loading Routine ( NEW )
	{
		std::cout << "Bananeinar, det verkar inte som om den vill ladda texturerna.";
		exit(1);
	}

	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );


	Varitherebefore=TRUE;

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
	/*
	 * EXERCISE:
	 * Replace this with a call to glFrustum.
	 */
	gluPerspective( 60.0, ratio, 1.0, 1024.0 );

	glMatrixMode( GL_MODELVIEW );
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	//glClearColor(0.0f, 0.0f, 0.0f, 0.2f);
	glClearDepth(1.0f);									// Depth Buffer Setup
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations


	return TRUE;										// Initialization Went OK
}

int RespondToKeys()
{


	/* PressedB[	Vad de olika värdena i den här strängen betyder...
	   0 = UP
	   1 = NER
	   2 = SPACE
	   3 = TAB
	   4 = VÄNSTER
	   5 = HÖGER
	   6 = RESPAWNA
	   */



	for(int PBtmp=0;PBtmp<8;PBtmp++)  // n0lla variabeln varje gång...
		PressedB[PBtmp]='0';


	if(bil.helhet==0) {
		if(keys[SDLK_RETURN]) {
			PressedB[6]='1';
			bil.helhet=100;
			bil.t1=1;
			mbil.Points++;

			cont_sound_stop(brinner, 1);
			play_sound(respawn);

		}
		else {
			cont_sound_play(brinner);
		}
	}


	if(!(bil.helhet==0)) {
		if(keys[SDLK_UP]) {
			bil.curspeed=bil.curspeed+bil.accspeed;
			PressedB[0]='1';
		}

		if(keys[SDLK_DOWN]) {
			bil.curspeed=bil.curspeed-bil.accspeed;
			PressedB[1]='1';
		}
	}

	bool brakepressed=FALSE;

	if(keys[SDLK_SPACE]) {
		PressedB[2]='1';
		brakepressed=TRUE;
		if(bil.curspeed<0.0f && bil.curspeed>-bil.bromsspeed)
			bil.curspeed=0.0f;
		if(bil.curspeed>0.0f && bil.curspeed<bil.bromsspeed)
			bil.curspeed=0.0f;

		if (!broms_in_progress) {
			play_sound(broms);
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
		PressedB[3]='1';
		cont_sound_play(tut);
	}
	else {
		cont_sound_stop(tut, 0);
	}

	float sttmp;



	if(keys[SDLK_LEFT]) {
		sttmp=bil.curspeed/bil.maxspeed;			// Omöjliggör styrning vid stillastående, och
		if(brakepressed)
			sttmp+=0.7f;

		if(bil.curspeed!=0.0f)
			bil.angle+=bil.turnspeed*sttmp;				// öka graden av styrmöjlighet ju snabbare det går.

		PressedB[4]='1';
	}

	if(keys[SDLK_RIGHT]) {
		sttmp=bil.curspeed/bil.maxspeed;
		//std::cout << "HOGER" << std::endl;
		if(brakepressed)
			sttmp+=0.7f;

		if(bil.curspeed!=0.0f)
			bil.angle-=bil.turnspeed*sttmp;				// öka graden av styrmöjlighet ju snabbare det går.

		PressedB[5]='1';
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
		debugBlend=TRUE;
	} else {
		debugBlend=FALSE;
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


	/*BASS_SampleStop(motorn);
	  if(!BASS_ChannelSetAttributes(motorn,22050,-1,-101))
	  MessageBox(NULL,"SKIT","SKIT",MB_OK);
	  BASS_SamplePlay(motorn); */

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

	//std::cout << "Nuvarande Hastighet:" << bil.curspeed << std::endl;

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
	//int tmprand;

	int einar=0; bool bra;

	if(!Network) {
		for(int loop1=0;loop1<nrgubbar;loop1++) {

			// den här funktionen som bestämmer vad gubbarna ska göra måste skrivas om,
			// Gubbarna är totalt urblåsta.
			srand((int)TimerGetTime()+loop1);		// det är jobbigt om gubbarna flyttar på sig så fort man svänger bilen...

			if(gubbar[loop1].alive) {
				tmprand=rand() % 100; // Ejjj, det wooorkar...

				if(tmprand==0 && tmprand<3)  // gubben ska bara vrida sig fååå gånger..
					gubbar[loop1].angle+=10;

				if(tmprand>=3 && tmprand<5)  // Ge även möjligheten att vända åt andra hållet...
					gubbar[loop1].angle-=10;

				if(tmprand>=5 && tmprand<=100)
					gubbar[loop1].curspeed=gubbar[loop1].curspeed+gubbar[loop1].accspeed;



			} else {
				gubbar[loop1].atimer++;

				if(gubbar[loop1].atimer>=gubbtid) {   // Jag antar att man borde randomiza ut platsen igen...
					gubbar[loop1].atimer=0;
					gubbar[loop1].alive=TRUE;
					bra=false;

					while(!bra) {
						einar++;
						srand(TimerGetTime()+einar);
						gubbar[loop1].posx=(float)((rand() % world.nrcubex*bsize*2)*100)/100;
						gubbar[loop1].posy=(float)((rand() % world.nrcubey*bsize*2)*100)/100;
						gubbar[loop1].angle=rand() % 360;
						bra=TRUE;
					}

				}
			}

			if(gubbar[loop1].curspeed<0) {
				if(gubbar[loop1].curspeed<gubbar[loop1].maxbspeed)
					gubbar[loop1].curspeed=gubbar[loop1].maxbspeed;
			} else {
				if(gubbar[loop1].curspeed>gubbar[loop1].maxspeed)
					gubbar[loop1].curspeed=gubbar[loop1].maxspeed;
			}

			if(gubbar[loop1].angle<0)
				gubbar[loop1].angle=gubbar[loop1].angle+360;
			if(gubbar[loop1].angle>359)
				gubbar[loop1].angle=gubbar[loop1].angle-360;


			tmpangle=(float)gubbar[loop1].angle;

			if(gubbar[loop1].angle>=0 && gubbar[loop1].angle<=90) {
				gubbar[loop1].tmpx=-((tmpangle/90.0f)*gubbar[loop1].curspeed);
				gubbar[loop1].tmpy=gubbar[loop1].curspeed+gubbar[loop1].tmpx;
			}

			if(gubbar[loop1].angle>=270 && gubbar[loop1].angle<=360) {
				tmpangle-=270.0f;
				gubbar[loop1].tmpy=(tmpangle/90.0f)*gubbar[loop1].curspeed;
				gubbar[loop1].tmpx=gubbar[loop1].curspeed-gubbar[loop1].tmpy;
			}

			if(gubbar[loop1].angle>90 && gubbar[loop1].angle<=180) {
				tmpangle-=90.0f;
				gubbar[loop1].tmpy=-((tmpangle/90.0f)*gubbar[loop1].curspeed);
				gubbar[loop1].tmpx=-(gubbar[loop1].curspeed+gubbar[loop1].tmpy);
			}

			if(gubbar[loop1].angle>180 && gubbar[loop1].angle<270) {
				tmpangle-=180.0f;
				gubbar[loop1].tmpx=(tmpangle/90.0f)*gubbar[loop1].curspeed;
				gubbar[loop1].tmpy=-(gubbar[loop1].curspeed-gubbar[loop1].tmpx);
			}

			if(!gubbar[loop1].alive) {
				gubbar[loop1].tmpx=0;
				gubbar[loop1].tmpy=0;
			}

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
						play_sound(krasch);
						player.krockar++;
						//std::cout << "KROCK!";


						if(bil.curspeed<0)
							bil.helhet+=(int)(bil.curspeed*4);
						if(bil.curspeed>0)
							bil.helhet-=(int)(bil.curspeed*5);

						if(bil.helhet<=0) {
							bil.helhet=0;
							bil.t1=14;
						}

						//dod=TRUE;
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

		int tmpsndch;
		for(loop1=0; loop1<nrgubbar; loop1++) {
			if(gubbar[loop1].alive)
				if(bil.posx+tmpx+tmpbilx/2>=gubbar[loop1].posx+gubbar[loop1].tmpx-(gubbar[loop1].x/2) && bil.posx+tmpx-tmpbilx/2<=gubbar[loop1].posx+gubbar[loop1].tmpx+(gubbar[loop1].x/2))
					if(bil.posy+tmpy+tmpbily/2>=gubbar[loop1].posy+gubbar[loop1].tmpy-(gubbar[loop1].y/2) && bil.posy+tmpy-tmpbily/2<=gubbar[loop1].posy+gubbar[loop1].tmpy+(gubbar[loop1].y/2)) {
						if(bil.curspeed>0.4f || bil.curspeed<-0.4f) {
							gubbar[loop1].alive=false;		 // Här ska man väl även få poäng, antar jag...

							struct timeval tv;
							gettimeofday(&tv, NULL);
							if(tv.tv_usec % 2)
								play_sound(aj0);
							else
								play_sound(aj1);
							player.runovers++;
							/*} else if(bil.curspeed<1.0f && bil.curspeed>-1.0f) { // det ska vara så att man skjuter dem framför om man kör på dem för sakta...
							  gubbar[loop1].tmpx=tmpx;
							  gubbar[loop1].tmpy=tmpy; */
					} else {
						//gubbar[loop1].angle=-gubbar[loop1].angle;
						gubbar[loop1].angle+=180;
						play_sound(move);
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

		for(int loop3=0 ;loop3<nrgubbar; loop3++)
			for(loop1=0 ;loop1<world.nrcubex;loop1++)
				for(int loop2=0;loop2<world.nrcubey;loop2++) {
					if(map_cube(world, loop1, loop2).z!=0.0f) {		// Om inte kuben är ett underlag...

						if(gubbar[loop3].posx+gubbar[loop3].tmpx+gubbar[loop3].x/2>=CalcMapPlace(loop1,loop2,0)-bsize && gubbar[loop3].posx+gubbar[loop3].tmpx-gubbar[loop3].x/2<=CalcMapPlace(loop1,loop2,0)+bsize)
							if(gubbar[loop3].posy+gubbar[loop3].tmpy+gubbar[loop3].y/2>=CalcMapPlace(loop1,loop2,1)-bsize && gubbar[loop3].posy+gubbar[loop3].tmpy-gubbar[loop3].y/2<=CalcMapPlace(loop1,loop2,1)+bsize) {

								gubbar[loop3].tmpx=0.0f; gubbar[loop3].tmpy=0.0f;
								// Einar...
								//	gubbar[loop3].angle=-gubbar[loop3].angle;
								gubbar[loop3].curspeed=-gubbar[loop3].curspeed;
								gubbar[loop3].angle+=60;


								sant=TRUE;
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

	if(dod) {
		krocktimer--;
		if(!krocktimer) {
			krocktimer=20;
			dod=FALSE;

		}
	}


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




	return TRUE;
}

int DrawGLScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
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
	// öj, det fungerar... Men det fungerar nog inte om man ska ha fler bilar...


	glLoadIdentity();
	glTranslatef(0.0f,0.0f,Distance+SpeedVar);
	glRotatef((float)bil.angle,0.0f,0.0f,1.0f);

	glBindTexture(GL_TEXTURE_2D,world.texIDs[bil.t1]);

	//glColor3f(1.0f,0.0f,1.0f);
	glBegin(GL_QUADS);

	// Tak...

	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(bil.x/2),0.0f+(bil.y/2),bil.z+bil.posz);// X-----------
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(bil.x/2),0.0f+(bil.y/2),bil.z+bil.posz);// -----------X
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(bil.x/2),0.0f-(bil.y/2),bil.z+bil.posz);// -----------X
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(bil.x/2),0.0f-(bil.y/2),bil.z+bil.posz);// X-----------

	// Inget golv, för det kommer inte att synas... tror jag.

	//Höger och vänster

	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(bil.x/2),0.0f+(bil.y/2),bil.z+bil.posz);// X-----------
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f-(bil.x/2),0.0f-(bil.y/2),bil.z+bil.posz);// X-----------
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f-(bil.x/2),0.0f-(bil.y/2),bil.posz);// X-----------
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(bil.x/2),0.0f+(bil.y/2),bil.posz);// X-----------

	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f+(bil.x/2),0.0f+(bil.y/2),bil.z+bil.posz);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(bil.x/2),0.0f-(bil.y/2),bil.z+bil.posz);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(bil.x/2),0.0f-(bil.y/2),bil.posz);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f+(bil.x/2),0.0f+(bil.y/2),bil.posz);

	// bak och fram

	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(bil.x/2),0.0f+(bil.y/2),bil.z+bil.posz);// X-----------
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(bil.x/2),0.0f+(bil.y/2),bil.z+bil.posz);// -----------X
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(bil.x/2),0.0f+(bil.y/2),bil.posz);// -----------X
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(bil.x/2),0.0f+(bil.y/2),bil.posz);// X-----------

	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(bil.x/2),0.0f-(bil.y/2),bil.z+bil.posz);// X-----------
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(bil.x/2),0.0f-(bil.y/2),bil.z+bil.posz);// -----------X
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(bil.x/2),0.0f-(bil.y/2),bil.posz);// -----------X
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(bil.x/2),0.0f-(bil.y/2),bil.posz);// X-----------


	glEnd();

	// NÄTVERKSSAK BORTTAGEN

	// Rita upp gubbbananerna...
	// Hoho! De SNURRAR!!! :)))))

	// glLoadIdentity();
	// glTranslatef(transx,transy,Distance+SpeedVar);

	if(!Network) {

		for(loop1=0;loop1<nrgubbar;loop1++) {

			// HAHA!!! Det gick till slut! :)
			glLoadIdentity();
			glTranslatef(gubbar[loop1].posx+transx,gubbar[loop1].posy+transy,Distance+SpeedVar);
			glRotatef((float)gubbar[loop1].angle,0.0f,0.0f,1.0f);

			if(gubbar[loop1].alive) {


				if(!NoBlend)
					glEnable(GL_BLEND);


				glColor4f(1.0f,1.0f,1.0f,255);
				glCallList(GubbeDispList);

				if(blendcolor==0.0f)
					glDisable(GL_BLEND);


			} else {

				// Är man överkörd står man nog inte upp längre... :) Det här blir bättre...
				glBindTexture(GL_TEXTURE_2D,world.texIDs[gubbar[loop1].dtexture]);


				glBegin(GL_QUADS);

				// Ovanifrån...

				glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),bsize);// X-----------
				glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),bsize);// -----------X
				glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),bsize);// -----------X
				glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),bsize);// X-----------
				glEnd();

			}
		}

	} // !Network

	glDisable(GL_TEXTURE_2D);

	glLoadIdentity();
	glTranslatef(5.0f,-7.0f,-18.0f);
	glColor3f(1.0f,0.0f,0.0f);
	glPrint("Helhet(%):  ");
	glPrint(ensiffrachar(bil.helhet));


	if(!Network) {
		glLoadIdentity();
		glTranslatef(-5.0,-7.0f,-18.0f);
		glColor3f(0.0f,0.0f,1.0f);
		glPrint("Overruns:    ");
		glPrint(ensiffrachar(player.runovers));


	} else {
		glLoadIdentity();
		glTranslatef(-5.0,-5.0f,-18.0f);
		glColor3f(0.0f,0.0f,1.0f);
		glPrint("Dina poäng:  ");
		glPrint(ensiffrachar(bil.Points));
		glTranslatef(0.0f,-2.0f,0.0f);
		glPrint("Annans poäng:  ");
		glPrint(ensiffrachar(mbil.Points));

		glLoadIdentity();
		glTranslatef(5.0f,-5.0f,-18.0f);
		glColor3f(1.0f,0.0f,0.0f);
		glPrint("Enemy pos:  ");
		glPrint(Riktning);

	}


	return true;

	// TYp.

}

void KillGLWindow()
{
	SDL_Quit();

}

int CheckaEvents()
{
	SDL_Event event;

	while( SDL_PollEvent( &event ) ){
		switch( event.type ){
		case SDL_KEYDOWN:
			keys[event.key.keysym.sym]=true;
			break;

		case SDL_KEYUP:
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

int print_help()
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
	int this_option_optind;
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
		this_option_optind = optind ? optind : 1;

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

	parse_options(argc, argv);

	// C++ SUGER SÅ MYCKET!!!1
	world.nrcubex = 20;
	world.nrcubey = 20;
	// De texturer som på nåt sätt ska laddas är:
	char *texture_filenames[] = {
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
	for(int tmpk=0;tmpk<350;tmpk++)
		keys[tmpk]=false;

	SDL_Surface *screen;

	bool done=false;

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)<0) {
		std::cout << std::endl << "Kunde inte initialisera SDL!";
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
		std::cout << "EInar";
		exit(1);
	}

	fullscreen=FALSE;							// Windowed Mode

	InitGL();
	LoadCars();
	LoadLevel();

	// HUVUDLOOPEN!!! Detta är själva spelet!
	// TODO: Implementera frameskip...
	Uint32 TimerTicks;
	Uint32 tmpTicks;

	init_sound();

	while(!done)
	{

		TimerTicks=SDL_GetTicks();
		tmpTicks=SDL_GetTicks();
		if(CheckaEvents()==1) {
			exit(0);
		}
		//std::cout << "KollaEvents:" << SDL_GetTicks-tmpTicks << std::endl;

		tmpTicks=SDL_GetTicks();
		RespondToKeys();
		//std::cout << "Respond:" << SDL_GetTicks-tmpTicks << std::endl;

		tmpTicks=SDL_GetTicks();
		CalcGameVars();
		//std::cout << "Calc:" << SDL_GetTicks-tmpTicks << std::endl;

		tmpTicks=SDL_GetTicks();
		DrawGLScene();
		//std::cout << "Draw:" << SDL_GetTicks-tmpTicks << std::endl;

		tmpTicks=SDL_GetTicks();
		SDL_GL_SwapBuffers();
		//std::cout << "Swap:" << SDL_GetTicks-tmpTicks << std::endl;

		while(TimerTicks+30>SDL_GetTicks()) { usleep(1); }

		if(keys[SDLK_ESCAPE])
		{
			std::cout << "Escape tryckt, avslutar..." << std::endl;
			SDL_Quit();
			done = 1;
		}

	}

	return 0; 	// SLYYT.
}
