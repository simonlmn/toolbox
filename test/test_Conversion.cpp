#include <yatest.h>
#include <toolbox/Conversion.h>

using namespace yatest;

namespace {
static const TestSuite& TestConversion =
    suite("Conversion")
        .tests("convert string to uint32", [] () {
            toolbox::strref input {"4E20"};
            toolbox::Maybe<uint32_t> value = toolbox::convert<uint32_t>::fromString(input, nullptr, 16);
            expect::isTrue(value.available());
            expect::equals(value.get(), 20000u);
        })
        .tests("convert partial string to uint32 and advance to next part", [] () {
            toolbox::strref input {"4E20 TEST"};
            toolbox::strref next {"INIT"};
            toolbox::Maybe<uint32_t> value = toolbox::convert<uint32_t>::fromString(input, &next, 16);
            expect::isTrue(value.available());
            expect::equals(value.get(), 20000u);
            expect::equals(next, " TEST");
        })
        .tests("convert string to uint32 with invalid input", [] () {
            toolbox::strref input {"TEST"};
            toolbox::strref next {"INIT"};
            toolbox::Maybe<uint32_t> value = toolbox::convert<uint32_t>::fromString(input, &next, 16);
            expect::isFalse(value.available());
            expect::equals(next, "INIT");
        })
        .tests("convert string to bool", [] () {
            toolbox::strref input {"true"};
            toolbox::Maybe<bool> value = toolbox::convert<bool>::fromString(input, nullptr);
            expect::isTrue(value.available());
            expect::equals(value.get(), true);
        });
}
