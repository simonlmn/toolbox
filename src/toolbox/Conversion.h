#ifndef _TOOLBOX_CONVERSION_H
#define _TOOLBOX_CONVERSION_H

#include "String.h"
#include "Maybe.h"

namespace toolbox {

template<typename T>
struct convert final {
  static strref toString(T value) { return ""; }
  static Maybe<T> fromString(const strref& string) { return {}; }
};

static char NUMBER_STRING_BUFFER[2 + 8 * sizeof(long)];

template<>
struct convert<char> final {
  static strref toString(char value) {
    NUMBER_STRING_BUFFER[0] = value;
    NUMBER_STRING_BUFFER[1] = '\0';
    return NUMBER_STRING_BUFFER;
  }

  static Maybe<char> fromString(const strref& string) {
    return string.charAt(0);
  }
};

template<>
struct convert<unsigned char> final {
  static strref toString(unsigned char value, int base) {
    utoa(value, NUMBER_STRING_BUFFER, base);
    return NUMBER_STRING_BUFFER;
  }

  static Maybe<unsigned char> fromString(const strref& string, int base) {
    char* end;
    auto value = strtoul(string.ensure_cstr().cstr(), &end, base);
  }
};

template<>
struct convert<short> final {
  static strref toString(short value, int base) {
    itoa(value, NUMBER_STRING_BUFFER, base);
    return NUMBER_STRING_BUFFER;
  }

  static short fromString(const strref& string, char** endptr, int base) {
    return static_cast<short>(strtol(string.ensure_cstr().cstr(), endptr, base));
  }
};

template<>
struct convert<unsigned short> final {
  static strref toString(unsigned short value, int base) {
    utoa(value, NUMBER_STRING_BUFFER, base);
    return NUMBER_STRING_BUFFER;
  }

  static unsigned short fromString(const strref& string, char** endptr, int base) {
    return static_cast<unsigned short>(strtoul(string.ensure_cstr().cstr(), endptr, base));
  }
};

template<>
struct convert<int> final {
  static strref toString(int value, int base) {
    itoa(value, NUMBER_STRING_BUFFER, base);
    return NUMBER_STRING_BUFFER;
  }

  static int fromString(const strref& string, char** endptr, int base) {
    return static_cast<int>(strtol(string.ensure_cstr().cstr(), endptr, base));
  }
};

template<>
struct convert<unsigned int> final {
  static strref toString(unsigned int value, int base) {
    utoa(value, NUMBER_STRING_BUFFER, base);
    return NUMBER_STRING_BUFFER;
  }

  static unsigned int fromString(const strref& string, char** endptr, int base) {
    return static_cast<unsigned int>(strtoul(string.ensure_cstr().cstr(), endptr, base));
  }
};

template<>
struct convert<long> final {
  static strref toString(long value, int base) {
    ltoa(value, NUMBER_STRING_BUFFER, base);
    return NUMBER_STRING_BUFFER;
  }

  static long fromString(const strref& string, char** endptr, int base) {
    return static_cast<long>(strtol(string.ensure_cstr().cstr(), endptr, base));
  }
};

template<>
struct convert<unsigned long> final {
  static strref toString(unsigned long value, int base) {
    ultoa(value, NUMBER_STRING_BUFFER, base);
    return NUMBER_STRING_BUFFER;
  }

  static unsigned long fromString(const strref& string, char** endptr, int base) {
    return static_cast<unsigned long>(strtoul(string.ensure_cstr().cstr(), endptr, base));
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

  static bool fromString(const strref& string, bool defaultValue, const char** endptr = nullptr, BoolFormat format = BoolFormat::Logic) {
    const char* start = string.ensure_cstr().cstr();
    while (*start == ' ') { start += 1; }
    
    switch (format) {
      default:
      case BoolFormat::Logic:
        if (strcmp(start, ("true")) == 0) {
          if (endptr != nullptr) { *endptr = start + 4; }
          return true;
        } else if (strcmp(start, ("false")) == 0) {
          if (endptr != nullptr) { *endptr = start + 5; }
          return false;
        } else {
          if (endptr != nullptr) { *endptr = string.ensure_cstr().cstr(); }
          return defaultValue;
        }
        break;
      case BoolFormat::Numeric:
        if (strcmp(start, ("1")) == 0) {
          if (endptr != nullptr) { *endptr = start + 1; }
          return true;
        } else if (strcmp(start, ("0")) == 0) {
          if (endptr != nullptr) { *endptr = start + 1; }
          return false;
        } else {
          if (endptr != nullptr) { *endptr = string.ensure_cstr().cstr(); }
          return defaultValue;
        }
        break;
      case BoolFormat::Io:
        if (strcmp(start, ("HIGH")) == 0) {
          if (endptr != nullptr) { *endptr = start + 4; }
          return true;
        } else if (strcmp(start, ("LOW")) == 0) {
          if (endptr != nullptr) { *endptr = start + 3; }
          return false;
        } else {
          if (endptr != nullptr) { *endptr = string.ensure_cstr().cstr(); }
          return defaultValue;
        }
        break;;
    }
  }
};

}

#endif
