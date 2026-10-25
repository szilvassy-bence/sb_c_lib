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

static uint32_t request_count = 0;

static Header *morecore(SbMemFreelist *fl, uint32_t nunits);
static void *sb_memory_malloc(SbMemFreelist *fl, uint32_t nunits);
static void stack_trace(const char *str, size_t level);
static uint32_t convert_header_to_bytes(uint32_t n);
static void get_previous_free_p(SbMemFreelist *fl, Header **p,
                                const Header *bp);
static const char *sb_check_heap(const SbMemFreelist *fl);
static uint32_t sb_memory_get_alloc_client_size(const Header *p);

void *sb_memory_realloc(SbMemFreelist *fl, void *client_data, uint32_t nbytes) {
    request_count++;
    stack_trace("Entered sb_memory_realloc", 0);

    // Fresh alloc needed
    if (client_data == NULL) {
        stack_trace("Null pointer", 1);
        sb_check_heap(fl);
        return sb_memory_malloc(fl, nbytes);
    }

    // without header
    Header *client_header = (Header *)client_data - 1;
    uint32_t client_alloc_size = sb_memory_get_alloc_client_size(client_header);
    uint32_t requested_unit_count =
        (nbytes + sizeof(Header) - 1) / sizeof(Header);

    // client allocation has not enough space for the requested amount
    if (requested_unit_count <= client_alloc_size) {
        sb_check_heap(fl);
        return client_data;
    }

    uint32_t needed_unit_count =
        requested_unit_count - (client_header->size - 1);

    Header *p = NULL;
    get_previous_free_p(fl, &p, client_header);

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
                if (fl->freep == nb)
                    fl->freep = p;
                sb_memory_free(fl, rest + 1);
            } else { // no extra space left in the direct neighbor
                client_header->size += needed_unit_count + extra;
                if (fl->freep == nb)
                    fl->freep = p;
                p->next = nb->next;
            }
            stack_trace("Existing pointer remains, next free neighbor merged",
                        1);
            sb_memory_print_free_list(fl);
            sb_check_heap(fl);
            return (void *)(client_header + 1);
        }
    }

    // not direct neighbor
    // or direct neighbor but has not enough space
    // copy needed
    void *new_client_data = sb_memory_malloc(fl, nbytes);
    memcpy(new_client_data, client_data,
           (size_t)convert_header_to_bytes(client_header->size - 1));
    sb_memory_free(fl, client_data);
    stack_trace("Existing pointer new address", 1);
    sb_memory_print_free_list(fl);
    sb_check_heap(fl);
    return (void *)new_client_data;
}

void sb_memory_free(SbMemFreelist *fl, void *ap) {
    if (ap == NULL)
        return;
    stack_trace("Entered sb_memory_free", 0);
    Header *bp, *p = NULL;
    bp = (Header *)ap - 1;
    get_previous_free_p(fl, &p, bp);
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

    fl->freep = p;
    sb_check_heap(fl);
}

void sb_memory_destroy(SbMemFreelist *fl) {
    Header *curr_arena = fl->arenas;
    while (curr_arena != NULL) {
        Header *temp = curr_arena;
        curr_arena = curr_arena->next;
        free(temp);
    }
    fl->base.next = fl->freep = fl->arenas = NULL;
}

Header *morecore(SbMemFreelist *fl, uint32_t nunits) {
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
    cp->next = fl->arenas;
    fl->arenas = cp;
    (cp + 1)->size = nunits;
    (cp + 1)->next = NULL;
    sb_memory_free(fl, (void *)(cp + 2));
    sb_memory_print_free_list(fl);
    sb_check_heap(fl);
    return fl->freep;
}

void *sb_memory_malloc(SbMemFreelist *fl, uint32_t nbytes) {
    stack_trace("Entered sb_memory_malloc", 0);
    Header *p, *prevp = NULL;

    // bytes to units conversion
    // rounded up and added an extra header
    // header included
    uint32_t nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;

    // init freep & set prevp
    if ((prevp = fl->freep) == NULL) {
        fl->base.next = fl->freep = prevp = &fl->base;
        fl->base.size = 0;
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
            fl->freep = prevp;
            stack_trace("Used block's free space", 1);
            sb_memory_print_free_list(fl);
            sb_check_heap(fl);
            return (void *)(p + 1);
        }
        if (p == fl->freep) { // wrapped around
            stack_trace("Allocated new memory", 1);
            if ((p = morecore(fl, nunits)) == NULL) {
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

static void get_previous_free_p(SbMemFreelist *fl, Header **p,
                                const Header *bp) {
    // exit condition: bp has room between the current pointer and its next
    for (*p = fl->freep; !(*p < bp && bp < (*p)->next); *p = (*p)->next) {
        // when p is in the highest address and p next is in the lowest
        // address
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
void sb_memory_print_free_list(SbMemFreelist *fl) {
    uint32_t count = 0;
    printf("\n-- PRINT FREE POINTERS --\nBase's address: %p\n",
           (void *)&fl->base);
    if (fl->freep == NULL || fl->freep == &fl->base) {
        printf("Free list is empty.\n");
    } else {
        Header *p = fl->freep;
        do {
            count++;
            printf("P%d points to: %p\n\tIts size is %d\n", count, (void *)p,
                   ((Header *)p)->size);
            p = p->next;
        } while (p != fl->freep);
        printf("-- ---- --\n");
    }
    printf("Request count: %d\n", request_count);
    printf("Count: %d\n", count);
    printf("-- END PRINTING --\n\n");
    return;
}
#endif

static const char *sb_check_heap(const SbMemFreelist *fl) {
#ifdef DEBUG_MODE
    if (fl->freep == NULL)
        return NULL;
    Header *p = &fl->base;
    uint32_t steps = 0;
    bool reached_free = false;
    bool wrapped = false;

    do {
        if (p != &fl->base && p->size == 0) {
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
        if (p == fl->freep)
            reached_free = true;

        p = p->next;
        steps++;
    } while (p != &fl->base);

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
    Header *p = (Header *)ptr - 1;
    printf("Memory block header address: %p \t size: %d\n", (void *)p, p->size);
    return;
}
#endif
