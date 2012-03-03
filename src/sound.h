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

int sound_play_on_channel(enum sounds sound, int channel);

int sound_play(enum sounds sound);

int sound_init(int music);

void sound_cont_play(enum sounds sound);

void sound_cont_stop(enum sounds sound, int halt);

#endif
