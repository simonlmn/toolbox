#ifndef TOOLBOX_CSTRING_H_
#define TOOLBOX_CSTRING_H_

#include <WString.h>
#include <algorithm>

#ifndef ARDUINO
#include <cstring>
#define strlen_P strlen
#define strncpy_P strncpy
#define memcpy_P memcpy
#define strcmp_P strcmp
class __FlashStringHelper; // Forward declare helper for strings stored in "PROGMEM"
#endif

namespace toolbox {

int strncmp_P2(PGM_P str1P, PGM_P str2P, size_t size = SIZE_MAX)
{
    int result = 0;

    while (size > 0)
    {
        char ch1 = pgm_read_byte(str1P++);
        char ch2 = pgm_read_byte(str2P++);
        result = ch1 - ch2;
        if (result != 0 || ch2 == '\0')
        {
            break;
        }

        size--;
    }

    return result;
}

/**
 * Lightweight, read-only wrapper around const char pointers to "C-style strings" - which can
 * be either stored in normal or PROGMEM memory - or String instances from the Arduino library.
 * 
 * It is meant to reduce the amount of code needed to handle the different options in other
 * library code. It has some run-time overhead as it has to select the correct variant of the
 * underlying string access functions (i.e. standard vs. *_P variant). This should not make a
 * difference in most cases, but may have an impact in extremely time critical/constrained
 * situations.
 */
class strref final {
public:
  static const char EMPTY[];
  static const char EMPTY_FPSTR[] PROGMEM;

  enum struct Type {
    String,
    ConstChar,
    ProgMem
  } _type;

private:
  union {
    const String* string;
    const char* constchar;
    const __FlashStringHelper* progmem;
  } _reference;
  
public:
  strref() : strref(EMPTY) {}
  strref(const String& string) : _type(Type::ProgMem) { _reference.string = &string; }
  strref(const char* string) : _type(Type::ConstChar) { _reference.constchar = string; }
  strref(const __FlashStringHelper* string) : _type(Type::ProgMem) { _reference.progmem = string; }
  
  Type type() const {
    return _type;
  }

  bool isInProgmem() const {
    return _type == Type::ProgMem;
  }

  const char* raw() const {
    switch (_type) {
      case Type::String: return _reference.string->c_str();
      case Type::ConstChar: return _reference.constchar;
      case Type::ProgMem: return reinterpret_cast<const char*>(_reference.progmem);
      default: return EMPTY;
    }
  }

  const char* cstr() const {
    switch (_type) {
      case Type::String: return _reference.string->c_str();
      case Type::ConstChar: return _reference.constchar;
      case Type::ProgMem: return EMPTY;
      default: return EMPTY;
    }
  }

  const __FlashStringHelper* fpstr() const {
    switch (_type) {
      case Type::String: return FPSTR(EMPTY_FPSTR);
      case Type::ConstChar: return FPSTR(EMPTY_FPSTR);
      case Type::ProgMem: return _reference.progmem;
      default: return FPSTR(EMPTY_FPSTR);
    }
  }

  String toString() const {
    switch (_type) {
      case Type::String: return *_reference.string;
      case Type::ConstChar: return _reference.constchar;
      case Type::ProgMem: return _reference.progmem;
      default: return {};
    }
  }

  /**
   * Copy the referenced string into a newly allocated array of characters.
   * 
   * NOTE: the caller is responsible for managing the ownership and eventually
   * de-allocation of the array behind the returned pointer!
   */
  char* toCharArray() const {
    size_t length = len();
    char* array = new char[length + 1];
    switch (_type) {
      case Type::String: _reference.string->toCharArray(array, length + 1); break;
      case Type::ConstChar: memcpy(array, _reference.constchar, length + 1); break;
      case Type::ProgMem: memcpy_P(array, reinterpret_cast<const char*>(_reference.progmem), length + 1);
      default: array[0] = '\0';
    }
    return array;
  }

  size_t len(size_t offset = 0u) const {
    switch (_type) {
      case Type::String: return _reference.string->length() - offset;
      case Type::ConstChar: return strlen(_reference.constchar + offset);
      case Type::ProgMem: return strlen_P(reinterpret_cast<const char*>(_reference.progmem) + offset);
      default: return 0;
    }
  }

  size_t copy(char* dest, size_t maxLength, size_t offset = 0u, size_t* destLength = nullptr) const {
    const size_t length = len(offset);
    size_t lengthToCopy = std::min(length, maxLength);
    
    switch (_type) {
      case Type::String: memcpy(dest, _reference.string->c_str() + offset, lengthToCopy); break;
      case Type::ConstChar: memcpy(dest, _reference.constchar + offset, lengthToCopy); break;
      case Type::ProgMem: memcpy_P(dest, reinterpret_cast<const char*>(_reference.progmem) + offset, lengthToCopy); break;
      default: return 0;
    }

    dest[lengthToCopy] = '\0';

    if (destLength != nullptr) {
      *destLength += lengthToCopy;
    }

    return length;
  }

  ssize_t indexOf(char c, size_t offset = 0u) const {
    switch (_type) {
      case Type::String: return _reference.string->indexOf(c, offset);
      case Type::ConstChar: {
        auto p = strchr(_reference.constchar + offset, c);
        return p ? p - _reference.constchar : -1;
      }
      case Type::ProgMem:
      {
        auto p = (const char*) memchr_P(reinterpret_cast<const char*>(_reference.progmem) + offset, c, len());
        return p ? p - _reference.constchar : -1;
      }
      default: return -1;
    }
  }

  int compare(const strref& other) const {
    switch (_type) {
      case Type::String:
        switch (other._type) {
          case Type::String: return _reference.string->compareTo(*other._reference.string);
          case Type::ConstChar: return strcmp(_reference.string->c_str(), other._reference.constchar);
          case Type::ProgMem: return strcmp_P(_reference.string->c_str(), reinterpret_cast<const char*>(_reference.progmem));
          default: return 1;
        }
      case Type::ConstChar:
        switch (other._type) {
          case Type::String: return strcmp(_reference.constchar, other._reference.string->c_str());
          case Type::ConstChar: return strcmp(_reference.constchar, other._reference.constchar);
          case Type::ProgMem: return strcmp_P(_reference.constchar, reinterpret_cast<const char*>(_reference.progmem));
          default: return 1;
        }
      case Type::ProgMem:
        switch (other._type) {
          case Type::String: return -strcmp_P(other._reference.string->c_str(), reinterpret_cast<const char*>(_reference.progmem));
          case Type::ConstChar: return -strcmp_P(other._reference.constchar, reinterpret_cast<const char*>(_reference.progmem));
          case Type::ProgMem: return strncmp_P2(reinterpret_cast<const char*>(_reference.progmem), reinterpret_cast<const char*>(other._reference.progmem));
          default: return 1;
        }
      default: return -1;
    }
  }

  bool operator==(const strref& other) const {
    return compare(other) == 0;
  }

  bool operator!=(const strref& other) const {
    return compare(other) != 0;
  }

  bool operator<(const strref& other) const {
    return compare(other) < 0;
  }

  bool operator>(const strref& other) const {
    return compare(other) > 0;
  }

  bool operator<=(const strref& other) const {
    return compare(other) <= 0;
  }

  bool operator>=(const strref& other) const {
    return compare(other) >= 0;
  }
};

const char strref::EMPTY[] = "";
const char strref::EMPTY_FPSTR[] = "";

}

#endif