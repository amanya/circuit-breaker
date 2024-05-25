#include "raylib.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

static const uint16_t screen_width = 800;
static const uint16_t screen_height = 450;

static bool game_over = false;

#define BOARD_WIDTH 4
#define BOARD_HEIGHT 4
#define CELL_SIZE 64

static Texture2D tile_attack;
static Texture2D tile_action;
static Texture2D tile_utility;

typedef enum {
    EMPTY,
    ATTACK,
    ACTION,
    UTILITY,
    NUM_TILE_TYPES,
} TileTypes;

typedef struct {
    int x, y;
    TileTypes tile_type;
    bool falling;
} BoardTile;

BoardTile board[BOARD_HEIGHT][BOARD_WIDTH] = {0};

void init_game(void) {
    tile_attack = LoadTexture("resources/tile_attack.png");
    tile_action = LoadTexture("resources/tile_action.png");
    tile_utility = LoadTexture("resources/tile_utility.png");
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            board[x][y].x = x * CELL_SIZE; 
            board[x][y].y = y * CELL_SIZE; 
            board[x][y].tile_type = rand() % NUM_TILE_TYPES;
        }
    }
}

void update_game(void) {
    if (!game_over && false) {
        for (int y = 0; y < BOARD_HEIGHT - 1; y++) {
            for (int x = 0; x < BOARD_WIDTH; x++) {
                if (board[x][y].tile_type != EMPTY && board[x][y + 1].tile_type == EMPTY && board[x][y].falling == false) {
                    board[x][y].falling = true;
                    board[x][y + 1].tile_type = board[x][y].tile_type;
                    board[x][y].tile_type = EMPTY;
                }
                if (board[x][y].falling == true) {
                    board[x][y].y++;
                    if (board[x][y].y % CELL_SIZE == 0) {
                        board[x][y].falling = false;
                    }
                }
            }
        }
    } else {
        if (IsKeyPressed(KEY_ENTER)) {
            init_game();
            game_over = false;
        }
    }
}

void draw_game(void) {
    BeginDrawing();
    ClearBackground(BLACK);

    uint16_t pos_x = screen_width / 2 - (CELL_SIZE / 2 * BOARD_WIDTH);
    uint16_t pos_y = screen_height / 2 - (CELL_SIZE / 2 * BOARD_HEIGHT);

    for (int y = 0; y < BOARD_WIDTH; y++) {
        for (int x = 0; x < BOARD_HEIGHT; x++) {
            Texture2D *tile = NULL;
            switch (board[x][y].tile_type) {
                case ATTACK:
                    tile = &tile_attack;
                    break;
                case ACTION:
                    tile = &tile_action;
                    break;
                case UTILITY:
                    tile = &tile_utility;
                    break;
                default:
                    break;
            }
            if (tile) {
                DrawTexture(*tile, pos_x + board[x][y].x, pos_y + board[x][y].y, WHITE);
            }
        }
    }

    EndDrawing();
}

void update_draw_game(void) {
    update_game();
    draw_game();
}

int main(int argc, char *argv[]) {
    InitWindow(screen_width, screen_height, "Circuit Breaker");

    init_game();

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        update_draw_game();
    }

    CloseWindow();
}
