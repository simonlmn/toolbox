#ifndef TOOLBOX_STREAMS_H
#define TOOLBOX_STREAMS_H

#include <Print.h>
#include <Stream.h>
#include <cstring>
#include <algorithm>
#include "String.h"

namespace toolbox {

/**
 * Minimal interface for output streams.
 */
class IOutput {
public:
  virtual size_t write(char c) = 0;
  virtual size_t write(const strref& data) = 0;
};

/**
 * Output implementation to allow writing to a plain C-string style
 * pre-allocated buffer.
 */
class StringOutput final : public IOutput {
  char* _string;
  size_t _maxLength;
  size_t _writePosition;

public:
  template<size_t N>
  StringOutput(char (&string)[N]) : StringOutput(string, N) {}

  StringOutput(char* string, size_t maxLength) : _string(string), _maxLength(maxLength), _writePosition(strlen(string)) {}

  virtual size_t write(char c) override {
    if (_writePosition < _maxLength) {
      _string[_writePosition++] = c;
      _string[_writePosition] = '\0';
      return 1;
    } else {
      return 0;
    }
  }

  size_t write(const strref& string) override {
    size_t length = string.copy(_string + _writePosition, _maxLength - _writePosition, true);
    _writePosition += length;
    return length;
  }
};

class PrintOutput final : public IOutput {
  Print& _print;

public:
  PrintOutput(Print& print) : _print(print) {}

  size_t write(char c) override {
    return _print.write(c);
  }

  size_t write(const strref& string) override {
    if (string.isZeroTerminated()) {
      return string.isInProgmem() ? _print.print(string.fpstr()) : _print.print(string.cstr());
    } else {
      if (string.isInProgmem()) {
        PGM_P p = reinterpret_cast<PGM_P>(string.fpstr());
        size_t length = string.length();
        size_t n = 0;
        while (n < length) {
          if (write(pgm_read_byte(p++))) n++;
          else break;
        }
        return n;
      } else {
        return _print.write(string.cstr(), string.length());
      }
    }
  }
};

/**
 * Minimal interface for input streams.
 */
struct IInput {
  virtual size_t available() const = 0;
  virtual size_t read(char* buffer, size_t bufferSize) = 0;
  virtual size_t readString(char* buffer, size_t bufferSize) = 0;
};

/**
 * Input implementation to read from a string (wrapped in
 * a strref to allow strings stored in normal or flash memory).
 */
class StringInput final : public IInput {
  strref _string;
  size_t _readPosition;

public:
  StringInput(strref string) : _string(string) {}

  size_t available() const override {
    return _string.length();
  }

  size_t read(char* buffer, size_t bufferSize) override {
    size_t length = _string.copy(buffer, bufferSize, false);
    _string = _string.skip(length);
    return length;
  }

  size_t readString(char* buffer, size_t bufferSize) override {
    size_t length = _string.copy(buffer, bufferSize, true);
    _string = _string.skip(length);
    return length;
  }
};

class StreamInput final : public IInput {
  Stream& _stream;

public:
  StreamInput(Stream& stream) : _stream(stream) {}

  size_t available() const override {
    return _stream.available();
  }

  size_t read(char* buffer, size_t bufferSize) override {
    return _stream.readBytes(buffer, bufferSize);
  }

  size_t readString(char* buffer, size_t bufferSize) override {
    if (bufferSize == 0) {
      return 0;
    }
    size_t length = _stream.readBytes(buffer, bufferSize - 1);
    buffer[length + 1] = '\0';
    return length;
  }
};

}

#endif