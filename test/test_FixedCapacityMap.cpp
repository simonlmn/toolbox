
#include "../src/toolbox/FixedCapacityMap.h"
#include <cstdio>
#include <cassert>

int main() {
  toolbox::FixedCapacityMap<int, int, 10> map {};

  assert(map.capacity() == 10);
  assert(map.size() == 0);
  assert(map.find(5) == nullptr);
  
  assert(map.insert(5, 17) == true);
  assert(map.size() == 1);
  assert(*map.find(5) == 17);

  assert(map.insert(5, 21) == true);
  assert(map.size() == 1);
  assert(*map.find(5) == 21);

  assert(map.insert(8, 42) == true);
  assert(map.size() == 2);
  assert(*map.find(5) == 21);
  assert(*map.find(8) == 42);

  assert(map.insert(3, 99) == true);
  assert(map.size() == 3);
  assert(*map.find(5) == 21);
  assert(*map.find(8) == 42);
  assert(*map.find(3) == 99);

  assert(map.remove(5) == true);
  assert(map.size() == 2);
  assert(map.find(5) == nullptr);
  assert(*map.find(8) == 42);
  assert(*map.find(3) == 99);
  assert(map.remove(5) == false);

  map.clear();
  assert(map.size() == 0);
  assert(map.find(5) == nullptr);
  assert(map.find(8) == nullptr);
  assert(map.find(3) == nullptr);

  return 0;
}
