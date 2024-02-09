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

static struct {
  size_t _bufferSize;
  size_t _bufferCount;
  size_t _currentBuffer;
  char* _buffers = nullptr;
  bool _firstInit = true;

  bool init(bool onlyIfFirst, size_t bufferSize = 128u, size_t bufferCount = 4u) {
    if (onlyIfFirst && !_firstInit) {
      return false;
    }

    if (_buffers) {
      delete[] _buffers;
      _buffers = nullptr;
    }
    _bufferSize = bufferSize;
    _bufferCount = bufferCount;
    _currentBuffer = 0;
    if (bufferSize > 0 && bufferCount > 0) {
      _buffers = new char[bufferSize * bufferCount];
    }

    _firstInit = false;
    return true;
  }
} SHARED_FORMAT_BUFFERS {};

void initFormatBuffers(size_t bufferSize = 128u, size_t bufferCount = 4u) {
  SHARED_FORMAT_BUFFERS.init(false, bufferSize, bufferCount);
}

char* format(strref fmt, ...) {
  static char NULL_BUFFER[1] = {'\0'};
  static const auto initialized = SHARED_FORMAT_BUFFERS.init(true);

  if (!SHARED_FORMAT_BUFFERS._buffers) {
    return NULL_BUFFER;
  }

  SHARED_FORMAT_BUFFERS._currentBuffer = (SHARED_FORMAT_BUFFERS._currentBuffer + 1) % SHARED_FORMAT_BUFFERS._bufferCount;
  Formatter formatter {
    SHARED_FORMAT_BUFFERS._buffers + (SHARED_FORMAT_BUFFERS._currentBuffer * SHARED_FORMAT_BUFFERS._bufferSize),
    SHARED_FORMAT_BUFFERS._bufferSize
    };

  va_list args;
  va_start(args, fmt);
  formatter.vformat(fmt, args);
  va_end(args);

  return formatter.buffer();
}

}

#endif