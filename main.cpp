// gta 2000... :) (Inte officielt namn)
// Detta ska f�rest�lla linux porten...



//////////////////////////////////////////////////////////////////////////
//		KATASTROF::		Koden blir mer och mer ostrukturerad			//
//						Oklart hur l�nge det g�r att fatta n�t av den :)//
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
#include "Tga.h"

#include "SDL.h"
#include "SDL_audio.h"
#include "SDL_mixer.h"

int width = 640;
int height = 480;
int bpp = 32; // Vi gillar 32 h�r d�va


// Hymmz, dessa verkar inte finnas i linux...
bool TRUE=true;
bool FALSE=false;

bool keys[350];			// Array Used For The Keyboard Routine
bool active=TRUE;		// Window Active Flag Set To TRUE By Default
bool fullscreen=TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default

bool NoMusic=FALSE;
bool NoSound=FALSE;
bool NoBlend=TRUE;

#define NUM_SOUNDS 10
enum sounds {
	aj0,
	aj1,
	brinner,
	broms,
	farlig,
	krasch,
	move,
	respawn,
	tut,
	welcome,
};
Mix_Chunk *sound_chunks[NUM_SOUNDS];
#define SAMPLE_PATH "data/ljud/"

// Iallafall s� �r stommen f�r n�tverket laggd...
// Hmmm, det tar sig...

// Enable:as bara ifall man ska f�rs�ka f� ig�ng n�tverket...
bool Network=FALSE;			// N�tverk eller singelplayer...
bool Server=FALSE;			// Om Server, annars klient.

// I vilkenservervillduanslutatill-rutan?
char TextEntered[256];

int broms_in_progress = 0;
int tut_in_progress = 0;
int brinner_channel = -1;
int background_channel = -1;


// Skaffa FPS r�knare... hur ska man g��ra?


// Tempor�ra grejjer!!!
bool dod=FALSE;
bool sant=FALSE;
int krocktimer=20;
int tmprand;
float debug1,debug2;	// Dessa kan jag nog ge lite olika uppgifter...

bool debugBlend=FALSE;
float blendcolor;

char mapFile[17]="data/default,map";




// Antalet texturer
const int nroftext=16;		// Alltid en mer en det verkliga antalet, typ...

Texture	texture[nroftext];
GLuint	GubbeDispList;



// Fina spel grejjer!
float speed=50.0f;		// Aj aj... det h�r var inte bra...

const int gubbtid=300;		// Hur l�ng tid en gubbe �r d�d... R�knas i frames :)
const int nrgubbar=100;


// ej, det h�r �r typ.n�tw�rch stuff. (Vilket spr�k jag har!)
/*
   WSADATA wsaData;
   SOCKET AnnanData;
   SOCKET listeningSocket;

   LPHOSTENT ServerInfo;

   SOCKADDR_IN saServer;
   int nRet;
   */

// Den h�r �r f�r stor, tar mycket bandbredd. (tror jag)
//char BuFFer[256];
char BuFFer[16];

// En array best�ende av 1:or och 0:or... alla knappar...
char PressedB[8];
// En till, fast f�r knapparna _mottagna_ fr�n andra datorn...
char mPressedB[8];

int IBuf;
float FBuf;

// I vilken rikting den andra bilen befinner sig...
char Riktning[2];



struct cube {
	// Vilket plan den �r p�. 0.0f �r det man g�r/�ker p�,
	float z;
	// Vilken textur som ska mappas till kuben...
	int texturenr;
	// Ett namn p� st�llet man �r.
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
	// Hur m�nga grader bilen �r vriden...
	int angle;
	// Hur hel bilen �r(%). 100 �r helhel, 0 �r heltrasig.
	int helhet;
	// Po�ng. I Multiplayer spel hur m�nga "frags" man har...
	int Points;


};

struct gubbe {
	// Storleken:
	float x,y,z;
	// Positionen:
	float posx,posy,posz;
	// Texturer. ltexture2=huvudet. ltexture=resten. dtexturer=texture d� gubben d�tt...
	int ltexture, ltexture2,  dtexture;
	// Fartsaker...
	float maxspeed,maxbspeed,curspeed,accspeed;
	// Hur m�nga grader gubben �r vriden...
	int angle;
	// Lever?
	bool alive;
	// Tid tills han lever igen...
	int atimer;
	// 2 tempor�ra grejjer... Anv�nds framf�rallt av f�rflyttnings funktionen...
	float tmpx,tmpy;

};

