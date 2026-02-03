#include <yatest.h>
#include <Arduino.h>

void test_Conversion();
void test_FixedCapacityMap();
void test_String();

int main() {
  test_Conversion();
  test_FixedCapacityMap();
  test_String();
  return 0;
}
