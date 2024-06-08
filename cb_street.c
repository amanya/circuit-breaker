#include <stdio.h>
#include "raylib.h"
#include "circuitbreaker.h"

typedef struct {
    Texture2D tex_background;
    Texture2D tex_midground;
    Texture2D tex_foreground;
    Texture2D tex_hero;
    Vector2 background_pos;
    Vector2 midground_pos;
    Vector2 foreground_pos;
    Vector2 hero_pos;
    uint8 hero_frame;
    real64 time;
    real64 last_time;
} Street;

Street street = {0};

void street_init() {
    street.tex_background = LoadTexture("resources/cyberpunk_street_background.png");
    street.tex_midground = LoadTexture("resources/cyberpunk_street_midground.png");
    street.tex_foreground = LoadTexture("resources/cyberpunk_street_foreground.png");
    street.tex_hero = LoadTexture("resources/Cyborg_run.png");
}

void street_update(real64 elapsed_time) {
    street.time += elapsed_time;
    street.background_pos.x += 5 * elapsed_time;
    street.midground_pos.x += 20 * elapsed_time;
    street.foreground_pos.x += 40 * elapsed_time;
    if (street.time > 0.2) {
        street.time = 0;
        street.hero_frame++;
        if (street.hero_frame > 5) {
            street.hero_frame = 0;
        }
    }
}

void hero_draw(uint16 pos_x, uint16 pos_y) {
    Rectangle source = {
        .x = 48 * street.hero_frame,
        .y = 0,
        .width = 48,
        .height = 48,
    };
    Rectangle dest = {
        .x = pos_x,
        .y = pos_y,
        .width = 48 * 2,
        .height = 48 * 2,
    };
    DrawTexturePro(street.tex_hero, source, dest, (Vector2){0}, 0.0, WHITE);
}

void street_draw(uint16 pos_x, uint16 pos_y) {
    // 512 x 192
    // 704 x 192
    Rectangle source = {
        .x = 0,
        .y = 0,
        .width = 128,
        .height = 192,
    };
    Rectangle dest = {
        .x = pos_x,
        .y = pos_y,
        .width = 128 * 2,
        .height = 192 * 2,
    };
    Vector2 origin = { 0 };
    DrawTexturePro(street.tex_background,
                   (Rectangle){street.background_pos.x, street.background_pos.y, 128, 192},
                   dest, origin, 0.0, WHITE); 
    DrawTexturePro(street.tex_midground, 
                   (Rectangle){street.midground_pos.x, street.midground_pos.y, 128, 192},
                   dest, origin, 0.0, WHITE); 
    DrawTexturePro(street.tex_foreground,
                   (Rectangle){street.foreground_pos.x, street.foreground_pos.y, 128, 192},
                   dest, origin, 0.0, WHITE); 

    hero_draw(pos_x, pos_y + 280);
}
