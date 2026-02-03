
#include <yatest.h>
#include <toolbox/String.h>
#include <cstring>

namespace {
static const yatest::TestSuite& TestString =
    yatest::suite("String")
        .tests("default strref", []() {
          toolbox::strref a;
          yatest::expect(a.length() == 0, "default length");
          yatest::expect(a.offset() == 0, "default offset");
          yatest::expect(a.charAt(1) == '\0', "default charAt");
          yatest::expect(a.indexOf('a') == -1, "default indexOf");
          yatest::expect(a.isInProgmem() == false, "default progmem");
          yatest::expect(a.isZeroTerminated() == true, "default zero terminated");
          yatest::expect(a.leftmost(5) == "", "default leftmost");
          yatest::expect(a.rightmost(5) == "", "default rightmost");
          yatest::expect(a.middle(2, 4) == "", "default middle");
          yatest::expect(a.substring(2, 2) == "", "default substring");
          yatest::expect(a.skip(3) == "", "default skip");
        })
        .tests("basic operations", []() {
          toolbox::strref a{"Hello World!"};
          yatest::expect(a.length() == 12, "length");
          yatest::expect(a.offset() == 0, "offset");
          yatest::expect(a.charAt(1) == 'e', "charAt");
          yatest::expect(a.indexOf('o') == 4, "indexOf");
          yatest::expect(a.isInProgmem() == false, "progmem");
          yatest::expect(a.isZeroTerminated() == true, "zero terminated");
          yatest::expect(a.leftmost(5) == "Hello", "leftmost");
          yatest::expect(a.leftmost(5).offset() == 0, "leftmost offset");
          yatest::expect(a.leftmost(5).length() == 5, "leftmost length");
          yatest::expect(a.leftmost(5).isZeroTerminated() == false, "leftmost zero terminated");
          yatest::expect(a.rightmost(5) == "orld!", "rightmost");
          yatest::expect(a.rightmost(5).offset() == 7, "rightmost offset");
          yatest::expect(a.rightmost(5).length() == 5, "rightmost length");
          yatest::expect(a.rightmost(5).isZeroTerminated() == true, "rightmost zero terminated");
          yatest::expect(a.middle(2, 4) == "ll", "middle");
          yatest::expect(a.middle(2, 4).offset() == 2, "middle offset");
          yatest::expect(a.middle(2, 4).length() == 2, "middle length");
          yatest::expect(a.middle(2, 4).isZeroTerminated() == false, "middle zero terminated");
          yatest::expect(a.substring(2, 2) == "ll", "substring");
          yatest::expect(a.substring(2, 2).offset() == 2, "substring offset");
          yatest::expect(a.substring(2, 2).length() == 2, "substring length");
          yatest::expect(a.substring(2, 2).isZeroTerminated() == false, "substring zero terminated");
          yatest::expect(a.skip(3) == "lo World!", "skip");
          yatest::expect(a.skip(3).offset() == 3, "skip offset");
          yatest::expect(a.skip(3).length() == 9, "skip length");
          yatest::expect(a.skip(3).isZeroTerminated() == true, "skip zero terminated");

          char buffer[15];
          yatest::expect(a.copy(buffer, 15, true) == 12, "copy length");
          yatest::expect(a.substring(3, 4).copy(buffer, 15, true) == 4, "substring copy length");

          char* array = a.toCharArray(false);
          delete[] array;

          yatest::expect(a.substring(8, 5) == "rld!", "substring rld");
          yatest::expect(a.substring(13, 5) == "", "substring out of range");
          yatest::expect(a.substring(0, 0) == "", "substring zero length");
          yatest::expect(a.substring(0, 15) == "Hello World!", "substring full");
        })
        .tests("progmem strings and comparisons", []() {
          toolbox::strref a{"Hello World!"};
          toolbox::strref b{FPSTR("Hello World2")};
          yatest::expect(b.length() == 12, "progmem length");
          yatest::expect(b.offset() == 0, "progmem offset");
          yatest::expect(b.charAt(1) == 'e', "progmem charAt");
          yatest::expect(b.indexOf('o') == 4, "progmem indexOf");
          yatest::expect(b.isInProgmem() == true, "progmem flag");
          yatest::expect(b.isZeroTerminated() == true, "progmem zero terminated");
          yatest::expect(b.leftmost(5) == "Hello", "progmem leftmost");
          yatest::expect(b.leftmost(5).offset() == 0, "progmem leftmost offset");
          yatest::expect(b.leftmost(5).length() == 5, "progmem leftmost length");
          yatest::expect(b.leftmost(5).isZeroTerminated() == false, "progmem leftmost zero terminated");
          yatest::expect(b.rightmost(5) == "orld2", "progmem rightmost");
          yatest::expect(b.rightmost(5).offset() == 7, "progmem rightmost offset");
          yatest::expect(b.rightmost(5).length() == 5, "progmem rightmost length");
          yatest::expect(b.rightmost(5).isZeroTerminated() == true, "progmem rightmost zero terminated");
          yatest::expect(b.middle(2, 4) == "ll", "progmem middle");
          yatest::expect(b.middle(2, 4).offset() == 2, "progmem middle offset");
          yatest::expect(b.middle(2, 4).length() == 2, "progmem middle length");
          yatest::expect(b.middle(2, 4).isZeroTerminated() == false, "progmem middle zero terminated");
          yatest::expect(b.substring(2, 2) == "ll", "progmem substring");
          yatest::expect(b.substring(2, 2).offset() == 2, "progmem substring offset");
          yatest::expect(b.substring(2, 2).length() == 2, "progmem substring length");
          yatest::expect(b.substring(2, 2).isZeroTerminated() == false, "progmem substring zero terminated");
          yatest::expect(b.skip(3) == "lo World2", "progmem skip");
          yatest::expect(b.skip(3).offset() == 3, "progmem skip offset");
          yatest::expect(b.skip(3).length() == 9, "progmem skip length");
          yatest::expect(b.skip(3).isZeroTerminated() == true, "progmem skip zero terminated");

          yatest::expect(a != b, "comparison a != b");
          yatest::expect(b != a, "comparison b != a");
          yatest::expect(a < b, "comparison a < b");
          yatest::expect(b > a, "comparison b > a");
        })
        .tests("materialize and shared_str", []() {
          toolbox::strref c{"abc\0def", 8};
          toolbox::strref d{"abc\0def", 8};
          yatest::expect(c.length() == 8, "length with embedded null");
          yatest::expect(c.isZeroTerminated() == false, "not zero terminated");
          yatest::expect(c == d, "equality with embedded null");

          toolbox::strref e{"abcdef"};
          toolbox::shared_str f = e.materialize();
          yatest::expect(!f.empty(), "materialize not empty");
          yatest::expect(f.length() == 6, "materialize length");
          yatest::expect(strcmp(f.cstr(), "abcdef") == 0, "materialize contents");
          yatest::expect(e == f, "materialize equality");

          toolbox::shared_str g = f;

          f.clear();
          yatest::expect(f.empty(), "clear empties");
          yatest::expect(f.length() == 0, "clear length");
          yatest::expect(strcmp(f.cstr(), "") == 0, "clear contents");

          yatest::expect(!g.empty(), "copy not empty");
          yatest::expect(g.length() == 6, "copy length");
          yatest::expect(strcmp(g.cstr(), "abcdef") == 0, "copy contents");
        });
}
