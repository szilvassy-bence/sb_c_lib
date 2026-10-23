#include "sb_int.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* *******************
 * Tests
 ********************/

/* *******************
 * Test of operations int32_t
 ********************/

void test_add_i32(void) {
  bool overflow;
  int32_t result;

  // normal case
  result = sb_int_add_i32(4, 6, &overflow);
  assert(result == 10);
  assert(overflow == false);

  // max value
  result = sb_int_add_i32(1, (INT32_MAX - 1), &overflow);
  assert(result == INT32_MAX);
  assert(overflow == false);

  // min value
  result = sb_int_add_i32(-1, (INT32_MIN + 1), &overflow);
  assert(result == INT32_MIN);
  assert(overflow == false);

  // overflow
  result = sb_int_add_i32(1, INT32_MAX, &overflow);
  assert(result == INT32_MAX);
  assert(overflow == true);

  // underflow
  result = sb_int_add_i32(-1, INT32_MIN, &overflow);
  assert(result == INT32_MIN);
  assert(overflow == true);
}

void test_sub_i32(void) {
  bool overflow;
  int32_t result;

  // normal case
  result = sb_int_sub_i32(4, 6, &overflow);
  assert(result == -2);
  assert(overflow == false);

  // min value
  result = sb_int_sub_i32(-1, INT32_MAX, &overflow);
  assert(result == INT32_MIN);
  assert(overflow == false);

  // underflow
  result = sb_int_sub_i32(-2, INT32_MAX, &overflow);
  assert(result == INT32_MIN);
  assert(overflow == true);

  // max value
  result = sb_int_sub_i32(-1, INT32_MIN, &overflow);
  assert(result == INT32_MAX);
  assert(overflow == false);

  // overflow
  result = sb_int_sub_i32(0, (INT32_MIN), &overflow);
  assert(result == INT32_MAX);
  assert(overflow == true);
}

void test_mul_i32() {
  int32_t result;
  bool overflow;

  result = sb_int_mul_i32(0, 1, &overflow);
  assert(result == 0);
  assert(overflow == false);

  result = sb_int_mul_i32(4, 0, &overflow);
  assert(result == 0);
  assert(overflow == false);

  // Min overflows when multiplied by -1
  result = sb_int_mul_i32(INT32_MIN, -1, &overflow);
  assert(result == INT32_MAX);
  assert(overflow == true);

  // a positive, b positive, overflows
  result = sb_int_mul_i32(INT32_MAX / 2, 3, &overflow);
  assert(result == INT32_MAX);
  assert(overflow == true);

  // a negative, b negative, overflows
  result = sb_int_mul_i32(INT32_MAX / -2, -3, &overflow);
  assert(result == INT32_MAX);
  assert(overflow == true);

  // a positive, b negative, overflows
  result = sb_int_mul_i32(INT32_MAX / 2, -3, &overflow);
  assert(result == INT32_MIN);
  assert(overflow == true);

  // a negative, b positive, overflows
  result = sb_int_mul_i32(INT32_MAX / -2, 3, &overflow);
  assert(result == INT32_MIN);
  assert(overflow == true);

  // a positive, b positive, does not overflow
  result = sb_int_mul_i32(500, 1000, &overflow);
  assert(result == 500000);
  assert(overflow == false);

  // a negative, b positive, does not overflow
  result = sb_int_mul_i32(-500, 1000, &overflow);
  assert(result == -500000);
  assert(overflow == false);
}

void test_div_i32() {
  int32_t result;
  bool overflow, success;

  // division by 0
  result = sb_int_div_i32(1, 0, &overflow, &success);
  assert(result == 0);
  assert(overflow == false);
  assert(success == false);

  // overflow
  result = sb_int_div_i32(INT32_MIN, -1, &overflow, &success);
  assert(result == INT32_MAX);
  assert(overflow == true);
  assert(success == true);

  // valid
  result = sb_int_div_i32(500, 5, &overflow, &success);
  assert(result == 100);
  assert(overflow == false);
  assert(success == true);

  // valid
  result = sb_int_div_i32(500, -5, &overflow, &success);
  assert(result == -100);
  assert(overflow == false);
  assert(success == true);
}

