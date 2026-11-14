#ifndef SB_INT_H
#define SB_INT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef ptrdiff_t isize;
#define ISIZE_MAX PTRDIFF_MAX
#define ISIZE_MIN PTRDIFF_MIN

_Static_assert(PTRDIFF_MAX <= SIZE_MAX, "isize must fit in size_t");

// SB SIGNED OPERATIONS

/*
 * function: sb_int_add_i32
 * inputs: a and b (32-bit signed integer), overflow bool pointer
 * outputs: result (32-bit signed integer), overflow (boolean)
 *
 * precondition:
 *  - overflow != NULL
 *
 * postconditions:
 *  - if a + b is representable as int32_t:
 *      result = a + b
 *      overflow = false
 *  - if a + b > INT32_MAX:
 *      result = INT32_MAX
 *      overflow = true
 *  - if a + b < INT32_MAX:
 *      result = INT32_MIN
 *      overflow = true
 */
int32_t sb_int_add_i32(int32_t a, int32_t b, bool *overflow);

/*
 * function: sb_int_sub_i32
 * inputs: a and b (32-bit signed integer), overflow bool pointer
 * outputs: result (32-bit signed integer)
 *
 * postconditions:
 *  - if a - b is representable as int32_t:
 *      result = a - b
 *      overflow = false
 *  - if a - b > INT32_MAX:
 *      result = INT32_MAX
 *      overflow = true
 *  - if a - b < INT32_MIN:
 *      result = INT32_MIN
 *      overflow = true
 */
int32_t sb_int_sub_i32(int32_t a, int32_t b, bool *overflow);

/*
 * function: sb_int_mul_i32
 * inputs: a and b (32-bit signed integer), overflow (bool) pointer
 * outputs: result (32-bit signed integer)
 *
 * precondition:
 *  - overflow != NULL
 *
 * postconditions:
 *  - if a * b is representable as int32_t:
 *      result = a * b
 *      overflow = false
 *  - if a * b > INT32_MAX:
 *      result = INT32_MAX
 *      overflow = true
 *  - if a * b < INT32_MIN:
 *      result = INT32_MIN
 *      overflow = true
 */
int32_t sb_int_mul_i32(int32_t a, int32_t b, bool *overflow);

/*
 * function: sb_int_div_i32
 * inputs: a and b (32-bit signed integer), overflow and success bool pointer
 * outputs: result (32-bit signed integer), overflow (bool)
 *
 * precondition
 *  - overflow != NULL
 *  - success != NULL
 *
 * postconditions
 *  - b == 0:
 *      overflow = false
 *      success = false
 *      result = 0
 *  - a == INT32_MIN && b == -1
 *      result = INT32_MAX
 *      overflow = true
 *      success = true
 *  - a / b is representable as int32_t:
 *      result = a / b
 *      overflow = false
 *      success = true
 */
int32_t sb_int_div_i32(int32_t a, int32_t b, bool *overflow, bool *success);


// SB UNSIGNED OPERATIONS

/*
 * function: sb_int_add_u32
 * inputs: a, b (32-bit unsigned integer)
 * output: result (32-bit unsigned integer), overflow (bool)
 *
 * precondition
 *  - overflow != NULL
 *
 * postconditions:
 *  - a + b > UINT32_MAX, overflow
 *      result = UINT32_MAX
 *      overflow = true
 *  - a + b is representable as unsigned integer
 *      result = a + b
 *      overflow = false
 */
uint32_t sb_int_add_u32(uint32_t a, uint32_t b, bool *overflow);

/*
 * function: sb_int_sub_u32
 * inputs: a, b (32-bit unsigned integer)
 * outputs: result (32-bit unsigned integer), overflow (bool)
 *
 * postconditions:
 *  - a - b < 0, underflows
 *      result = 0
 *      overflow = true
 *  - a - b is representable as uint32_t
 *      result = a - b
 *      overflow = false
 */
uint32_t sb_int_sub_u32(uint32_t a, uint32_t b, bool *overflow);

