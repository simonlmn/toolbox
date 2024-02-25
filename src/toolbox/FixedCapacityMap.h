#ifndef TOOLBOX_FIXEDCAPACITYMAP_H_
#define TOOLBOX_FIXEDCAPACITYMAP_H_

#include <cstdlib>
#include <cstring>

namespace toolbox {

template<typename T>
struct Iterable {
  T* _begin;
  T* _end;

  Iterable(T* begin, T* end) : _begin(begin), _end(end) {}

  T* begin() const { return _begin; }
  T* end() const { return _end; }
};

template<typename K, typename V>
class Mapping {
  K _key;
  V _value;

public:
  Mapping() : _key(), _value() {}
  Mapping(const K& key, const V& value) : _key(key), _value(value) {}

  const K& key() const { return _key; }
  const V& value() const { return _value; }
  V& value() { return _value; }
};

template<typename K, typename V, size_t CAPACITY>
class FixedCapacityMap {
  using EntryType = Mapping<K, V>;

  EntryType _entries[CAPACITY];
  size_t _currentSize;

  size_t findIndex(const K& key) const {
    if (_currentSize == 0u) {
      return 0u;
    }

    size_t first = 0u;
    size_t last = _currentSize;
    while (first != last) {
      size_t middle = (first + last) / 2u;
      if (key <= _entries[middle].key()) {
        last = middle;
      } else {
        first = middle + 1;
      }
    }

    return first;
  }

public:
  FixedCapacityMap() : _entries(), _currentSize(0u) {}

  size_t capacity() const {
    return CAPACITY;
  }

  size_t size() const {
    return _currentSize;
  }

  void clear() {
    _currentSize = 0u;
  }

  bool insert(const K& key, const V& value) {
    if (_currentSize == CAPACITY) {
      return false;
    }

    size_t i = findIndex(key);
    if (i < _currentSize) {
      if (_entries[i].key() != key) {
        memmove(&_entries[i + 1], &_entries[i], (_currentSize - i) * sizeof(EntryType));
        _currentSize += 1u;
      }
    } else {
      _currentSize += 1u;
    }
    _entries[i] = { key, value };
    return true;
  }

  bool remove(const K& key) {
    size_t i = findIndex(key);
    if (i < _currentSize && _entries[i].key() == key) {
      memmove(&_entries[i], &_entries[i + 1], (_currentSize - i - 1) * sizeof(EntryType));
      _currentSize -= 1;
      return true;
    } else {
      return false;
    }
  }

  const V* find(const K& key) const {
    size_t i = findIndex(key);
    return (i < _currentSize) && (_entries[i].key() == key) ? &_entries[i].value() : nullptr;
  }

  V* find(const K& key) {
    size_t i = findIndex(key);
    return (i < _currentSize) && (_entries[i].key() == key) ? &_entries[i].value() : nullptr;
  }

  const EntryType* begin() const {
    return &_entries[0];
  }

  EntryType* begin() {
    return &_entries[0];
  }

  const EntryType* end() const {
    return &_entries[_currentSize];
  }
  
  EntryType* end() {
    return &_entries[_currentSize];
  }
};

}

#endif