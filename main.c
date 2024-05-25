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

static Font font;

static Texture2D tile_attack;
static Texture2D tile_action;
static Texture2D tile_utility;

typedef enum {
    EMPTY,
    ATTACK,
    ACTION,
    UTILITY,
    NUM_TILE_TYPES,
} TileType;

typedef struct {
    int x, y;
    TileType tile_type;
    bool falling;
} Tile;

Tile *board[BOARD_HEIGHT][BOARD_WIDTH] = {0};
Tile tiles[BOARD_HEIGHT * BOARD_WIDTH] = { 0 };

void init_game(void) {
    time_t t;
    srand((unsigned) time(&t));
    font = LoadFont("resources/fonts/mecha.png");
    tile_attack = LoadTexture("resources/tile_attack.png");
    tile_action = LoadTexture("resources/tile_action.png");
    tile_utility = LoadTexture("resources/tile_utility.png");
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            TileType tile_type = rand() % NUM_TILE_TYPES;
            if (tile_type != EMPTY) {
                tiles[y * BOARD_WIDTH + x] = (Tile){ .x = x * CELL_SIZE, .y = y * CELL_SIZE, .tile_type = tile_type, .falling = false};
                board[x][y] = &tiles[y * BOARD_WIDTH + x];
            }
        }
    }
}

void update_game(void) {
    if (!game_over && true) {
        //for (int y = 0; y < BOARD_HEIGHT - 1; y++) {
        for (int y = BOARD_HEIGHT - 2; y >= 0; y--) {
            for (int x = 0; x < BOARD_WIDTH; x++) {
                if (board[x][y] == NULL) {
                    continue;
                }
                if (board[x][y + 1] == NULL && board[x][y]->falling == false) {
                    board[x][y]->falling = true;
                    if (y > 0) {
                        for (int n = y - 1; n >= 0; n--) {
                            if (board[x][n] != NULL) {
                                board[x][n]->falling = true;
                            }
                        }
                    }
                }
                if (board[x][y]->falling == true) {
                    board[x][y]->y++;
                    if (board[x][y]->y % CELL_SIZE == 0) {
                        board[x][y]->falling = false;
                        board[x][y + 1] = board[x][y];
                        board[x][y] = NULL;
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
            if (board[x][y] != NULL) {
                switch (board[x][y]->tile_type) {
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
            }
            if (tile) {
                DrawTexture(*tile, pos_x + board[x][y]->x, pos_y + board[x][y]->y, WHITE);
                const char *text = TextFormat("%d|%d\n(%d,%d)\n(%d,%d)", board[x][y]->tile_type, board[x][y]->falling, x, y, board[x][y]->x, board[x][y]->y);
                Vector2 text_size = MeasureTextEx(font, text, font.baseSize, 2);
                DrawTextEx(font, text, (Vector2){pos_x + board[x][y]->x + 32 - (text_size.x / 2), pos_y + board[x][y]->y + 32 - (text_size.y / 2)}, font.baseSize, 2.0, WHITE);
            } else {
                const char *text = TextFormat("NULL\n(%d,%d)", x, y);
                Vector2 text_size = MeasureTextEx(font, text, font.baseSize, 2);
                DrawTextEx(font, text, (Vector2){pos_x + x * CELL_SIZE + 32 - (text_size.x / 2), pos_y + y * CELL_SIZE + 32 - (text_size.y / 2)}, font.baseSize, 2.0, WHITE);
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
