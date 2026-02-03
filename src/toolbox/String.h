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

namespace toolbox {

inline const char EMPTY_CSTR[] = "";
inline const char EMPTY_FPSTR[] PROGMEM = "";

/**
 * Compare two PROGMEM strings by content.
 */
int memcmp_P2(PGM_P str1P, PGM_P str2P, size_t size);

class strref;

/**
 * Reference-counted shared string storage.
 * 
 * This class provides an immutable string storage with reference counting
 * to allow efficient sharing of string data without unnecessary copies. It still
 * allocates memory on the heap, so it can lead to fragmentation if used heavily.
 */
class shared_str final {
  struct storage final {
    uint16_t refCounter;
    char* buffer;
    size_t length;

    storage(size_t len) : refCounter(1), length(len) {
      buffer = new char[len + 1];
      buffer[len] = '\0';
    }
    ~storage() {
      delete[] buffer;
    }

    void add_shared() { ++refCounter; }
    void release_shared() { if (--refCounter == 0) { delete this; } }
  };

  storage* _storage;

public:
  shared_str() : _storage(nullptr) {}
  ~shared_str() {
    if (_storage != nullptr) {
      _storage->release_shared();
    }
  }

  explicit shared_str(const strref& string);

  shared_str(const shared_str& other) : _storage(other._storage) {
    if (_storage != nullptr) {
      _storage->add_shared();
    }
  }

  shared_str(shared_str&& other) : _storage(other._storage) {
    other._storage = nullptr;
  }
  
  shared_str& operator=(const shared_str& other) {
    if (this != &other) {
      if (_storage != nullptr) {
        _storage->release_shared();
      }
      _storage = other._storage;
      if (_storage != nullptr) {
        _storage->add_shared();
      }
    }
    return *this;
  }

  shared_str& operator=(shared_str&& other) {
    if (this != &other) {
      if (_storage != nullptr) {
        _storage->release_shared();
      }
      _storage = other._storage;
      other._storage = nullptr;
    }
    return *this;
  }

  size_t length() const {
    return _storage != nullptr ? _storage->length : 0u;
  }

  bool empty() const {
    return length() == 0u;
  }

  const char* cstr() const {
    return _storage != nullptr ? _storage->buffer : EMPTY_CSTR;
  }

  void clear() {
    if (_storage != nullptr) {
      _storage->release_shared();
      _storage = nullptr;
    }
  }
};

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
  static const strref EMPTY;

  enum struct Type : uint8_t {
    String,
    ConstChar,
    ProgMem,
    SharedStr
  };

private:
  union Ref {
    const String* string;
    const char* constchar;
    const __FlashStringHelper* progmem;
    shared_str* sharedstr;
    Ref() {}
    Ref(const String* string) : string(string) {}
    Ref(const char* constchar) : constchar(constchar) {}
    Ref(const __FlashStringHelper* progmem) : progmem(progmem) {}
    Ref(shared_str* sharedstr) : sharedstr(sharedstr) {}
  };

  Type _type;
  Ref _reference;
  size_t _offset;
  size_t _length;
  bool _zeroTerminated;

  strref(Type type, Ref reference, size_t offset, size_t length, bool zeroTerminated) : _type(type), _reference(reference), _offset(offset), _length(length), _zeroTerminated(zeroTerminated) {
    if (_type == Type::SharedStr) {
      _reference.sharedstr = new shared_str(*reference.sharedstr);
    }
  }

public:
  strref() : strref(EMPTY_CSTR, 0, true) {}
  strref(const strref& other) : _type(other._type), _reference(other._reference), _offset(other._offset), _length(other._length), _zeroTerminated(other._zeroTerminated) {
    if (_type == Type::SharedStr) {
      _reference.sharedstr = new shared_str(*other._reference.sharedstr);
    }
  }
  strref(strref&& other) : _type(other._type), _reference(other._reference), _offset(other._offset), _length(other._length), _zeroTerminated(other._zeroTerminated) {
    other._type = Type::ConstChar;
    other._reference = Ref(EMPTY_CSTR);
    other._offset = 0;
    other._length = 0;
    other._zeroTerminated = true;
  }
  strref& operator=(const strref& other) {
    if (this != &other) {
      if (_type == Type::SharedStr) {
        delete _reference.sharedstr;
      }
      _type = other._type;
      _reference = other._reference;
      _offset = other._offset;
      _length = other._length;
      _zeroTerminated = other._zeroTerminated;
      if (_type == Type::SharedStr) {
        _reference.sharedstr = new shared_str(*other._reference.sharedstr);
      }
    }
    return *this;
  }
  strref& operator=(strref&& other) {
    if (this != &other) {
      if (_type == Type::SharedStr) {
        delete _reference.sharedstr;
      }
      _type = other._type;
      _reference = other._reference;
      _offset = other._offset;
      _length = other._length;
      _zeroTerminated = other._zeroTerminated;

      other._type = Type::ConstChar;
      other._reference = Ref(EMPTY_CSTR);
      other._offset = 0;
      other._length = 0;
      other._zeroTerminated = true;
    }
    return *this;
  }
  ~strref() {
    if (_type == Type::SharedStr) {
      delete _reference.sharedstr;
    }
  }
  
