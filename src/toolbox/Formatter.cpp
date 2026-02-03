#include "Formatter.h"

#ifndef ARDUINO_AVR_NANO
#include <cstdarg>
#endif

namespace toolbox {

namespace {
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
} // namespace

void initFormatBuffers(size_t bufferSize, size_t bufferCount) {
  SHARED_FORMAT_BUFFERS.init(false, bufferSize, bufferCount);
}

char* format(strref fmt, ...) {
  static char NULL_BUFFER[1] = {'\0'};
  static const auto initialized = SHARED_FORMAT_BUFFERS.init(true);
  (void)initialized;
  
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

} // namespace toolbox