struct spelare {
	// Po�ng
	int points;
	// Krockar
	int krockar;
	// �verk�rda m�nniskor
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

// Storleken p� banan skulle beh�vas laddas in fr�n en fil, men f�r tillf�llet vet jag inte riktigt hur det skulle g� till...
const int nrcubex=20;
const int nrcubey=20;

cube map[nrcubex][nrcubey];




car bil;
car mbil;		 // andra bilen i multiplayer


// Bra att ha f�r att rotera och ha sig...
GLfloat	zrot=0,xrot=0,yrot=0;
GLfloat transx=0.0f, transy=0.0f;
GLfloat Distance=-30.0f, SpeedVar, tmpSpeedVar;

// LjYYYYd.

int LoadSample(char *file, enum sounds sound) {
	sound_chunks[sound] = Mix_LoadWAV(file);
	if (sound == NULL) {
		fprintf(stderr, "Unable to load WAV file: %s\n", Mix_GetError());
	}
}

int LoadSamples()				// H�r loadar vi alla bananiga samples vi ska dra ig�ng...
{
	int errors = 0;
	errors += LoadSample(SAMPLE_PATH "aj.ogg", aj0);
	errors += LoadSample(SAMPLE_PATH "aj2.ogg", aj1);
	errors += LoadSample(SAMPLE_PATH "brinner.ogg", brinner);
	errors += LoadSample(SAMPLE_PATH "broms.ogg", broms);
	errors += LoadSample(SAMPLE_PATH "farlig.ogg", farlig);
	errors += LoadSample(SAMPLE_PATH "krasch.ogg", krasch);
	errors += LoadSample(SAMPLE_PATH "move.ogg", move);
	errors += LoadSample(SAMPLE_PATH "respawn.ogg", respawn);
	errors += LoadSample(SAMPLE_PATH "tut.ogg", tut);
	errors += LoadSample(SAMPLE_PATH "welcome.ogg", welcome);

	return errors;
}

int PlaySoundChannel(enum sounds sound, int channel)
{
	if (sound_chunks[sound] == NULL) {
		fprintf(stderr, "Sound chunk is not loaded: %d\n", sound);
	}

	channel = Mix_PlayChannel(channel, sound_chunks[sound], 0);
	if (channel == -1) {
		fprintf(stderr, "Unable to play WAV file: %s\n", Mix_GetError());
	}

	return channel;
}

int PlaySound(enum sounds sound)
{
	return PlaySoundChannel(sound, -1);
}

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
	// Ejj, kommer direkt fr�n mitt chartest program :)
	// (Undrar om den verkligen fungerar, eller om den l�cker minne 2000)

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

	bool status=false;

	/*
	// De texturer som p� n�t s�tt ska laddas �r:
	if ((TextureImage[0]=LoadBMP("data/test.bmp")) &&
	(TextureImage[1]=LoadBMP("data/carroof.bmp")) &&
	(TextureImage[2]=LoadBMP("data/road1.bmp")) &&
	(TextureImage[3]=LoadBMP("data/road2.bmp")) &&
	(TextureImage[4]=LoadBMP("data/building1.bmp")) &&
	(TextureImage[5]=LoadBMP("data/road3.bmp")) &&
	(TextureImage[6]=LoadBMP("data/road4.bmp")) &&
	(TextureImage[7]=LoadBMP("data/road5.bmp")) &&
	(TextureImage[8]=LoadBMP("data/road6.bmp")) &&
	(TextureImage[9]=LoadBMP("data/road7.bmp")) &&
	(TextureImage[10]=LoadBMP("data/dhcred.bmp")) &&
	(TextureImage[11]=LoadBMP("data/gubbel.bmp")) &&
	(TextureImage[12]=LoadBMP("data/gubbed.bmp")) &&
	(TextureImage[13]=LoadBMP("data/gubbel2.bmp")) &&
	(TextureImage[14]=LoadBMP("data/carroof2.bmp")) &&
	(TextureImage[15]=LoadBMP("data/buske.bmp"))) {
	*/

