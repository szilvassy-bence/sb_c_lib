#ifndef SB_STR_H
#define SB_STR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  const char *data;
  size_t len;
} SbStrView;

/**
 * Invariants
 *  - data != NULL <=> len < cap
 *  - data == NULL <=> len == 0 && cap == 0
 *  - if data != NULL, then data[len] == '\0'
 * Invariant check: sb_str_buf_check
 */
typedef struct {
  char *data;
  size_t len;
  size_t cap;
} SbStrBuf;

/*
 * pre:
 *  - str != NULL
 */
size_t sb_str_cstr_safe_strlen(const char *str, size_t max_len);

/*
 * pre:
 *  - b != NULL
 *  - if data = NULL then len = cap = 0 and reverse also
 *  - cap - 1 >= len
 *  - data == NULL || data[len] == '\0'
 */
void sb_str_buf_check(const SbStrBuf *b);

/*
 * pre:
 *  - b != NULL
 * post:
 *  - SIZE_MAX - extra >= len + 1
 *  - cap > len + extra
 * inv: len
 */
bool sb_str_buf_reserve(SbStrBuf *b, size_t extra);

/*
 * pre: b != NULL
 * post: len = 0, data[len] = data[0] = '\0'
 */
bool sb_str_buf_clear(SbStrBuf *b);

/*
 * post:
 *  - data = NULL
 *  - len = cap = 0
 */
bool sb_str_buf_free(SbStrBuf *b);

/*
 * post: if n > 0 then cap >= n+1
 */
bool sb_str_buf_init(SbStrBuf *b, size_t n);

/*
 * pre: b != NULL
 */
bool sb_str_buf_push(SbStrBuf *b, char c);

/*
 * pre:
 *  - v != NULL && str != NULL
 * post: v->len == strlen(str)
 */
bool sb_str_view_from_cstr(SbStrView *v, const char *str);

/*
 * pre:
 *  - str != NULL
 * post: v->len == min(strlen(str), limit),
 *       where limit = (max_len == 0) ? SB_STR_MAX_LEN : max_len
 */
bool sb_str_view_from_bytes(SbStrView *v, const char *str, size_t max_len);

/**
 * pre:  b != NULL
 * post: if start > b->len, then returns "" and truncated = true
 *       if len > b->len || b->len - len < start then truncated = true
 *       else truncated = false
 * inv:  returned pointer is invalid after
 *       reserve / push / append_* / free calls
 */
bool sb_str_view_from_buf(const SbStrBuf *b, SbStrView *v, size_t start,
                          size_t len, bool *is_truncated);
/*
 * pre: b != NULL
 * post:
 *  - if success is true,  then b len = v len + b len
 *  - if success is false, then b len = b len
 */
bool sb_str_buf_append_view(SbStrBuf *b, SbStrView v);

/*
 * pre:
 *  - b != NULL
 * post:
 *  - if success != NULL, then success out-param
 *  - success indicates of getting char
 */
char sb_str_buf_char_at(const SbStrBuf *b, size_t i, bool *success);

bool sb_str_view_equals(SbStrView str1, SbStrView str2);
SbStrView sb_str_view_trim_l(SbStrView str);
SbStrView sb_str_view_trim_r(SbStrView str);
SbStrView sb_str_view_trim_both(SbStrView str);

/*
 * pre: str != NULL
 */
bool sb_str_buf_append_cstr(SbStrBuf *b, const char *str);

/*
 * pre:
 *  - str != NULL
 *  - str NUL terminated, guaranteed by caller
 */
bool sb_str_buf_from_cstr(SbStrBuf *b, const char *str);

/*
 * pre: -
 * post: NUL terminated C string; if b == NULL or b->data == NULL,
 *       then returns ""
 * inv: len unchanged
 *      data[0..len] content is unchanged
 *      data POINTER may change (realloc) -> every previously
 *      outputted pointer/view invalid
 */
const char *sb_str_buf_to_cstr(const SbStrBuf *b);
void sb_str_view_print(SbStrView v);

#endif
