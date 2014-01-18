#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <SDL.h>

void input_field_activate();
void input_field_deactivate();
int input_field_is_active();
int input_field_key_event(SDL_Keysym key, int type);
int input_field_add_text(char *text);

#endif