	if(LoadTGA(&texture[0],"data/test.tga") &&
			LoadTGA(&texture[1],"data/carroof.tga") &&
			LoadTGA(&texture[2],"data/road1.tga") &&
			LoadTGA(&texture[3],"data/road2.tga") &&
			LoadTGA(&texture[4],"data/building1.tga") &&
			LoadTGA(&texture[5],"data/road3.tga") &&
			LoadTGA(&texture[6],"data/road4.tga") &&
			LoadTGA(&texture[7],"data/road5.tga") &&
			LoadTGA(&texture[8],"data/road6.tga") &&
			LoadTGA(&texture[9],"data/road7.tga") &&
			LoadTGA(&texture[10],"data/dhcred.tga") &&
			LoadTGA(&texture[11],"data/gubbel.tga") &&
			LoadTGA(&texture[12],"data/gubbed.tga") &&
			LoadTGA(&texture[13],"data/gubbel2.tga") &&
			LoadTGA(&texture[14],"data/carroof2.tga") &&
			LoadTGA(&texture[15],"data/buske.tga")) {

		status=true;

		for(int loop=0;loop<nroftext;loop++) {
			glGenTextures(1,&texture[loop].texID);
			glBindTexture(GL_TEXTURE_2D,texture[loop].texID);
			glTexImage2D(GL_TEXTURE_2D,0,3,texture[loop].width,texture[loop].height,0,GL_RGB,GL_UNSIGNED_BYTE,texture[loop].imageData);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		}
	}



	return status;							// Return The Status
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

	// Anv�nds inte, borttaget f�r att spara plats...
	// Humm... jag kom p� att det visst anv�nds. :)
	// Bara att kopiera tillbaks...

	// LADDA IN!!!!

	int loop1 = 0, loop2 = 0;

	for(loop1=0;loop1<nrcubex;loop1++)
		for(loop2=0;loop2<nrcubey;loop2++) {
			map[loop1][loop2].z=0.0f;
			map[loop1][loop2].texturenr=0;
			map[loop1][loop2].beskrivning="Testbeskrivning";
		}

	map[0][0].z=0.0f;
	map[0][0].texturenr=1;

	map[0][1].z=0.0f;
	map[0][1].texturenr=1;

	// V�gen -------------------------------
	for(loop1=0;loop1<nrcubey;loop1++)
		map[1][loop1].texturenr=2;

	for(loop1=0;loop1<nrcubey;loop1++)
		map[2][loop1].texturenr=3;

	for(loop1=0;loop1<nrcubex;loop1++)
		map[loop1][nrcubey-2].texturenr=8;

	for(loop1=2;loop1<nrcubex;loop1++)
		map[loop1][nrcubey-3].texturenr=9;

	map[1][nrcubey-2].texturenr=5;
	map[2][nrcubey-2].texturenr=6;
	map[2][nrcubey-3].texturenr=7;


	// "V�ggen" runtomkring
	for(loop1=0;loop1<nrcubey;loop1++) {
		map[0][loop1].texturenr=4;
		map[0][loop1].z=1.0f;
	}

	for(loop1=0;loop1<nrcubey;loop1++) {
		map[nrcubex-1][loop1].texturenr=4;
		map[nrcubex-1][loop1].z=1.0f;
	}

	for(loop1=0;loop1<nrcubex;loop1++) {
		map[loop1][0].texturenr=4;
		map[loop1][0].z=1.0f;
	}
	for(loop1=0;loop1<nrcubex;loop1++) {
		map[loop1][nrcubey-1].texturenr=4;
		map[loop1][nrcubey-1].z=1.0f;
	}

	// Vi l�gger in lite buskar
	for(loop1=1;loop1<(nrcubey/2-1);loop1+=2) {
		map[nrcubex/2][loop1].texturenr=15;
		map[nrcubex/2][loop1].z=1.0f;
	}

	// V�gen in till mitten och den fina credits saken d�r.
	for(loop1=3;loop1<nrcubex/2;loop1++)
		map[loop1][nrcubey/2].texturenr=7;

