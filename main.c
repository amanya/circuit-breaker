#include "raylib.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

typedef struct {
    const uint16_t screen_width;
    const uint16_t screen_height;
    bool game_over;
    Font font;
} Game;

static Game game = {
    .screen_width = 800,
    .screen_height = 460,
    .game_over = true,
};

typedef enum {
    TILETYPE_EMPTY,
    TILETYPE_ATTACK,
    TILETYPE_ACTION,
    TILETYPE_UTILITY,
    TILETYPE_CNT,
} TileType;

#define BOARD_WIDTH 4
#define BOARD_HEIGHT 5
#define CELL_SIZE 64
#define HALF_CELL_SIZE 32

typedef struct {
    int x, y;
    int row_dest; 
    float hspeed;
    float vspeed;
    TileType tile_type;
    bool falling;
    bool hrotating;
} Tile;

typedef struct {
    Texture2D tile_textures[TILETYPE_CNT];
    Texture2D arrow_texture;
    Tile tiles[BOARD_HEIGHT][BOARD_WIDTH];
    int cursor_row;
} Board;

Board board = {
    .tile_textures = { 0 },
    .tiles = {0},
    .cursor_row = 1,
};

bool board_tiles_empty() {
    for (int y = 1; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board.tiles[y][x].tile_type == TILETYPE_EMPTY) {
                return true;
            }
        }
    }
    return false;
}

