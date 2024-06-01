#include "raylib.h"
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "circuitbreaker.h"

typedef struct {
    const uint16 screen_width;
    const uint16 screen_height;
    bool game_over;
    Font font;
} Game;

global_variable Game game = {
    .screen_width = 800,
    .screen_height = 460,
    .game_over = true,
};

void game_init(void) {
    time_t t;
    srand((unsigned) time(&t));
    game.font = LoadFont("resources/fonts/mecha.png");

    board_init();
}

void game_update(void) {
    if (!game.game_over && true) {
        board_update();
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

    uint16_t pos_x = game.screen_width / 2 - (HALF_CELL_SIZE * BOARD_WIDTH);
    uint16_t pos_y = game.screen_height / 2 - (HALF_CELL_SIZE * (BOARD_HEIGHT + 1));

    board_draw(pos_x, pos_y);

    EndDrawing();
}

void game_update_and_draw(void) {
    game_update();
    game_draw();
}

int main(int argc, char *argv[]) {
    InitWindow(game.screen_width, game.screen_height, "Circuit Breaker");

    game_init();

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        game_update_and_draw();
    }

    CloseWindow();
}
