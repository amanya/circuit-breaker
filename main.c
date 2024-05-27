#include "raylib.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

static const uint16_t screen_width = 800;
static const uint16_t screen_height = 450;

static bool game_over = false;
static bool rotating = false;

#define BOARD_WIDTH 4
#define BOARD_HEIGHT 5
#define CELL_SIZE 64
#define HALF_CELL_SIZE 32

static Font font;

static Texture2D tile_attack;
static Texture2D tile_action;
static Texture2D tile_utility;
static Texture2D arrow;

static int cursor_row = 1;

typedef enum {
    EMPTY,
    ATTACK,
    ACTION,
    UTILITY,
    NUM_TILE_TYPES,
} TileType;

typedef struct {
    int x, y;
    int row_dest; 
    float speed;
    TileType tile_type;
    bool falling;
} Tile;

Tile board[BOARD_HEIGHT][BOARD_WIDTH] = {0};

bool empty_tiles_in_board() {
    for (int y = 1; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board[y][x].tile_type == EMPTY) {
                return true;
            }
        }
    }
    return false;
}

bool tiles_falling() {
    for (int y = 1; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board[y][x].falling == true) {
                return true;
            }
        }
    }
    return false;
}

typedef struct {
    int x, y;
} BoardPos;

typedef struct {
    BoardPos start;
    BoardPos end;
} BoardRange;

int find_horiz_matches(BoardRange *ranges) {
    int ranges_cnt = 0;
    for (int y = 1; y < BOARD_HEIGHT; y++) {
        TileType type = board[y][0].tile_type;
        ranges[ranges_cnt].start = (BoardPos){0, y};
        int tiles_matching = 1;
        for (int x = 1; x < BOARD_WIDTH; x++) {
            if (board[y][x].tile_type == type) {
                tiles_matching++;
                ranges[ranges_cnt].end = (BoardPos){x, y};
            } else {
                if (tiles_matching >= 3) {
                    ranges_cnt++;
                    ranges[ranges_cnt].end = (BoardPos){x, y};
                }
                type = board[y][x].tile_type;
                tiles_matching = 1;
                ranges[ranges_cnt].start = (BoardPos){x, y};
            }
        }
        if (tiles_matching >= 3) {
            ranges_cnt++;
        }
    }
    return ranges_cnt;
}

int find_vert_matches(BoardRange *ranges) {
    int ranges_cnt = 0;
    for (int x = 0; x < BOARD_WIDTH; x++) {
        TileType type = board[1][x].tile_type;
        ranges[ranges_cnt].start = (BoardPos){x, 1};
        int tiles_matching = 1;
        for (int y = 2; y < BOARD_HEIGHT; y++) {
            if (board[y][x].tile_type == type) {
                tiles_matching++;
                ranges[ranges_cnt].end = (BoardPos){x, y};
            } else {
                if (tiles_matching >= 3) {
                    ranges_cnt++;
                    ranges[ranges_cnt].end = (BoardPos){x, y};
                }
                type = board[y][x].tile_type;
                tiles_matching = 1;
                ranges[ranges_cnt].start = (BoardPos){x, y};
            }
        }
        if (tiles_matching >= 3) {
            ranges_cnt++;
        }
    }
    return ranges_cnt;
}

void delete_horiz_range(BoardRange range) {
    for (int x = range.start.x; x <= range.end.x; x++) {
        board[range.start.y][x].x = 0;
        board[range.start.y][x].y = 0;
        board[range.start.y][x].tile_type = EMPTY;
        board[range.start.y][x].falling = false;
        board[range.start.y][x].speed = 0;
        board[range.start.y][x].row_dest = 0;
    }
}

void delete_vert_range(BoardRange range) {
    for (int y = range.start.y; y <= range.end.y; y++) {
        board[y][range.start.x].x = 0;
        board[y][range.start.x].y = 0;
        board[y][range.start.x].tile_type = EMPTY;
        board[y][range.start.x].falling = false;
        board[y][range.start.x].speed = 0;
        board[y][range.start.x].row_dest = 0;
    }
}

void rotate_row_left(int row) {
    Tile temp = board[row][0];
    int n = 1;
    for (; n < BOARD_WIDTH; n++) {
        board[row][n - 1].tile_type = board[row][n].tile_type;
    }
    board[row][n - 1].tile_type = temp.tile_type;
}

void rotate_row_right(int row) {
    Tile temp = board[row][BOARD_WIDTH - 1];
    int n = BOARD_WIDTH - 2;
    for (; n >= 0; n--) {
        board[row][n + 1].tile_type = board[row][n].tile_type;
    }
    board[row][0].tile_type = temp.tile_type;
}

