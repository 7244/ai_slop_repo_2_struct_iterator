#include <cstdint>
#include <type_traits>
#include <utility>
#include "../structreg.h"

int main() {
  int x = 5;
  x.structreg_get<0>();
}
