#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "raylib.h"
#include "circuitbreaker.h"

typedef enum {
    BOARDSTATE_IDLE,
    BOARDSTATE_BREAKING,
    BOARDSTATE_FALLING,
    BOARDSTATE_ADDING,
    BOARDSTATE_ROTATING,
    BOARDSTATE_CNT,
} BoardState;

/*
 * IDLE -> left|right -> ROTATING
 * ROTATING -> end key press -> IDLE
 * IDLE -> empty cells -> FALLING
 * FALLING -> end falling -> IDLE
 * IDLE -> match -> BREAKING
 * BREAKING -> end breaking -> IDLE
 * IDLE -> empty cells top -> ADDING
 * ADDING -> end adding -> IDLE
*/

typedef enum {
    TILETYPE_EMPTY,
    TILETYPE_ATTACK,
    TILETYPE_ACTION,
    TILETYPE_UTILITY,
    TILETYPE_CNT,
} TileType;

typedef struct {
    real32 x, y;
    uint32 row_dest; 
    real32 vspeed;
    TileType tile_type;
    bool falling;
    bool hrotating;
} Tile;

typedef struct {
    uint32 x, y;
} BoardPos;

typedef struct {
    BoardPos start;
    BoardPos end;
} BoardRange;

typedef enum {
    ROTATING_LEFT,
    ROTATING_RIGHT,
} BoardRotationDirection;

typedef struct {
    BoardState state;
    Texture2D tile_textures[TILETYPE_CNT];
    Texture2D arrow_texture;
    Tile tiles[BOARD_HEIGHT][BOARD_WIDTH];
    uint32 cursor_row;
    uint32 horiz_ranges_cnt;
    uint32 vert_ranges_cnt;
    BoardRange horiz_ranges[BOARD_HEIGHT];
    BoardRange vert_ranges[BOARD_WIDTH];
    BoardRotationDirection direction;
    real32 hspeed;
} Board;

Board board = {
    .state = BOARDSTATE_IDLE,
    .tile_textures = { 0 },
    .tiles = {0},
    .cursor_row = 1,
    .horiz_ranges = { 0 },
    .vert_ranges = { 0 },
};

void board_rotate_row_left(int row);
void board_rotate_row_right(int row);
int board_find_row_dest(int x, int start);
bool board_tiles_falling();
bool board_tiles_empty();
bool board_first_row_tiles_empty();
void board_find_horiz_matches();
void board_find_vert_matches();
void board_delete_horiz_range(BoardRange range);
void board_delete_vert_range(BoardRange range);

void board_debug_print() {
    printf("Board state: %d\n", board.state);
    for (int y  = 0; y < BOARD_HEIGHT; y++) {
        for (int x  = 0; x < BOARD_WIDTH; x++) {
            printf("%d%c(%3f,%3f) ",
                   board.tiles[y][x].tile_type,
                   board.tiles[y][x].falling ? 'f' : ' ',
                   board.tiles[y][x].x,
                   board.tiles[y][x].y);
        }
        printf("\n");
    }
}

void board_range_print() {
    printf("ranges h:%d v:%d\n", board.horiz_ranges_cnt, board.vert_ranges_cnt);
    for (int n = 0; n < board.horiz_ranges_cnt; n++) {
        printf("hrange: (%d,%d) (%d,%d)\n",
               board.horiz_ranges[n].start.x,
               board.horiz_ranges[n].start.y,
               board.horiz_ranges[n].end.x,
               board.horiz_ranges[n].end.y);
    }
    for (int n = 0; n < board.vert_ranges_cnt; n++) {
        printf("vrange: (%d,%d) (%d,%d)\n",
               board.vert_ranges[n].start.x,
               board.vert_ranges[n].start.y,
               board.vert_ranges[n].end.x,
               board.vert_ranges[n].end.y);
    }
}

