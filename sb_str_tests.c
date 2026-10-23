#include "sb_str.h"
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_sb_str_cstr_safe_strlen() {
  const char *str = "abcde";
  assert(0 == sb_str_cstr_safe_strlen(NULL, 10));

  size_t len = sb_str_cstr_safe_strlen(str, 100);
  assert(5 == len);

  len = sb_str_cstr_safe_strlen(str, 3);
  assert(3 == len);
}

void test_sb_str_buf_reserve() {
  size_t extra = 10;
  SbStrBuf b = (SbStrBuf){0};
  sb_str_buf_reserve(&b, extra);
  assert(b.data != NULL);
  assert(b.cap == 16);

  extra = SIZE_MAX - 10;
  SbStrBuf b2 = (SbStrBuf){0};
  assert(sb_str_buf_reserve(&b2, extra) == false);
  assert(b2.data == NULL && b2.cap == 0);

  char *p = (char *)realloc(b2.data, 16);
  b2.data = p;
  const char *str = "abc";
  size_t len = sb_str_cstr_safe_strlen(str, SIZE_MAX);
  b2.len = 0;
  b2.cap = 16;
  memmove(b.data + b.len, str, len);
  b2.len = len;
  b2.data[b2.len] = '\0';
  assert(b2.data != NULL && b2.cap == 16 && b2.len == 3);

  // empty allocation on data is enough
  assert(sb_str_buf_reserve(&b2, 10) == true);
  assert(b2.data != NULL && b2.cap == 16 && b2.len == 3);

  // empty allocation on data is not enough
  assert(sb_str_buf_reserve(&b2, 18) == true);
  assert(b2.data != NULL && b2.cap == 32 && b2.len == 3);
}

void test_sb_str_buf_clear() {
  size_t extra = 10;
  SbStrBuf b = (SbStrBuf){0};
  sb_str_buf_reserve(&b, extra);
  sb_str_buf_clear(&b);
  assert(b.len == 0 && b.data[b.len] == '\0' && b.cap == 16);
}

void test_sb_str_buf_free() {
  SbStrBuf b = (SbStrBuf){0};
  sb_str_buf_reserve(&b, 32);
  sb_str_buf_free(&b);
  assert(b.data == NULL && b.len == 0 && b.cap == 0);
}

void test_sb_str_buf_init() {
  SbStrBuf b = (SbStrBuf){0};
  sb_str_buf_init(&b, 32);
  assert(b.data != NULL && b.len == 0 && b.cap == 64);
  assert(sb_str_buf_init(NULL, 32) == false);
}

void test_sb_str_buf_push() {
  SbStrBuf b = (SbStrBuf){0};
  sb_str_buf_init(&b, 31);
  sb_str_buf_push(&b, 'A');
  assert(sb_str_buf_push(NULL, 'A') == false);
  assert(b.data != NULL && b.len == 1 && b.cap == 32 && b.data[b.len] == '\0');
}

void test_sb_str_view_from_cstr() {
  assert(sb_str_view_from_cstr(NULL, NULL) == false);

  assert(sb_str_view_from_cstr(NULL, "valami") == false);
  SbStrView v = (SbStrView){0};
  assert(sb_str_view_from_cstr(&v, "valami") == true);
  assert(v.len == 6);
}

void test_sb_str_view_from_bytes() {
  assert(sb_str_view_from_cstr(NULL, NULL) == false);

  assert(sb_str_view_from_bytes(NULL, "valami", SIZE_MAX) == false);
  SbStrView v = (SbStrView){0};
  assert(sb_str_view_from_bytes(&v, "valami", SIZE_MAX) == true);
  assert(v.len == 6);

  assert(sb_str_view_from_bytes(&v, "valami", 4) == true);
  assert(v.len == 4);
}

void test_sb_str_view_from_buf() {
  SbStrBuf b = (SbStrBuf){0};
  const char *src = "valami";
  size_t len = sb_str_cstr_safe_strlen(src, SIZE_MAX);
  bool is_truncated;
  for (size_t i = 0; i < len; i++) {
    assert(sb_str_buf_push(&b, src[i]) == true);
  }
  assert(b.data[b.len] == '\0' && b.len == len && b.cap == 16);

  // requested view wholly inside buffer
  SbStrView v = (SbStrView){0};
  sb_str_view_from_buf(&b, &v, 0, 3, &is_truncated);
  assert(v.len == 3 && v.data[0] == 'v' && is_truncated == false);

  // requested view cannot be served
  sb_str_view_from_buf(&b, &v, 3, 4, &is_truncated);
  assert(v.len == 3 && v.data[0] == 'a' && v.data[2] == 'i' &&
         is_truncated == true);
}

void test_sb_str_buf_append_view() {
  SbStrView v = (SbStrView){0};
  const char *src = "valami";
  bool is_truncated;
  sb_str_view_from_cstr(&v, src);

  SbStrBuf b = (SbStrBuf){0};
  assert(sb_str_buf_append_view(NULL, v) == false);
  assert(sb_str_buf_append_view(&b, v) == true);
  assert(b.len == 6 && b.cap == 16 && b.data[b.len] == '\0');

  // append other distinct view
  SbStrView v2 = (SbStrView){0};
  src = "test";
  sb_str_view_from_cstr(&v2, src);
  assert(sb_str_buf_append_view(&b, v2) == true);
  assert(b.data[6] == 't' && b.len == 10);

  // append self view
  sb_str_view_from_buf(&b, &v, 0, (size_t)6, &is_truncated);
  assert(sb_str_buf_append_view(&b, v) == true);
  assert(b.data[10] == 'v' && b.len == 16 && b.cap == 32);
}

