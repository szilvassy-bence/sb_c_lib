#include "sb_str.h"
#include <stddef.h>
#define _DEFAULT_SOURCE // a legelső sor, minden #include előtt

#include "sb_int.h"
#include "sb_mem_freelist.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define NALLOC 1024
#define MIN_BLOCK_UNITS 2

typedef struct header {
    _Alignas(max_align_t) struct header *next;
    uint32_t size;
} Header;

// static variable in .bss
static Header base;
static Header *freep = NULL;
static Header *arenas = NULL;
static uint32_t request_count = 0;

static Header *morecore(uint32_t nunits);
static void *sb_memory_malloc(uint32_t nunits);
static void stack_trace(const char *str, size_t level);
static uint32_t convert_header_to_bytes(uint32_t n);
static void get_previous_free_p(Header **p, const Header *bp);
static const char *sb_check_heap();
static uint32_t sb_memory_get_alloc_client_size(const Header *p);

void *sb_memory_realloc(void *client_data, uint32_t nbytes) {
    request_count++;
    stack_trace("Entered sb_memory_realloc", 0);

    // Fresh alloc needed
    if (client_data == NULL) {
        stack_trace("Null pointer", 1);
        sb_check_heap();
        return sb_memory_malloc(nbytes);
    }

    // without header
    Header *client_header = (Header *)client_data - 1;
    uint32_t client_alloc_size = sb_memory_get_alloc_client_size(client_header);
    uint32_t requested_unit_count =
        (nbytes + sizeof(Header) - 1) / sizeof(Header);

    // client allocation has not enough space for the requested amount
    if (requested_unit_count <= client_alloc_size) {
        sb_check_heap();
        return client_data;
    }

    uint32_t needed_unit_count =
        requested_unit_count - (client_header->size - 1);

    Header *p = NULL;
    get_previous_free_p(&p, client_header);

    // client allocation has not enough space for the requested amount
    // direct neighbor
    if (p->next == client_header + client_header->size) {
        Header *nb = p->next;
        if (needed_unit_count <= nb->size) {
            uint32_t extra = nb->size - needed_unit_count;

            // extra space is left in the direct neighbor and freed
            if (extra >= MIN_BLOCK_UNITS) {
                Header *rest = nb + needed_unit_count;
                p->next = nb->next;
                rest->size = extra;
                client_header->size += needed_unit_count;
                if (freep == nb)
                    freep = p;
                sb_memory_free(rest + 1);
            } else { // no extra space left in the direct neighbor
                client_header->size += needed_unit_count + extra;
                if (freep == nb)
                    freep = p;
                p->next = nb->next;
            }
            stack_trace("Existing pointer remains, next free neighbor merged",
                        1);
            sb_memory_print_free_list();
            sb_check_heap();
            return (void *)(client_header + 1);
        }
    }

    // not direct neighbor
    // or direct neighbor but has not enough space
    // copy needed
    void *new_client_data = sb_memory_malloc(nbytes);
    memcpy(new_client_data, client_data,
           (size_t)convert_header_to_bytes(client_header->size - 1));
    sb_memory_free(client_data);
    stack_trace("Existing pointer new address", 1);
    sb_memory_print_free_list();
    sb_check_heap();
    return (void *)new_client_data;
}

void sb_memory_free(void *ap) {
    if (ap == NULL)
        return;
    stack_trace("Entered sb_memory_free", 0);
    Header *bp, *p = NULL;
    bp = (Header *)ap - 1;
    get_previous_free_p(&p, bp);
    if (bp + bp->size == p->next) { // join when CASE B and direct neighbor
        bp->size += p->next->size;
        bp->next = p->next->next;
    } else {
        bp->next = p->next;
    }

    if (p + p->size == bp) { // join case when A and direct neighbor
        p->size += bp->size;
        p->next = bp->next;
    } else {
        p->next = bp;
    }

    freep = p;
    sb_check_heap();
}

void sb_memory_destroy(void) {
    Header *curr_arena = arenas;
    while (curr_arena != NULL) {
        Header *temp = curr_arena;
        curr_arena = curr_arena->next;
        free(temp);
    }
    base.next = freep = arenas = NULL;
}

Header *morecore(uint32_t nunits) {
    stack_trace("Entered morecore", 0);
    Header *cp;

    if (nunits < NALLOC) {
        nunits = NALLOC;
    }

    // an extra header for the arena
    cp = (Header *)malloc((nunits + 1) * sizeof(Header));
    if (cp == NULL) {
        return NULL;
    }
    cp->next = arenas;
    arenas = cp;
    (cp + 1)->size = nunits;
    (cp + 1)->next = NULL;
    sb_memory_free((void *)(cp + 2));
    sb_memory_print_free_list();
    sb_check_heap();
    return freep;
}