void board_init() {
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

void board_update_state() {
    if (board.state == BOARDSTATE_IDLE) {
        if (board_tiles_falling()) {
            board.state = BOARDSTATE_FALLING;
            return;
        }
        if (board.vert_ranges_cnt > 0 || board.horiz_ranges_cnt > 0) {
            board.state = BOARDSTATE_BREAKING;
            return;
        }
        if (board_first_row_tiles_empty()) {
            board.state = BOARDSTATE_ADDING;
            return;
        }
    }
    if (board.state == BOARDSTATE_FALLING) {
        if (!board_tiles_falling()) {
            board.state = BOARDSTATE_IDLE;
            return;
        }
    }
}

void board_update() {
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
        if (board.state == BOARDSTATE_IDLE) {
            board.state = BOARDSTATE_ROTATING;
            board.hspeed = 1;
            board.direction = ROTATING_LEFT;
        }
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        if (board.state == BOARDSTATE_IDLE) {
            board.state = BOARDSTATE_ROTATING;
            board.hspeed = 1;
            board.direction = ROTATING_RIGHT;
        }
    }
    if (board.state == BOARDSTATE_IDLE) {
        for (int y = BOARD_HEIGHT - 2; y >= 0; y--) {
            for (int x = 0; x < BOARD_WIDTH; x++) {
                if (board.tiles[y][x].tile_type == TILETYPE_EMPTY) {
                    continue;
                }
                if (board.tiles[y + 1][x].tile_type == TILETYPE_EMPTY && board.tiles[y][x].falling == false) {
                    board.tiles[y][x].falling = true;
                    board.tiles[y][x].vspeed = 1;
                    board.tiles[y][x].row_dest = board_find_row_dest(x, y) * CELL_SIZE;
                    if (y > 0) {
                        for (int n = y - 1; n >= 0; n--) {
                            if (board.tiles[n][x].tile_type != TILETYPE_EMPTY) {
                                board.tiles[n][x].falling = true;
                                board.tiles[n][x].vspeed = 1;
                                board.tiles[n][x].row_dest = board_find_row_dest(x, n) * CELL_SIZE;
                            }
                        }
                    }
                }
            }
        }
    }
    if (board.state == BOARDSTATE_FALLING) {
        for (int y = BOARD_HEIGHT - 2; y >= 0; y--) {
            for (int x = 0; x < BOARD_WIDTH; x++) {
                if (board.tiles[y][x].falling == true) {
                    board.tiles[y][x].y += board.tiles[y][x].vspeed;
                    board.tiles[y][x].vspeed *= 1.1 + ((real32)rand()/(real32)(RAND_MAX)) * 0.4;
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
    }
    if (board.state == BOARDSTATE_ROTATING) {
        if (board.direction == ROTATING_LEFT) {
            board.hspeed *= 1.2;
            for (int n = 0; n < BOARD_WIDTH; n++) {
                board.tiles[board.cursor_row][n].x -= board.hspeed;
            }
            if (board.tiles[board.cursor_row][1].x <= 0) {
                for (int n = 0; n < BOARD_WIDTH; n++) {
                    board.tiles[board.cursor_row][n].x = n * CELL_SIZE;
                }
                board_rotate_row_left(board.cursor_row);
                board.state = BOARDSTATE_IDLE;
            }
        }
        else if (board.direction == ROTATING_RIGHT) {
            board.hspeed *= 1.2;
            for (int n = 0; n < BOARD_WIDTH; n++) {
                board.tiles[board.cursor_row][n].x += board.hspeed;
            }
            if (board.tiles[board.cursor_row][0].x >= CELL_SIZE) {
                for (int n = 0; n < BOARD_WIDTH; n++) {
                    board.tiles[board.cursor_row][n].x = n * CELL_SIZE;
                }
                board_rotate_row_right(board.cursor_row);
                board.state = BOARDSTATE_IDLE;
            }
        }
    }
    if (board.state == BOARDSTATE_ADDING) {
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
        board.state = BOARDSTATE_IDLE;
    }
    if (board.state == BOARDSTATE_BREAKING) {
        // Destroy tiles
        for (int n = 0; n < board.horiz_ranges_cnt; n++) {
            board_delete_horiz_range(board.horiz_ranges[n]);
        }
        for (int n = 0; n < board.vert_ranges_cnt; n++) {
            board_delete_vert_range(board.vert_ranges[n]);
        }
        board.state = BOARDSTATE_IDLE;
    }
    board_debug_print();
    board_find_horiz_matches();
    board_find_vert_matches();
    board_update_state();
}

void board_draw(uint16 pos_x, uint16 pos_y) {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            Texture2D *tile = NULL;
            if (board.tiles[y][x].tile_type != TILETYPE_EMPTY) {
                tile = &board.tile_textures[board.tiles[y][x].tile_type];
                Rectangle source = { .x = 0, .y = 0, .width = CELL_SIZE, .height = CELL_SIZE };
                Vector2 position = { .x = pos_x + board.tiles[y][x].x, .y = pos_y + board.tiles[y][x].y };
                if (board.tiles[y][x].x < 0) {
                    int32 clamp = fabsf(board.tiles[y][x].x);
                    source.x = clamp;
                    source.width = CELL_SIZE - clamp;
                    position.x = pos_x;
                    Rectangle source2 = {
                        .x = 0,
                        .y = 0,
                        .width = clamp,
                        .height = CELL_SIZE
                    };
                    Vector2 position2 = {
                        .x = pos_x + board.tiles[y][BOARD_WIDTH - 1].x + CELL_SIZE,
                        .y = pos_y + board.tiles[y][x].y,
                    };
                    DrawTextureRec(*tile, source2, position2, WHITE);
                    printf("Source .x %f .width %f\n", source2.x, source2.width);
                    printf("Position .x %f .y %f\n", position2.x, position2.y);
                }
                else if (board.tiles[y][x].x > (BOARD_WIDTH - 1) * CELL_SIZE) {
                    int32 clamp = board.tiles[y][x].x - (BOARD_WIDTH - 1) * CELL_SIZE;
                    source.x = 0;
                    source.width = CELL_SIZE - clamp;
                    Rectangle source2 = {
                        .x = CELL_SIZE - clamp,
                        .y = 0,
                        .width = clamp,
                        .height = CELL_SIZE
                    };
                    Vector2 position2 = {
                        .x = pos_x,
                        .y = pos_y + board.tiles[y][x].y,
                    };
                    DrawTextureRec(*tile, source2, position2, WHITE);
                    printf("Source .x %f .width %f\n", source2.x, source2.width);
                    printf("Position .x %f .y %f\n", position2.x, position2.y);
                }
                DrawTextureRec(*tile, source, position, WHITE);
            }
        }
    }
    DrawTextureEx(board.arrow_texture, (Vector2){pos_x, pos_y + CELL_SIZE * (board.cursor_row + 1)}, 180, 1, WHITE);
    DrawTexture(board.arrow_texture, pos_x + CELL_SIZE * BOARD_WIDTH, pos_y + CELL_SIZE * board.cursor_row, WHITE);
}

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

bool board_first_row_tiles_empty() {
    for (int x = 0; x < BOARD_WIDTH; x++) {
        if (board.tiles[0][x].tile_type == TILETYPE_EMPTY && board.tiles[1][x].tile_type == TILETYPE_EMPTY) {
            return true;
        }
    }
    return false;
}

bool board_tiles_falling() {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board.tiles[y][x].falling == true) {
                return true;
            }
        }
    }
    return false;
}