void test_sb_str_buf_char_at() {
  SbStrView v = (SbStrView){0};
  const char *src = "test";
  sb_str_view_from_cstr(&v, src);

  SbStrBuf b = (SbStrBuf){0};
  sb_str_buf_append_view(&b, v);
  bool success;
  assert(sb_str_buf_char_at(&b, 0, &success) == 't' && success == true);
  assert(sb_str_buf_char_at(&b, 4, &success) == '\0' && success == false);
  assert(sb_str_buf_char_at(&b, 5, &success) == '\0' && success == false);
}

void test_sb_str_view_equals() {
  SbStrView v1 = (SbStrView){0};
  SbStrView v2 = (SbStrView){0};
  const char *src = "valami";
  const char *src2 = "val";
  sb_str_view_from_cstr(&v1, src);
  sb_str_view_from_cstr(&v2, src);
  assert(sb_str_view_equals(v1, v2) == true);
  sb_str_view_from_cstr(&v2, src2);
  assert(sb_str_view_equals(v1, v2) == false);
}

void test_sb_str_view_trim_l() {
  SbStrView v = (SbStrView){0};
  SbStrView v2 = (SbStrView){0};
  const char *src = "\tvalami";
  const char *src2 = "   valami";
  sb_str_view_from_cstr(&v, src);
  sb_str_view_from_cstr(&v2, src2);
  assert(v.len == 7);
  SbStrView res_v1 = sb_str_view_trim_l(v);
  SbStrView res_v2 = sb_str_view_trim_l(v2);
  assert(res_v1.len == 6 && res_v1.data[0] == 'v');
  assert(res_v2.len == 6 && res_v2.data[0] == 'v');
}

void test_sb_str_view_trim_r() {
  SbStrView v = (SbStrView){0};
  SbStrView v2 = (SbStrView){0};
  const char *src = "valami\t";
  const char *src2 = " valami   ";
  sb_str_view_from_cstr(&v, src);
  sb_str_view_from_cstr(&v2, src2);
  assert(v.len == 7);
  SbStrView res_v1 = sb_str_view_trim_r(v);
  SbStrView res_v2 = sb_str_view_trim_r(v2);
  assert(res_v1.len == 6 && res_v1.data[0] == 'v');
  assert(res_v2.len == 7 && res_v2.data[0] == ' ');
}

void test_sb_str_view_trim_both() {
  SbStrView v = (SbStrView){0};
  const char *src = "\tvalami   ";
  sb_str_view_from_cstr(&v, src);
  assert(v.len == 10 && v.data[0] == '\t' && v.data[v.len - 1] == ' ');

  SbStrView res_both = sb_str_view_trim_both(v);
  assert(res_both.len == 6 && res_both.data[0] == 'v' &&
         res_both.data[res_both.len - 1] == 'i');
}

void test_sb_str_buf_append_cstr() {
  SbStrBuf b = (SbStrBuf){0};
  const char *str_null = NULL;
  const char *str_valid = "valid";
  bool success;
  assert(sb_str_buf_append_cstr(&b, str_null) == false);
  assert(sb_str_buf_append_cstr(&b, str_valid) == true);

  SbStrView base = (SbStrView){0};
  sb_str_view_from_cstr(&base, str_valid);

  SbStrView check = (SbStrView){0};
  sb_str_view_from_buf(&b, &check, 0, SIZE_MAX, &success);

  assert(sb_str_view_equals(base, check) == true);
}

void test_sb_str_buf_from_cstr() {
  SbStrView v = (SbStrView){0};
  sb_str_view_from_cstr(&v, "valami");
  SbStrBuf b = (SbStrBuf){0};
  sb_str_buf_append_view(&b, v);

  assert(sb_str_buf_from_cstr(&b, "mas") == true);
  assert(b.len == 3 && b.data[0] == 'm' && b.data[b.len] == '\0');
}

void test_sb_str_buf_to_cstr() {
  SbStrBuf b;
  sb_str_buf_init(&b, 0);
  const char *src = "valami";
  sb_str_buf_append_cstr(&b, src);
  const char *str = sb_str_buf_to_cstr(&b);

  assert(strncmp(src, str, 6) == 0);
  assert(sb_str_cstr_safe_strlen(sb_str_buf_to_cstr(NULL), 100) == (size_t)0);
}

int main() {
  test_sb_str_cstr_safe_strlen();
  test_sb_str_buf_reserve();
  test_sb_str_buf_clear();
  test_sb_str_buf_free();
  test_sb_str_buf_init();
  test_sb_str_buf_push();
  test_sb_str_view_from_cstr();
  test_sb_str_view_from_bytes();
  test_sb_str_view_from_buf();
  test_sb_str_buf_append_view();
  test_sb_str_buf_char_at();
  test_sb_str_view_equals();
  test_sb_str_view_trim_l();
  test_sb_str_view_trim_r();
  test_sb_str_view_trim_both();
  test_sb_str_buf_append_cstr();
  test_sb_str_buf_from_cstr();
  test_sb_str_buf_to_cstr();
}
