#include <stdlib.h>
#include <sys/param.h>

#include <SDL_audio.h>
#include <SDL_mixer.h>

#include "sound.h"

// LjYYYYd.

#define NUM_SOUNDS 10
Mix_Chunk *sound_chunks[NUM_SOUNDS];
#define SAMPLE_PATH "data/ljud/"

#define NUM_CHANNELS 4

struct cont_sound {
	enum sounds sound;
	int channel;
};
struct cont_sound cont_sounds[NUM_CHANNELS];

int background_channel = -1;

int LoadSample(char *file, enum sounds sound) {
	char path_buf[PATH_MAX];

	snprintf(path_buf, PATH_MAX, "%s%s", SAMPLE_PATH,
			file);

	sound_chunks[sound] = Mix_LoadWAV(path_buf);
	if (sound_chunks[sound] == NULL) {
		fprintf(stderr, "Unable to load file '%s': %s\n", file, Mix_GetError());
	}

	return 0;
}

int LoadSamples()				// Här loadar vi alla bananiga samples vi ska dra igång...
{
	int errors = 0;
	errors += LoadSample("aj.ogg", aj0);
	errors += LoadSample("aj2.ogg", aj1);
	errors += LoadSample("brinner.ogg", brinner);
	errors += LoadSample("broms.ogg", broms);
	errors += LoadSample("farlig.ogg", farlig);
	errors += LoadSample("krasch.ogg", krasch);
	errors += LoadSample("move.ogg", move);
	errors += LoadSample("respawn.ogg", respawn);
	errors += LoadSample("tut.ogg", tut);
	errors += LoadSample("welcome.ogg", welcome);

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

void channel_finished(int channel) {
	int i;

	for (i = 0; i < NUM_CHANNELS; i++) {
		if (cont_sounds[i].channel == channel) {
			cont_sounds[i].channel =
				PlaySoundChannel(cont_sounds[i].sound,
						cont_sounds[i].channel);
			return;
		}
	}

}

void cont_sound_play(enum sounds sound) {
	int i;

	/* Already playing? */
	for (i = 0; i < NUM_CHANNELS; i++) {
		if (cont_sounds[i].sound == sound &&
				cont_sounds[i].channel != -1) {
			return;
		}
	}

	/* Find free slot */
	for (i = 0; i < NUM_CHANNELS; i++) {
		if (cont_sounds[i].channel == -1) {
			cont_sounds[i].sound = sound;
			cont_sounds[i].channel = PlaySound(sound);
			return;
		}
	}

}

void cont_sound_stop(enum sounds sound, int halt) {
	int i, channel;

	/* Halt and free slot */
	for (i = 0; i < NUM_CHANNELS; i++) {
		if (cont_sounds[i].sound == sound &&
				cont_sounds[i].channel != -1) {
			channel = cont_sounds[i].channel;
			cont_sounds[i].channel = -1;
			if (halt)
				Mix_HaltChannel(channel);
		}
	}

}

int init_sound() {
	int i;

	int audio_rate = 22050;
	Uint16 audio_format = AUDIO_S16SYS;
	int audio_buffers = 4096;

	if (Mix_OpenAudio(audio_rate, audio_format, NUM_CHANNELS, audio_buffers) != 0) {
		fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
		return 1;
	}

	Mix_ChannelFinished(channel_finished);

	LoadSamples();

	for (i = 0; i < NUM_CHANNELS; i++) {
		cont_sounds[i].channel = -1;
	}

	PlaySound(welcome);
	cont_sound_play(farlig);

	return 0;
}
