#include "sb_int.h"
#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

int32_t sb_int_add_i32(int32_t a, int32_t b, bool *overflow) {
  assert(overflow != NULL);

  if (b > 0 && a > INT32_MAX - b) {
    *overflow = true;
    return INT32_MAX;
  }
  if (b < 0 && a < INT32_MIN - b) {
    *overflow = true;
    return INT32_MIN;
  }
  *overflow = false;
  return a + b;
}

int32_t sb_int_sub_i32(int32_t a, int32_t b, bool *overflow) {
  if (b < 0 && a > INT32_MAX + b) {
    *overflow = true;
    return INT32_MAX;
  }
  if (b > 0 && a < INT32_MIN + b) {
    *overflow = true;
    return INT32_MIN;
  }
  *overflow = false;
  return a - b;
}

int32_t sb_int_mul_i32(int32_t a, int32_t b, bool *overflow) {
  assert(overflow != NULL);

  // Handle zero cases: no overflow
  if (a == 0 || b == 0) {
    *overflow = false;
    return 0;
  }

  // Special case: INT32_MIN * -1 overflows
  if (a == INT32_MIN && b == -1) {
    *overflow = true;
    return INT32_MAX;
  }
  if (b == INT32_MIN && a == -1) {
    *overflow = true;
    return INT32_MAX;
  }

  if (a > 0 && b > 0) {
    // a & b positive: overflow if a > INT32_MAX / b
    if (a > INT32_MAX / b) {
      *overflow = true;
      return INT32_MAX;
    }
  } else if (a < 0 && b < 0) {
    // a & b negative: overflow if a > INT32_MAX / b
    if (a < INT32_MAX / b) {
      *overflow = true;
      return INT32_MAX;
    }
    // a positive, b negative: overflow if b < INT32_MAX / a
  } else if (a > 0 && b < 0) {
    if (b < INT32_MIN / a) {
      *overflow = true;
      return INT32_MIN;
    }
  } else {
    // a negative, b positive: overflow if a < INT32_MAX / b
    if (a < INT32_MIN / b) {
      *overflow = true;
      return INT32_MIN;
    }
  }
  *overflow = false;
  return a * b;
}

int32_t sb_int_div_i32(int32_t a, int32_t b, bool *overflow, bool *success) {
  assert(overflow != NULL && success != NULL);

  // denominator is 0
  if (b == 0) {
    *overflow = false;
    *success = false;
    return 0;
  }

  // numerator is int32_min, denominator is -1, overflow
  if (a == INT32_MIN && b == -1) {
    *overflow = true;
    *success = true;
    return INT32_MAX;
  }

  *overflow = false;
  *success = true;
  return a / b;
}

uint32_t sb_int_add_u32(uint32_t a, uint32_t b, bool *overflow) {
  assert(overflow != NULL);

  // overflow
  if (a > UINT32_MAX - b) {
    *overflow = true;
    return UINT32_MAX;
  }

  *overflow = false;
  return a + b;
}

uint32_t sb_int_sub_u32(uint32_t a, uint32_t b, bool *overflow) {
  assert(overflow != NULL);

  // underflows
  if (a < b) {
    *overflow = true;
    return 0;
  }

  *overflow = false;
  return a - b;
}

uint32_t sb_int_mul_u32(uint32_t a, uint32_t b, bool *overflow) {
  assert(overflow != NULL);

  // zero cases
  if (a == 0 || b == 0) {
    *overflow = false;
    return 0;
  }

  // underflows
  if (a > UINT32_MAX / b) {
    *overflow = true;
    return UINT32_MAX;
  }

  *overflow = false;
  return a * b;
}

uint32_t sb_int_div_u32(uint32_t a, uint32_t b, bool *overflow, bool *success) {
  assert(overflow != NULL && success != NULL);

  // denominator is 0
  if (b == 0) {
    *overflow = false;
    *success = false;
    return 0;
  }

  *overflow = false;
  *success = true;
  return a / b;
}

int8_t sb_int_i16_to_i8(int16_t a, bool *truncated) {
  assert(truncated != NULL);

  // overflow
  if (a > INT8_MAX) {
    *truncated = true;
    return INT8_MAX;
  }

  // underflow
  if (a < INT8_MIN) {
    *truncated = true;
    return INT8_MIN;
  }

  // normal
  *truncated = false;
  return (int8_t)a;
}

