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
} BoardPosition;

typedef struct {
    int x, y;
    TileTypes tile_type;
} BoardTile;

BoardTile board[BOARD_HEIGHT][BOARD_WIDTH] = {0};

int find_hole(BoardPosition positions[]);

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

typedef struct {
    int from;
    int to;
    int displacement;
    bool falling;
} ColumnFalling;

ColumnFalling colums_falling[BOARD_WIDTH] = { 0 };

void update_game(void) {
    static bool running_gravity = false;
    static int holes = 0;
    static BoardPosition positions[BOARD_HEIGHT * BOARD_WIDTH] = { 0 };

    if (!game_over) {
        if (!running_gravity) {
            holes = find_hole(positions);
            printf("Holes found %d\n", holes);
            if (holes > 0) {
                running_gravity = true;
            }
        } else {
            for (int x = 0; x < holes; x++) {
                printf("Hole at %d %d\n", positions[x].x, positions[x].y);
                colums_falling[positions[x].x].from = positions[x].y;
                colums_falling[positions[x].x].displacement = 0;
                colums_falling[positions[x].x].falling = true;
            }
            holes = 0;
            for (int x = 0; x < BOARD_WIDTH; x++) {
                if (colums_falling[x].falling == true) {
                    colums_falling[x].displacement += 1;
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

int find_hole(BoardPosition positions[]) {
    bool columns_found[BOARD_WIDTH] = { false };
    int count = 0;
    for (int y = 1; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board[x][y].tile_type == EMPTY && columns_found[x] == false) {
                columns_found[x] = true;
                positions[count].x = x;
                positions[count].y = y;
                count++;
            }
        }
    }
    return count;
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
