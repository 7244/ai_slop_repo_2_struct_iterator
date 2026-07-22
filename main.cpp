#define STRUCTREG_NAME salsa
#include "struct_registry.h"

#include <cstdio>
#include <cassert>

// --- single anonymous struct (indices 0,1,2) -------------------------------
struct{
STRUCT(st0, int x; float y);
STRUCT(st1, char tag);
STRUCT(st2, double d);
}pile;

int main() {
  // === 1. type count and type access ===
  static_assert(type_count<>() == 3);

  using T0 = std::remove_reference_t<decltype(pile.get<0>())>; // pile.st0
  using T1 = std::remove_reference_t<decltype(pile.get<1>())>; // pile.st1
  using T2 = std::remove_reference_t<decltype(pile.get<2>())>; // pile.st2

  static_assert(std::is_same_v<T0, decltype(pile.st0)>);
  static_assert(std::is_same_v<T1, decltype(pile.st1)>);
  static_assert(std::is_same_v<T2, decltype(pile.st2)>);

  // === 2. get<I>() returns the correct member ===
  pile.get<0>().x = 10;
  pile.get<0>().y = 3.14f;
  pile.get<1>().tag = 'A';
  pile.get<2>().d = 2.718;

  assert(pile.st0.x == 10);
  assert(pile.st0.y == 3.14f);
  assert(pile.st1.tag == 'A');
  assert(pile.st2.d == 2.718);

  // === 3. same object: mutate through name, read through get ===
  pile.st0.x = 99;
  assert(pile.get<0>().x == 99);

  // === 4. get<I>() returns the correct type ===
  static_assert(std::is_same_v<decltype(pile.get<0>()), T0&>);
  static_assert(std::is_same_v<decltype(pile.get<1>()), T1&>);
  static_assert(std::is_same_v<decltype(pile.get<2>()), T2&>);

  // === 5. sub-member type correctness via get ===
  // decltype(obj.member) gives the declared type, not a reference
  static_assert(std::is_same_v<decltype(pile.get<0>().x), int>);
  static_assert(std::is_same_v<decltype(pile.get<0>().y), float>);
  static_assert(std::is_same_v<decltype(pile.get<1>().tag), char>);
  static_assert(std::is_same_v<decltype(pile.get<2>().d), double>);

  // === 6. type alias + get for typed access ===
  T0& ref0 = pile.get<0>();
  T1& ref1 = pile.get<1>();
  ref0.x = 7;
  ref0.y = 1.5f;
  ref1.tag = 'Z';
  assert(pile.st0.x == 7);
  assert(pile.st1.tag == 'Z');

  // === 7. output ===
  pile.get<0>().x = 1; pile.get<0>().y = 2.5f;
  std::printf("pile.get<0> same as pile.st0: %s\n",
    &pile.get<0>() == &pile.st0 ? "yes" : "no");
  std::printf("pile.st0: x=%d, y=%.2f\n", pile.st0.x, double(pile.st0.y));
  std::printf("pile.st1: tag=%c\n", pile.st1.tag);
  std::printf("pile.st2: d=%.3f\n", pile.st2.d);

  return 0;
}
