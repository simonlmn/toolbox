#include <yatest.h>
#include <toolbox/Decimal.h>

namespace {
static const yatest::TestSuite& TestDecimal =
    yatest::suite("Decimal")
        .tests("convert fixed point to string", [] () {
            toolbox::Decimal d1 = toolbox::Decimal::fromFixedPoint(12345, 2);
            yatest::expect(d1.toString() == "123.45", "");
        })
        .tests("convert negative fixed point to string", [] () {
            toolbox::Decimal d1 = toolbox::Decimal::fromFixedPoint(-12345, 2);
            yatest::expect(d1.toString() == "-123.45", "");
        })
        .tests("convert fixed point with many decimals to string", [] () {
            toolbox::Decimal d1 = toolbox::Decimal::fromFixedPoint(123456789, 8);
            yatest::expect(d1.toString() == "1.23456789", "");
        })
        .tests("parse fixed point from string", [] () {
            auto d1 = toolbox::Decimal::fromString("123.45");
            yatest::expect(d1.available(), "");
            yatest::expect(d1.get().toFixedPoint(2) == 12345, "");
        })
        .tests("parse negative fixed point from string", [] () {
            auto d1 = toolbox::Decimal::fromString("-123.45");
            yatest::expect(d1.available(), "");
            yatest::expect(d1.get().toFixedPoint(2) == -12345, "");
        })
        .tests("parse fixed point with many decimals from string", [] () {
            auto d1 = toolbox::Decimal::fromString("1.23456789");
            yatest::expect(d1.available(), "");
            yatest::expect(d1.get().toFixedPoint(8) == 123456789, "");
        });

static const yatest::TestSuite& TestDecimalHelpers =
    yatest::suite("Decimal Helpers")
        .tests("number of digits", [] () {
            yatest::expect(toolbox::numberOfDigits(0) == 1, "zero has 1 digit");
            yatest::expect(toolbox::numberOfDigits(1) == 1, "1 has 1 digit");
            yatest::expect(toolbox::numberOfDigits(-1) == 1, "-1 has 1 digit");
            yatest::expect(toolbox::numberOfDigits(9) == 1, "9 has 1 digit");
            yatest::expect(toolbox::numberOfDigits(10) == 2, "10 has 2 digits");
            yatest::expect(toolbox::numberOfDigits(99) == 2, "99 has 2 digits");
            yatest::expect(toolbox::numberOfDigits(100) == 3, "100 has 3 digits");
            yatest::expect(toolbox::numberOfDigits(1000000) == 7, "1000000 has 7 digits");
            yatest::expect(toolbox::numberOfDigits(-1000000) == 7, "-1000000 has 7 digits");
            yatest::expect(toolbox::numberOfDigits(1000000000) == 10, "1000000000 has 10 digits");
            yatest::expect(toolbox::numberOfDigits(-1000000000) == 10, "-1000000000 has 10 digits");
        });
}
