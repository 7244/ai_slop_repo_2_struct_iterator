#include <cstdint>
#include <type_traits>
#include "../structreg.h"

constexpr auto hash = _structreg_::type_hash<volatile int>();
