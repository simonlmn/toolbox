#ifndef TOOLBOX_STRING_H_
#define TOOLBOX_STRING_H_

#include <Arduino.h>
#include <WString.h>
#include <Stream.h>
#ifdef ARDUINO_ARCH_ESP8266
#include <StreamDev.h>
#endif
#ifndef ARDUINO_AVR_NANO
#include <algorithm>
using std::min;
#else
using ssize_t = long;
#endif
#include "Maybe.h"

#ifndef ARDUINO
#include <cstring>
class __FlashStringHelper; // Forward declare helper for strings stored in "PROGMEM"
#define PROGMEM
#define PGM_P const char*
#define FPSTR(x) reinterpret_cast<const __FlashStringHelper*>(x)
#define strlen_P strlen
#define memcpy_P memcpy
#define memchr_P memchr
#define memcmp_P memcmp
#define pgm_read_byte(x) *(x)
#endif

namespace toolbox {

int memcmp_P2(PGM_P str1P, PGM_P str2P, size_t size);

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
  static const char EMPTY_CSTR[];
  static const char EMPTY_FPSTR[] PROGMEM;
  static const strref EMPTY;

  enum struct Type {
    String,
    ConstChar,
    ProgMem
  };

private:
  union Ref {
    const String* string;
    const char* constchar;
    const __FlashStringHelper* progmem;
    Ref() {}
    Ref(const String* string) : string(string) {}
    Ref(const char* constchar) : constchar(constchar) {}
    Ref(const __FlashStringHelper* progmem) : progmem(progmem) {}
  };

  Type _type;
  Ref _reference;
  size_t _offset;
  size_t _length;
  bool _zeroTerminated;

  strref(Type type, Ref reference, size_t offset, size_t length, bool zeroTerminated) : _type(type), _reference(reference), _offset(offset), _length(length), _zeroTerminated(zeroTerminated) {}

public:
  strref() : strref(EMPTY_CSTR, 0) { _zeroTerminated = true; }
  
  strref(const String& string) : _type(Type::String), _reference(&string), _offset(0), _length(string.length()), _zeroTerminated(true) {}
  strref(const String& string, size_t length) : _type(Type::String), _reference(&string), _offset(0), _length(length), _zeroTerminated(false) {}
  
  strref(const char* string) : _type(Type::ConstChar), _reference(string), _offset(0), _length(strlen(string)), _zeroTerminated(true) {}
  strref(const char* string, size_t length) : _type(Type::ConstChar), _reference(string), _offset(0), _length(length), _zeroTerminated(false) {}
  
  strref(const __FlashStringHelper* string) : _type(Type::ProgMem), _reference(string), _offset(0), _length(strlen_P(reinterpret_cast<const char*>(string))), _zeroTerminated(true) {}
  strref(const __FlashStringHelper* string, size_t length) : _type(Type::ProgMem), _reference(string), _offset(0), _length(length), _zeroTerminated(false) {} 

  Type type() const {
    return _type;
  }

  size_t offset() const {
    return _offset;
  }

  size_t length() const {
    return _length;
  }

  bool empty() const {
    return _length == 0u;
  }

  bool isZeroTerminated() const {
    return _zeroTerminated;
  }

  bool isInProgmem() const {
    return _type == Type::ProgMem;
  }

  const char* ref() const {
    switch (_type) {
      case Type::String: return _reference.string->c_str();
      case Type::ConstChar: return _reference.constchar;
      case Type::ProgMem: return reinterpret_cast<const char*>(_reference.progmem);
      default: return EMPTY_CSTR;
    }
  }

  const char* cstr() const {
    switch (_type) {
      case Type::String: return _reference.string->c_str() + _offset;
      case Type::ConstChar: return _reference.constchar + _offset;
      case Type::ProgMem: return EMPTY_CSTR;
      default: return EMPTY_CSTR;
    }
  }

  const __FlashStringHelper* fpstr() const {
    switch (_type) {
      case Type::String: return reinterpret_cast<const __FlashStringHelper*>(EMPTY_FPSTR);
      case Type::ConstChar: return reinterpret_cast<const __FlashStringHelper*>(EMPTY_FPSTR);
      case Type::ProgMem: return reinterpret_cast<const __FlashStringHelper*>(reinterpret_cast<const char*>(_reference.progmem) + _offset);
      default: return reinterpret_cast<const __FlashStringHelper*>(EMPTY_FPSTR);
    }
  }

  strref ensure_cstr() const {
    if (isInProgmem() || !_zeroTerminated) {
      return toString();
    } else {
      return *this;
    }
  }

  strref substring(size_t offset, size_t length) const {
    offset = min(offset, _length);
    length = min(length, _length - offset);
    return {_type, _reference, _offset + offset, length, offset + length == _length ? _zeroTerminated : false };
  }

  strref leftmost(size_t length) const {
    return substring(0, length);
  }

  strref rightmost(size_t length) const {
    return substring(length < _length ? _length - length : 0, length);
  }

  strref middle(size_t start, size_t end) const {
    return substring(start, end - start);
  }

  strref skip(size_t offset) const {
    return substring(offset, _length);
  }

  strref skipTo(const char* cstr_ptr) const {
    const char* this_cstr = cstr();
    if (cstr_ptr < this_cstr || cstr_ptr > this_cstr + _length) {
      return EMPTY;
    }
    return substring(static_cast<size_t>(cstr_ptr - cstr()), _length);
  }