void *sb_memory_malloc(uint32_t nbytes) {
    stack_trace("Entered sb_memory_malloc", 0);
    Header *p, *prevp = NULL;

    // bytes to units conversion
    // rounded up and added an extra header
    // header included
    uint32_t nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;

    // init freep & set prevp
    if ((prevp = freep) == NULL) {
        base.next = freep = prevp = &base;
        base.size = 0;
    }

    for (p = prevp->next;; prevp = p, p = p->next) {
        if (p->size >= nunits) {
            uint32_t extra = p->size - nunits;
            if (extra >= MIN_BLOCK_UNITS) {
                p->size -= nunits;
                p += p->size;
                p->size = nunits;
            } else {
                prevp->next = p->next;
            }
            freep = prevp;
            stack_trace("Used block's free space", 1);
            sb_memory_print_free_list();
            sb_check_heap();
            return (void *)(p + 1);
        }
        if (p == freep) { // wrapped around
            stack_trace("Allocated new memory", 1);
            if ((p = morecore(nunits)) == NULL) {
                return NULL;
            }
        }
    }
}

void stack_trace(const char *str, size_t level) {
#ifdef DEBUG_MODE
    return;
    SbStrBuf sb;
    sb_str_buf_init(&sb, level * 2);
    for (size_t i = 0; i < level; i++) {
        sb_str_buf_append_cstr(&sb, "--");
    }
    printf("%s%s\n", sb_str_buf_to_cstr(&sb), str);
    sb_str_buf_free(&sb);
#endif
}

static uint32_t convert_header_to_bytes(uint32_t n) {
    bool overflow;
    uint32_t result = sb_int_mul_u32(n, sizeof(Header), &overflow);
    if (overflow) {
        printf("Error: overflow\n");
        exit(1);
    }
    return result;
}

static void get_previous_free_p(Header **p, const Header *bp) {
    // exit condition: bp has room between the current pointer and its next
    for (*p = freep; !(*p < bp && bp < (*p)->next); *p = (*p)->next) {
        // when p is in the highest address and p next is in the lowest address
        //     CASE A: and bp is even higher than p
        //     CASE B: or bp is even lower than p's next
        //      which is not really possible, since the lowest free is base
        //      which is on .bss
        if ((*p)->next <= (*p) && (bp < (*p)->next || (*p) < bp)) {
            break;
        }
    }
}

#ifdef DEBUG_MODE
void sb_memory_print_free_list(void) {
    uint32_t count = 0;
    printf("\n-- PRINT FREE POINTERS --\nBase's address: %p\n", (void *)&base);
    if (freep == NULL || freep == &base) {
        printf("Free list is empty.\n");
    } else {
        Header *p = freep;
        do {
            count++;
            printf("P%d points to: %p\n\tIts size is %d\n", count, (void *)p,
                   ((Header *)p)->size);
            p = p->next;
        } while (p != freep);
        printf("-- ---- --\n");
    }
    printf("Request count: %d\n", request_count);
    printf("Count: %d\n", count);
    printf("-- END PRINTING --\n\n");
    return;
}
#endif

static const char *sb_check_heap() {
#ifdef DEBUG_MODE
    if (freep == NULL)
        return NULL;
    Header *p = &base;
    uint32_t steps = 0;
    bool reached_free = false;
    bool wrapped = false;

    do {
        if (p != &base && p->size == 0) {
            printf("Invalid block, size cannot be 0.\n");
            exit(1);
        }
        if (p > p->next) {
            if (wrapped) {
                printf("Multiple wrap\n");
                exit(1);
            }
            wrapped = true;
        } else if (p + p->size > p->next) {
            printf("Overlap\n");
            exit(1);
        }
        if (p + p->size == p->next) {
            printf("Missed merge\n");
            exit(1);
        }
        if (steps > UINT32_MAX) {
            printf("Limit reached\n");
            exit(1);
        }
        if (p == freep)
            reached_free = true;

        p = p->next;
        steps++;
    } while (p != &base);

    if (!reached_free) {
        printf("Not reached free\n");
        exit(1);
    }
    if (!wrapped) {
        printf("Not wrapped\n");
        exit(1);
    }
#endif
    return NULL;
}

uint32_t sb_memory_get_alloc_client_size(const Header *p) {
    if (p == NULL)
        return 0;
    return p->size - 1;
}

#ifdef DEBUG_MODE
void sb_memory_debug_from_client_ptr(void *ptr) {
    if (ptr == NULL) {
        printf("Memory block is NULL");
        return;
    }
    Header *p = (Header *)ptr;
    printf("Memory block header address: %p \t size: %d\n", p, p->size);
    return;
}
#endif