/*
 * function: sb_int_mul_u32
 * inputs: a, b (32-bit unsigned integer), overflow bool pointer
 * outputs: result (32-bit unsigned integer), overflow (bool)
 *
 * precondition
 *  - overflow != NULL
 *
 * postconditions:
 *  - a == 0 or b == 0
 *      result = 0
 *      overflow = false
 *  - a * b > 0, overflows
 *      result = UINT32_MAX
 *      overflow = true
 *  - a * b is representable as uint32_t
 *      result = a * b
 *      overflow = false
 */
uint32_t sb_int_mul_u32(uint32_t a, uint32_t b, bool *overflow);

/*
 * function: sb_int_div_u32
 * inputs: a, b (32-bit unsigned integer), overflow and success bool pointer
 * outputs: result (32-bit unsigned integer), overflow (bool), success(bool)
 *
 * preconditions:
 *  - overflow != NULL
 *  - success != NULL
 *
 * postcondition:
 *  - b == 0
 *      result = 0
 *      overflow = false
 *      success = false
 */
uint32_t sb_int_div_u32(uint32_t a, uint32_t b, bool *overflow, bool *success);


// SB SIGNED TO SIGNED CONVERSION

/*
 * function: sb_int_i16_to_i8
 * inputs: a (16-bit signed integer), truncated (bool) pointer
 * outputs: 8-bit signed integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - a > INT8_MAX
 *      result = INT8_MAX
 *      truncated = true
 *  - a < INT8_MIN
 *      result = INT8_MIN
 *      truncated = true
 *  - a is representable as 8-bit signed integer
 *      result = a
 *      truncated = false
 */
int8_t sb_int_i16_to_i8(int16_t a, bool *truncated);

/*
 * function: sb_int_i32_to_i8
 * inputs: a (32-bit signed integer), truncated (bool) pointer
 * outputs: 8-bit signed integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - a > INT8_MAX
 *      result = INT8_MAX
 *      truncated = true
 *  - a < INT8_MIN
 *      result = INT8_MIN
 *      truncated = true
 *  - a is representable as 8-bit signed integer
 *      result = a
 *      truncated = false
 */
int8_t sb_int_i32_to_i8(int32_t a, bool *truncated);

/*
 * function: sb_int_i64_to_i8
 * inputs: a (64-bit signed integer), truncated (bool) pointer
 * outputs: 8-bit signed integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - a > INT8_MAX
 *      result = INT8_MAX
 *      truncated = true
 *  - a < INT8_MIN
 *      result = INT8_MIN
 *      truncated = true
 *  - a is representable as 8-bit signed integer
 *      result = a
 *      truncated = false
 */
int8_t sb_int_i64_to_i8(int64_t a, bool *truncated);

/*
 * function: sb_int_i32_to_i16
 * inputs: value (32-bit signed integer)
 * outputs: result (16-bit signed integer), truncated (boolean) pointer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If value is representable as int16_t:
 *      result = value
 *      truncated = false
 *  - If value > INT16_MAX:
 *      result = INT16_MAX
 *      truncated = true
 *  - If value > INT16_MIN:
 *      result = INT16_MIN
 *      truncated = true
 */
int16_t sb_int_i32_to_i16(int32_t value, bool *truncated);

/*
 * function: sb_int_i64_to_i16
 * inputs: value (64-bit signed integer)
 * outputs: result (16-bit signed integer), truncated (boolean) pointer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If value is representable as int16_t:
 *      result = value
 *      truncated = false
 *  - If value > INT16_MAX:
 *      result = INT16_MAX
 *      truncated = true
 *  - If value > INT16_MIN:
 *      result = INT16_MIN
 *      truncated = true
 */
int16_t sb_int_i64_to_i16(int64_t value, bool *truncated);

/*
 * function: sb_int_i64_to_i32
 * inputs: value (64-bit signed integer), truncated bool pointer
 * outputs: result (32-bit signed integer), truncated (boolean)
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If value is representable as int16_t:
 *      result = value
 *      truncated = false
 *  - If value > INT32_MAX:
 *      result = INT32_MAX
 *      truncated = true
 *  - If value > INT32_MIN:
 *      result = INT32_MIN
 *      truncated = true
 */
int32_t sb_int_i64_to_i32(int64_t value, bool *truncated);


// SB UNSIGNED TO UNSIGNED CONVERSION