  strref ltrim(char c) const {
    size_t offset = 0;
    while (offset < _length && charAt(offset) == c) {
      offset++;
    }
    return substring(offset, _length - offset);
  }

  String toString() const {
    switch (_type) {
      case Type::String: return _reference.string->substring(_offset, _length);
      case Type::ConstChar: return String{cstr()}.substring(0, _length);
      case Type::ProgMem: return String{fpstr()}.substring(0, _length);
      default: return {};
    }
  }

#ifdef ARDUINO_ARCH_ESP8266
  StreamConstPtr toStream() const {
    return {ref() + _offset, _length}; // StreamConstPtr has auto-detection for PROGMEM addresses
  }
#endif

  /**
   * Copy the referenced string into a newly allocated array of characters.
   * 
   * NOTE: the caller is responsible for managing the ownership and eventually
   * de-allocation of the array behind the returned pointer!
   */
  char* toCharArray(bool zeroTerminated) const {
    size_t arraySize = _length + (zeroTerminated ? 1 : 0);
    char* array = new char[arraySize];
    copy(array, arraySize, zeroTerminated);
    return array;
  }

  size_t copy(char* dest, size_t destSize, bool zeroTerminated) const {
    if (destSize == 0) {
      return 0;
    }
    
    size_t lengthToCopy = min(destSize - (zeroTerminated ? 1 : 0), _length);
    
    switch (_type) {
      case Type::String: memcpy(dest, _reference.string->c_str() + _offset, lengthToCopy); break;
      case Type::ConstChar: memcpy(dest, _reference.constchar + _offset, lengthToCopy); break;
      case Type::ProgMem: memcpy_P(dest, reinterpret_cast<const char*>(_reference.progmem) + _offset, lengthToCopy); break;
      default: lengthToCopy = 0; break;
    }

    if (zeroTerminated) {
      dest[lengthToCopy] = '\0';
    }
    
    return lengthToCopy;
  }

  char charAt(size_t i) const {
    switch (_type) {
      case Type::String: return _reference.string->charAt(_offset + i);
      case Type::ConstChar: return _reference.constchar[_offset + i];
      case Type::ProgMem: return pgm_read_byte(reinterpret_cast<const char*>(_reference.progmem) + _offset + i);
      default: return '\0';
    }
  }

  ssize_t indexOf(char c) const {
    switch (_type) {
      case Type::String: return _reference.string->indexOf(c, _offset);
      case Type::ConstChar: {
        auto p = static_cast<const char*>(memchr(_reference.constchar + _offset, c, _length));
        return p ? p - (_reference.constchar + _offset) : -1;
      }
      case Type::ProgMem:
      {
        auto p = static_cast<const char*>(memchr_P(reinterpret_cast<const char*>(_reference.progmem) + _offset, c, _length));
        return p ? p - (reinterpret_cast<const char*>(_reference.progmem) + _offset) : -1;
      }
      default: return -1;
    }
  }

  int compare(const strref& other) const {
    if (_length != other._length) {
      return _length < other._length ? -1 : 1;
    }
    switch (_type) {
      case Type::String:
        switch (other._type) {
          case Type::String: return _reference.string->compareTo(*other._reference.string);
          case Type::ConstChar: return memcmp(_reference.string->c_str() + _offset, other._reference.constchar + other._offset, _length);
          case Type::ProgMem: return memcmp_P(_reference.string->c_str() + _offset, reinterpret_cast<const char*>(other._reference.progmem) + other._offset, _length);
          default: return 1;
        }
      case Type::ConstChar:
        switch (other._type) {
          case Type::String: return memcmp(_reference.constchar + _offset, other._reference.string->c_str() + other._offset, _length);
          case Type::ConstChar: return memcmp(_reference.constchar + _offset, other._reference.constchar + other._offset, _length);
          case Type::ProgMem: return memcmp_P(_reference.constchar + _offset, reinterpret_cast<const char*>(other._reference.progmem) + other._offset, _length);
          default: return 1;
        }
      case Type::ProgMem:
        switch (other._type) {
          case Type::String: return -memcmp_P(other._reference.string->c_str() + other._offset, reinterpret_cast<const char*>(_reference.progmem) + _offset, _length);
          case Type::ConstChar: return -memcmp_P(other._reference.constchar + other._offset, reinterpret_cast<const char*>(_reference.progmem) + _offset, _length);
          case Type::ProgMem: return memcmp_P2(reinterpret_cast<const char*>(_reference.progmem) + _offset, reinterpret_cast<const char*>(other._reference.progmem) + other._offset, _length);
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

/**
 * Lightweight/minimal wrapper around statically allocated strings / char arrays.
 */
template<size_t length>
class str {
  static constexpr size_t BUFFER_SIZE = length + 1;
  char _buffer[BUFFER_SIZE] {'\0'};

public:
  str() {}
  
  operator const char*() const { return _buffer; }

  str(const strref& string) { string.copy(_buffer, BUFFER_SIZE, true); }
  str& operator=(const strref& string) { string.copy(_buffer, BUFFER_SIZE, true); return *this; }
  operator strref() const { return {_buffer}; }

  str(const Maybe<strref>& string) { if (string) string.get().copy(_buffer, BUFFER_SIZE, true); }
  str& operator=(const Maybe<strref>& string) { if (string) string.get().copy(_buffer, BUFFER_SIZE, true); return *this; }
  operator Maybe<strref>() const { return strref{_buffer}; }
};

}

#endif