/* *******************
 * Test of operations uint32_t
 ********************/

void test_add_u32() {
  uint32_t result;
  bool overflow;

  // overflows
  result = sb_int_add_u32(UINT32_MAX, 1, &overflow);
  assert(result == UINT32_MAX);
  assert(overflow == true);

  // normal
  result = sb_int_add_u32(1000, 1, &overflow);
  assert(result == 1001);
  assert(overflow == false);
}

void test_sub_u32() {
  uint32_t result;
  bool overflow;

  // underflows
  result = sb_int_sub_u32(1, 10, &overflow);
  assert(result == 0);
  assert(overflow == true);

  // normal
  result = sb_int_sub_u32(1000, 1, &overflow);
  assert(result == 999);
  assert(overflow == false);
}

void test_mul_u32() {
  uint32_t result;
  bool overflow;

  // underflows
  result = sb_int_mul_u32(UINT32_MAX / 2, 3, &overflow);
  assert(result == UINT32_MAX);
  assert(overflow == true);

  // normal
  result = sb_int_mul_u32(1000, 33, &overflow);
  assert(result == 33000);
  assert(overflow == false);
}

void test_div_u32() {
  uint32_t result;
  bool overflow, success;

  // underflows
  result = sb_int_div_u32(10, 0, &overflow, &success);
  assert(result == 0);
  assert(overflow == false);
  assert(success == false);

  // normal
  result = sb_int_div_u32(1000, 33, &overflow, &success);
  assert(result == 30);
  assert(overflow == false);
  assert(success == true);
}

/* *******************
 * Test of conversions
 ********************/

void test_i16_to_i8(void) {
  bool truncated;
  int16_t result;

  // normal case
  result = sb_int_i16_to_i8(120, &truncated);
  assert(result == 120);
  assert(truncated == false);

  // overflow
  result = sb_int_i16_to_i8((INT8_MAX + 1), &truncated);
  assert(result == INT8_MAX);
  assert(truncated == true);

  // underflow
  result = sb_int_i16_to_i8((INT8_MIN - 1), &truncated);
  assert(result == INT8_MIN);
  assert(truncated == true);

  // exactly on limit
  result = sb_int_i16_to_i8(INT8_MIN, &truncated);
  assert(result == INT8_MIN);
  assert(truncated == false);

  // exactly on limit
  result = sb_int_i16_to_i8(INT8_MAX, &truncated);
  assert(result == INT8_MAX);
  assert(truncated == false);
}

void test_i32_to_i8(void) {
  bool truncated;
  int16_t result;

  // normal case
  result = sb_int_i32_to_i8(120, &truncated);
  assert(result == 120);
  assert(truncated == false);

  // overflow
  result = sb_int_i32_to_i8((INT8_MAX + 1), &truncated);
  assert(result == INT8_MAX);
  assert(truncated == true);

  // underflow
  result = sb_int_i32_to_i8((INT8_MIN - 1), &truncated);
  assert(result == INT8_MIN);
  assert(truncated == true);

  // exactly on limit
  result = sb_int_i32_to_i8(INT8_MIN, &truncated);
  assert(result == INT8_MIN);
  assert(truncated == false);

  // exactly on limit
  result = sb_int_i32_to_i8(INT8_MAX, &truncated);
  assert(result == INT8_MAX);
  assert(truncated == false);
}

void test_i64_to_i8(void) {
  bool truncated;
  int16_t result;

  // normal case
  result = sb_int_i64_to_i8(120, &truncated);
  assert(result == 120);
  assert(truncated == false);

  // overflow
  result = sb_int_i64_to_i8((INT8_MAX + 1), &truncated);
  assert(result == INT8_MAX);
  assert(truncated == true);

  // underflow
  result = sb_int_i64_to_i8((INT8_MIN - 1), &truncated);
  assert(result == INT8_MIN);
  assert(truncated == true);

  // exactly on limit
  result = sb_int_i64_to_i8(INT8_MIN, &truncated);
  assert(result == INT8_MIN);
  assert(truncated == false);

  // exactly on limit
  result = sb_int_i64_to_i8(INT8_MAX, &truncated);
  assert(result == INT8_MAX);
  assert(truncated == false);
}

