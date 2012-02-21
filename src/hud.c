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

#include <stdio.h>
#include <stdarg.h>

#include <GL/gl.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "hud.h"

#define HUD_IMAGE_W  1024
#define HUD_IMAGE_H  1024
#define HUD_TEXT_SIZE 30

#define min(a,b) (((a) > (b)) ? (b) : (a))

struct chat_line {
    struct chat_line *next, *prev;
    SDL_Surface *surface;
    Uint32 timestamp;
};

struct hud_data_struct {
    SDL_Surface *surface;
    SDL_Color fg_color;
    TTF_Font *font;

    struct chat_line *chatlines;

    SDL_Surface *dmg_surface;
    SDL_Surface *score_surface;
    /* TODO: Multiplayer */

    int show_input_field;
    SDL_Surface *input_surface;

    unsigned int texture;
};
static struct hud_data_struct *hud_data;

static void chat_line_add(SDL_Surface *line) {
    struct chat_line *new_line = malloc(sizeof(struct chat_line));
    new_line->surface = line;
    new_line->timestamp = SDL_GetTicks();
    new_line->prev = NULL;
    new_line->next = hud_data->chatlines;

    if(new_line->next) {
        new_line->next->prev = new_line;
    }

    hud_data->chatlines = new_line;
}

static void chat_line_del_trailing(struct chat_line *cl) {
    if(!cl)
        return;
    chat_line_del_trailing(cl->next);
    SDL_FreeSurface(cl->surface);
    free(cl);
}

static SDL_Color default_fg_color = {255, 255, 0};

static void hud_clear() {
    SDL_FillRect(hud_data->surface, NULL, 0);
}

int hud_init() {
    if(TTF_Init() == -1) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        return 0;
    }

    hud_data = malloc(sizeof(struct hud_data_struct));
    hud_data->fg_color = default_fg_color;

    hud_data->surface = SDL_CreateRGBSurface(0, HUD_IMAGE_W, HUD_IMAGE_H, 32,
                                            0xFF, 0xFF00, 0xFF0000, 0xFF000000);
    if(!hud_data->surface) {
        printf("Failed to create surface for hud\n");
        return 0;
    }

    hud_data->font = TTF_OpenFont("data/font/Vera.ttf", HUD_TEXT_SIZE);
    if(!hud_data->font) {
        printf("Failed to open font: %s\n", TTF_GetError());
        return 0;
    }

    glGenTextures(1, &hud_data->texture);
    if(!hud_data->texture) {
        printf("Failed to generate texure");
        return 0;
    }

    hud_data->show_input_field = 0;
    hud_data->chatlines = NULL;
    hud_data->score_surface = NULL;
    hud_data->dmg_surface = NULL;
    hud_data->input_surface = NULL;
    hud_show_input_field(0);

    hud_clear();

    return 1;
}

