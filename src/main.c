#include "sb_mem_arena.h"

int main() {
    SbArena arena = {0};
    sb_mem_arena_init(&arena, 10);
    long x = 12;
    void *p = SB_MEM_ARENA_PUSH(&arena, int);
    void *r = SB_MEM_ARENA_PUSH(&arena, long);
    void *q = SB_MEM_ARENA_PUSH(&arena, long double);
    sb_mem_arena_reset(&arena);
    return 0;
}
