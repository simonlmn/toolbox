
#include <yatest.h>
#include <toolbox/FixedCapacityMap.h>

namespace {
static const yatest::TestSuite& TestFixedCapacityMap =
    yatest::suite("FixedCapacityMap")
        .tests("insert, update, remove, clear", []() {
          toolbox::FixedCapacityMap<int, int, 10> map{};

          yatest::expect(map.capacity() == 10, "capacity is 10");
          yatest::expect(map.size() == 0, "size starts at 0");
          yatest::expect(map.find(5) == nullptr, "find missing key returns nullptr");

          yatest::expect(map.insert(5, 17) == true, "insert first value");
          yatest::expect(map.size() == 1, "size after insert");
          yatest::expect(*map.find(5) == 17, "value after insert");

          yatest::expect(map.insert(5, 21) == true, "update existing key");
          yatest::expect(map.size() == 1, "size unchanged after update");
          yatest::expect(*map.find(5) == 21, "value after update");

          yatest::expect(map.insert(8, 42) == true, "insert second key");
          yatest::expect(map.size() == 2, "size after second insert");
          yatest::expect(*map.find(5) == 21, "value for first key");
          yatest::expect(*map.find(8) == 42, "value for second key");

          yatest::expect(map.insert(3, 99) == true, "insert third key");
          yatest::expect(map.size() == 3, "size after third insert");
          yatest::expect(*map.find(5) == 21, "value still for first key");
          yatest::expect(*map.find(8) == 42, "value still for second key");
          yatest::expect(*map.find(3) == 99, "value for third key");

          yatest::expect(map.remove(5) == true, "remove existing key");
          yatest::expect(map.size() == 2, "size after remove");
          yatest::expect(map.find(5) == nullptr, "removed key missing");
          yatest::expect(*map.find(8) == 42, "value for second key after remove");
          yatest::expect(*map.find(3) == 99, "value for third key after remove");
          yatest::expect(map.remove(5) == false, "remove missing key returns false");

          map.clear();
          yatest::expect(map.size() == 0, "size after clear");
          yatest::expect(map.find(5) == nullptr, "key missing after clear");
          yatest::expect(map.find(8) == nullptr, "key missing after clear");
          yatest::expect(map.find(3) == nullptr, "key missing after clear");
        });
}
