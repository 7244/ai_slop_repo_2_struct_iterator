#include <cstdint>
#include <type_traits>
#include <utility>
#include "../structreg.h"

int STRUCTREG_VAR(a);
float STRUCTREG_VAR(b);

int main() {
  structreg_get<999>();
}