/*
 * function: sb_int_u16_to_u8
 * inputs: a (16-bit unsigned integer), truncated bool pointer
 * outputs: 8-bit unsigned integer, truncated (bool)
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If value is representable as uint8_t:
 *      result = value
 *      truncated = false
 *  - If value > UINT8_MAX:
 *      result = UINT8_MAX
 *      truncated = true
 */
uint8_t sb_int_u16_to_u8(uint16_t a, bool *truncated);

/*
 * function: sb_int_u32_to_u8
 * inputs: a (32-bit unsigned integer), truncated (bool) pointer
 * outputs: 8-bit unsigned integer, truncated (bool)
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If value is representable as uint8_t:
 *      result = value
 *      truncated = false
 *  - If value > UINT8_MAX:
 *      result = UINT8_MAX
 *      truncated = true
 */
uint8_t sb_int_u32_to_u8(uint32_t a, bool *truncated);

/*
 * function: sb_int_u32_to_u16
 * inputs: a (32-bit unsigned integer), truncated bool pointer
 * outputs: 16-bit unsigned integer, truncated (bool)
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If value is representable as uint16_t:
 *      result = value
 *      truncated = false
 *  - If value > UINT16_MAX:
 *      result = UINT16_MAX
 *      truncated = true
 */
uint16_t sb_int_u32_to_u16(uint32_t a, bool *truncated);

/*
 * function: sb_int_u64_to_u8
 * inputs: a (64-bit unsigned integer), truncated bool pointer
 * outputs: 8-bit unsigned integer, truncated (bool)
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If value is representable as uint8_t:
 *      result = value
 *      truncated = false
 *  - If value > UINT8_MAX:
 *      result = UINT8_MAX
 *      truncated = true
 */
uint8_t sb_int_u64_to_u8(uint64_t a, bool *truncated);

/*
 * function: sb_int_u64_to_u16
 * inputs: a (64-bit unsigned integer), truncated bool pointer
 * outputs: 16-bit unsigned integer, truncated (bool)
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If value is representable as uint16_t:
 *      result = value
 *      truncated = false
 *  - If value > UINT16_MAX:
 *      result = UINT16_MAX
 *      truncated = true
 */
uint16_t sb_int_u64_to_u16(uint64_t a, bool *truncated);

/*
 * function: sb_int_u64_to_u32
 * inputs: a (64-bit unsigned integer), truncated bool pointer
 * outputs: 32-bit unsigned integer, truncated (bool)
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If value is representable as uint32_t:
 *      result = value
 *      truncated = false
 *  - If value > UINT32_MAX:
 *      result = UINT32_MAX
 *      truncated = true
 */
uint32_t sb_int_u64_to_u32(uint64_t a, bool *truncated);


// SB SIGNED TO UNSIGNED CONVERSIONS

/*
 * function: sb_int_i8_to_u8
 * inputs: a (8-bit signed integer), truncated bool pointer
 * outputs: 8-bit unsigned integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If value < 0
 *      result = 0
 *      truncated = true
 *  - If value >= 0
 *      result = a
 *      truncated = false
 */
uint8_t sb_int_i8_to_u8(int8_t a, bool *truncated);

/*
 * function: sb_int_i16_to_u8
 * inputs: a (16-bit signed integer), truncated bool pointer
 * outputs: 8-bit unsigned integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a < 0
 *      result = 0
 *      truncated = true
 *  - If a > UINT8_MAX
 *      result = UINT8_MAX
 *      truncated = true
 *  - If a >= 0 and a <= UINT8_MAX
 *      result = a
 *      truncated = false
 */
uint8_t sb_int_i16_to_u8(int16_t a, bool *truncated);

/*
 * function: sb_int_i16_to_u16
 * inputs: a (16-bit signed integer), truncated bool pointer
 * outputs: 16-bit unsigned integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a < 0
 *      result = 0
 *      truncated = true
 *  - If a >= 0
 *      result = a
 *      truncated = false
 */
uint16_t sb_int_i16_to_u16(int16_t a, bool *truncated);

/*
 * function: sb_int_i32_to_u8
 * inputs: a (32-bit signed integer), truncated bool pointer
 * outputs: 8-bit unsigned integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a < 0
 *      result = 0
 *      truncated = true
 *  - If a > UINT8_MAX
 *      result = UINT8_MAX
 *      truncated = true
 *  - If a >= 0 and a <= UINT8_MAX
 *      result = a
 *      truncated = false
 */
