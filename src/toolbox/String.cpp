#include "String.h"

namespace toolbox {

int memcmp_P2(PGM_P str1P, PGM_P str2P, size_t size)
{
    int result = 0;

    while (size > 0)
    {
        char ch1 = pgm_read_byte(str1P++);
        char ch2 = pgm_read_byte(str2P++);
        result = ch1 - ch2;
        if (result != 0)
        {
            break;
        }

        size--;
    }

    return result;
}

const char strref::EMPTY_CSTR[] = "";
const char strref::EMPTY_FPSTR[] PROGMEM = "";
const strref strref::EMPTY {};

} // namespace toolbox
