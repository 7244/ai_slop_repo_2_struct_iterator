#include <cstdint>
#include <type_traits>
#include "../structreg.h"

enum class hash_enum : unsigned {
  value
};

static_assert(_structreg_::hash_mix(0, 0) == 0x9e3779b97f4a7c15);
static_assert(_structreg_::type_name_hash<int>() != 0);
static_assert(_structreg_::type_name_hash<int>() !=
              _structreg_::type_name_hash<float>());

static_assert(_structreg_::type_hash<int>() ==
              _structreg_::type_hash<const int&>());
static_assert(_structreg_::type_hash<int>() !=
              _structreg_::type_hash<float>());
static_assert(_structreg_::type_hash<int>() !=
              _structreg_::type_hash<int*>());
static_assert(_structreg_::type_hash<int>() !=
              _structreg_::type_hash<hash_enum>());

static_assert(_structreg_::type_hash<int[2]>() !=
              _structreg_::type_hash<int[3]>());
static_assert(_structreg_::type_hash<int[2][3]>() !=
              _structreg_::type_hash<int[2][4]>());

int main() {
  return 0;
}