	map[nrcubex/2][nrcubey/2].texturenr=10;
	map[nrcubex/2][nrcubey/2].z=2.0f;




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
	bil.posz=bsize; // Ska nog inte initialiseras h�r..

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
	// Orginal v�rdet
	// bil.turnspeed=6;
	// Nytt v�rde, den sv�nger trotsallt lite segt...
	bil.turnspeed=8;

	bil.angle=0;

	bil.Points=0;

	// "N�tverks"-bilen
	mbil.x=3;
	mbil.y=5;
	mbil.z=2;

	mbil.helhet=100;

	mbil.posx=10;
	mbil.posy=10;
	mbil.posz=bsize; // Ska nog inte initialiseras h�r..

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




	// Kicka ig�ng alla gubbar
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
		// Voila, en slumpgenerator... d� var det bara collisiondetection grejjen kvar...
		// den f�rbannade doningen FUNGERAR INTE!
		while(!bra) {
			srand(TimerGetTime()+einar);
			einar = einar +1;
			std::cout << einar << std::endl;
			//std::cout << "J:" << (float)(rand() % nrcubex)*100.0f << ":J-" << std::endl;
			gubbar[loop1].posx=(float)((rand() % nrcubex*bsize*2)*100)/100.0f;
			gubbar[loop1].posy=(float)((rand() % nrcubey*bsize*2)*100)/100.0f;
			gubbar[loop1].angle=rand() % 360;
			std::cout << "D: X:" << gubbar[loop1].posx << "Y:" << gubbar[loop1].posy << "-" << std::endl;

			bra=TRUE;
		}


		//gubbar[loop1].posx=10.0f;
		//gubbar[loop1].posy=10.0f+loop1*(gubbar[loop1].y+2.2f);

		gubbar[loop1].posz=bsize;

