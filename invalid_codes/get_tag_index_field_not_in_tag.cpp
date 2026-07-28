#include <cstdint>
#include <type_traits>
#include <utility>
#include "../structreg.h"

STRUCTREG_TAG(my_tag);
STRUCTREG_TAG(other_tag);
struct {
  int STRUCTREG(x, my_tag);
} s;

int main() {
  constexpr auto idx = structreg_get_tag_index<s, s.x, other_tag>();
}
