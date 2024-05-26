#include "raylib.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

static const uint16_t screen_width = 800;
static const uint16_t screen_height = 450;

static bool game_over = false;

#define BOARD_WIDTH 4
#define BOARD_HEIGHT 5
#define CELL_SIZE 64
#define HALF_CELL_SIZE 32

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
    int dest; 
    float speed;
    TileType tile_type;
    bool falling;
} Tile;

Tile *board[BOARD_HEIGHT][BOARD_WIDTH] = {0};
Tile tiles[BOARD_HEIGHT * BOARD_WIDTH] = {0};

int find_empty_tile() {
    for (int i = 0; i < BOARD_HEIGHT * BOARD_WIDTH; i++) {
        printf("%d(%d,%d)|", tiles[i].tile_type, tiles[i].x, tiles[i].y);
        if (tiles[i].tile_type == EMPTY) {
            return i;
        }
    }
    return -1;
}

bool tiles_falling() {
    for (int i = 0; i < BOARD_HEIGHT * BOARD_WIDTH; i++) {
        if (tiles[i].falling == true) {
            return true;
        }
    }
    return false;
}

void init_game(void) {
    time_t t;
    srand((unsigned) time(&t));
    font = LoadFont("resources/fonts/mecha.png");
    tile_attack = LoadTexture("resources/tile_attack.png");
    tile_action = LoadTexture("resources/tile_action.png");
    tile_utility = LoadTexture("resources/tile_utility.png");

    int tile_num = 0;
    for (int y = 1; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            TileType tile_type = rand() % NUM_TILE_TYPES;
            if (tile_type != EMPTY) {
                tiles[tile_num] = (Tile){ .x = x * CELL_SIZE, .y = y * CELL_SIZE, .tile_type = tile_type, .falling = false};
                board[y][x] = &tiles[tile_num];
                tile_num++;
            }
        }
    }
}

void update_game(void) {
    if (!game_over && true) {
        for (int y = BOARD_HEIGHT - 2; y >= 0; y--) {
            for (int x = 0; x < BOARD_WIDTH; x++) {
                if (board[y][x] == NULL) {
                    continue;
                }
                if (board[y + 1][x] == NULL && board[y][x]->falling == false) {
                    board[y][x]->falling = true;
                    board[y][x]->speed = 1;
                    board[y][x]->dest = (y + 1) * CELL_SIZE;
                    if (y > 0) {
                        for (int n = y - 1; n >= 0; n--) {
                            if (board[n][x] != NULL) {
                                board[n][x]->falling = true;
                                board[n][x]->speed = 1;
                                board[n][x]->dest = (n + 1) * CELL_SIZE;
                            }
                        }
                    }
                }
                if (board[y][x]->falling == true) {
                    board[y][x]->y += board[y][x]->speed;
                    board[y][x]->speed *= 1.2;
                    if (board[y][x]->y >= board[y][x]->dest) {
                        board[y][x]->falling = false;
                        board[y][x]->y = board[y][x]->dest;
                        board[y][x]->dest = 0;
                        board[y][x]->speed = 0;
                        board[y + 1][x] = board[y][x];
                        board[y][x] = NULL;
                    }
                }
            }
        }
        if (!tiles_falling()) {
            // Add tiles
            for (int n = 0; n < BOARD_WIDTH; n++) {
                if (board[1][n] == NULL) {
                    int p = find_empty_tile();
                    printf("p: %d\n", p);
                    if (p >= 0) {
                        tiles[p].tile_type = rand() % (NUM_TILE_TYPES - 1) + 1;
                        tiles[p].x = n * CELL_SIZE;
                        tiles[p].y = 0;
                        tiles[p].speed = 0;
                        tiles[p].dest = 0;
                        tiles[p].falling = false;
                        board[0][n] = &tiles[p];
                    }
                }
            }
            // Destroy tiles
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

    uint16_t pos_x = screen_width / 2 - (HALF_CELL_SIZE * BOARD_WIDTH);
    uint16_t pos_y = screen_height / 2 - (HALF_CELL_SIZE * BOARD_HEIGHT);

    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            Texture2D *tile = NULL;
            if (board[y][x] != NULL) {
                switch (board[y][x]->tile_type) {
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
                DrawTexture(*tile, pos_x + board[y][x]->x, pos_y + board[y][x]->y, WHITE);
                const char *text = TextFormat("%d|%d\n(%d,%d)\n(%d,%d)", board[y][x]->tile_type, board[y][x]->falling, x, y, board[y][x]->x, board[y][x]->y);
                Vector2 text_size = MeasureTextEx(font, text, font.baseSize, 2);
                DrawTextEx(font, text, (Vector2){pos_x + board[y][x]->x + HALF_CELL_SIZE - (text_size.x / 2), pos_y + board[y][x]->y + HALF_CELL_SIZE - (text_size.y / 2)}, font.baseSize, 2.0, WHITE);
            } else {
                const char *text = TextFormat("NULL\n(%d,%d)", x, y);
                Vector2 text_size = MeasureTextEx(font, text, font.baseSize, 2);
                DrawTextEx(font, text, (Vector2){pos_x + x * CELL_SIZE + HALF_CELL_SIZE - (text_size.x / 2), pos_y + y * CELL_SIZE + HALF_CELL_SIZE - (text_size.y / 2)}, font.baseSize, 2.0, WHITE);
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