  strref(const String& string) : _type(Type::String), _reference(&string), _offset(0), _length(string.length()), _zeroTerminated(true) {}
  strref(const String& string, size_t length) : _type(Type::String), _reference(&string), _offset(0), _length(length), _zeroTerminated(false) {}
  
  strref(const char* string) : _type(Type::ConstChar), _reference(string), _offset(0), _length(strlen(string)), _zeroTerminated(true) {}
  strref(const char* string, size_t length, bool zeroTerminated=false) : _type(Type::ConstChar), _reference(string), _offset(0), _length(length), _zeroTerminated(zeroTerminated) {}
  
  strref(const __FlashStringHelper* string) : _type(Type::ProgMem), _reference(string), _offset(0), _length(strlen_P(reinterpret_cast<const char*>(string))), _zeroTerminated(true) {}
  strref(const __FlashStringHelper* string, size_t length, bool zeroTerminated=false) : _type(Type::ProgMem), _reference(string), _offset(0), _length(length), _zeroTerminated(zeroTerminated) {} 

  strref(const shared_str& string) : _type(Type::SharedStr), _reference(new shared_str(string)), _offset(0), _length(string.length()), _zeroTerminated(true) {}
  strref(const shared_str& string, size_t length) : _type(Type::SharedStr), _reference(new shared_str(string)), _offset(0), _length(length), _zeroTerminated(false) {}

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

