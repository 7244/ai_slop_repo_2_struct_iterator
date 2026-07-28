#include <cstdint>
#include <type_traits>
#include <utility>
#include "../structreg.h"

int x;

int main() {
  constexpr auto idx = structreg_get_tag_index<x>();
}
