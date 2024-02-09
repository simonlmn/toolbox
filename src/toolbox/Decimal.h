#ifndef TOOLBOX_DECIMAL_H
#define TOOLBOX_DECIMAL_H

#include <functional>
#include <algorithm>
#include "Maybe.h"
#include "String.h"

namespace toolbox {

/**
 * Get the number of digits of an integer (in base 10).
 */
uint8_t numberOfDigits(int32_t x) {
  // Note: the implementation uses binary search to make the lookup take similar time for all orders of magnitude.
  x = abs(x);
  if (x < 100000u) {
    if (x < 1000u) {
      if (x < 10u) {
        return 1;
      } else if (x < 100u) {
        return 2;
      } else {
        return 3;
      }
    } else {
      if (x < 10000u) {
        return 4;
      } else {
        return 5;
      }
    }
  } else {
    if (x < 10000000u) {
      if (x < 1000000u) {
        return 6;
      } else {
        return 7;
      }
    } else {
      if (x < 100000000u) {
        return 8;
      } else if (x < 1000000000u) {
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

/**
 * Representation of decimal numbers for in- and output purposes.
 * 
 * It mainly provides conversion to and from strings, but also can convert
 * the number to other numeric primitive types, but that may loose precision
 * or may not be representable at all.
 * 
 * It is limited in the range of numbers it can represent by the size of the
 * underlying integer type (64 bit), which also means that only a limited range
 * of decimal places can be safely used (up to 18).
 * 
 */
class Decimal final {
  static constexpr auto MAX_DOT_POSITION = 21;
  static constexpr auto MAX_STRING_LENGTH = 22;
  static char BUFFER[MAX_STRING_LENGTH + 1];

  int64_t _number;
  uint8_t _decimalPlaces;

  Decimal(int64_t number, uint8_t decimalPlaces) : _number(number), _decimalPlaces(decimalPlaces) {}

public:
  Decimal() : Decimal(0, 0) {}

  int64_t integer() const {
    return _decimalPlaces > 0 ? _number / powerOfTen(_decimalPlaces) : _number;
  }

  uint64_t decimals() const {
    return _decimalPlaces > 0 ? std::abs(_number) % powerOfTen(_decimalPlaces) : 0;
  }

  uint8_t decimalPlaces() const {
    return _decimalPlaces;
  }

  bool isInteger() const {
    return _decimalPlaces == 0;
  }

  float toFloat() const {
    float i = float(integer());
    float d = float(decimals()) / powerOfTen(_decimalPlaces);
    return _number < 0 ? i - d : i + d;
  }

  double toDouble() const {
    double i = double(integer());
    double d = double(decimals()) / powerOfTen(_decimalPlaces);
    return _number < 0 ? i - d : i + d;
  }

  int64_t toFixedPoint(uint8_t decimalPlaces) {
    if (decimalPlaces == _decimalPlaces) {
      return _number;
    } else {
      return rescale(_number, int8_t(decimalPlaces) - int8_t(_decimalPlaces));
    }
  }

  static Decimal fromFixedPoint(int64_t fixedPoint, uint8_t decimalPlaces) {
    return Decimal{fixedPoint, decimalPlaces};
  }

  /**
   * Converts the number into a string.
   * 
   * When not passing in a buffer, an internal buffer is used which means that
   * the contents of the returned string will only stay until the next invocation
   * of this method.
   * 
   * If a buffer is passed in, it must have at least space for MAX_STRING_LENGTH + 1 characters.
   */
  strref toString(char* buffer = nullptr) const {
    buffer = buffer ? buffer : BUFFER;

    auto length = sprintf(buffer, "%lli", _number);
    if (_decimalPlaces > 0) {
      auto signLength = _number < 0 ? 1 : 0;
      length -= signLength;
      if (_decimalPlaces >= length) {
        memmove(buffer + signLength + _decimalPlaces - length + 1, buffer + signLength, length + 1);
        memset(buffer + signLength, '0', _decimalPlaces - length + 1);
        length += _decimalPlaces - length + 1;
      }
      // insert .
      memmove(buffer + signLength + length - _decimalPlaces + 1, buffer + signLength + length - _decimalPlaces, _decimalPlaces + 1);
      buffer[signLength + length - _decimalPlaces] = '.';
    } else {
      // we always append .0 to have a consistent output format.
      strcpy(buffer + length, ".0");
    }

    return buffer;
  }

  static Maybe<Decimal> fromString(const strref& string) {
    if (string.length() > MAX_STRING_LENGTH) {
      return {};
    }

    string.copy(BUFFER, MAX_STRING_LENGTH, true);
    uint8_t decimalPlaces = 0;

    ssize_t dotPosition = string.indexOf('.');
    if (dotPosition >= 0) {
      if (dotPosition > MAX_DOT_POSITION) {
        return {};
      }
      
      decimalPlaces = string.skip(dotPosition + 1).copy(BUFFER + dotPosition, MAX_STRING_LENGTH - dotPosition, true);
    }

    char* end = nullptr;
    int64_t number = strtoll(BUFFER, &end, 10);
    if (*end == '\0') {
      return {Decimal{number, decimalPlaces}};
    } else {
      return {};
    }
  }
};

char Decimal::BUFFER[Decimal::MAX_STRING_LENGTH + 1] {};

}

#endif