#include <cassert>
#include <cstdint>
#include <type_traits>
#include "../structreg.h"

struct hash_record {
  int id;
  float score;
};

struct hash_nested {
  hash_record record;
  char flag;
};

struct hash_empty {};

[[maybe_unused]] struct { int x; } hash_anonymous_a;
[[maybe_unused]] struct { int y; } hash_anonymous_b;

static_assert(_structreg_::type_hash<hash_record>() != 0);
static_assert(_structreg_::type_hash<hash_nested>() != 0);
static_assert(_structreg_::type_hash<hash_empty>() != 0);
static_assert(!std::is_same_v<
  decltype(hash_anonymous_a),
  decltype(hash_anonymous_b)
>);
static_assert(_structreg_::type_hash<decltype(hash_anonymous_a)>() ==
              _structreg_::type_hash<decltype(hash_anonymous_b)>());

int main() {
  hash_record record{1, 2.0f};
  const auto hash = _structreg_::type_hash<hash_record>();

  record.id = 42;
  record.score = 3.0f;
  assert(hash == _structreg_::type_hash<hash_record>());
  return 0;
}
