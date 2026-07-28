#include <cstdint>
#include <type_traits>
#include <utility>
#include "../structreg.h"

struct NotATag {};
STRUCTREG_TAG(b);
int STRUCTREG_VAR(x, b);

int main() {
  constexpr auto idx = structreg_convert_tag_index<NotATag, 0, b>();
}
