
#include <yatest.h>
#include <toolbox/FixedCapacityMap.h>

using namespace yatest;

namespace {
static const TestSuite& TestFixedCapacityMap =
    suite("FixedCapacityMap")
        .tests("insert, update, remove, clear", []() {
            toolbox::FixedCapacityMap<int, int, 10> map{};

            expect::equals(map.capacity(), 10u, "capacity is 10");
            expect::equals(map.size(), 0u, "size starts at 0");
            expect::isNull(map.find(5), "find missing key returns nullptr");

            expect::equals(map.insert(5, 17), true, "insert first value");
            expect::equals(map.size(), 1u, "size after insert");
            expect::equals(*map.find(5), 17, "value after insert");
            expect::equals(map.insert(5, 21), true, "update existing key");
            expect::equals(map.size(), 1u, "size unchanged after update");
            expect::equals(*map.find(5), 21, "value after update");

            expect::equals(map.insert(8, 42), true, "insert second key");
            expect::equals(map.size(), 2u, "size after second insert");
            expect::equals(*map.find(5), 21, "value for first key");
            expect::equals(*map.find(8), 42, "value for second key");

            expect::equals(map.insert(3, 99), true, "insert third key");
            expect::equals(map.size(), 3u, "size after third insert");
            expect::equals(*map.find(5), 21, "value still for first key");
            expect::equals(*map.find(8), 42, "value still for second key");
            expect::equals(*map.find(3), 99, "value for third key");
            expect::equals(map.remove(5), true, "remove existing key");
            expect::equals(map.size(), 2u, "size after remove");
            expect::isNull(map.find(5), "removed key missing");
            expect::equals(*map.find(8), 42, "value for second key after remove");
            expect::equals(*map.find(3), 99, "value for third key after remove");
            expect::equals(map.remove(5), false, "remove missing key returns false");

            map.clear();
            expect::equals(map.size(), 0u, "size after clear");
            expect::isNull(map.find(5), "key missing after clear");
            expect::isNull(map.find(8), "key missing after clear");
            expect::isNull(map.find(3), "key missing after clear");
        });
}