void init_game(void) {
    time_t t;
    srand((unsigned) time(&t));
    font = LoadFont("resources/fonts/mecha.png");
    tile_attack = LoadTexture("resources/tile_attack.png");
    tile_action = LoadTexture("resources/tile_action.png");
    tile_utility = LoadTexture("resources/tile_utility.png");
    arrow = LoadTexture("resources/arrow.png");

    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            TileType tile_type = y == 0 ? EMPTY : rand() % NUM_TILE_TYPES;
            board[y][x] = (Tile){ .x = x * CELL_SIZE, .y = y * CELL_SIZE, .tile_type = tile_type, .falling = false};
        }
    }
}

void update_game(void) {
    if (!game_over && true) {
        if (IsKeyPressed(KEY_UP)) {
            if (cursor_row > 1) {
                cursor_row--;
            }
        }
        if (IsKeyPressed(KEY_DOWN)) {
            if (cursor_row < BOARD_HEIGHT - 1) {
                cursor_row++;
            }
        }
        if (IsKeyPressed(KEY_LEFT)) {
            rotate_row_left(cursor_row);
        }
        if (IsKeyPressed(KEY_RIGHT)) {
            rotate_row_right(cursor_row);
        }
        for (int y = BOARD_HEIGHT - 2; y >= 0; y--) {
            for (int x = 0; x < BOARD_WIDTH; x++) {
                if (board[y][x].tile_type == EMPTY) {
                    continue;
                }
                if (board[y + 1][x].tile_type == EMPTY && board[y][x].falling == false) {
                    board[y][x].falling = true;
                    board[y][x].speed = 1;
                    board[y][x].row_dest = (y + 1) * CELL_SIZE;
                    if (y > 0) {
                        for (int n = y - 1; n >= 0; n--) {
                            if (board[n][x].tile_type != EMPTY) {
                                board[n][x].falling = true;
                                board[n][x].speed = 1;
                                board[n][x].row_dest = (n + 1) * CELL_SIZE;
                            }
                        }
                    }
                }
                if (board[y][x].falling == true) {
                    board[y][x].y += board[y][x].speed;
                    board[y][x].speed *= 1.4;
                    if (board[y][x].y >= board[y][x].row_dest) {
                        board[y][x].falling = false;
                        board[y][x].y = board[y][x].row_dest;
                        board[y][x].row_dest = 0;
                        board[y][x].speed = 0;
                        board[y + 1][x] = board[y][x];
                        board[y][x].tile_type = EMPTY;
                    }
                }
            }
        }
        if (!tiles_falling() && true) {
            // Add tiles
            for (int n = 0; n < BOARD_WIDTH; n++) {
                if (board[0][n].tile_type == EMPTY && board[1][n].tile_type == EMPTY) {
                    board[0][n].tile_type = rand() % (NUM_TILE_TYPES - 1) + 1;
                    board[0][n].x = n * CELL_SIZE;
                    board[0][n].y = 0;
                    board[0][n].speed = 0;
                    board[0][n].row_dest = 0;
                    board[0][n].falling = false;
                }
            }
            // Destroy tiles
            if (!empty_tiles_in_board() && true) {
                BoardRange horiz_ranges[BOARD_HEIGHT] = {0};
                int num_horiz_ranges = find_horiz_matches(horiz_ranges);
                for (int n = 0; n < num_horiz_ranges; n++) {
                    delete_horiz_range(horiz_ranges[n]);
                }
            }
            if (!empty_tiles_in_board() && true) {
                BoardRange vert_ranges[BOARD_WIDTH] = {0};
                int num_vert_ranges = find_vert_matches(vert_ranges);
                for (int n = 0; n < num_vert_ranges; n++) {
                    delete_vert_range(vert_ranges[n]);
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

    uint16_t pos_x = screen_width / 2 - (HALF_CELL_SIZE * BOARD_WIDTH);
    uint16_t pos_y = screen_height / 2 - (HALF_CELL_SIZE * (BOARD_HEIGHT + 1));

    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            Texture2D *tile = NULL;
            if (board[y][x].tile_type != EMPTY) {
                switch (board[y][x].tile_type) {
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
                DrawTexture(*tile, pos_x + board[y][x].x, pos_y + board[y][x].y, WHITE);
                const char *text = TextFormat("%d|%d\n(%d,%d)\n(%d,%d)", board[y][x].tile_type, board[y][x].falling, x, y, board[y][x].x, board[y][x].y);
                Vector2 text_size = MeasureTextEx(font, text, font.baseSize, 2);
                DrawTextEx(font, text, (Vector2){pos_x + board[y][x].x + HALF_CELL_SIZE - (text_size.x / 2), pos_y + board[y][x].y + HALF_CELL_SIZE - (text_size.y / 2)}, font.baseSize, 2.0, WHITE);
            }
        }
    }

    DrawTextureEx(arrow, (Vector2){pos_x, pos_y + CELL_SIZE * (cursor_row + 1)}, 180, 1, WHITE);
    DrawTexture(arrow, pos_x + CELL_SIZE * BOARD_WIDTH, pos_y + CELL_SIZE * cursor_row, WHITE);

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