void test_i32_to_i16(void) {
  bool truncated;
  int16_t result;

  // normal case
  result = sb_int_i32_to_i16(342, &truncated);
  assert(result == 342);
  assert(truncated == false);

  // overflow
  result = sb_int_i32_to_i16((INT16_MAX + 1), &truncated);
  assert(result == INT16_MAX);
  assert(truncated == true);

  // underflow
  result = sb_int_i32_to_i16((INT16_MIN - 1), &truncated);
  assert(result == INT16_MIN);
  assert(truncated == true);

  // exactly on limit
  result = sb_int_i32_to_i16(INT16_MIN, &truncated);
  assert(result == INT16_MIN);
  assert(truncated == false);
  // exactly on limit
  result = sb_int_i32_to_i16(INT16_MAX, &truncated);
  assert(result == INT16_MAX);
  assert(truncated == false);
}

void test_i64_to_i16(void) {
  bool truncated;
  int16_t result;

  // normal case
  result = sb_int_i64_to_i16(342, &truncated);
  assert(result == 342);
  assert(truncated == false);

  // overflow
  result = sb_int_i64_to_i16((INT16_MAX + 1), &truncated);
  assert(result == INT16_MAX);
  assert(truncated == true);

  // underflow
  result = sb_int_i64_to_i16((INT16_MIN - 1), &truncated);
  assert(result == INT16_MIN);
  assert(truncated == true);

  // exactly on limit
  result = sb_int_i64_to_i16(INT16_MIN, &truncated);
  assert(result == INT16_MIN);
  assert(truncated == false);

  // exactly on limit
  result = sb_int_i64_to_i16(INT16_MAX, &truncated);
  assert(result == INT16_MAX);
  assert(truncated == false);
}

void test_i64_to_i32() {
  bool truncated;
  int32_t result;

  // normal case
  result = sb_int_i64_to_i32((int64_t)230, &truncated);
  assert(result == 230);
  assert(truncated == false);

  // overflow
  result = sb_int_i64_to_i32((int64_t)INT32_MAX + 1, &truncated);
  assert(result == INT32_MAX);
  assert(truncated == true);

  // underflow
  result = sb_int_i64_to_i32((int64_t)INT32_MIN - 1, &truncated);
  assert(result == INT32_MIN);
  assert(truncated == true);

  // exactly on limit
  result = sb_int_i64_to_i32((int64_t)INT32_MIN, &truncated);
  assert(result == INT32_MIN);
  assert(truncated == false);

  // exactly on limit
  result = sb_int_i64_to_i32((int64_t)INT32_MAX, &truncated);
  assert(result == INT32_MAX);
  assert(truncated == false);
}

void test_u16_to_u8() {
  uint8_t result;
  bool truncated;

  result = sb_int_u16_to_u8(230, &truncated);
  assert(result == 230);
  assert(truncated == false);

  result = sb_int_u16_to_u8(UINT8_MAX + 1, &truncated);
  assert(result == UINT8_MAX);
  assert(truncated == true);
}

void test_u32_to_u8() {
  uint8_t result;
  bool truncated;

  result = sb_int_u32_to_u8(230, &truncated);
  assert(result == 230);
  assert(truncated == false);

  result = sb_int_u32_to_u8(UINT8_MAX + 1, &truncated);
  assert(result == UINT8_MAX);
  assert(truncated == true);
}

void test_u64_to_u8() {
  uint8_t result;
  bool truncated;

  result = sb_int_u64_to_u8(230, &truncated);
  assert(result == 230);
  assert(truncated == false);

  result = sb_int_u64_to_u8(UINT8_MAX + 1, &truncated);
  assert(result == UINT8_MAX);
  assert(truncated == true);
}

void test_u32_to_u16() {
  uint16_t result;
  bool truncated;

  result = sb_int_u32_to_u16(UINT16_MAX, &truncated);
  assert(result == UINT16_MAX);
  assert(truncated == false);

  result = sb_int_u32_to_u16(UINT16_MAX + 1, &truncated);
  assert(result == UINT16_MAX);
  assert(truncated == true);
}

