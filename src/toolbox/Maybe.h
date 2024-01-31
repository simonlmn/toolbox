#ifndef TOOLBOX_MAYBE_H
#define TOOLBOX_MAYBE_H

#include <functional>

namespace toolbox {

/**
 * Basic implementation of a type of values which may be not available (e.g. due to an error), similar to std::optional.
 */
template<typename T>
class Maybe final {
  bool _hasValue;
  T _value;

public:
  Maybe() : _hasValue(false) {}
  Maybe(T value) : _hasValue(true), _value(value) {}
  Maybe(bool hasValue, T value) : _hasValue(_hasValue), _value(value) {}

  bool hasValue() const { return _hasValue; }
  const T& value() const { return _value; }

  operator bool() const { return hasValue(); }

  bool operator==(const Maybe& other) const { return _hasValue == other._hasValue && (*this == other._value); }
  bool operator!=(const Maybe& other) const { return !(*this == other); }
  bool operator==(const T& other) const { return _hasValue && (_value == other); }
  bool operator!=(const T& other) const { return _hasValue && (_value != other); }

  template<typename F>
  Maybe<typename std::invoke_result<F, const T&>::type> then(F f) const {
    if (hasValue()) {
      return f(value());
    } else {
      return {};
    }
  }

  template<typename F>
  const T& otherwise(F f) const {
    if (hasValue()) {
      return value();
    } else {
      return f();
    }
  }

  const T& otherwise(const T& v) const {
    if (hasValue()) {
      return value();
    } else {
      return v;
    }
  }
};

}

#endif