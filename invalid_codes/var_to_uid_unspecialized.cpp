#include <cstdint>
#include <type_traits>
#include <utility>
#include "../structreg.h"

int x;

int main() {
  constexpr auto v = _structreg_::var_to_uid<x>::value;
}