void board_find_horiz_matches() {
    int ranges_cnt = 0;
    for (int y = 1; y < BOARD_HEIGHT; y++) {
        TileType type = board.tiles[y][0].tile_type;
        if (type == TILETYPE_EMPTY) {
            continue;
        }
        board.horiz_ranges[ranges_cnt].start = (BoardPos){0, y};
        int tiles_matching = 1;
        for (int x = 1; x < BOARD_WIDTH; x++) {
            if (board.tiles[y][x].tile_type == type) {
                tiles_matching++;
                board.horiz_ranges[ranges_cnt].end = (BoardPos){x, y};
            } else {
                if (tiles_matching >= 3) {
                    ranges_cnt++;
                    board.horiz_ranges[ranges_cnt].end = (BoardPos){x, y};
                }
                type = board.tiles[y][x].tile_type;
                if (type == TILETYPE_EMPTY) {
                    continue;
                }
                tiles_matching = 1;
                board.horiz_ranges[ranges_cnt].start = (BoardPos){x, y};
            }
        }
        if (tiles_matching >= 3) {
            ranges_cnt++;
        }
    }
    board.horiz_ranges_cnt = ranges_cnt;
}

void board_find_vert_matches() {
    int ranges_cnt = 0;
    for (int x = 0; x < BOARD_WIDTH; x++) {
        TileType type = board.tiles[1][x].tile_type;
        if (type == TILETYPE_EMPTY) {
            continue;
        }
        board.vert_ranges[ranges_cnt].start = (BoardPos){x, 1};
        int tiles_matching = 1;
        for (int y = 2; y < BOARD_HEIGHT; y++) {
            if (board.tiles[y][x].tile_type == type) {
                tiles_matching++;
                board.vert_ranges[ranges_cnt].end = (BoardPos){x, y};
            } else {
                if (tiles_matching >= 3) {
                    ranges_cnt++;
                    board.vert_ranges[ranges_cnt].end = (BoardPos){x, y};
                }
                type = board.tiles[y][x].tile_type;
                if (type == TILETYPE_EMPTY) {
                    continue;
                }
                tiles_matching = 1;
                board.vert_ranges[ranges_cnt].start = (BoardPos){x, y};
            }
        }
        if (tiles_matching >= 3) {
            ranges_cnt++;
        }
    }
    board.vert_ranges_cnt = ranges_cnt;
}

