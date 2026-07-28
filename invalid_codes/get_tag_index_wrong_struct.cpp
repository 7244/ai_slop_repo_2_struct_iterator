#include <cstdint>
#include <type_traits>
#include <utility>
#include "../structreg.h"

struct A {
  int x;
} a;
struct B {
  int y;
} b;

int main() {
  constexpr auto idx = structreg_get_tag_index<a, b.y>();
}
