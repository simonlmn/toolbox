#ifndef TOOLBOX_MAYBE_H
#define TOOLBOX_MAYBE_H

#ifndef ARDUINO_AVR_NANO
#include <functional>
#endif

namespace toolbox {

/**
 * Basic implementation of a type of values which may be not available (e.g. due to an error), similar to std::optional.
 */
template<typename T>
class Maybe final {
  bool _available;
  T _value;

public:
  Maybe() : _available(false) {}
  Maybe(T value) : _available(true), _value(value) {}
  Maybe(bool available, T value) : _available(available), _value(value) {}

  bool available() const { return _available; }
  const T& get() const { return _value; }
  T& get() { return _value; }

  operator bool() const { return available(); }

  bool operator==(const Maybe& other) const { return _available == other._available && (*this == other._value); }
  bool operator!=(const Maybe& other) const { return !(*this == other); }
  bool operator==(const T& other) const { return _available && (_value == other); }
  bool operator!=(const T& other) const { return _available && (_value != other); }

  #ifndef ARDUINO_AVR_NANO
  template<typename F>
  Maybe<typename std::invoke_result<F, const T&>::type> then(F f) const {
    if (available()) {
      return f(get());
    } else {
      return {};
    }
  }
  #else
  template<typename F, typename R>
  Maybe<R> then(F f) const {
    if (available()) {
      return f(get());
    } else {
      return {};
    }
  }
  #endif

  template<typename F>
  const T& otherwise(F f) const {
    if (available()) {
      return get();
    } else {
      return f();
    }
  }

  const T& otherwise(const T& v) const {
    if (available()) {
      return get();
    } else {
      return v;
    }
  }
};

}

#endif