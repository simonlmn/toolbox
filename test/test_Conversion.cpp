
#include <toolbox/Conversion.h>
#include <cstdio>
#include <cassert>

void test_Conversion() {
  {
    toolbox::strref input {"4E20"};
    toolbox::Maybe<uint32_t> value = toolbox::convert<uint32_t>::fromString(input, nullptr, 16);
    assert(value.available());
    assert(value.get() == 20000u);
  }

  {
    toolbox::strref input {"4E20 TEST"};
    toolbox::strref next {"INIT"};
    toolbox::Maybe<uint32_t> value = toolbox::convert<uint32_t>::fromString(input, &next, 16);
    assert(value.available());
    assert(value.get() == 20000u);
    assert(next == " TEST");
  }

  {
    toolbox::strref input {"TEST"};
    toolbox::strref next {"INIT"};
    toolbox::Maybe<uint32_t> value = toolbox::convert<uint32_t>::fromString(input, &next, 16);
    assert(!value.available());
    assert(next == "INIT");
  }

  {
    toolbox::strref input {"true"};
    toolbox::Maybe<bool> value = toolbox::convert<bool>::fromString(input, nullptr);
    assert(value.available());
    assert(value.get() == true);
  }

}
