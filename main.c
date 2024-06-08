#include "raylib.h"
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "circuitbreaker.h"

typedef struct {
    const uint16 screen_width;
    const uint16 screen_height;
    real64 last_time;
    real64 elapsed_time;
    real64 current_time;
    real64 wait_time;
    real64 delta_time;
    Font font;
    bool game_over;
} Game;

global_variable Game game = {
    .screen_width = 1280,
    .screen_height = 720,
    .game_over = false,
};

void game_init(void) {
    time_t t;
    srand((unsigned) time(&t));
    game.font = LoadFont("resources/fonts/mecha.png");

    board_init();
    street_init();
}

void game_update(void) {
    if (!game.game_over && true) {
        board_update(game.delta_time);
        street_update(game.delta_time);
    } else {
        if (IsKeyPressed(KEY_ENTER)) {
            game_init();
            game.game_over = false;
        }
    }
}

void game_draw(void) {
    BeginDrawing();
    ClearBackground(BLACK);

    uint16 pos_x = game.screen_width / 2 - (HALF_CELL_SIZE * BOARD_WIDTH);
    uint16 pos_y = ((game.screen_height / 8) * 6) - (HALF_CELL_SIZE * (BOARD_HEIGHT + 1));

    board_draw(pos_x, pos_y);

    pos_y = 16;
    street_draw(pos_x, pos_y);

    EndDrawing();
}

void game_update_and_draw(void) {
    game_update();
    game_draw();
}

int main(int argc, char *argv[]) {
    InitWindow(game.screen_width, game.screen_height, "Circuit Breaker");

    game_init();

    real64 target_fps = 60;

    game.last_time = GetTime();
    while (!WindowShouldClose()) {
        game_update_and_draw();

        game.current_time = GetTime();
        game.elapsed_time = game.current_time - game.last_time;

        if (target_fps > 0.0) {
            game.wait_time = (1.0f/(real64)target_fps) - game.elapsed_time;
            if (game.wait_time > 0.0) {
                WaitTime(game.wait_time);
                game.current_time = GetTime();
                game.delta_time = (real64)(game.current_time - game.last_time);
            }
        } else {
            game.delta_time = game.elapsed_time;
        }

        game.last_time = game.current_time;

    }

    CloseWindow();
}
