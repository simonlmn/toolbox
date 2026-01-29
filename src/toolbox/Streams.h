#ifndef TOOLBOX_STREAMS_H
#define TOOLBOX_STREAMS_H

#include <Print.h>
#include <Stream.h>
#include <cstring>
#include <algorithm>
#include "String.h"

namespace toolbox {

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
  StringInput(const strref& string) : _string(string) {}

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

/**
 * Input implementation that reads from an Arduino Stream.
 */
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

/**
 * Adapter that exposes an IInput instance as an Arduino Stream.
 */
class InputStream final : public Stream {
  IInput& _input;

public:
  InputStream(IInput& input) : _input(input) {}

  size_t write(uint8_t) override { return 0; }

  size_t write(const uint8_t*, size_t) override { return 0; }

  int availableForWrite() override { return 0; }

  int available() override { return _input.available(); }

  int read() override {
    char c;
    if (_input.read(&c, 1) == 1) {
      return c;
    } else {
      return -1;
    }
  }

  int peek() override {
      return -1;
  }

#ifdef ARDUINO_ARCH_ESP8266
  size_t readBytes(char* buffer, size_t len) override {
    return _input.read(buffer, len);
  }

  int read(uint8_t* buffer, size_t len) override {
    return _input.read((char*)buffer, len);
  }

  bool outputCanTimeout() override {
    return false;
  }

  bool inputCanTimeout() override {
    return false;
  }

  ssize_t streamRemaining() override {
      return -1;
  }
#else
  size_t sendAll(Print& output) {
    size_t totalSize = 0;
    char buffer[32];
    while (_input.available()) {
      size_t bufferSize = _input.read(buffer, std::size(buffer));
      size_t writeSize = output.write(buffer, bufferSize);
      totalSize += writeSize;
      if (writeSize != bufferSize) {
        break;
      }
    }
    return totalSize;
  }
#endif
};

/**
 * Minimal interface for output streams.
 */
class IOutput {
public:
  virtual size_t write(char c) = 0;
  virtual size_t write(const strref& data) = 0;
#ifdef TOOLBOX_IOUTPUT_IINPUT_SUPPORT
  virtual size_t write(IInput& input) = 0;
#endif
};

/**
 * Output implementation to allow writing to a plain C-string style
 * pre-allocated buffer.
 */
class StringOutput final : public IOutput {
  char* _string;
  size_t _maxLength;
  size_t _writePosition;

  size_t available() const {
    return _maxLength - _writePosition;
  }

public:
  template<size_t N>
  StringOutput(char (&string)[N]) : StringOutput(string, N - 1) {}

  StringOutput(char* string, size_t maxLength) : _string(string), _maxLength(maxLength), _writePosition(strlen(string)) {}

  size_t write(char c) override {
    if (available() > 0) {
      _string[_writePosition++] = c;
      _string[_writePosition] = '\0';
      return 1;
    } else {
      return 0;
    }
  }

  size_t write(const strref& string) override {
    size_t length = string.copy(_string + _writePosition, available() + 1, true);
    _writePosition += length;
    return length;
  }

#ifdef TOOLBOX_IOUTPUT_IINPUT_SUPPORT
  size_t write(IInput& input) override {
    size_t totalLength = 0;
    while (available() && input.available()) {
      size_t length = input.read(_string + _writePosition, available());
      totalLength += length;
      _writePosition += length;
      _string[_writePosition] = '\0';
    }
    return totalLength;
  }
#endif
};

/**
 * Output implementation that writes to an Arduino Print.
 */
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

#ifdef TOOLBOX_IOUTPUT_IINPUT_SUPPORT
  size_t write(IInput& input) override {
    return InputStream{input}.sendAll(_print);
  }
#endif
};

}

#endif