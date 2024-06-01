#if !defined(CIRCUITBREAKER_H)
#define CIRCUITBREAKER_H

#include <stdbool.h>
#include <stdint.h>

#define BOARD_WIDTH 4
#define BOARD_HEIGHT 5
#define CELL_SIZE 64
#define HALF_CELL_SIZE 32


#if defined(CIRCUITBREAKER_SLOW)
#define Assert(Expression) if(!(Expression)) {*(volatile int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef float real64;

void board_init();
void board_update();
void board_draw(uint16 pos_x, uint16 pos_y);

struct GameMemory {
	bool32 is_initialized;

	uint64 permanent_storage_size;
	void *permanent_storage;

	uint64 transient_storage_size;
	void *transient_storage;
};

#endif
