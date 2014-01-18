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
#include <SDL.h>
#include <sys/time.h>
#include <sys/param.h>
#include <getopt.h>

#include "main.h"
#include "world.h"
#include "sound.h"
#include "network.h"
#include "hud.h"
#include "object.h"
#include "stl.h"
#include "car.h"
#include "gubbe.h"
#include "gl.h"
#include "console.h"

int initial_width = 640;
int initial_height = 480;
const int sdl_video_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

int keys[SDL_NUM_SCANCODES];			// Array Used For The Keyboard Routine

int NoBlend=1;

int sound = 1;
int sound_music = 1;

// Enable:as bara ifall man ska försöka få igång nätverket...
int Network=0;			// Nätverk eller singelplayer...

char *server_addr = NULL;
unsigned server_port = 9378;
char *nick = NULL;
int proto_only = 0;

int debugBlend=0;
float blendcolor;


static int RespondToKeys()
{
        static int broms_in_progress = 0;

	if(keys[SDL_SCANCODE_SPACE]) {
		world.bil.brakeForce = 200.0f;

		if (!broms_in_progress) {
			sound_play(broms);
			broms_in_progress = 1;
		}
	}
	else {
		broms_in_progress = 0;
		world.bil.brakeForce = 0.0f;
	}

        if(keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_LEFT]) {
            if(keys[SDL_SCANCODE_RIGHT]) {
                world.bil.steering -= 0.5f;
            } else if(keys[SDL_SCANCODE_LEFT]) {
                world.bil.steering += 0.5f;
            }

            if(world.bil.steering > 0.7f)
                world.bil.steering = 0.7f;
            else if(world.bil.steering < -0.7f)
                world.bil.steering = -0.7f;

        } else {

            /* Go straight */
            world.bil.steering *= 0.05f;
        }


	if(world.bil.helhet==0) {
		if(keys[SDL_SCANCODE_RETURN]) {
			world.bil.helhet=100;
			world.bil.t1=1;
			/* mbil.Points++; */

			sound_cont_stop(brinner, 1);
			sound_play(respawn);

		}
		else {
			sound_cont_play(brinner);
		}
	}

        if(world.bil.helhet) {
            if(keys[SDL_SCANCODE_UP]) {
                world.bil.engineForce += 50.0f;
            } else if(keys[SDL_SCANCODE_DOWN]) {
                world.bil.engineForce -= 50.0f;
            } else {
                world.bil.engineForce *= 0.2f;
            }

            if(world.bil.engineForce > 1000.0f)
                world.bil.engineForce = 1000.0f;
        }

        /* FIXME */
        plRaycastVehicle_ApplyEngineForce(world.bil.bt_vehicle, world.bil.engineForce, 2);
        plRaycastVehicle_ApplyEngineForce(world.bil.bt_vehicle, world.bil.engineForce, 3);
        plRaycastVehicle_SetBrake(world.bil.bt_vehicle, world.bil.brakeForce, 2);
        plRaycastVehicle_SetBrake(world.bil.bt_vehicle, world.bil.brakeForce, 3);

        plRaycastVehicle_SetSteeringValue(world.bil.bt_vehicle, world.bil.steering, 0);
        plRaycastVehicle_SetSteeringValue(world.bil.bt_vehicle, world.bil.steering, 1);


        if(keys[SDL_SCANCODE_U]) {
            plVector3 force, rel_pos;
            force[0] = 0.0f; force[1] = 0.0f; force[2] = 50.0f;
            rel_pos[0] = 0.0f; rel_pos[1] = 0.0f; rel_pos[2] = 0.0f;

            plRigidBody_ApplyForce(world.bil.bt_rbody, force, rel_pos);
        }
        if(keys[SDL_SCANCODE_N]) {
            plVector3 force, rel_pos;
            force[0] = 0.0f; force[1] = 0.0f; force[2] = -50.0f;
            rel_pos[0] = 0.0f; rel_pos[1] = 0.0f; rel_pos[2] = 0.0f;

            plRigidBody_ApplyForce(world.bil.bt_rbody, force, rel_pos);
        }

	if(keys[SDL_SCANCODE_TAB]) {
		sound_cont_play(tut);
	}
	else {
		sound_cont_stop(tut, 0);
	}

	// Styr kameran
	if(keys[SDL_SCANCODE_F5]) {
		world.camera.z-=0.5f;
	}
	if(keys[SDL_SCANCODE_F6]) {
		world.camera.z+=0.5f;
	}

	if(keys[SDL_SCANCODE_F8]) {
		world.camera.x+=0.9f;
	}
	if(keys[SDL_SCANCODE_F7]) {
		world.camera.x-=0.9f;
	}
	if(keys[SDL_SCANCODE_F3]) {
		world.camera.y+=0.9f;
	}
	if(keys[SDL_SCANCODE_F4]) {
		world.camera.y-=0.9f;
	}

	if(keys[SDL_SCANCODE_F9])
		NoBlend=!NoBlend;

	if(keys[SDL_SCANCODE_F2]) {
		debugBlend=1;
	} else {
		debugBlend=0;
	}
        if(keys[SDL_SCANCODE_T]) {
	    input_field_activate();
        }

        if(keys[SDL_SCANCODE_ESCAPE])
        {
            printf("Escape tryckt, avslutar...\n");
            return 0;
        }

	return 1;
}


