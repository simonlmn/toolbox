#ifndef TOOLBOX_MAYBE_H
#define TOOLBOX_MAYBE_H

#ifndef ARDUINO_AVR_NANO
#include <functional>
#include <type_traits>
#endif

namespace toolbox {

template<typename T>
class Maybe;

template<typename T>
struct is_maybe : std::false_type {};

template<typename U>
struct is_maybe<Maybe<U>> : std::true_type {};

template<typename R>
struct unwrap_maybe {
  using type = R;
};

template<typename U>
struct unwrap_maybe<Maybe<U>> {
  using type = U;
};

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

  /**
   * Apply a mapping function when a value is available.
   */
  #ifndef ARDUINO_AVR_NANO
  template<typename F>
  Maybe<typename unwrap_maybe<std::invoke_result_t<F, const T&>>::type> then(F f) const {
    using R = std::invoke_result_t<F, const T&>;
    if (available()) {
      if constexpr (is_maybe<R>::value) {
        return f(get());
      } else {
        return Maybe<R>(f(get()));
      }
    } else {
      return {};
    }
  }
  #else
  template<typename R, typename F>
  Maybe<R> then(F f) const {
    if (available()) {
      return f(get());
    } else {
      return {};
    }
  }
  #endif

  /**
   * Provide a fallback value by invoking a thunk when empty.
   */
  #ifndef ARDUINO_AVR_NANO
  template<typename F, std::enable_if_t<std::is_invocable<F>::value, bool> = true>
  const T& otherwise(F f) const {
    if (available()) {
      return get();
    } else {
      return f();
    }
  }
  #endif

  /**
   * Provide a fallback value when empty.
   */
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