#ifndef TOOLBOX_DECIMAL_H
#define TOOLBOX_DECIMAL_H

#include <functional>
#include <algorithm>
#include "Maybe.h"

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

int32_t powerOfTen(uint8_t exp) {
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
    default: return 1;
  }
}

/**
 * Representation of decimal numbers for in- and output purposes.
 * 
 * It mainly provides conversion to and from strings, but also can convert
 * the number to other numeric primitive types, but that may loose precision
 * or may not be representable at all.
 * 
 * It is limited to at most 9 integer digits and 9 decimal places due to the
 * internal 32-bit representation of these numbers.
 */
class Decimal final {
  static char BUFFER[23];

  int32_t _integer;
  uint32_t _decimals;
  uint8_t _decimalPlaces;

public:
  static const unsigned int MAX_INT32_DIGITS;
  static const int32_t MAX_INT32_DECIMAL = 999999999;
  static const int32_t MIN_INT32_DECIMAL = -999999999;

  static unsigned int limitDecimalPlaces(unsigned int decimalPlaces) { return std::min(decimalPlaces, MAX_INT32_DIGITS); }

  Decimal() : Decimal(0, 0, 0) {}
  Decimal(int32_t integer, uint32_t decimals, unsigned int decimalPlaces) : _integer(integer), _decimals(decimals), _decimalPlaces(limitDecimalPlaces(decimalPlaces)) {
    if (_decimalPlaces == 0) {
      _decimals = 0;
    } else {
      auto decimalDigits = numberOfDigits(_decimals);
      if (_decimalPlaces < decimalDigits) {
        _decimals /= powerOfTen(decimalDigits - _decimalPlaces);
      }
    }
  }

  int32_t integer() const {
    return _integer;
  }

  uint32_t decimals() const {
    return _decimals;
  }

  unsigned int decimalPlaces() const {
    return _decimalPlaces;
  }

  bool isInteger() const {
    return _decimalPlaces == 0;
  }

  float toFloat() const {
    float integer = float(_integer);
    float decimals = float(_decimals) / powerOfTen(_decimalPlaces);
    return _integer < 0 ? integer - decimals : integer + decimals;
  }

  double toDouble() const {
    double integer = double(_integer);
    double decimals = double(_decimals) / powerOfTen(_decimalPlaces);
    return _integer < 0 ? integer - decimals : integer + decimals;
  }

  Maybe<int32_t> toFixedPoint(unsigned int decimalPlaces) {
    if (decimalPlaces == 0) {
      return _integer;
    }
    
    decimalPlaces = limitDecimalPlaces(decimalPlaces);

    auto integerDigits = numberOfDigits(_integer);
    if ((integerDigits + decimalPlaces) > MAX_INT32_DIGITS) {
      return {};
    }

    int32_t integer = _integer * powerOfTen(decimalPlaces); // we want 5 decimal places: 123 -> 12300000

    int32_t decimals = _decimals;
    if (_decimalPlaces > decimalPlaces) { // e.g. we have 5 but want 2: 123.12345 -> 123.12
      decimals /= powerOfTen(_decimalPlaces - decimalPlaces);
    } else { // e.g. we have 2 but want 5: 123.12 -> 123.12000
      decimals *= powerOfTen(decimalPlaces - _decimalPlaces);
    }

    return integer < 0 ? integer - decimals : integer + decimals;
  }

  const char* toString(char* buffer = nullptr) const {
    buffer = buffer ? buffer : BUFFER;

    auto integerLength = snprintf(buffer, std::size(BUFFER), "%i", _integer);
    buffer[integerLength] = '.';
    auto decimalsLength = snprintf(buffer + (integerLength + 1), std::size(BUFFER) - (integerLength + 1), "%u", _decimals);
    if (decimalsLength < _decimalPlaces) {
      auto numberOfZeroesToAdd = _decimalPlaces - decimalsLength;
      memmove(buffer + (integerLength + 1 + numberOfZeroesToAdd), buffer + (integerLength + 1), decimalsLength + 1);
      memset(buffer + (integerLength + 1), '0', numberOfZeroesToAdd);
    }

    return buffer;
  }

  static Decimal fromFixedPoint(int32_t fixedPoint, unsigned int decimalPlaces) {
    if (decimalPlaces == 0) {
      return Decimal{fixedPoint, 0, 0};
    }

    decimalPlaces = limitDecimalPlaces(decimalPlaces);
    auto divisor = powerOfTen(decimalPlaces);

    return Decimal{fixedPoint / divisor, uint32_t(std::abs(fixedPoint) % divisor), decimalPlaces};
  }

  static Maybe<Decimal> fromString(const char* string) {
    Decimal result {};

    const char* dot = strchr(string, '.');
    if (dot) {
      char* end = nullptr;
      result._integer = strtol(string, &end, 10);
      if (end == dot) {
        result._decimalPlaces = limitDecimalPlaces(strlen(dot + 1));
        memcpy(BUFFER, dot + 1, result._decimalPlaces);
        BUFFER[result._decimalPlaces] = '\0';
        result._decimals = strtoul(BUFFER, &end, 10);
        if (*end == '\0') {
          return {result};
        } else {
          return {};
        }
      } else {
        return {};
      }
    } else {
      char* end = nullptr;
      result._integer = strtol(string, &end, 10);
      if (*end == '\0') {
        return {result};
      } else {
        return {};
      }
    }
  }
};

char Decimal::BUFFER[23] {};
const unsigned int Decimal::MAX_INT32_DIGITS = 9u;

}

#endif