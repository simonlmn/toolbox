#ifndef TOOLBOX_CONSTSTR_H_
#define TOOLBOX_CONSTSTR_H_

#ifndef ARDUINO_MAIN
#include <cstring>
#define strlen_P strlen
#define strncpy_P strncpy
#define memcpy_P memcpy
#define strcmp_P strcmp
class __FlashStringHelper; // Forward declare helper for strings stored in "PROGMEM"
#endif

#include <algorithm>

namespace toolbox {

/**
 * Lightweight wrapper around const char pointers to "C-style strings" which can be either
 * stored in normal or PROGMEM memory. It is meant to reduce the amount of code needed to
 * handle both options in other library code.
 * 
 * It has some run-time overhead as it has to select the correct variant of the underlying
 * string access functions (i.e. standard vs. *_P variant). This should not make a difference
 * in most cases, but may have an impact in extremely time critical/constrained situations.
 */
class ConstStr final {
  const char* _string;
  bool _progmem;

public:
  ConstStr() : ConstStr("") {}
  ConstStr(const char* string) : _string(string), _progmem(false) {}
  ConstStr(const __FlashStringHelper* string) : _string(reinterpret_cast<const char*>(string)), _progmem(true) {}

  const char* raw() const {
    return _string;
  }

  const char* operator*() const {
    return raw();
  }

  bool isInProgmem() const {
    return _progmem;
  }

  size_t len(size_t offset = 0u) const {
    return _progmem ? strlen_P(_string + offset) : strlen(_string + offset);
  }

  size_t copy(char* dest, size_t maxLength, size_t offset = 0u, size_t* destLength = nullptr) const {
    const size_t length = len(offset);
    size_t lengthToCopy = std::min(length, maxLength);
    
    if (_progmem) {
      memcpy_P(dest, _string + offset, lengthToCopy);
    } else {
      memcpy(dest, _string + offset, lengthToCopy);
    }
    dest[lengthToCopy] = '\0';

    if (destLength != nullptr) {
      *destLength += lengthToCopy;
    }

    return length;
  }

  int compare(const char* other) const {
    if (_progmem) {
      return strcmp_P(other, _string);
    } else {
      return strcmp(other, _string);
    }
  }

  bool operator==(const char* other) const {
    return compare(other) == 0;
  }

  bool operator!=(const char* other) const {
    return !(*this == other);
  }

  bool operator==(const ConstStr& other) const {
    if (_progmem) {
      if (other._progmem) {
        return false; // we cannot compare two progmem strings at the moment
      } else {
        return *this == other.raw();
      }
    } else{
      return other == raw();
    }
  }

  bool operator!=(const ConstStr& other) const {
    return !(*this == other);
  }
};

}

#endif