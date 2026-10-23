#include "sb_str.h"
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SB_STR_MAX_LEN SIZE_MAX

/*
 * pre:
 *  - str is a valid pointer
 *  - max_len >= 0
 * returns length of the string or max length
 * extra info returned size_t < SB_STR_MAX_LEN
 */
size_t sb_str_cstr_safe_strlen(const char *str, size_t max_len) {
    if (str == NULL) {
        return 0;
    }
    for (size_t i = 0; i < max_len; i++) {
        if (str[i] == '\0')
            return i;
    }
    return max_len;
}

void sb_str_buf_check(const SbStrBuf *b) {
    assert(b != NULL);
    assert((b->data == NULL) == (b->len == 0 && b->cap == 0));
    assert(b->data == NULL || b->len < b->cap);
    // thus b->len < SB_STR_MAX_LEN
    assert((b->data == NULL) || (b->data[b->len] == '\0'));
}

bool sb_str_buf_reserve(SbStrBuf *b, size_t extra) {
    if (b == NULL)
        return false;
    sb_str_buf_check(b);

    // len < SIZE_MAX (invariant) => SIZE_MAX - len - 1 cannot
    // underflow
    if (extra > SIZE_MAX - b->len - 1)
        return false;
    size_t need = extra + b->len + 1;
    if (need <= b->cap)
        return true;

    size_t new_cap = b->cap ? b->cap : 16;
    while (new_cap < need) {
        if (new_cap > SIZE_MAX / 2) {
            new_cap = need;
            break;
        }
        new_cap *= 2;
    }

    char *p = (char *)realloc(b->data, new_cap);
    if (p == NULL)
        return false;

    b->data = p;
    b->cap = new_cap;
    b->data[b->len] = '\0';
    sb_str_buf_check(b);
    return true;
}

bool sb_str_buf_clear(SbStrBuf *b) {
    if (b == NULL)
        return false;
    sb_str_buf_check(b);
    b->len = 0;
    if (b->data)
        b->data[0] = '\0';
    sb_str_buf_check(b);
    return true;
}

bool sb_str_buf_free(SbStrBuf *b) {
    if (b == NULL)
        return false;
    sb_str_buf_check(b);
    free(b->data);
    b->data = NULL;
    b->len = b->cap = 0;
    sb_str_buf_check(b);
    return true;
}

bool sb_str_buf_init(SbStrBuf *b, size_t n) {
    if (b == NULL)
        return false;
    *b = (SbStrBuf){0};
    return n == 0 || sb_str_buf_reserve(b, n);
}

bool sb_str_buf_push(SbStrBuf *b, char c) {
    if (b == NULL)
        return false;
    sb_str_buf_check(b);
    if (!sb_str_buf_reserve(b, 1))
        return false;
    b->data[b->len++] = c;
    b->data[b->len] = '\0';
    sb_str_buf_check(b);
    return true;
}

bool sb_str_view_from_cstr(SbStrView *v, const char *str) {
    if (str == NULL || v == NULL)
        return false;
    v->data = str;
    v->len = strlen(str);
    return true;
}

bool sb_str_view_from_bytes(SbStrView *v, const char *str, size_t max_len) {
    if (str == NULL || v == NULL)
        return false;
    v->data = str;
    v->len = sb_str_cstr_safe_strlen(str, max_len);
    return true;
}

bool sb_str_view_from_buf(const SbStrBuf *b, SbStrView *v, size_t start,
                          size_t len, bool *is_truncated) {
    if (b == NULL || v == NULL || is_truncated == NULL)
        return false;
    sb_str_buf_check(b);
    if (b->data == NULL) {
        *v = (SbStrView){0};
        *is_truncated = false;
        return true;
    }
    if (start > b->len)
        start = b->len;
    size_t available = b->len - start;
    if (len > available) {
        len = available;
        *is_truncated = true;
    } else {
        *is_truncated = false;
    }
    *v = (SbStrView){.data = b->data + start, .len = len};
    return true;
}

/*
 * offset based append
 */
bool sb_str_buf_append_view(SbStrBuf *b, SbStrView v) {
    if (b == NULL)
        return false;
    sb_str_buf_check(b);
    if (v.len == 0)
        return true;
    // flat address space pointer comparison
    bool self =
        b->data != NULL && b->data <= v.data && b->data + b->cap > v.data;
    size_t off = self ? (size_t)(v.data - b->data) : 0;

    if (!sb_str_buf_reserve(b, v.len))
        return false;

    const char *src = self ? b->data + off : v.data;
    memmove(b->data + b->len, src, v.len);
    b->len += v.len;
    b->data[b->len] = '\0';

    sb_str_buf_check(b);
    return true;
}

char sb_str_buf_char_at(const SbStrBuf *b, size_t i, bool *success) {
    if (b == NULL || i >= b->len) {
        if (success != NULL)
            *success = false;
        return '\0';
    }
    if (success != NULL)
        *success = true;
    return b->data[i];
}

bool sb_str_view_equals(SbStrView str1, SbStrView str2) {
    if (str1.len != str2.len) {
        return false;
    }
    for (size_t i = 0; i < str1.len; i++) {
        if (*(str1.data + i) != *(str2.data + i)) {
            return false;
        }
    }
    return true;
}

SbStrView sb_str_view_trim_l(SbStrView str) {
    size_t i = 0;
    while (i < str.len && isspace((unsigned char)*(str.data + i))) {
        i++;
    }
    return (SbStrView){.data = str.data + i, .len = str.len - i};
}

SbStrView sb_str_view_trim_r(SbStrView str) {
    size_t i = 0;
    while (i < str.len &&
           isspace((unsigned char)*(str.data + str.len - 1 - i))) {
        i++;
    }
    return (SbStrView){.data = str.data, .len = str.len - i};
}

SbStrView sb_str_view_trim_both(SbStrView str) {
    SbStrView str_l = sb_str_view_trim_l(str);
    return sb_str_view_trim_r(str_l);
}

bool sb_str_buf_append_cstr(SbStrBuf *b, const char *str) {
    if (b == NULL || str == NULL)
        return false;
    SbStrView v = (SbStrView){0};
    if (sb_str_view_from_cstr(&v, str))
        return sb_str_buf_append_view(b, v);
    return false;
}

bool sb_str_buf_from_cstr(SbStrBuf *b, const char *str) {
    if (str == NULL)
        return false;
    if (sb_str_buf_clear(b)) {
        return sb_str_buf_append_cstr(b, str);
    }
    return false;
}

const char *sb_str_buf_to_cstr(const SbStrBuf *b) {
    return b == NULL || b->data == NULL ? "" : b->data;
}

void sb_str_view_print(SbStrView v) { fwrite(v.data, 1, v.len, stdout); }
