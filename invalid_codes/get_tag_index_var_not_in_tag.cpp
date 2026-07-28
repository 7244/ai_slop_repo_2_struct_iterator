#include <cstdint>
#include <type_traits>
#include <utility>
#include "../structreg.h"

STRUCTREG_TAG(my_tag);
int STRUCTREG_VAR(x);

int main() {
  constexpr auto idx = structreg_get_tag_index<x, my_tag>();
}