uint8_t sb_int_i32_to_u8(int32_t a, bool *truncated);

/*
 * function: sb_int_i32_to_u16
 * inputs: a (32-bit signed integer), truncated bool pointer
 * outputs: 16-bit unsigned integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a < 0
 *      result = 0
 *      truncated = true
 *  - If a > UINT16_MAX
 *      result = UINT16_MAX
 *      truncated = true
 *  - If a >= 0 and a <= UINT16_MAX
 *      result = a
 *      truncated = false
 */
uint16_t sb_int_i32_to_u16(int32_t a, bool *truncated);

/*
 * function: sb_int_i32_to_u32
 * inputs: a (32-bit signed integer), truncated bool pointer
 * outputs: 32-bit unsigned integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a < 0
 *      result = 0
 *      truncated = true
 *  - If a >= 0
 *      result = a
 *      truncated = false
 */
uint32_t sb_int_i32_to_u32(int32_t a, bool *truncated);

/*
 * function: sb_int_i64_to_u8
 * inputs: a (64-bit signed integer), truncated bool pointer
 * outputs: 8-bit unsigned integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a < 0
 *      result = 0
 *      truncated = true
 *  - If a > UINT8_MAX
 *      result = UINT8_MAX
 *      truncated = true
 *  - If a >= 0 and a <= UINT8_MAX
 *      result = a
 *      truncated = false
 */
uint8_t sb_int_i64_to_u8(int64_t a, bool *truncated);

/*
 * function: sb_int_i64_to_u16
 * inputs: a (64-bit signed integer), truncated bool pointer
 * outputs: 16-bit unsigned integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a < 0
 *      result = 0
 *      truncated = true
 *  - If a > UINT16_MAX
 *      result = UINT16_MAX
 *      truncated = true
 *  - If a >= 0 and a <= UINT16_MAX
 *      result = a
 *      truncated = false
 */
uint16_t sb_int_i64_to_u16(int64_t a, bool *truncated);

/*
 * function: sb_int_i64_to_u32
 * inputs: a (64-bit signed integer), truncated bool pointer
 * outputs: 32-bit unsigned integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a < 0
 *      result = 0
 *      truncated = true
 *  - If a > UINT32_MAX
 *      result = UINT32_MAX
 *      truncated = true
 *  - If a >= 0 and a <= UINT32_MAX
 *      result = a
 *      truncated = false
 */
uint32_t sb_int_i64_to_u32(int64_t a, bool *truncated);

/*
 * function: sb_int_i64_to_u64
 * inputs: a (64-bit signed integer), truncated bool pointer
 * outputs: 64-bit unsigned integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a < 0
 *      result = 0
 *      truncated = true
 *  - If a >= 0
 *      result = a
 *      truncated = false
 */
uint64_t sb_int_i64_to_u64(int64_t a, bool *truncated);


/*
 * function: sb_int_isize_to_size
 * inputs: a (isize), truncated bool pointer
 * outputs: size_t
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a < 0
 *      result = 0
 *      truncated = true
 *  - If a >= 0
 *      result = a
 *      truncated = false
 */
size_t sb_int_isize_to_size(isize a, bool *truncated);
// SB UNSIGNED TO SIGNED CONVERSIONS

/*
 * function: sb_int_u8_to_i8
 * inputs: a (8-bit unsigned integer), truncated bool pointer
 * outputs: 8-bit signed integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a > INT8_MAX
 *      result = INT8_MAX
 *      truncated = true
 *  - Else
 *      result = a
 *      truncated = false
 */
int8_t sb_int_u8_to_i8(uint8_t a, bool *truncated);

/*
 * function: sb_int_u16_to_i8
 * inputs: a (16-bit unsigned integer), truncated bool pointer
 * outputs: 8-bit signed integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a > INT8_MAX
 *      result = INT8_MAX
 *      truncated = true
 *  - Else
 *      result = a
 *      truncated = false
 */
int8_t sb_int_u16_to_i8(uint16_t a, bool *truncated);

/*
 * function: sb_int_u16_to_i16
 * inputs: a (16-bit unsigned integer), truncated bool pointer
 * outputs: 16-bit signed integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a > INT16_MAX
 *      result = INT16_MAX
 *      truncated = true
 *  - Else
 *      result = a
 *      truncated = false
 */
