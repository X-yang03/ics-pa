#ifndef __FLOAT_H__
#define __FLOAT_H__

#include "assert.h"

typedef int FLOAT;

union _float{
    struct {
      uint32_t man : 23;
      uint32_t exp : 8;
      uint32_t sign : 1;
    };
    uint32_t val;
};

static inline int F2int(FLOAT a) {
  
  return a>>16;
}

static inline FLOAT int2F(int a) {
  return a<<12;
}

static inline FLOAT F_mul_int(FLOAT a, int b) {
  return a*b;
}

static inline FLOAT F_div_int(FLOAT a, int b) {
  return a/b;
}

FLOAT f2F(float);
FLOAT F_mul_F(FLOAT, FLOAT);
FLOAT F_div_F(FLOAT, FLOAT);
FLOAT Fabs(FLOAT);
FLOAT Fsqrt(FLOAT);
FLOAT Fpow(FLOAT, FLOAT);

#endif