  bool isPartial() const {
    return _offset != 0u || !_zeroTerminated;
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
      case Type::SharedStr: return _reference.sharedstr->cstr();
      default: return EMPTY_CSTR;
    }
  }

  const char* cstr() const {
    switch (_type) {
      case Type::String: return _reference.string->c_str() + _offset;
      case Type::ConstChar: return _reference.constchar + _offset;
      case Type::ProgMem: return reinterpret_cast<const char*>(_reference.progmem) + _offset;
      case Type::SharedStr: return _reference.sharedstr->cstr() + _offset;
      default: return EMPTY_CSTR;
    }
  }

  const __FlashStringHelper* fpstr() const {
    switch (_type) {
      case Type::ProgMem: return reinterpret_cast<const __FlashStringHelper*>(reinterpret_cast<const char*>(_reference.progmem) + _offset);
      default: return reinterpret_cast<const __FlashStringHelper*>(EMPTY_FPSTR);
    }
  }

  strref ensure_cstr() const {
    if (isInProgmem() || !_zeroTerminated) {
      return materialize();
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

  strref skipTo(const char* ref_ptr) const {
    const char* this_ref = ref() + _offset;
    if (ref_ptr < this_ref || ref_ptr > this_ref + _length) {
      return EMPTY;
    }
    return substring(static_cast<size_t>(ref_ptr - this_ref), _length);
  }

  strref ltrim(char c) const {
    size_t offset = 0;
    while (offset < _length && charAt(offset) == c) {
      offset++;
    }
    return substring(offset, _length - offset);
  }

  shared_str materialize() const {
    if (empty()) {
      return shared_str{};
    }

    switch (_type) {
      case Type::String: return shared_str{ *this };
      case Type::ConstChar: return shared_str{ *this };
      case Type::ProgMem: return shared_str{ *this };
      case Type::SharedStr: return isPartial() ? shared_str{ *this } : *_reference.sharedstr;
      default: return shared_str{};
    }
  }

  String toString() const {
    if (empty()) {
      return {};
    }

    switch (_type) {
      case Type::String: return _reference.string->substring(_offset, _length);
      case Type::ConstChar: return String{ cstr() }.substring(0, _length);
      case Type::ProgMem: return String{ fpstr() }.substring(0, _length);
      case Type::SharedStr: return String{ cstr() }.substring(0, _length);
      default: return {};
    }
  }

#ifdef ARDUINO_ARCH_ESP8266
  /**
   * Convert to StreamConstPtr for use with ESP8266 Stream classes.
   * 
   * @attention The underlying data must remain valid while the StreamConstPtr is used!
   * This is particularly important when this is referencing a temporary shared_str
   * instance created by toString() or ensure_cstr().
   */
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
      case Type::SharedStr: memcpy(dest, _reference.sharedstr->cstr() + _offset, lengthToCopy); break;
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
      case Type::SharedStr: return _reference.sharedstr->cstr()[_offset + i];
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
      case Type::SharedStr: {
        const char* str = _reference.sharedstr->cstr() + _offset;
        auto p = static_cast<const char*>(memchr(str, c, _length));
        return p ? p - str : -1;
      }
      default: return -1;
    }
  }

  bool startsWith(const strref& prefix) const {
    if (prefix._length > _length) {
      return false;
    }
    return leftmost(prefix._length) == prefix;
  }

  bool endsWith(const strref& suffix) const {
    if (suffix._length > _length) {
      return false;
    }
    return rightmost(suffix._length) == suffix;
  }

  int compare(const strref& other) const {
    if (_length != other._length) {
      return _length < other._length ? -1 : 1;
    }
    switch (_type) {
      case Type::String:
        switch (other._type) {
          case Type::String: return memcmp(_reference.string->c_str() + _offset, other._reference.string->c_str() + other._offset, _length);
          case Type::ConstChar: return memcmp(_reference.string->c_str() + _offset, other._reference.constchar + other._offset, _length);
          case Type::ProgMem: return memcmp_P(_reference.string->c_str() + _offset, reinterpret_cast<const char*>(other._reference.progmem) + other._offset, _length);
          case Type::SharedStr: return memcmp(_reference.string->c_str() + _offset, other._reference.sharedstr->cstr() + other._offset, _length);
          default: return 1;
        }
      case Type::ConstChar:
        switch (other._type) {
          case Type::String: return memcmp(_reference.constchar + _offset, other._reference.string->c_str() + other._offset, _length);
          case Type::ConstChar: return memcmp(_reference.constchar + _offset, other._reference.constchar + other._offset, _length);
          case Type::ProgMem: return memcmp_P(_reference.constchar + _offset, reinterpret_cast<const char*>(other._reference.progmem) + other._offset, _length);
          case Type::SharedStr: return memcmp(_reference.constchar + _offset, other._reference.sharedstr->cstr() + other._offset, _length);
          default: return 1;
        }
      case Type::ProgMem:
        switch (other._type) {
          case Type::String: return -memcmp_P(other._reference.string->c_str() + other._offset, reinterpret_cast<const char*>(_reference.progmem) + _offset, _length);
          case Type::ConstChar: return -memcmp_P(other._reference.constchar + other._offset, reinterpret_cast<const char*>(_reference.progmem) + _offset, _length);
          case Type::ProgMem: return memcmp_P2(reinterpret_cast<const char*>(_reference.progmem) + _offset, reinterpret_cast<const char*>(other._reference.progmem) + other._offset, _length);
          case Type::SharedStr: return -memcmp_P(other._reference.sharedstr->cstr() + other._offset, reinterpret_cast<const char*>(_reference.progmem) + _offset, _length);
          default: return 1;
        }
      case Type::SharedStr:
        switch (other._type) {
          case Type::String: return memcmp(_reference.sharedstr->cstr() + _offset, other._reference.string->c_str() + other._offset, _length);
          case Type::ConstChar: return memcmp(_reference.sharedstr->cstr() + _offset, other._reference.constchar + other._offset, _length);
          case Type::ProgMem: return memcmp_P(_reference.sharedstr->cstr() + _offset, reinterpret_cast<const char*>(other._reference.progmem) + other._offset, _length);
          case Type::SharedStr: return memcmp(_reference.sharedstr->cstr() + _offset, other._reference.sharedstr->cstr() + other._offset, _length);
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
  
  const char* cstr() const { return _buffer; }

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