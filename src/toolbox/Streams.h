#ifndef TOOLBOX_STREAMS_H
#define TOOLBOX_STREAMS_H

#include <Print.h>
#include <Stream.h>
#include <cstring>
#include <algorithm>
#include "ConstStr.h"

namespace toolbox {

/**
 * Minimal interface for character-based output streams.
 */
class IOutput {
public:
  virtual size_t write(char c) = 0;
  virtual size_t write(const char* string) = 0;
  virtual size_t write(const __FlashStringHelper* string) = 0;
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

  virtual size_t write(const char* string) override {
    return write(ConstStr{string});
  }
  
  virtual size_t write(const __FlashStringHelper* string) override {
    return write(ConstStr{string});
  }

  size_t write(const ConstStr& string) {
    auto oldPosition = _writePosition;
    string.copy(_string + _writePosition, _maxLength - _writePosition, 0, &_writePosition);
    return _writePosition - oldPosition;
  }
};

class PrintOutput final : public IOutput {
  Print& _print;

public:
  PrintOutput(Print& print) : _print(print) {}

  size_t write(char c) override {
    return _print.write(c);
  }

  size_t write(const char* string) override {
    return _print.write(string);
  }
  
  size_t write(const __FlashStringHelper* string) override {
    return _print.print(string);
  }
};

/**
 * Minimal interface for charachter-based input streams.
 */
struct IInput {
  virtual size_t available() const = 0;
  virtual size_t read(char* buffer, size_t bufferSize) = 0;
};

/**
 * Input implementation to read from a plain C-string (wrapped in
 * a ConstStr to allow strings stored in normal or flash memory).
 */
class StringInput final : public IInput {
  ConstStr _string;
  size_t _readPosition;

public:
  StringInput(ConstStr string) : _string(string), _readPosition(0) {}

  size_t available() const override {
    return _string.len() - _readPosition;
  }

  size_t read(char* buffer, size_t bufferSize) override {
    auto oldReadPosition = _readPosition;
    _string.copy(buffer, bufferSize, _readPosition, &_readPosition);
    return _readPosition - oldReadPosition;
  }
};

}

#endif