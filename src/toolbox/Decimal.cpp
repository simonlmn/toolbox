#include "Decimal.h"

#ifndef ARDUINO_AVR_NANO
#include <cstdio>
#include <cstdlib>
#include <cstring>
#endif

namespace toolbox {

uint8_t numberOfDigits(int32_t x) {
  // Note: the implementation uses binary search to make the lookup take similar time for all orders of magnitude.
  x = abs(x);
  if (x < 100000) {
    if (x < 1000) {
      if (x < 10) {
        return 1;
      } else if (x < 100) {
        return 2;
      } else {
        return 3;
      }
    } else {
      if (x < 10000) {
        return 4;
      } else {
        return 5;
      }
    }
  } else {
    if (x < 10000000) {
      if (x < 1000000) {
        return 6;
      } else {
        return 7;
      }
    } else {
      if (x < 100000000) {
        return 8;
      } else if (x < 1000000000) {
        return 9;
      } else {
        return 10;
      }
    }
  }
}

int64_t powerOfTen(uint8_t exp) {
  switch (exp) {
    case 0: return 1;
    case 1: return 10;
    case 2: return 100;
    case 3: return 1000;
    case 4: return 10000;
    case 5: return 100000;
    case 6: return 1000000;
    case 7: return 10000000;
    case 8: return 100000000;
    case 9: return 1000000000;
    case 10: return 10000000000;
    case 11: return 100000000000;
    case 12: return 1000000000000;
    case 13: return 10000000000000;
    case 14: return 100000000000000;
    case 15: return 1000000000000000;
    case 16: return 10000000000000000;
    case 17: return 100000000000000000;
    case 18: return 1000000000000000000;
    default: return 1;
  }
}

int64_t rescale(int64_t number, int8_t exp) {
  return exp < 0 ? number / powerOfTen(-exp) : number * powerOfTen(exp);
}

char Decimal::BUFFER[Decimal::MAX_STRING_LENGTH + 1] {};

} // namespace toolbox