static void finalize_hud() {

    /* chatt/meddelanden */
    static int chat_min_x = 10;
    static int chat_min_y = 10;
    static int chat_max_y = HUD_IMAGE_H - 100;
    static int chat_fade_delay = 10000;
    SDL_Rect hud_rect = {chat_min_x, chat_max_y, 0, 0}; /* Botten */

    struct chat_line *cl = hud_data->chatlines;
    Uint32 time_now = SDL_GetTicks();
    for(; cl; cl = cl->next) {
        if(hud_rect.y - cl->surface->h < chat_min_y) {

            /* Dumpa det som inte får plats */
            if(cl->prev == NULL)
                hud_data->chatlines = NULL;
            else
                cl->prev->next = NULL;

            chat_line_del_trailing(cl);
            break;
        }

        if(!hud_data->show_input_field && (time_now - cl->timestamp >= chat_fade_delay)) {
            /* We need to go through the rest anyway to keep the list clean */
            continue;
        }

        hud_rect.y -= cl->surface->h;
        hud_rect.w = min(HUD_IMAGE_W-hud_rect.x, cl->surface->w);
        hud_rect.h = cl->surface->h;

        if( SDL_BlitSurface(cl->surface, NULL, hud_data->surface, &hud_rect) == -1 ) {
            printf("SDL_BlitSurface failed.");
        }
    }

    /* Input field */
    if(hud_data->show_input_field && hud_data->input_surface) {
        hud_rect.x = chat_min_x;
        hud_rect.y = chat_max_y + 5;
        hud_rect.w = hud_data->input_surface->w;
        hud_rect.h = hud_data->input_surface->h;

        if( SDL_BlitSurface(hud_data->input_surface, NULL, hud_data->surface, &hud_rect) == -1 ) {
            printf("SDL_BlitSurface failed.");
        }
    }

    /* Status */
    int y_pos = HUD_IMAGE_H;
    if(hud_data->score_surface) {

        hud_rect.w = hud_data->score_surface->w;
        hud_rect.h = hud_data->score_surface->h;

        y_pos -= hud_rect.h;
        hud_rect.x = HUD_IMAGE_W - hud_rect.w;
        hud_rect.y = y_pos;

        if( SDL_BlitSurface(hud_data->score_surface, NULL, hud_data->surface, &hud_rect) == -1 ) {
            printf("SDL_BlitSurface failed.");
        }
    }
    if(hud_data->dmg_surface) {
        hud_rect.w = hud_data->dmg_surface->w;
        hud_rect.h = hud_data->dmg_surface->h;

        y_pos -= hud_rect.h;
        hud_rect.x = HUD_IMAGE_W - hud_rect.w;
        hud_rect.y = y_pos;

        if( SDL_BlitSurface(hud_data->dmg_surface, NULL, hud_data->surface, &hud_rect) == -1 ) {
            printf("SDL_BlitSurface failed.");
        }
    }

}

void hud_render() {
    finalize_hud();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, hud_data->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, 4,
                 hud_data->surface->w, hud_data->surface->h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, hud_data->surface->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, hud_data->texture);
    glColor3f(1.0f, 1.0f, 1.0f);

    static float hud_min_w = -1, hud_max_w = 1;
    static float hud_min_h = -1, hud_max_h = 1;
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(hud_min_w,hud_max_h,0.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(hud_max_w,hud_max_h,0.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(hud_max_w,hud_min_h,0.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(hud_min_w,hud_min_h,0.0f);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    hud_clear();
}

static SDL_Surface *vprintf_surface(const char *fmt, va_list args)
{
    static char buf[512];
    vsnprintf(buf, 512, fmt, args);

    SDL_Surface *text_surface = TTF_RenderText_Solid(hud_data->font, buf, hud_data->fg_color);
    if(!text_surface) {
        printf("TTF_RenderText_Solid failed: %s\n", TTF_GetError());
        return NULL;
    }

    return text_surface;
}
static SDL_Surface *printf_surface(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    SDL_Surface *text_surface = vprintf_surface(fmt, args);
    va_end(args);

    return text_surface;
}

void hud_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    SDL_Surface *text_surface = vprintf_surface(fmt, args);
    va_end(args);

    chat_line_add(text_surface);
}

void hud_set_damage(int new_damage) {
    if(hud_data->dmg_surface)
        SDL_FreeSurface(hud_data->dmg_surface);

    hud_data->dmg_surface = printf_surface("Damage: %4d", new_damage);
}

void hud_set_score(int new_score) {
    if(hud_data->score_surface) {
        SDL_FreeSurface(hud_data->score_surface);
    }

    hud_data->score_surface = printf_surface("Score: %4d", new_score);
}

void hud_show_input_field(int onoff) {
    hud_data->show_input_field = onoff;
}

int hud_input_field_active() {
    return hud_data->show_input_field;
}

void hud_update_input_field(const char *text) {
    if(hud_data->input_surface) {
        SDL_FreeSurface(hud_data->input_surface);
    }

    hud_data->input_surface = printf_surface("] %s", text);
}
