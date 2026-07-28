#include <cstdint>
#include <type_traits>
#include <utility>
#include "../structreg.h"

struct {
  int x;
} s;

int main() {
  constexpr auto idx = structreg_get_tag_index<s, s.x>();
}
