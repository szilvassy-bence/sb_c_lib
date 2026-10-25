#include "sb_mem_arena.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SB_MEM_ARENA_SIZE (1024 * 1024)

void test_sb_mem_arena_init() {
    SbArena arena = {0};
    bool result = NULL;
    result = sb_mem_arena_init(&arena, SB_MEM_ARENA_SIZE);
    assert(result == true);
    result = sb_mem_arena_init(&arena, SB_MEM_ARENA_SIZE);
    assert(result == false);
    assert(arena.current != NULL);
    assert(arena.current->size > 0);
    assert(arena.current->used == 0);
    while (arena.current) {
        SbArenaBlock *p = arena.current;
        arena.current = p->prev;
        free(p);
    }
}

void test_sb_mem_arena_alloc_align() {
    SbArena arena = {0};
    sb_mem_arena_init(&arena, SB_MEM_ARENA_SIZE);
    void *a = sb_mem_arena_alloc(&arena, sizeof(int), _Alignof(int));
    void *b = sb_mem_arena_alloc(&arena, sizeof(long), _Alignof(long));
    void *c = sb_mem_arena_alloc(&arena, sizeof(int), _Alignof(int));
    void *d =
        sb_mem_arena_alloc(&arena, sizeof(long double), _Alignof(long double));
    assert(arena.current->used == 48);
    assert((char *)b == (char *)a + 8);
    assert((char *)c == (char *)b + 8);
    assert((char *)d == (char *)c + 16);
    while (arena.current) {
        SbArenaBlock *p = arena.current;
        arena.current = p->prev;
        free(p);
    }
}

void test_sb_mem_arena_alloc_full() {
    SbArena arena = {0};
    sb_mem_arena_init(&arena, SB_MEM_ARENA_SIZE);
    for (int i = 0; i < SB_MEM_ARENA_SIZE / 8; i++) {
        void *b = sb_mem_arena_alloc(&arena, sizeof(long), _Alignof(long));
        assert(b != NULL);
    }
    SbArenaBlock *p = arena.current;
    void *c = sb_mem_arena_alloc(&arena, sizeof(int), _Alignof(int));
    assert(c != p);
    while (arena.current) {
        SbArenaBlock *p = arena.current;
        arena.current = p->prev;
        free(p);
    }
}

void test_sb_mem_arena_destroy() {
    SbArena arena = {0};
    sb_mem_arena_init(&arena, SB_MEM_ARENA_SIZE);
    void *b = sb_mem_arena_alloc(&arena, SB_MEM_ARENA_SIZE, 16);
    SbArenaBlock *p = arena.current;
    void *c = sb_mem_arena_alloc(&arena, sizeof(int), _Alignof(int));
    sb_mem_arena_destroy(&arena);
    assert(arena.current == NULL);
    assert(arena.block_size == 0);
}

void test_sb_mem_arena_reset() {
    SbArena arena = {0};
    sb_mem_arena_init(&arena, SB_MEM_ARENA_SIZE);
    void *c = sb_mem_arena_alloc(&arena, sizeof(int), _Alignof(int));
    SbArenaBlock *first = arena.current;
    assert(arena.current->used == 4);
    void *d = sb_mem_arena_alloc(&arena, SB_MEM_ARENA_SIZE, 16);
    assert(arena.current->prev == first);
    sb_mem_arena_reset(&arena);
    assert(arena.current->used == 0);
    assert(arena.current == first);
    sb_mem_arena_destroy(&arena);
}

void test_sb_mem_arena_mark() {
    SbArena arena = {0};
    sb_mem_arena_init(&arena, SB_MEM_ARENA_SIZE);
    void *a = sb_mem_arena_alloc(&arena, SB_MEM_ARENA_SIZE / 2, _Alignof(int));
    SbArenaMark mark_1 = sb_mem_arena_mark(&arena);

    void *b = sb_mem_arena_alloc(&arena, SB_MEM_ARENA_SIZE / 2, _Alignof(long));
    assert(arena.current->used == SB_MEM_ARENA_SIZE);
    assert(arena.current->prev == NULL);
    SbArenaBlock *p = arena.current;

    void *c = sb_mem_arena_alloc(&arena, 1024 * 100, _Alignof(int));
    SbArenaMark mark_2 = sb_mem_arena_mark(&arena);
    void *d = sb_mem_arena_alloc(&arena, 1024 * 200, _Alignof(long));
    assert(p != arena.current);

    assert(mark_1.block == p);
    assert(mark_1.used == 1024 * 512);
    assert(mark_2.block == arena.current);
    assert(mark_2.used == 1024 * 100);
    sb_mem_arena_destroy(&arena);
}

void test_sb_mem_arena_restore() {
    SbArena arena = {0};
    sb_mem_arena_init(&arena, SB_MEM_ARENA_SIZE);
    void *a = sb_mem_arena_alloc(&arena, SB_MEM_ARENA_SIZE / 2, _Alignof(int));
    SbArenaMark mark_1 = sb_mem_arena_mark(&arena);
    SbArenaBlock *p_1 = arena.current;
    size_t used_1 = arena.current->used;

    void *b = sb_mem_arena_alloc(&arena, SB_MEM_ARENA_SIZE / 2, _Alignof(long));

    void *c = sb_mem_arena_alloc(&arena, 1024 * 100, _Alignof(int));
    SbArenaMark mark_2 = sb_mem_arena_mark(&arena);
    SbArenaBlock *p_2 = arena.current;
    size_t used_2 = arena.current->used;
    void *d = sb_mem_arena_alloc(&arena, 1024 * 200, _Alignof(long));

    sb_mem_arena_restore(&arena, mark_2);

    assert(arena.current == p_2);
    assert(arena.current->used == used_2);

    sb_mem_arena_restore(&arena, mark_1);

    assert(arena.current == p_1);
    assert(arena.current->used == used_1);

    sb_mem_arena_destroy(&arena);
}

void test_sb_mem_arena_restore_bad_case() {

    SbArena arena = {0};
    sb_mem_arena_init(&arena, SB_MEM_ARENA_SIZE);
    void *a = sb_mem_arena_alloc(&arena, SB_MEM_ARENA_SIZE / 2, _Alignof(int));
    SbArenaMark mark_1 = sb_mem_arena_mark(&arena);
    SbArenaBlock *p_1 = arena.current;
    size_t used_1 = arena.current->used;

    void *b = sb_mem_arena_alloc(&arena, SB_MEM_ARENA_SIZE / 2, _Alignof(long));

    void *c = sb_mem_arena_alloc(&arena, 1024 * 100, _Alignof(int));
    SbArenaMark mark_2 = sb_mem_arena_mark(&arena);
    SbArenaBlock *p_2 = arena.current;
    size_t used_2 = arena.current->used;
    void *d = sb_mem_arena_alloc(&arena, 1024 * 200, _Alignof(long));

    sb_mem_arena_restore(&arena, mark_1);
    sb_mem_arena_restore(&arena, mark_2);

    sb_mem_arena_destroy(&arena);
}
int main() {
    test_sb_mem_arena_init();
    test_sb_mem_arena_alloc_align();
    test_sb_mem_arena_alloc_full();
    test_sb_mem_arena_destroy();
    test_sb_mem_arena_reset();
    test_sb_mem_arena_mark();
    test_sb_mem_arena_restore();
    // test_sb_mem_arena_restore_bad_case();
    printf("\nsb_mem_arena tests are successful.\n");
    return 0;
}
