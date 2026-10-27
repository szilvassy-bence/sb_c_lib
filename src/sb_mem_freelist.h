#ifndef sb_mem_freelist_h
#define sb_mem_freelist_h

#include <stddef.h>
#include <stdint.h>

#if defined(__GNUC__)
#define SB_MEMORY_REALLOC_ATTR                                                 \
    __attribute__((warn_unused_result, alloc_size(3)))
#else
#define SB_MEMORY_REALLOC_ATTR
#endif

typedef struct header {
    _Alignas(max_align_t) struct header *next;
    uint32_t size;
} Header;

// self referencing struct
// carefully copy instance
typedef struct {
    Header base;
    Header *freep;
    Header *arenas;
} SbMemFreelist;

void *sb_memory_realloc(SbMemFreelist *fl, void *ptr,
                        uint32_t nbytes) SB_MEMORY_REALLOC_ATTR;
void sb_memory_free(SbMemFreelist *fl, void *ap);
void sb_memory_destroy(SbMemFreelist *fl);

#ifdef DEBUG_MODE
void sb_memory_print_free_list(SbMemFreelist *fl);
void sb_memory_debug_from_client_ptr(void *ptr);
#else
#define sb_memory_print_free_list(fl) ((void)fl)
#define sb_memory_debug_from_client_ptr(ptr) ((void)0)
#endif

#endif