bool board_tiles_falling() {
    for (int y = 1; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board.tiles[y][x].falling == true) {
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
        TileType type = board.tiles[y][0].tile_type;
        ranges[ranges_cnt].start = (BoardPos){0, y};
        int tiles_matching = 1;
        for (int x = 1; x < BOARD_WIDTH; x++) {
            if (board.tiles[y][x].tile_type == type) {
                tiles_matching++;
                ranges[ranges_cnt].end = (BoardPos){x, y};
            } else {
                if (tiles_matching >= 3) {
                    ranges_cnt++;
                    ranges[ranges_cnt].end = (BoardPos){x, y};
                }
                type = board.tiles[y][x].tile_type;
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
        TileType type = board.tiles[1][x].tile_type;
        ranges[ranges_cnt].start = (BoardPos){x, 1};
        int tiles_matching = 1;
        for (int y = 2; y < BOARD_HEIGHT; y++) {
            if (board.tiles[y][x].tile_type == type) {
                tiles_matching++;
                ranges[ranges_cnt].end = (BoardPos){x, y};
            } else {
                if (tiles_matching >= 3) {
                    ranges_cnt++;
                    ranges[ranges_cnt].end = (BoardPos){x, y};
                }
                type = board.tiles[y][x].tile_type;
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
        board.tiles[range.start.y][x].x = 0;
        board.tiles[range.start.y][x].y = 0;
        board.tiles[range.start.y][x].tile_type = TILETYPE_EMPTY;
        board.tiles[range.start.y][x].falling = false;
        board.tiles[range.start.y][x].vspeed = 0;
        board.tiles[range.start.y][x].row_dest = 0;
    }
}

void delete_vert_range(BoardRange range) {
    for (int y = range.start.y; y <= range.end.y; y++) {
        board.tiles[y][range.start.x].x = 0;
        board.tiles[y][range.start.x].y = 0;
        board.tiles[y][range.start.x].tile_type = TILETYPE_EMPTY;
        board.tiles[y][range.start.x].falling = false;
        board.tiles[y][range.start.x].vspeed = 0;
        board.tiles[y][range.start.x].row_dest = 0;
    }
}

void rotate_row_left(int row) {
    Tile temp = board.tiles[row][0];
    int n = 1;
    for (; n < BOARD_WIDTH; n++) {
        board.tiles[row][n - 1].tile_type = board.tiles[row][n].tile_type;
    }
    board.tiles[row][n - 1].tile_type = temp.tile_type;
}

void rotate_row_right(int row) {
    Tile temp = board.tiles[row][BOARD_WIDTH - 1];
    int n = BOARD_WIDTH - 2;
    for (; n >= 0; n--) {
        board.tiles[row][n + 1].tile_type = board.tiles[row][n].tile_type;
    }
    board.tiles[row][0].tile_type = temp.tile_type;
}

int find_row_dest(int x, int start) {
    int dest = start;
    for (; dest < BOARD_HEIGHT - 1; dest++) {
        if (board.tiles[dest + 1][x].tile_type != TILETYPE_EMPTY) {
            break;
        }
    }
    return dest;
}

void init_game(void) {
    time_t t;
    srand((unsigned) time(&t));
    game.font = LoadFont("resources/fonts/mecha.png");
    board.tile_textures[TILETYPE_ATTACK] = LoadTexture("resources/tile_attack.png");
    board.tile_textures[TILETYPE_ACTION] = LoadTexture("resources/tile_action.png");
    board.tile_textures[TILETYPE_UTILITY] = LoadTexture("resources/tile_utility.png");
    board.arrow_texture = LoadTexture("resources/arrow.png");

    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            TileType tile_type = y == 0 ? TILETYPE_EMPTY : rand() % TILETYPE_CNT;
            board.tiles[y][x] = (Tile){ .x = x * CELL_SIZE, .y = y * CELL_SIZE, .tile_type = tile_type, .falling = false};
        }
    }
}

void update_game(void) {
    if (!game.game_over && true) {
        if (IsKeyPressed(KEY_UP)) {
            if (board.cursor_row > 1) {
                board.cursor_row--;
            }
        }
        if (IsKeyPressed(KEY_DOWN)) {
            if (board.cursor_row < BOARD_HEIGHT - 1) {
                board.cursor_row++;
            }
        }
        if (IsKeyPressed(KEY_LEFT)) {
            rotate_row_left(board.cursor_row);
        }
        if (IsKeyPressed(KEY_RIGHT)) {
            rotate_row_right(board.cursor_row);
        }
        for (int y = BOARD_HEIGHT - 2; y >= 0; y--) {
            for (int x = 0; x < BOARD_WIDTH; x++) {
                if (board.tiles[y][x].tile_type == TILETYPE_EMPTY) {
                    continue;
                }
                if (board.tiles[y + 1][x].tile_type == TILETYPE_EMPTY && board.tiles[y][x].falling == false) {
                    board.tiles[y][x].falling = true;
                    board.tiles[y][x].vspeed = 1;
                    board.tiles[y][x].row_dest = find_row_dest(x, y) * CELL_SIZE;
                    if (y > 0) {
                        for (int n = y - 1; n >= 0; n--) {
                            if (board.tiles[n][x].tile_type != TILETYPE_EMPTY) {
                                board.tiles[n][x].falling = true;
                                board.tiles[n][x].vspeed = 1;
                                board.tiles[n][x].row_dest = find_row_dest(x, n) * CELL_SIZE;
                            }
                        }
                    }
                }
                if (board.tiles[y][x].falling == true) {
                    board.tiles[y][x].y += board.tiles[y][x].vspeed;
                    board.tiles[y][x].vspeed *= 1.1 + ((float)rand()/(float)(RAND_MAX)) * 0.4;
                    if (board.tiles[y][x].y >= board.tiles[y][x].row_dest) {
                        board.tiles[y][x].falling = false;
                        board.tiles[y][x].y = board.tiles[y][x].row_dest;
                        board.tiles[y][x].row_dest = 0;
                        board.tiles[y][x].vspeed = 0;
                        board.tiles[y + 1][x] = board.tiles[y][x];
                        board.tiles[y][x].tile_type = TILETYPE_EMPTY;
                    }
                }
            }
        }
        if (!board_tiles_falling() && true) {
            // Add tiles
            for (int n = 0; n < BOARD_WIDTH; n++) {
                if (board.tiles[0][n].tile_type == TILETYPE_EMPTY && board.tiles[1][n].tile_type == TILETYPE_EMPTY) {
                    board.tiles[0][n].tile_type = rand() % (TILETYPE_CNT - 1) + 1;
                    board.tiles[0][n].x = n * CELL_SIZE;
                    board.tiles[0][n].y = 0;
                    board.tiles[0][n].vspeed = 0;
                    board.tiles[0][n].row_dest = 0;
                    board.tiles[0][n].falling = false;
                }
            }
            // Destroy tiles
            if (!board_tiles_empty() && true) {
                BoardRange horiz_ranges[BOARD_HEIGHT] = {0};
                int num_horiz_ranges = find_horiz_matches(horiz_ranges);
                for (int n = 0; n < num_horiz_ranges; n++) {
                    delete_horiz_range(horiz_ranges[n]);
                }
            }
            if (!board_tiles_empty() && true) {
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
            game.game_over = false;
        }
    }
}

void draw_game(void) {
    BeginDrawing();
    ClearBackground(BLACK);

    uint16_t pos_x = game.screen_width / 2 - (HALF_CELL_SIZE * BOARD_WIDTH);
    uint16_t pos_y = game.screen_height / 2 - (HALF_CELL_SIZE * (BOARD_HEIGHT + 1));

    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            Texture2D *tile = NULL;
            if (board.tiles[y][x].tile_type != TILETYPE_EMPTY) {
                tile = &board.tile_textures[board.tiles[y][x].tile_type];
                DrawTexture(*tile, pos_x + board.tiles[y][x].x, pos_y + board.tiles[y][x].y, WHITE);
                const char *text = TextFormat("%d|%d\n(%d,%d)\n(%d,%d)", board.tiles[y][x].tile_type, board.tiles[y][x].falling, x, y, board.tiles[y][x].x, board.tiles[y][x].y);
                Vector2 text_size = MeasureTextEx(game.font, text, game.font.baseSize, 2);
                DrawTextEx(game.font, text, (Vector2){pos_x + board.tiles[y][x].x + HALF_CELL_SIZE - (text_size.x / 2), pos_y + board.tiles[y][x].y + HALF_CELL_SIZE - (text_size.y / 2)}, game.font.baseSize, 2.0, WHITE);
            }
        }
    }

    DrawTextureEx(board.arrow_texture, (Vector2){pos_x, pos_y + CELL_SIZE * (board.cursor_row + 1)}, 180, 1, WHITE);
    DrawTexture(board.arrow_texture, pos_x + CELL_SIZE * BOARD_WIDTH, pos_y + CELL_SIZE * board.cursor_row, WHITE);

    EndDrawing();
}

void update_draw_game(void) {
    update_game();
    draw_game();
}

int main(int argc, char *argv[]) {
    InitWindow(game.screen_width, game.screen_height, "Circuit Breaker");

    init_game();

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        update_draw_game();
    }

    CloseWindow();
}
