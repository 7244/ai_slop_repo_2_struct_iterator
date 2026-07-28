#include <cstdint>
#include <type_traits>
#include <utility>
#include "../structreg.h"

STRUCTREG_TAG(a);
STRUCTREG_TAG(b);
int STRUCTREG_VAR(x, a);
float STRUCTREG_VAR(y, a);

int main() {
  constexpr auto idx = structreg_convert_tag_index<a, 5, b>();
}
