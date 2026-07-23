#include "struct_registry.h"
#include <cstdio>
#include <cassert>

struct {
  using structreg = decltype([]{});
  struct{int x; float y;} STRUCTREG(st0);
  struct{char tag;} STRUCTREG(st1);
  struct{double d;} STRUCTREG(st2);
} pile;

using tag_a = decltype([]{});
using tag_b = decltype([]{});
struct {
  using structreg = decltype([]{});
  int STRUCTREG(x, tag_a);
  float STRUCTREG(y, tag_a);
  char STRUCTREG(z, tag_b);
  double STRUCTREG(w, tag_a, tag_b);
} multi;

int main() {
  using tag_t = decltype(pile)::structreg;

  static_assert(type_count<tag_t>() == 3);

  using T0 = std::remove_reference_t<decltype(pile.get<tag_t, 0>())>;
  using T1 = std::remove_reference_t<decltype(pile.get<tag_t, 1>())>;
  using T2 = std::remove_reference_t<decltype(pile.get<tag_t, 2>())>;

  static_assert(std::is_same_v<T0, decltype(pile.st0)>);
  static_assert(std::is_same_v<T1, decltype(pile.st1)>);
  static_assert(std::is_same_v<T2, decltype(pile.st2)>);

  pile.get<tag_t, 0>().x = 10;
  pile.get<tag_t, 0>().y = 3.14f;
  pile.get<tag_t, 1>().tag = 'A';
  pile.get<tag_t, 2>().d = 2.718;

  assert(pile.st0.x == 10);
  assert(pile.st0.y == 3.14f);
  assert(pile.st1.tag == 'A');
  assert(pile.st2.d == 2.718);

  pile.st0.x = 99;
  assert((pile.get<tag_t, 0>().x == 99));

  static_assert(std::is_same_v<decltype(pile.get<tag_t, 0>()), T0&>);
  static_assert(std::is_same_v<decltype(pile.get<tag_t, 1>()), T1&>);
  static_assert(std::is_same_v<decltype(pile.get<tag_t, 2>()), T2&>);

  static_assert(std::is_same_v<decltype(pile.get<tag_t, 0>().x), int>);
  static_assert(std::is_same_v<decltype(pile.get<tag_t, 0>().y), float>);
  static_assert(std::is_same_v<decltype(pile.get<tag_t, 1>().tag), char>);
  static_assert(std::is_same_v<decltype(pile.get<tag_t, 2>().d), double>);

  T0& ref0 = pile.get<tag_t, 0>();
  T1& ref1 = pile.get<tag_t, 1>();
  ref0.x = 7;
  ref0.y = 1.5f;
  ref1.tag = 'Z';
  assert(pile.st0.x == 7);
  assert(pile.st1.tag == 'Z');

  pile.get<tag_t, 0>().x = 1; pile.get<tag_t, 0>().y = 2.5f;
  std::printf("pile.get<tag_t, 0> same as pile.st0: %s\n",
    &pile.get<tag_t, 0>() == &pile.st0 ? "yes" : "no");
  std::printf("pile.st0: x=%d, y=%.2f\n", pile.st0.x, double(pile.st0.y));
  std::printf("pile.st1: tag=%c\n", pile.st1.tag);
  std::printf("pile.st2: d=%.3f\n", pile.st2.d);

  static_assert(type_count<tag_a>() == 3);
  static_assert(type_count<tag_b>() == 2);

  static_assert(std::is_same_v<
    std::remove_reference_t<decltype(multi.get<tag_a, 0>())>, int>);
  static_assert(std::is_same_v<
    std::remove_reference_t<decltype(multi.get<tag_a, 1>())>, float>);
  static_assert(std::is_same_v<
    std::remove_reference_t<decltype(multi.get<tag_a, 2>())>, double>);
  static_assert(std::is_same_v<
    std::remove_reference_t<decltype(multi.get<tag_b, 0>())>, char>);
  static_assert(std::is_same_v<
    std::remove_reference_t<decltype(multi.get<tag_b, 1>())>, double>);

  multi.get<tag_a, 0>() = 42;
  multi.get<tag_a, 1>() = 3.14f;
  multi.get<tag_b, 0>() = 'Q';

  assert(multi.x == 42);
  assert(multi.y == 3.14f);
  assert(multi.z == 'Q');

  multi.get<tag_a, 2>() = 2.718;
  assert(multi.w == 2.718);

  multi.get<tag_b, 1>() = 1.618;
  assert(multi.w == 1.618);

  static_assert(std::is_same_v<decltype(multi.get<tag_a, 0>()), int&>);
  static_assert(std::is_same_v<decltype(multi.get<tag_a, 1>()), float&>);
  static_assert(std::is_same_v<decltype(multi.get<tag_a, 2>()), double&>);
  static_assert(std::is_same_v<decltype(multi.get<tag_b, 0>()), char&>);
  static_assert(std::is_same_v<decltype(multi.get<tag_b, 1>()), double&>);

  std::printf("multi.x=%d, multi.y=%.2f, multi.w=%.3f, multi.z=%c\n",
    multi.x, double(multi.y), multi.w, multi.z);

  return 0;
}
