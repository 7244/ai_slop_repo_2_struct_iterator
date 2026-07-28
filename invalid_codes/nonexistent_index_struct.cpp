#include <cstdint>
#include <type_traits>
#include <utility>
#include "../structreg.h"

struct {
  int STRUCTREG(a);
  float STRUCTREG(b);
} s;

int main() {
  s.structreg_get<999>();
}
