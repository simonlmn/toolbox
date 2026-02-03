
#include <yatest.h>
#include <toolbox/String.h>
#include <cstring>

using namespace yatest;

namespace {
static const TestSuite& TestString =
    suite("String")
        .tests("default strref", []() {
            toolbox::strref a;
            expect::equals(a.length(), 0u, "default length");
            expect::equals(a.offset(), 0u, "default offset");
            expect::equals(a.charAt(1), '\0', "default charAt");
            expect::equals(a.indexOf('a'), -1, "default indexOf");
            expect::isFalse(a.isInProgmem(), "default progmem");
            expect::isTrue(a.isZeroTerminated(), "default zero terminated");
            expect::equals(a.leftmost(5), "", "default leftmost");
            expect::equals(a.rightmost(5), "", "default rightmost");
            expect::equals(a.middle(2, 4), "", "default middle");
            expect::equals(a.substring(2, 2), "", "default substring");
            expect::equals(a.skip(3), "", "default skip");
        })
        .tests("basic operations", []() {
            toolbox::strref a{"Hello World!"};
            expect::equals(a.length(), 12u, "length");
            expect::equals(a.offset(), 0u, "offset");
            expect::equals(a.charAt(1), 'e', "charAt");
            expect::equals(a.indexOf('o'), 4, "indexOf");
            expect::isFalse(a.isInProgmem(), "progmem");
            expect::isTrue(a.isZeroTerminated(), "zero terminated");
            expect::equals(a.leftmost(5), "Hello", "leftmost");
            expect::equals(a.leftmost(5).offset(), 0u, "leftmost offset");
            expect::equals(a.leftmost(5).length(), 5u, "leftmost length");
            expect::isFalse(a.leftmost(5).isZeroTerminated(), "leftmost zero terminated");
            expect::equals(a.rightmost(5), "orld!", "rightmost");
            expect::equals(a.rightmost(5).offset(), 7u, "rightmost offset");
            expect::equals(a.rightmost(5).length(), 5u, "rightmost length");
            expect::isTrue(a.rightmost(5).isZeroTerminated(), "rightmost zero terminated");
            expect::equals(a.middle(2, 4), "ll", "middle");
            expect::equals(a.middle(2, 4).offset(), 2u, "middle offset");
            expect::equals(a.middle(2, 4).length(), 2u, "middle length");
            expect::isFalse(a.middle(2, 4).isZeroTerminated(), "middle zero terminated");
            expect::equals(a.substring(2, 2), "ll", "substring");
            expect::equals(a.substring(2, 2).offset(), 2u, "substring offset");
            expect::equals(a.substring(2, 2).length(), 2u, "substring length");
            expect::isFalse(a.substring(2, 2).isZeroTerminated(), "substring zero terminated");
            expect::equals(a.skip(3), "lo World!", "skip");
            expect::equals(a.skip(3).offset(), 3u, "skip offset");
            expect::equals(a.skip(3).length(), 9u, "skip length");
            expect::isTrue(a.skip(3).isZeroTerminated(), "skip zero terminated");
            char buffer[15];
            expect::equals(a.copy(buffer, 15, true), 12u, "copy length");
            expect::equals(a.substring(3, 4).copy(buffer, 15, true), 4u, "substring copy length");
            char* array = a.toCharArray(false);
            delete[] array;

            expect::equals(a.substring(8, 5), "rld!", "substring rld");
            expect::equals(a.substring(13, 5), "", "substring out of range");
            expect::equals(a.substring(0, 0), "", "substring zero length");
            expect::equals(a.substring(0, 15), "Hello World!", "substring full");
        })
        .tests("progmem strings and comparisons", []() {
            toolbox::strref a{"Hello World!"};
            toolbox::strref b{FPSTR("Hello World2")};
            expect::equals(b.length(), 12u, "progmem length");
            expect::equals(b.offset(), 0u, "progmem offset");
            expect::equals(b.charAt(1), 'e', "progmem charAt");
            expect::equals(b.indexOf('o'), 4, "progmem indexOf");
            expect::isTrue(b.isInProgmem(), "progmem flag");
            expect::isTrue(b.isZeroTerminated(), "progmem zero terminated");
            expect::equals(b.leftmost(5), "Hello", "progmem leftmost");
            expect::equals(b.leftmost(5).offset(), 0u, "progmem leftmost offset");
            expect::equals(b.leftmost(5).length(), 5u, "progmem leftmost length");
            expect::isFalse(b.leftmost(5).isZeroTerminated(), "progmem leftmost zero terminated");
            expect::equals(b.rightmost(5), "orld2", "progmem rightmost");
            expect::equals(b.rightmost(5).offset(), 7u, "progmem rightmost offset");
            expect::equals(b.rightmost(5).length(), 5u, "progmem rightmost length");
            expect::isTrue(b.rightmost(5).isZeroTerminated(), "progmem rightmost zero terminated");
            expect::equals(b.middle(2, 4), "ll", "progmem middle");
            expect::equals(b.middle(2, 4).offset(), 2u, "progmem middle offset");
            expect::equals(b.middle(2, 4).length(), 2u, "progmem middle length");
            expect::isFalse(b.middle(2, 4).isZeroTerminated(), "progmem middle zero terminated");
            expect::equals(b.substring(2, 2), "ll", "progmem substring");
            expect::equals(b.substring(2, 2).offset(), 2u, "progmem substring offset");
            expect::equals(b.substring(2, 2).length(), 2u, "progmem substring length");
            expect::isFalse(b.substring(2, 2).isZeroTerminated(), "progmem substring zero terminated");
            expect::equals(b.skip(3), "lo World2", "progmem skip");
            expect::equals(b.skip(3).offset(), 3u, "progmem skip offset");
            expect::equals(b.skip(3).length(), 9u, "progmem skip length");
            expect::isTrue(b.skip(3).isZeroTerminated(), "progmem skip zero terminated");

            expect::isTrue(a != b, "comparison a != b");
            expect::isTrue(b != a, "comparison b != a");
            expect::isTrue(a < b, "comparison a < b");
            expect::isTrue(b > a, "comparison b > a");
        })
        .tests("materialize and shared_str", []() {
            toolbox::strref c{"abc\0def", 8};
            toolbox::strref d{"abc\0def", 8};
            expect::equals(c.length(), 8u, "length with embedded null");
            expect::isFalse(c.isZeroTerminated(), "not zero terminated");
            expect::isTrue(c == d, "equality with embedded null");

            toolbox::strref e{"abcdef"};
            toolbox::shared_str f = e.materialize();
            expect::isFalse(f.empty(), "materialize not empty");
            expect::equals(f.length(), 6u, "materialize length");
            expect::equals(strcmp(f.cstr(), "abcdef"), 0, "materialize contents");
            expect::isTrue(e == f, "materialize equality");

            toolbox::shared_str g = f;

            f.clear();
            expect::isTrue(f.empty(), "clear empties");
            expect::equals(f.length(), 0u, "clear length");
            expect::equals(strcmp(f.cstr(), ""), 0, "clear contents");

            expect::isFalse(g.empty(), "copy not empty");
            expect::equals(g.length(), 6u, "copy length");
            expect::equals(strcmp(g.cstr(), "abcdef"), 0, "copy contents");
        });
}