void test_u64_to_u16() {
  uint16_t result;
  bool truncated;

  result = sb_int_u64_to_u16(UINT16_MAX, &truncated);
  assert(result == UINT16_MAX);
  assert(truncated == false);

  result = sb_int_u64_to_u16(UINT16_MAX + 1, &truncated);
  assert(result == UINT16_MAX);
  assert(truncated == true);
}

void test_u64_to_u32() {
  uint32_t result;
  bool truncated;

  result = sb_int_u64_to_u32(UINT32_MAX, &truncated);
  assert(result == UINT32_MAX);
  assert(truncated == false);

  result = sb_int_u64_to_u32((uint64_t)UINT32_MAX + 1, &truncated);
  assert(result == UINT32_MAX);
  assert(truncated == true);
}

void test_i8_to_u8() {
  int8_t a;
  bool truncated;
  uint8_t result;

  a = INT8_MIN;
  result = sb_int_i8_to_u8(a, &truncated);
  assert(result == 0);
  assert(truncated == true);

  a = -1;
  result = sb_int_i8_to_u8(a, &truncated);
  assert(result == 0);
  assert(truncated == true);

  a = 0;
  result = sb_int_i8_to_u8(a, &truncated);
  assert(result == 0);
  assert(truncated == false);

  a = INT8_MAX;
  result = sb_int_i8_to_u8(a, &truncated);
  assert(result == (uint8_t)INT8_MAX);
  assert(truncated == false);
}

void test_i16_to_u8() {
  int16_t a;
  bool truncated;
  uint8_t result;

  a = INT16_MIN;
  result = sb_int_i16_to_u8(a, &truncated);
  assert(result == 0);
  assert(truncated == true);

  a = -1;
  result = sb_int_i16_to_u8(a, &truncated);
  assert(result == 0);
  assert(truncated == true);

  a = 0;
  result = sb_int_i16_to_u8(a, &truncated);
  assert(result == 0);
  assert(truncated == false);

  a = (int16_t)UINT8_MAX + 1;
  result = sb_int_i16_to_u8(a, &truncated);
  assert(result == UINT8_MAX);
  assert(truncated == true);

  a = INT16_MAX;
  result = sb_int_i16_to_u8(a, &truncated);
  assert(result == UINT8_MAX);
  assert(truncated == true);

  a = (int16_t)UINT8_MAX;
  result = sb_int_i16_to_u8(a, &truncated);
  assert(result == UINT8_MAX);
  assert(truncated == false);
}

void test_i32_to_u8() {
  int32_t a;
  bool truncated;
  uint8_t result;

  a = INT32_MIN;
  result = sb_int_i32_to_u8(a, &truncated);
  assert(result == 0);
  assert(truncated == true);

  a = -1;
  result = sb_int_i32_to_u8(a, &truncated);
  assert(result == 0);
  assert(truncated == true);

  a = 0;
  result = sb_int_i32_to_u8(a, &truncated);
  assert(result == 0);
  assert(truncated == false);

  a = (int32_t)UINT8_MAX + 1;
  result = sb_int_i32_to_u8(a, &truncated);
  assert(result == UINT8_MAX);
  assert(truncated == true);

  a = INT32_MAX;
  result = sb_int_i32_to_u8(a, &truncated);
  assert(result == UINT8_MAX);
  assert(truncated == true);

  a = (int32_t)UINT8_MAX;
  result = sb_int_i32_to_u8(a, &truncated);
  assert(result == UINT8_MAX);
  assert(truncated == false);
}

int main() {
  test_add_i32();
  test_sub_i32();
  test_mul_i32();
  test_div_i32();
  test_add_u32();
  test_sub_u32();
  test_mul_u32();
  test_div_u32();
  test_i16_to_i8();
  test_i32_to_i8();
  test_i64_to_i8();
  test_i32_to_i16();
  test_i64_to_i16();
  test_i64_to_i32();
  test_u16_to_u8();
  test_u32_to_u8();
  test_u64_to_u8();
  test_u32_to_u16();
  test_u64_to_u16();
  test_u64_to_u32();
  test_i8_to_u8();
  test_i16_to_u8();
  test_i32_to_u8();
  printf("sb_int lib tests ran successfully\n");
}