int8_t sb_int_i32_to_i8(int32_t a, bool *truncated) {
  assert(truncated != NULL);

  // overflow
  if (a > INT8_MAX) {
    *truncated = true;
    return INT8_MAX;
  }

  // underflow
  if (a < INT8_MIN) {
    *truncated = true;
    return INT8_MIN;
  }

  // normal
  *truncated = false;
  return (int8_t)a;
}

int8_t sb_int_i64_to_i8(int64_t a, bool *truncated) {
  assert(truncated != NULL);

  // overflow
  if (a > INT8_MAX) {
    *truncated = true;
    return INT8_MAX;
  }

  // underflow
  if (a < INT8_MIN) {
    *truncated = true;
    return INT8_MIN;
  }

  // normal
  *truncated = false;
  return (int8_t)a;
}

int16_t sb_int_i32_to_i16(int32_t value, bool *truncated) {
  assert(truncated != NULL);

  // value > INT16_MAX
  if (value > INT16_MAX) {
    *truncated = true;
    return INT16_MAX;
  }
  // value < INT16_MIN
  if (value < INT16_MIN) {
    *truncated = true;
    return INT16_MIN;
  }
  *truncated = false;
  return (int16_t)value;
}

int16_t sb_int_i64_to_i16(int64_t value, bool *truncated) {
  assert(truncated != NULL);

  // value > INT16_MAX
  if (value > INT16_MAX) {
    *truncated = true;
    return INT16_MAX;
  }
  // value < INT16_MIN
  if (value < INT16_MIN) {
    *truncated = true;
    return INT16_MIN;
  }
  *truncated = false;
  return (int16_t)value;
}

int32_t sb_int_i64_to_i32(int64_t value, bool *truncated) {
  assert(truncated != NULL);

  if (value > INT32_MAX) {
    *truncated = true;
    return INT32_MAX;
  }

  if (value < INT32_MIN) {
    *truncated = true;
    return INT32_MIN;
  }
  *truncated = false;
  return (int32_t)value;
}

uint8_t sb_int_u16_to_u8(uint16_t a, bool *truncated) {
  assert(truncated != NULL);

  if (a > UINT8_MAX) {
    *truncated = true;
    return UINT8_MAX;
  }

  *truncated = false;
  return (uint8_t)a;
}

uint8_t sb_int_u32_to_u8(uint32_t a, bool *truncated) {
  assert(truncated != NULL);

  if (a > UINT8_MAX) {
    *truncated = true;
    return UINT8_MAX;
  }

  *truncated = false;
  return (uint8_t)a;
}

uint16_t sb_int_u32_to_u16(uint32_t a, bool *truncated) {
  assert(truncated != NULL);

  if (a > UINT16_MAX) {
    *truncated = true;
    return UINT16_MAX;
  }

  *truncated = false;
  return (uint16_t)a;
}

uint16_t sb_int_u64_to_u16(uint64_t a, bool *truncated) {
  assert(truncated != NULL);

  if (a > UINT16_MAX) {
    *truncated = true;
    return UINT16_MAX;
  }

  *truncated = false;
  return (uint16_t)a;
}

uint32_t sb_int_u64_to_u32(uint64_t a, bool *truncated) {
  assert(truncated != NULL);

  if (a > UINT32_MAX) {
    *truncated = true;
    return UINT32_MAX;
  }

  *truncated = false;
  return (uint32_t)a;
}

uint8_t sb_int_u64_to_u8(uint64_t a, bool *truncated) {
  assert(truncated != NULL);

  if (a > UINT8_MAX) {
    *truncated = true;
    return UINT8_MAX;
  }

  *truncated = false;
  return (uint8_t)a;
}

uint8_t sb_int_i8_to_u8(int8_t a, bool *truncated) {
  assert(truncated != NULL);

  if (a < 0) {
    *truncated = true;
    return (uint8_t)0;
  }
  *truncated = false;
  return (uint8_t)a;
}

uint8_t sb_int_i16_to_u8(int16_t a, bool *truncated) {
  assert(truncated != NULL);

  if (a < 0) {
    *truncated = true;
    return (uint8_t)0;
  }
  if (a > UINT8_MAX) {
    *truncated = true;
    return UINT8_MAX;
  }
  *truncated = false;
  return (uint8_t)a;
}

uint8_t sb_int_i32_to_u8(int32_t a, bool *truncated) {
  assert(truncated != NULL);

  if (a < 0) {
    *truncated = true;
    return (uint8_t)0;
  }
  if (a > UINT8_MAX) {
    *truncated = true;
    return UINT8_MAX;
  }
  *truncated = false;
  return (uint8_t)a;
}
