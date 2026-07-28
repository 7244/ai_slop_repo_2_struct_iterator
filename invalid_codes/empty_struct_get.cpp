#include <cstdint>
#include <type_traits>
#include <utility>
#include "../structreg.h"

struct {
} s;

int main() {
  s.structreg_get<0>();
}
