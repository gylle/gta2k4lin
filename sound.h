#ifndef SOUND_H
#define SOUND_H

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

int LoadSamples();

int PlaySoundChannel(enum sounds sound, int channel);

int PlaySound(enum sounds sound);

int init_sound();

void cont_sound_play(enum sounds sound);

void cont_sound_stop(enum sounds sound);

#endif
