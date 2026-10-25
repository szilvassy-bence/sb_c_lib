#ifndef sb_mem_arena
#define sb_mem_arena

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SB_MEM_ARENA_PUSH(a, T) sb_mem_arena_alloc((a), sizeof(T), _Alignof(T))

typedef struct SbArenaBlock {
    struct SbArenaBlock *prev;
    size_t size;
    size_t used;
    _Alignas(max_align_t) unsigned char data[];
} SbArenaBlock;

typedef struct {
    SbArenaBlock *current;
    size_t block_size;
} SbArena;

typedef struct {
    SbArenaBlock *block;
    size_t used;
} SbArenaMark;

bool sb_mem_arena_init(SbArena *arena, size_t block_size);
void *sb_mem_arena_alloc(SbArena *arena, size_t size, size_t align);
void sb_mem_arena_reset(SbArena *arena);
void sb_mem_arena_destroy(SbArena *arena);
SbArenaMark sb_mem_arena_mark(const SbArena *arena);
void sb_mem_arena_restore(SbArena *arena, SbArenaMark mark);

#endif