int16_t sb_int_u16_to_i16(uint16_t a, bool *truncated);

/*
 * function: sb_int_u32_to_i8
 * inputs: a (32-bit unsigned integer), truncated bool pointer
 * outputs: 8-bit signed integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a > INT8_MAX
 *      result = INT8_MAX
 *      truncated = true
 *  - Else
 *      result = a
 *      truncated = false
 */
int8_t sb_int_u32_to_i8(uint32_t a, bool *truncated);

/*
 * function: sb_int_u32_to_i16
 * inputs: a (32-bit unsigned integer), truncated bool pointer
 * outputs: 16-bit signed integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a > INT16_MAX
 *      result = INT16_MAX
 *      truncated = true
 *  - Else
 *      result = a
 *      truncated = false
 */
int16_t sb_int_u32_to_i16(uint32_t a, bool *truncated);

/*
 * function: sb_int_u32_to_i32
 * inputs: a (32-bit unsigned integer), truncated bool pointer
 * outputs: 32-bit signed integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a > INT32_MAX
 *      result = INT32_MAX
 *      truncated = true
 *  - Else
 *      result = a
 *      truncated = false
 */
int32_t sb_int_u32_to_i32(uint32_t a, bool *truncated);

/*
 * function: sb_int_u64_to_i8
 * inputs: a (64-bit unsigned integer), truncated bool pointer
 * outputs: 8-bit signed integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a > INT8_MAX
 *      result = INT8_MAX
 *      truncated = true
 *  - Else
 *      result = a
 *      truncated = false
 */
int8_t sb_int_u64_to_i8(uint64_t a, bool *truncated);

/*
 * function: sb_int_u64_to_i16
 * inputs: a (64-bit unsigned integer), truncated bool pointer
 * outputs: 16-bit signed integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a > INT16_MAX
 *      result = INT16_MAX
 *      truncated = true
 *  - Else
 *      result = a
 *      truncated = false
 */
int16_t sb_int_u64_to_i16(uint64_t a, bool *truncated);

/*
 * function: sb_int_u64_to_i32
 * inputs: a (64-bit unsigned integer), truncated bool pointer
 * outputs: 32-bit signed integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a > INT32_MAX
 *      result = INT32_MAX
 *      truncated = true
 *  - Else
 *      result = a
 *      truncated = false
 */
int32_t sb_int_u64_to_i32(uint64_t a, bool *truncated);

/*
 * function: sb_int_u64_to_i64
 * inputs: a (64-bit unsigned integer), truncated bool pointer
 * outputs: 64-bit signed integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a > INT64_MAX
 *      result = INT64_MAX
 *      truncated = true
 *  - Else
 *      result = a
 *      truncated = false
 */
int64_t sb_int_u64_to_i64(uint64_t a, bool *truncated);

/*
 * function: sb_int_size_to_i32
 * inputs: a (size_t), truncated bool pointer
 * outputs: 32-bit signed integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a > INT32_MAX
 *      result = INT32_MAX
 *      truncated = true
 *  - Else
 *      result = a
 *      truncated = false
 */
int32_t sb_int_size_to_i32(size_t a, bool *truncated);

/*
 * function: sb_int_size_to_i64
 * inputs: a (size_t), truncated bool pointer
 * outputs: 64-bit signed integer
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a > INT64_MAX
 *      result = INT64_MAX
 *      truncated = true
 *  - Else
 *      result = a
 *      truncated = false
 */
int64_t sb_int_size_to_i64(size_t a, bool *truncated);

/*
 * function: sb_int_size_to_isize
 * inputs: a (size_t), truncated bool pointer
 * outputs: isize
 *
 * preconditions:
 *  - truncated != NULL
 *
 * postconditions:
 *  - If a > ISIZE_MAX
 *      result = ISIZE_MAX
 *      truncated = true
 *  - Else
 *      result = a
 *      truncated = false
 */
isize sb_int_size_to_isize(size_t a, bool *truncated);

size_t sb_int_min_size_t(size_t a, size_t b);
size_t sb_int_max_size_t(size_t a, size_t b);

#endif
