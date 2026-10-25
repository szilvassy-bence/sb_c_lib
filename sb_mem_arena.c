#include "sb_mem_arena.h"
#include "sb_int.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SB_MEM_ARENA_SIZE (1024 * 1024)

static void check_arena(const SbArena *arena);
static void check_arena_block(SbArenaBlock *block);
static inline uintptr_t align_up(uintptr_t address, size_t align);
static SbArenaBlock *new_block(size_t data_size, SbArenaBlock *prev);
static void *try_alloc_in(SbArenaBlock *b, size_t size, size_t align);

bool sb_mem_arena_init(SbArena *arena, size_t block_size) {
    if (arena == NULL) {
        return false;
    }
    if (arena->current)
        return false;

    SbArenaBlock *block = new_block(block_size, NULL);
    if (!block) {
        return false;
    }

    arena->current = block;
    arena->block_size = block_size;
    check_arena(arena);
    return true;
}

void *sb_mem_arena_alloc(SbArena *arena, size_t size, size_t align) {
    check_arena(arena);

    void *p = try_alloc_in(arena->current, size, align);
    if (p)
        return p;

    size_t need = sb_int_max_size_t(arena->block_size, size + align - 1);

    SbArenaBlock *b = new_block(need, arena->current);
    if (!b)
        return NULL;
    arena->current = b;
    p = try_alloc_in(b, size, align);
    assert(p);
    return p;
}

void sb_mem_arena_reset(SbArena *arena) {
    while (arena->current->prev != NULL) {
        SbArenaBlock *p = arena->current;
        arena->current = p->prev;
        free(p);
    }
    arena->current->used = 0;
}

void sb_mem_arena_destroy(SbArena *arena) {
    while (arena->current) {
        SbArenaBlock *p = arena->current;
        arena->current = p->prev;
        free(p);
    }
    arena->current = NULL;
    arena->block_size = 0;
}

void check_arena(const SbArena *arena) {
    check_arena_block(arena->current);
    assert(arena->block_size > 0);
}

void check_arena_block(SbArenaBlock *block) {
    SbArenaBlock *p = block;
    do {
        assert(p != NULL);
        void *data = (void *)(p + 1);
        assert((data == NULL) == (p->size == 0 && p->used == 0));
        assert((data != NULL) == (p->size > 0));
        assert((data == NULL) || (p->size >= p->used));
        p = p->prev;
    } while (p);
}

SbArenaMark sb_mem_arena_mark(const SbArena *arena) {
    return (SbArenaMark){.block = arena->current, .used = arena->current->used};
}

void sb_mem_arena_restore(SbArena *arena, SbArenaMark mark) {
    SbArenaBlock *curr = arena->current;
    while (curr != mark.block) {
        SbArenaBlock *p = curr;
        assert(p);
        curr = p->prev;
        free(p);
    }
    if (curr == arena->current) {
        assert(mark.used <= curr->used);
    } else {
        assert(mark.used <= curr->size);
    }
    arena->current = curr;
    arena->current->used = mark.used;
    return;
}

uintptr_t align_up(uintptr_t address, size_t align) {
    // check if align is any power of 2
    assert(align != 0 && (align & (align - 1)) == 0);
    return (address + (uintptr_t)align - 1) & ~(align - 1);
}

SbArenaBlock *new_block(size_t data_size, SbArenaBlock *prev) {
    SbArenaBlock *block =
        (SbArenaBlock *)malloc(sizeof(SbArenaBlock) + data_size);
    if (block == NULL) {
        return NULL;
    }
    block->size = data_size;
    block->used = 0;
    block->prev = prev;
    return block;
}

void *try_alloc_in(SbArenaBlock *b, size_t size, size_t align) {

    void *current_data = b->data;

    uintptr_t last_used_addr = (uintptr_t)current_data + b->used;
    uintptr_t start = align_up((uintptr_t)last_used_addr, align);
    uintptr_t offset = start - (uintptr_t)current_data;
    if (offset >= b->size || size > b->size - offset) {
        return NULL;
    }

    b->used = (uint32_t)(offset + size);
#ifdef DEBUG_MODE
    printf("Arena's current block's data: %p, requested: %zu, start %p, arena "
           "size: %d, "
           "arena "
           "used: %d\n",
           current_data, size, (void *)start, b->size, b->used);
#endif
    return (void *)start;
}
