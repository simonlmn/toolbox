
#include "../src/toolbox/String.h"
#include <cstdio>
#include <cassert>

int main() {
  toolbox::strref a;
  assert(a.length() == 0);
  assert(a.offset() == 0);
  assert(a.charAt(1) == '\0');
  assert(a.indexOf('a') == -1);
  assert(a.isInProgmem() == false);
  assert(a.isZeroTerminated() == true);
  assert(a.leftmost(5) == "");
  assert(a.rightmost(5) == "");
  assert(a.middle(2,4) == "");
  assert(a.substring(2,2) == "");
  assert(a.skip(3) == "");

  a = toolbox::strref{"Hello World!"};
  assert(a.length() == 12);
  assert(a.offset() == 0);
  assert(a.charAt(1) == 'e');
  assert(a.indexOf('o') == 4);
  assert(a.isInProgmem() == false);
  assert(a.isZeroTerminated() == true);
  assert(a.leftmost(5) == "Hello");
  assert(a.leftmost(5).offset() == 0);
  assert(a.leftmost(5).length() == 5);
  assert(a.leftmost(5).isZeroTerminated() == false);
  assert(a.rightmost(5) == "orld!");
  assert(a.rightmost(5).offset() == 7);
  assert(a.rightmost(5).length() == 5);
  assert(a.rightmost(5).isZeroTerminated() == true);
  assert(a.middle(2,4) == "ll");
  assert(a.middle(2,4).offset() == 2);
  assert(a.middle(2,4).length() == 2);
  assert(a.middle(2,4).isZeroTerminated() == false);
  assert(a.substring(2,2) == "ll");
  assert(a.substring(2,2).offset() == 2);
  assert(a.substring(2,2).length() == 2);
  assert(a.substring(2,2).isZeroTerminated() == false);
  assert(a.skip(3) == "lo World!");
  assert(a.skip(3).offset() == 3);
  assert(a.skip(3).length() == 9);
  assert(a.skip(3).isZeroTerminated() == true);

  char buffer[15];
  assert(a.copy(buffer, 15, true) == 12);
  assert(a.substring(3,4).copy(buffer, 15, true) == 4);

  char* array = a.toCharArray(false);
  delete[] array;

  assert(a.substring(8, 5) == "rld!");
  assert(a.substring(13, 5) == "");
  assert(a.substring(0, 0) == "");
  assert(a.substring(0, 15) == "Hello World!");

  toolbox::strref b {FPSTR("Hello World2")};
  assert(b.length() == 12);
  assert(b.offset() == 0);
  assert(b.charAt(1) == 'e');
  assert(b.indexOf('o') == 4);
  assert(b.isInProgmem() == true);
  assert(b.isZeroTerminated() == true);
  assert(b.leftmost(5) == "Hello");
  assert(b.leftmost(5).offset() == 0);
  assert(b.leftmost(5).length() == 5);
  assert(b.leftmost(5).isZeroTerminated() == false);
  assert(b.rightmost(5) == "orld2");
  assert(b.rightmost(5).offset() == 7);
  assert(b.rightmost(5).length() == 5);
  assert(b.rightmost(5).isZeroTerminated() == true);
  assert(b.middle(2,4) == "ll");
  assert(b.middle(2,4).offset() == 2);
  assert(b.middle(2,4).length() == 2);
  assert(b.middle(2,4).isZeroTerminated() == false);
  assert(b.substring(2,2) == "ll");
  assert(b.substring(2,2).offset() == 2);
  assert(b.substring(2,2).length() == 2);
  assert(b.substring(2,2).isZeroTerminated() == false);
  assert(b.skip(3) == "lo World2");
  assert(b.skip(3).offset() == 3);
  assert(b.skip(3).length() == 9);
  assert(b.skip(3).isZeroTerminated() == true);

  assert(a != b);
  assert(b != a);
  assert(a < b);
  assert(b > a);

  toolbox::strref c{"abc\0def", 8};
  toolbox::strref d{"abc\0def", 8};
  assert(c.length() == 8);
  assert(c.isZeroTerminated() == false);
  assert(c == d);
  
  return 0;
}