static void peer_send_line(const char *nick, const char *input) {
	hud_printf("%s> %s", nick, input);
}



static int CheckaEvents()
{
	SDL_Event event;

	while( SDL_PollEvent( &event ) ){
		switch( event.type ){
		case SDL_KEYDOWN:
			if(!input_field_key_event(event.key.keysym, SDL_KEYDOWN))
				keys[event.key.keysym.scancode]=1;
			break;

		case SDL_KEYUP:
		    //input_field_key_event(event.key.keysym, SDL_KEYUP);
			keys[event.key.keysym.scancode]=0;
			break;

		case SDL_WINDOWEVENT:
			switch (event.window.event) {
			case SDL_WINDOWEVENT_RESIZED:
				gl_resize(event.window.data1, event.window.data2);
				break;
			}
			break;

		case SDL_TEXTINPUT:
			input_field_add_text(event.text.text);
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
			sound = 0;
			break;
		case 'M':
			sound_music = 0;
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
	SDL_Window *window;
	char mbuf[1024];

	srand(time(NULL));

	parse_options(argc, argv);

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)<0) {
		printf("Kunde inte initialisera SDL!\n");
		exit(1);
	}
	atexit(SDL_Quit);
	SDL_StopTextInput();

	window = SDL_CreateWindow("gta2k4lin",
				  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				  initial_width, initial_height,
				  sdl_video_flags);
	if (window == NULL)
	{
		printf("SDL_CreateWindow failed\n");
		exit(1);
	}


	network_init();
	Network=0;

	if (server_addr != NULL) {
		if (network_connect(server_addr, server_port, nick, proto_only)) {
			fprintf(stderr, "Connect failed, exiting :(\n");
			return 28;
		}
		Network = 1;
	}


	gl_init(window, initial_width, initial_height);

	world_init();

        hud_init();
	if (sound)
		sound_init(sound_music);

	// TODO: Implementera frameskip...
	Uint32 TimerTicks;
	int done=0;
	while(!done)
	{
		/* Bullet */
		plStepSimulation(world.dynamics_world, 1.0f/60.0f);

		TimerTicks=SDL_GetTicks();
		if(CheckaEvents()==1) {
			exit(0);
		}

                if(!hud_input_field_active()) {
                    if(!RespondToKeys())
                        done = 1;
                }

                camera_move_for_car(&world.bil);

		gl_drawscene();
                hud_render();
		SDL_GL_SwapWindow(window);

		if (Network) {
			network_put_position(&(world.bil.o));
			network_get_positions(world.opponents);

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