void board_delete_horiz_range(BoardRange range) {
    for (int x = range.start.x; x <= range.end.x; x++) {
        board.tiles[range.start.y][x].x = 0;
        board.tiles[range.start.y][x].y = 0;
        board.tiles[range.start.y][x].tile_type = TILETYPE_EMPTY;
        board.tiles[range.start.y][x].falling = false;
        board.tiles[range.start.y][x].vspeed = 0;
        board.tiles[range.start.y][x].row_dest = 0;
    }
}

void board_delete_vert_range(BoardRange range) {
    for (int y = range.start.y; y <= range.end.y; y++) {
        board.tiles[y][range.start.x].x = 0;
        board.tiles[y][range.start.x].y = 0;
        board.tiles[y][range.start.x].tile_type = TILETYPE_EMPTY;
        board.tiles[y][range.start.x].falling = false;
        board.tiles[y][range.start.x].vspeed = 0;
        board.tiles[y][range.start.x].row_dest = 0;
    }
}

void board_rotate_row_left(int row) {
    Tile temp = board.tiles[row][0];
    int n = 1;
    for (; n < BOARD_WIDTH; n++) {
        board.tiles[row][n - 1].tile_type = board.tiles[row][n].tile_type;
    }
    board.tiles[row][n - 1].tile_type = temp.tile_type;
}

void board_rotate_row_right(int row) {
    Tile temp = board.tiles[row][BOARD_WIDTH - 1];
    int n = BOARD_WIDTH - 2;
    for (; n >= 0; n--) {
        board.tiles[row][n + 1].tile_type = board.tiles[row][n].tile_type;
    }
    board.tiles[row][0].tile_type = temp.tile_type;
}

int board_find_row_dest(int x, int start) {
    int dest = start;
    for (; dest < BOARD_HEIGHT - 1; dest++) {
        if (board.tiles[dest + 1][x].tile_type != TILETYPE_EMPTY) {
            break;
        }
    }
    return dest;
}

