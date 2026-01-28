#ifndef _TOOLBOX_CONVERSION_H
#define _TOOLBOX_CONVERSION_H

#include "String.h"
#include "Maybe.h"
#include <limits.h>

namespace toolbox {

/**
 * Generic conversion struct template.
 * 
 * The default implementation returns empty strings and empty Maybe objects.
 * Specializations for specific types provide methods to convert to/from strings.
 * 
 * Note: The result provided by toString() can only be assumed to be valid until
 * the next call to toString() on any converter!
 */
template<typename T>
struct convert final {
  static strref toString(T value) { return ""; }
  static Maybe<T> fromString(const strref& string) { return {}; }
};

inline char NUMBER_STRING_BUFFER[2 + 8 * sizeof(long)];

template<>
struct convert<char> final {
  static strref toString(char value) {
    NUMBER_STRING_BUFFER[0] = value;
    NUMBER_STRING_BUFFER[1] = '\0';
    return NUMBER_STRING_BUFFER;
  }

  static Maybe<char> fromString(const strref& string, strref* end = nullptr) {
    if (string.length() < 1) {
      return {};
    }
    if (end != nullptr) {
      *end = string.skip(1);
    }
    return string.charAt(0);
  }
};

template<>
struct convert<unsigned char> final {
  static strref toString(unsigned char value, int base) {
    utoa(value, NUMBER_STRING_BUFFER, base);
    return NUMBER_STRING_BUFFER;
  }

  static Maybe<unsigned char> fromString(const strref& string, strref* end = nullptr, int base = 0) {
    strref str = string.ensure_cstr();
    char* endptr;
    auto value = strtoul(str.cstr(), &endptr, base);
    if (endptr == str.cstr() || value > UCHAR_MAX) {
      return {};
    }
    if (end != nullptr) {
      *end = str.skipTo(endptr);
    }
    return static_cast<unsigned char>(value);
  }
};

template<>
struct convert<short> final {
  static strref toString(short value, int base) {
    itoa(value, NUMBER_STRING_BUFFER, base);
    return NUMBER_STRING_BUFFER;
  }

  static Maybe<short> fromString(const strref& string, strref* end = nullptr, int base = 0) {
    strref str = string.ensure_cstr();
    char* endptr;
    auto value = strtol(str.cstr(), &endptr, base);
    if (endptr == str.cstr() || value < SHRT_MIN || value > SHRT_MAX) {
      return {};
    }
    if (end != nullptr) {
      *end = str.skipTo(endptr);
    }
    return static_cast<short>(value);
  }
};

template<>
struct convert<unsigned short> final {
  static strref toString(unsigned short value, int base) {
    utoa(value, NUMBER_STRING_BUFFER, base);
    return NUMBER_STRING_BUFFER;
  }

  static Maybe<unsigned short> fromString(const strref& string, strref* end = nullptr, int base = 0) {
    strref str = string.ensure_cstr();
    char* endptr;
    auto value = strtoul(str.cstr(), &endptr, base);
    if (endptr == str.cstr() || value > USHRT_MAX) {
      return {};
    }
    if (end != nullptr) {
      *end = str.skipTo(endptr);
    }
    return static_cast<unsigned short>(value);
  }
};

template<>
struct convert<int> final {
  static strref toString(int value, int base) {
    itoa(value, NUMBER_STRING_BUFFER, base);
    return NUMBER_STRING_BUFFER;
  }

  static Maybe<int> fromString(const strref& string, strref* end = nullptr, int base = 0) {
    strref str = string.ensure_cstr();
    char* endptr;
    auto value = strtol(str.cstr(), &endptr, base);
    if (endptr == str.cstr() || value < INT_MIN || value > INT_MAX) {
      return {};
    }
    if (end != nullptr) {
      *end = str.skipTo(endptr);
    }
    return static_cast<int>(value);
  }
};

template<>
struct convert<unsigned int> final {
  static strref toString(unsigned int value, int base) {
    utoa(value, NUMBER_STRING_BUFFER, base);
    return NUMBER_STRING_BUFFER;
  }

  static Maybe<unsigned int> fromString(const strref& string, strref* end = nullptr, int base = 0) {
    strref str = string.ensure_cstr();
    char* endptr;
    auto value = strtoul(str.cstr(), &endptr, base);
    if (endptr == str.cstr() || value > UINT_MAX) {
      return {};
    }
    if (end != nullptr) {
      *end = str.skipTo(endptr);
    }
    return static_cast<unsigned int>(value);
  }
};

template<>
struct convert<long> final {
  static strref toString(long value, int base) {
    ltoa(value, NUMBER_STRING_BUFFER, base);
    return NUMBER_STRING_BUFFER;
  }

  static Maybe<long> fromString(const strref& string, strref* end = nullptr, int base = 0) {
    strref str = string.ensure_cstr();
    char* endptr;
    auto value = strtol(str.cstr(), &endptr, base);
    if (endptr == str.cstr()) {
      return {};
    }
    if (end != nullptr) {
      *end = str.skipTo(endptr);
    }
    return static_cast<long>(value);
  }
};

template<>
struct convert<unsigned long> final {
  static strref toString(unsigned long value, int base) {
    ultoa(value, NUMBER_STRING_BUFFER, base);
    return NUMBER_STRING_BUFFER;
  }

  static Maybe<unsigned long> fromString(const strref& string, strref* end = nullptr, int base = 0) {
    strref str = string.ensure_cstr();
    char* endptr;
    auto value = strtoul(str.cstr(), &endptr, base);
    if (endptr == str.cstr()) {
      return {};
    }
    if (end != nullptr) {
      *end = str.skipTo(endptr);
    }
    return static_cast<unsigned long>(value);
  }
};

enum struct BoolFormat {
  Logic,
  Numeric,
  Io
};

template<>
struct convert<bool> final {
  static strref toString(bool value, BoolFormat format = BoolFormat::Logic) {
    switch (format) {
      default:
      case BoolFormat::Logic: return value ? ("true") : ("false");
      case BoolFormat::Numeric: return value ? ("1") : ("0");
      case BoolFormat::Io: return value ? ("HIGH") : ("LOW");
    }
  }

  static Maybe<bool> fromString(const strref& string, strref* end = nullptr, BoolFormat format = BoolFormat::Logic) {
    strref str = string.ltrim(' ').ensure_cstr();
    switch (format) {
      default:
      case BoolFormat::Logic:
        if (str.leftmost(4) == "true") {
          if (end != nullptr) { *end = str.skip(4); }
          return true;
        } else if (str.leftmost(5) == "false") {
          if (end != nullptr) { *end = str.skip(5); }
          return false;
        } else {
          if (end != nullptr) { *end = str; }
          return {};
        }
        break;
      case BoolFormat::Numeric:
        if (str.leftmost(1) == "1") {
          if (end != nullptr) { *end = str.skip(1); }
          return true;
        } else if (str.leftmost(1) == "0") {
          if (end != nullptr) { *end = str.skip(1); }
          return false;
        } else {
          if (end != nullptr) { *end = str; }
          return {};
        }
        break;
      case BoolFormat::Io:
        if (str.leftmost(4) == "HIGH") {
          if (end != nullptr) { *end = str.skip(4); }
          return true;
        } else if (str.leftmost(3) == "LOW") {
          if (end != nullptr) { *end = str.skip(3); }
          return false;
        } else {
          if (end != nullptr) { *end = str; }
          return {};
        }
        break;
    }
  }
};

}

#endif
