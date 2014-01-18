#include <SDL.h>

#include "console.h"
#include "hud.h"
#include "network.h"
#include "main.h"

struct {
    char *text;
    int length;
} input_field_data;

static const int input_field_max_length = 80;

static void input_send_line(const char *input) {
    if (Network)
        network_amsg_send((char*)input);
    hud_printf("Me> %s", input);
}

void input_field_activate() {
    hud_show_input_field(1);

    if(input_field_data.text == NULL) {
        input_field_data.text = (char*)malloc(input_field_max_length+1);
        input_field_data.text[0] = '\0';
        input_field_data.length = 0;
    }

    SDL_StartTextInput();
}

void input_field_deactivate() {
    SDL_StopTextInput();
    free(input_field_data.text);
    input_field_data.text = NULL;
    hud_update_input_field("");
    hud_show_input_field(0);
}

int input_field_is_active() {
    return hud_input_field_active();
}

int input_field_key_event(SDL_Keysym key, int type) {
    if(!input_field_is_active())
        return 0;

    if(type == SDL_KEYUP) {
        return 0;
    }

    if(key.scancode == SDL_SCANCODE_ESCAPE) {
        input_field_deactivate();
    } else if(key.scancode == SDL_SCANCODE_BACKSPACE && input_field_data.length > 0) {
        /* FIXME: utf-8 */
        input_field_data.text[--input_field_data.length] = '\0';
        hud_update_input_field(input_field_data.text);
    } else if(key.scancode == SDL_SCANCODE_RETURN) {
        if(input_field_data.length > 0)
            input_send_line(input_field_data.text);

        input_field_deactivate();
    }

    return 1; /* We handled the key (or should ignore it), stop processing it. */
}

int input_field_add_text(char *text) {
    if(!input_field_is_active()) {
        printf("BUG: input_field_add_text called when input field inactive.\n");
        return 0;
    }

    size_t len = strlen(text);
    if(input_field_data.length + len < input_field_max_length) {
        strcat(input_field_data.text, text);
        input_field_data.length += len;
        hud_update_input_field(input_field_data.text);
        return 1;
    }

    return 0;
}