		gubbar[loop1].ltexture=11;
		gubbar[loop1].ltexture2=13;
		gubbar[loop1].dtexture=12;


	}


	// Vi bygger en Display List!!! EJJJJ!!!(som i �j) :)

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
	// F�r att det finns massa som refererar till detta:
	loop1=1;
	glBindTexture(GL_TEXTURE_2D,texture[gubbar[loop1].ltexture2].texID);

	glBegin(GL_QUADS);


	// Ovanifr�n...

	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// X-----------
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f+(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// -----------X
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// -----------X
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(gubbar[loop1].x/2),0.0f-(gubbar[loop1].y/2),gubbar[loop1].z+gubbar[loop1].posz);// X-----------
	glEnd();

	// B�rja en ny glBegin f�r att vi ska kunna texturemappa huvudet och resten seperat...
	glBindTexture(GL_TEXTURE_2D,texture[gubbar[loop1].ltexture].texID);

	glBegin(GL_QUADS);

	//H�ger och v�nster


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
	// F�rn�rvarande inget st�d f�r n�tverk i linux versionen--
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


	/* PressedB[	Vad de olika v�rdena i den h�r str�ngen betyder...
	   0 = UP
	   1 = NER
	   2 = SPACE
	   3 = TAB
	   4 = V�NSTER
	   5 = H�GER
	   6 = RESPAWNA
	   */



	for(int PBtmp=0;PBtmp<8;PBtmp++)  // n0lla variabeln varje g�ng...
		PressedB[PBtmp]='0';


	if(bil.helhet==0) {
		if(keys[SDLK_RETURN]) {
			PressedB[6]='1';
			bil.helhet=100;
			bil.t1=1;
			mbil.Points++;

			if (brinner_channel != -1) {
				Mix_HaltChannel(brinner_channel);
				brinner_channel = -1;
			}
			PlaySound(respawn);

		}
		else {
			if (brinner_channel == -1 ||
					! Mix_Playing(brinner_channel)) {

				brinner_channel = PlaySound(brinner);
			}
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
			PlaySound(broms);
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
		if (!tut_in_progress) {
			PlaySound(tut);
			tut_in_progress = 1;
		}
	}
	else {
		tut_in_progress = 0;
	}

	float sttmp;



	if(keys[SDLK_LEFT]) {
		sttmp=bil.curspeed/bil.maxspeed;			// Om�jligg�r styrning vid stillast�ende, och
		if(brakepressed)
			sttmp+=0.7f;

		if(bil.curspeed!=0.0f)
			bil.angle+=bil.turnspeed*sttmp;				// �ka graden av styrm�jlighet ju snabbare det g�r.

		PressedB[4]='1';
	}

	if(keys[SDLK_RIGHT]) {
		sttmp=bil.curspeed/bil.maxspeed;
		//std::cout << "HOGER" << std::endl;
		if(brakepressed)
			sttmp+=0.7f;

		if(bil.curspeed!=0.0f)
			bil.angle-=bil.turnspeed*sttmp;				// �ka graden av styrm�jlighet ju snabbare det g�r.

		PressedB[5]='1';
	}

	// Detta �r debug grejjer/saker som inte ska vara kvar i "riktiga" versionen...
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
	// D� skickar vi str�ngen PressedB, och tar emot mPressedB
	// Hmm, det h�r var snabbare �n mitt f�rra s�tt, men jag tycker att det borde finnas �nnu snabbare s�tt att
	// Skicka data p�... Undrar hur quake fungerar, tex... :)


	// N�TVERKSSAK BORTTAGEN.

	///////////////////////////////////////////////////////////
	// Ber�kna samma saker fast f�r den andra bilen!! ////////////////////////77
	/////////////////////////////////////////////////////////////////////////////


	// N�TVERKSSAK BORTTAGEN

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


	// Sv�nger bilen s� att den �ker �t r�tt h�ll

	float tmpx=0.0f,tmpy=0.0f;
	float tmpangle;

	tmpangle=(float)bil.angle;

	// Det h�r fungerar, nu �r det bara 3/4 kvar att fixa :D
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

	// F�RDIGT!!!
	if(bil.angle>180 && bil.angle<270) {
		tmpangle-=180.0f;
		tmpx=(tmpangle/90.0f)*bil.curspeed;
		tmpy=-(bil.curspeed-tmpx);
	}

	//std::cout << "Nuvarande Hastighet:" << bil.curspeed << std::endl;

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///			Styr �ven n�tverksbilarna...						 //
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	float mtmpx=0.0f,mtmpy=0.0f;

	//N�TVERKSSAK BORTTAGEN



	// Styr de datorkontrollerade gubbarna (och senare �ven bilar?)
	// ----------------------------------------------------
	// Nej, jag har best�mt mig. Vi st�nger av gubbarna n�r vi k�r med networch...
	//int tmprand;

	int einar=0; bool bra;

	if(!Network) {
		for(int loop1=0;loop1<nrgubbar;loop1++) {

			// den h�r funktionen som best�mmer vad gubbarna ska g�ra m�ste skrivas om,
			// Gubbarna �r totalt urbl�sta.
			srand((int)TimerGetTime()+loop1);		// det �r jobbigt om gubbarna flyttar p� sig s� fort man sv�nger bilen...

			if(gubbar[loop1].alive) {
				tmprand=rand() % 100; // Ejjj, det wooorkar...

				if(tmprand==0 && tmprand<3)  // gubben ska bara vrida sig f��� g�nger..
					gubbar[loop1].angle+=10;

				if(tmprand>=3 && tmprand<5)  // Ge �ven m�jligheten att v�nda �t andra h�llet...
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
						gubbar[loop1].posx=(float)((rand() % nrcubex*bsize*2)*100)/100;
						gubbar[loop1].posy=(float)((rand() % nrcubey*bsize*2)*100)/100;
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





	// Nu �r det kollisiondetection som g�ller h�r...
	// F�r att inte g�ra det f�r sv�rt, g�r vi bara kuber med ett z:a v�rde p� 0.0f �kbara...

	// Kontrollera s� att den inte krockar med en kuuub...
	/* -------------------------------------------------------------
	   H����R �R DET VIIIKTIGAST JUST NUUUUUU!!!!!!!!!!!!!!!!!!
	   ------------------------------------------------------------ */

	float tmpbilx, tmpbily;

	/*if((bil.angle>=75 && bil.angle<=105) || (bil.angle>=255 && bil.angle<= 285)) {
	  tmpbilx=bil.y;  // Kompensera lite f�r att bilen �r olika stor och bred....
	  tmpbily=bil.x;	// BANAN!! Detta fungerade, men risken �r att bilen fastnar i v�ggen...
	  } else { */
	tmpbilx=bil.x;
	tmpbily=bil.y;
	//}

	/////////////////////////////////////////////////////////////////////////////////
	/// EINAR!!! Det h�r kommer ju att dra lika mycket CPU som... jag vet inte vad...
	/////////////////////////////////////////////////////////////////////////////////


	int loop1 = 0, loop2 = 0;


	// Det kan tyckas vara on�digt att kolla alla kuber p� banan... fixa s� att den kollar bara de n�rmaste...
	// kontrollera s� att inte bilen krockar med en stor KUUB!
	for(loop1=0 ;loop1<nrcubex;loop1++)
		for(loop2=0;loop2<nrcubey;loop2++) {
			if(map[loop1][loop2].z!=0.0f) {		// Om inte kuben �r ett underlag...

				if(bil.posx+tmpx+tmpbilx/2>=CalcMapPlace(loop1,loop2,0)-bsize && bil.posx+tmpx-tmpbilx/2<=CalcMapPlace(loop1,loop2,0)+bsize)
					if(bil.posy+tmpy+tmpbily/2>=CalcMapPlace(loop1,loop2,1)-bsize && bil.posy+tmpy-tmpbily/2<=CalcMapPlace(loop1,loop2,1)+bsize) {
						// Vi stannar bilen under den h�r framen f�r att inte bilen ska �ka in d�r iallafall...
						tmpx=0.0f; tmpy=0.0f;
						// EJJ, S� H�R SKA DET JU VARA!!!!!
						bil.curspeed=-bil.curspeed;
						PlaySound(krasch);
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


	// Om vi spelar n�tverk, s� ska vi �ven dra CPU p� att kolla om andra bilen har krockat...
	/////////////////////////////////////////////////////////////////////////////////////////
	// Detta �r allts� n�tverksbilen som kollas emot krockar!!!!

	//N�TVERKSSAK BORTTAGEN


	// Kolla s� att inte de sm� bilbananerna krockar med varandra...

	//N�TVERKSSAK BORTTAGEN




	// Kolla s� att inte gubbarna krockar med bilen...
	// Vi l�gger detta f�re de andra gubb grejjerna, f�r att jag kanske knuffar lite p� gubbarna h�r...

	if(!Network) {

		if((bil.angle>=75 && bil.angle<=105) || (bil.angle>=255 && bil.angle<= 285)) {
			tmpbilx=bil.y;  // Kompensera lite f�r att bilen �r olika l�ng och bred....
			tmpbily=bil.x;	// Detta fungerade inte s� bra till v�ggarna, men jag tror det g�r underverk f�r gubbarna...
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
							gubbar[loop1].alive=false;		 // H�r ska man v�l �ven f� po�ng, antar jag...

							struct timeval tv;
							gettimeofday(&tv, NULL);
							if(tv.tv_usec % 2)
								PlaySound(aj0);
							else
								PlaySound(aj1);
							player.runovers++;
							/*} else if(bil.curspeed<1.0f && bil.curspeed>-1.0f) { // det ska vara s� att man skjuter dem framf�r om man k�r p� dem f�r sakta...
							  gubbar[loop1].tmpx=tmpx;
							  gubbar[loop1].tmpy=tmpy; */
					} else {
						//gubbar[loop1].angle=-gubbar[loop1].angle;
						gubbar[loop1].angle+=180;
						PlaySound(move);
					}
					}

		}

		// Se till s� att inte gubbarna krockar med sig sj�lva
		// denna funktion kan optimeras tusenfalt!!
		// Men det v�rsta m�ste v�l vara att den inte fungerar...
		// Orkar inte kr�ngla med den nu, jag vill f� ig�ng lite roliga saker...

		//TRASIG GUBBSAK BORTTAGEN

		// Se till s� att inte gubbarna krockar med n�t v�ggaktigt...
		// Fungerar, men vad som h�nder med gubbarna beh�ver absolut finjusteras...

		// Oj, oj, oj... precis n�r jag trodde att jag n�tt CPU toppen f�r en liten funktion...

		for(int loop3=0 ;loop3<nrgubbar; loop3++)
			for(loop1=0 ;loop1<nrcubex;loop1++)
				for(int loop2=0;loop2<nrcubey;loop2++) {
					if(map[loop1][loop2].z!=0.0f) {		// Om inte kuben �r ett underlag...

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



	// R�kna ut i vilket v�derstreck den andra bilen befinner sig...
	if(Network) {

		// N�TVERKSSAK BORTTAGEN

	}

	if(!Network)
		for(loop1=0;loop1<nrgubbar;loop1++) {
			gubbar[loop1].posx+=gubbar[loop1].tmpx;
			gubbar[loop1].posy+=gubbar[loop1].tmpy;
		}


	// F� kameran att h�jas och s�nkas beroende p� hastigheten...
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
	// Hmm, nu ska vi rita upp spelplanen... sen ska den optimeras s� den bara ritar upp det n�dv�ndiga...

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
	for(loop1=0; loop1<nrcubex; loop1++) {
		for(loop2=0; loop2<nrcubey; loop2++) {


			lp1bstmp=(float)loop1*(bsize*2);
			lp2bstmp=(float)loop2*(bsize*2);
			ztmp=map[loop1][loop2].z*(bsize*2);

			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,texture[map[loop1][loop2].texturenr].texID);

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


			// V�nster sida.
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
	// �j, det fungerar... Men det fungerar nog inte om man ska ha fler bilar...


	glLoadIdentity();
	glTranslatef(0.0f,0.0f,Distance+SpeedVar);
	glRotatef((float)bil.angle,0.0f,0.0f,1.0f);

	glBindTexture(GL_TEXTURE_2D,texture[bil.t1].texID);

	//glColor3f(1.0f,0.0f,1.0f);
	glBegin(GL_QUADS);

	// Tak...

	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f-(bil.x/2),0.0f+(bil.y/2),bil.z+bil.posz);// X-----------
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f+(bil.x/2),0.0f+(bil.y/2),bil.z+bil.posz);// -----------X
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f+(bil.x/2),0.0f-(bil.y/2),bil.z+bil.posz);// -----------X
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f-(bil.x/2),0.0f-(bil.y/2),bil.z+bil.posz);// X-----------

	// Inget golv, f�r det kommer inte att synas... tror jag.

	//H�ger och v�nster

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

	// N�TVERKSSAK BORTTAGEN

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

				// �r man �verk�rd st�r man nog inte upp l�ngre... :) Det h�r blir b�ttre...
				glBindTexture(GL_TEXTURE_2D,texture[gubbar[loop1].dtexture].texID);


				glBegin(GL_QUADS);

				// Ovanifr�n...

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
		glPrint("Dina po�ng:  ");
		glPrint(ensiffrachar(bil.Points));
		glTranslatef(0.0f,-2.0f,0.0f);
		glPrint("Annans po�ng:  ");
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

void channel_finished(int channel) {

	// quick exit
	if (channel != background_channel)
		return;

	background_channel = PlaySoundChannel(farlig, background_channel);
}

int main()
{

	// H�r �r den f�rsta delen av porten....

	// Nollst�ll knapp-arrayen...
	for(int tmpk=0;tmpk<350;tmpk++)
		keys[tmpk]=false;

	SDL_Surface *screen;

	bool done=false;

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)<0) {
		std::cout << std::endl << "Kunde inte initialisera SDL!";
		exit(1);
	}

	atexit(SDL_Quit);

	screen = SDL_SetVideoMode(width, height, bpp, SDL_OPENGL);
	if (screen == NULL)
	{
		std::cout << "EInar";
		exit(1);
	}

	Network=false;

	fullscreen=FALSE;							// Windowed Mode


	int audio_rate = 22050;
	Uint16 audio_format = AUDIO_S16SYS;
	int audio_channels = 1;
	int audio_buffers = 4096;
	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
		fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
		exit(1);
	}

	InitGL();
	LoadCars();
	LoadLevel();
	LoadSamples();


	// HUVUDLOOPEN!!! Detta �r sj�lva spelet!
	// TODO: Implementera frameskip...
	Uint32 TimerTicks;
	Uint32 tmpTicks;

	Mix_ChannelFinished(channel_finished);
	background_channel = PlaySound(welcome);

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
			std::cout << "Escape tryckt, avslutar...";
			SDL_Quit();
		}

	}

	return 0; 	// SLYYT.
}
