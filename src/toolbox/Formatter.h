#ifndef TOOLBOX_FORMATTER_H_
#define TOOLBOX_FORMATTER_H_

#include "String.h"

namespace toolbox {

class Formatter final {
  char* _buffer;
  size_t _size;

public:
  Formatter(char* buffer, size_t size) : _buffer(buffer), _size(size) {}

  char* buffer() const { return _buffer; }

  char* vformat(const char* fmt, va_list args) {
    vsnprintf(_buffer, _size, fmt, args);
    _buffer[_size - 1] = '\0';
    return _buffer;
  }

  char* vformat(const __FlashStringHelper* fmt, va_list args) {
    vsnprintf_P(_buffer, _size, (PGM_P)fmt, args);
    _buffer[_size - 1] = '\0';
    return _buffer;
  }

  char* vformat(const strref& fmt, va_list args) {
    if (fmt.isZeroTerminated()) {
      if (fmt.isInProgmem()) {
        vsnprintf_P(_buffer, _size, (PGM_P)fmt.fpstr(), args);
      } else {
        vsnprintf(_buffer, _size, fmt.cstr(), args);
      }
      _buffer[_size - 1] = '\0';
    } else {
      _buffer[0] = '\0';
    }
    return _buffer;
  }
  
  char* format(strref fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vformat(fmt, args);
    va_end(args);
    return _buffer;
  }
};

template<size_t size>
char* format(char (&buffer)[size], strref fmt, ...) {
  Formatter formatter {buffer, size};

  va_list args;
  va_start(args, fmt);
  formatter.vformat(fmt, args);
  va_end(args);

  return formatter.buffer();
}

void initFormatBuffers(size_t bufferSize = 128u, size_t bufferCount = 4u);

char* format(strref fmt, ...);

}

#endif