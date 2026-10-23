#ifndef sb_memory_h
#define sb_memory_h

#include <stddef.h>
#include <stdint.h>

#if defined(__GNUC__)
#define SB_MEMORY_REALLOC_ATTR                                                 \
    __attribute__((warn_unused_result, alloc_size(2)))
#else
#define SB_MEMORY_REALLOC_ATTR
#endif

void *sb_memory_realloc(void *ptr, uint32_t nbytes) SB_MEMORY_REALLOC_ATTR;
void sb_memory_free(void *ap);
void sb_memory_destroy(void);

#ifdef DEBUG_MODE
void sb_memory_print_free_list(void);
#else
#define sb_memory_print_free_list() ((void)0)
#endif

#ifdef DEBUG_MODE
void sb_memory_debug_from_client_ptr(void *ptr);
#else
#define sb_memory_debug_from_client_ptr(ptr) ((void)0)
#endif

#endif
