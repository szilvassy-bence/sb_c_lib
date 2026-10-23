#include "sb_mem_freelist.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SLOTS 16

typedef struct {
    uint64_t a;
    uint64_t b;
} TestU64;

static uint32_t seed = 12345;
static uint32_t next_rand(void) {
    seed = seed * 1103515245u + 12345u;
    return seed >> 16;
}
static uint32_t random_size() {
    if (next_rand() % 50 == 0) {
        return 20000;
    } else {
        return 1 + next_rand() % 512;
    }
}

void test_sb_realloc_copy() {
    TestU64 *a = (TestU64 *)sb_memory_realloc(NULL, 16);
    a->a = 10;
    a->b = 20;
    TestU64 *b = (TestU64 *)sb_memory_realloc(a, 32);
    b[1].a = 30;
    b[1].b = 40;
    assert(b[0].a == 10);
    assert(b[0].b == 20);
    assert(b[1].a == 30);
    assert(b[1].b == 40);
    sb_memory_free(b);
    sb_memory_destroy();
}

void test_sb_realloc_in_place() {
    TestU64 *a = (TestU64 *)sb_memory_realloc(NULL, 16);
    a->a = 10;
    a->b = 20;
    TestU64 *b = (TestU64 *)sb_memory_realloc(a, 32);
    b[1].a = 30;
    b[1].b = 40;
    TestU64 *c = (TestU64 *)sb_memory_realloc(b, 64);
    assert(c == b);
    assert(c[0].a == 10);
    assert(c[0].b == 20);
    assert(c[1].a == 30);
    assert(c[1].b == 40);
    sb_memory_free(c);
    sb_memory_destroy();
}

void test_sb_realloc_free() {
    TestU64 *a = (TestU64 *)sb_memory_realloc(NULL, 32);
    TestU64 *b = (TestU64 *)sb_memory_realloc(NULL, 64);
    sb_memory_free(a);
    TestU64 *c = (TestU64 *)sb_memory_realloc(NULL, 32);
    assert(a == c);
    sb_memory_free(b);
    sb_memory_destroy();
}

void test_sb_realloc_20_block_diff_sizes() {
    void *blocks[20];
    uint32_t sizes[20];

    for (uint32_t i = 0; i < 20; i++) {
        sizes[i] = 8 + i * 24;
        blocks[i] = sb_memory_realloc(NULL, sizes[i]);
        memset(blocks[i], i, sizes[i]);
    }

    for (uint32_t i = 0; i < 20; i++) {
        uint8_t *byte = (uint8_t *)blocks[i];
        for (uint32_t j = 0; j < sizes[i]; j++) {
            assert(byte[j] == (uint8_t)i);
        }
        assert((uintptr_t)blocks[i] % 16 == 0);
    }
    for (uint32_t i = 0; i < 20; i += 2) {
        sb_memory_free(blocks[i]);
    }
    for (uint32_t i = 1; i < 20; i += 2) {
        uint8_t *byte = (uint8_t *)blocks[i];
        for (uint32_t j = 0; j < sizes[i]; j++) {
            assert(byte[j] == (uint8_t)i);
        }
    }
    for (uint32_t i = 0; i < 20; i += 2) {
        blocks[i] = sb_memory_realloc(NULL, sizes[i]);
        memset(blocks[i], i, sizes[i]);
    }
    for (uint32_t i = 0; i < 20; i++) {
        uint8_t *byte = (uint8_t *)blocks[i];
        for (uint32_t j = 0; j < sizes[i]; j++) {
            assert(byte[j] == (uint8_t)i);
        }
        sb_memory_free(blocks[i]);
    }
    sb_memory_destroy();
}

void test_req_fits_its_current_allocation() {
    void *a = sb_memory_realloc(NULL, 8);
    void *b = sb_memory_realloc(a, 16);
    assert(a == b);
    sb_memory_destroy();
}

void test_shrinking_keeps_value() {
    void *a = sb_memory_realloc(NULL, 64);
    memset(a, 1, 64);
    void *b = sb_memory_realloc(a, 16);
    for (uint32_t i = 0; i < 16; i++) {
        assert(*((uint8_t *)b + i) == 1);
    }
    sb_memory_free(b);
    sb_memory_destroy();
}

void test_merge() {
    void *a = sb_memory_realloc(NULL, 128);
    void *b = sb_memory_realloc(NULL, 256);

    sb_memory_free(a);
    sb_memory_free(b);

    void *c = sb_memory_realloc(NULL, 384 + 16); // extra 16 for header
    assert(c == b);
    sb_memory_free(c);

    a = sb_memory_realloc(NULL, 128);
    b = sb_memory_realloc(NULL, 256);

    sb_memory_free(b);
    sb_memory_free(a);

    c = sb_memory_realloc(NULL, 384 + 16); // extra 16 for header
    assert(c == b);
    sb_memory_free(c);
    sb_memory_destroy();
}

void test_large_request() {
    void *a = sb_memory_realloc(NULL, 1024 * 20);
    memset(a, 2, 1024 * 20);
    void *b = sb_memory_realloc(NULL, 1024);
    memset(b, 3, 1024);
    for (uint32_t i = 0; i < 1024 * 20; i++) {
        assert(*((uint8_t *)a + i) == 2);
    }
    for (uint32_t j = 0; j < 1024; j++) {
        assert(*((uint8_t *)b + j) == 3);
    }
    sb_memory_free(a);
    sb_memory_free(b);
    sb_memory_destroy();
}

void test_sb_free_null() { sb_memory_free(NULL); }

void test_chaos() {
    void *ptr[SLOTS] = {0};
    uint32_t sizes[SLOTS] = {0};

    for (int i = 0; i < 1000; i++) {
        uint32_t idx = next_rand() % SLOTS;
        uint32_t n = random_size();
        if (ptr[idx] == NULL) {
            ptr[idx] = sb_memory_realloc(NULL, n);
            memset(ptr[idx], idx, n);
            sizes[idx] = n;
        } else {
            if (next_rand() % 2 == 0) {
                sb_memory_free(ptr[idx]);
                ptr[idx] = NULL;
                sizes[idx] = 0;
            } else {
                uint32_t old_size = sizes[idx];
                uint32_t min = old_size > n ? n : old_size;
                ptr[idx] = sb_memory_realloc(ptr[idx], n);
                for (uint32_t j = 0; j < min; j++) {
                    assert(*((uint8_t *)ptr[idx] + j) == idx);
                }
                memset(ptr[idx], idx, n);
                sizes[idx] = n;
            }
        }
        for (uint32_t k = 0; k < SLOTS; k++) {
            uint8_t *byte = ptr[k];
            for (uint32_t l = 0; l < sizes[k]; l++) {
                assert(*(byte + l) == k);
            }
        }
    }
    for (uint32_t i = 0; i < SLOTS; i++) {
        sb_memory_free(ptr[i]);
    }
    sb_memory_destroy();
}

int main() {
    test_sb_realloc_copy();
    test_sb_realloc_in_place();
    test_sb_realloc_free();
    test_sb_realloc_20_block_diff_sizes();
    test_req_fits_its_current_allocation();
    test_shrinking_keeps_value();
    test_merge();
    test_large_request();
    test_sb_free_null();
    test_chaos();
    printf("sb_memory tests are successful\n");
    return 0;
}
