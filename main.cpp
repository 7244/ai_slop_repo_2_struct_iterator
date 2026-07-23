#include "struct_registry.h"
#include <cstdio>
#include <cassert>

struct {
  STRUCTREG_TAG(structreg);
  struct{int x; float y;} STRUCTREG(st0);
  struct{char tag;} STRUCTREG(st1);
  struct{double d;} STRUCTREG(st2);
} pile;

STRUCTREG_TAG(tag_a);
STRUCTREG_TAG(tag_b);
struct {
  STRUCTREG_TAG(structreg);
  int STRUCTREG(x, tag_a);
  float STRUCTREG(y, tag_a);
  char STRUCTREG(z, tag_b);
  double STRUCTREG(w, tag_a, tag_b);
} multi;

// --- standalone variable tests ---
STRUCTREG_TAG(structreg);
struct{int x; float y;} STRUCTREG_VAR(sv0);
struct{char tag;} STRUCTREG_VAR(sv1);
struct{double d;} STRUCTREG_VAR(sv2);

static_assert(structreg::count() == 3);

using SV0 = std::remove_reference_t<decltype(get<structreg, 0>())>;
using SV1 = std::remove_reference_t<decltype(get<structreg, 1>())>;
using SV2 = std::remove_reference_t<decltype(get<structreg, 2>())>;

static_assert(std::is_same_v<SV0, decltype(sv0)>);
static_assert(std::is_same_v<SV1, decltype(sv1)>);
static_assert(std::is_same_v<SV2, decltype(sv2)>);

static_assert(std::is_same_v<decltype(get<structreg, 0>()), SV0&>);
static_assert(std::is_same_v<decltype(get<structreg, 1>()), SV1&>);
static_assert(std::is_same_v<decltype(get<structreg, 2>()), SV2&>);

static_assert(std::is_same_v<decltype(get<structreg, 0>().x), int>);
static_assert(std::is_same_v<decltype(get<structreg, 0>().y), float>);
static_assert(std::is_same_v<decltype(get<structreg, 1>().tag), char>);
static_assert(std::is_same_v<decltype(get<structreg, 2>().d), double>);

// --- standalone variable tests (multi-tag) ---
STRUCTREG_TAG(mv_tag_a);
STRUCTREG_TAG(mv_tag_b);
int   STRUCTREG_VAR(mv_x, mv_tag_a);
float STRUCTREG_VAR(mv_y, mv_tag_a);
char  STRUCTREG_VAR(mv_z, mv_tag_b);
double STRUCTREG_VAR(mv_w, mv_tag_a, mv_tag_b);

static_assert(structreg::count() == 7);
static_assert(mv_tag_a::count() == 3);
static_assert(mv_tag_b::count() == 2);

static_assert(std::is_same_v<std::remove_reference_t<decltype(get<mv_tag_a, 0>())>, int>);
static_assert(std::is_same_v<std::remove_reference_t<decltype(get<mv_tag_a, 1>())>, float>);
static_assert(std::is_same_v<std::remove_reference_t<decltype(get<mv_tag_a, 2>())>, double>);
static_assert(std::is_same_v<std::remove_reference_t<decltype(get<mv_tag_b, 0>())>, char>);
static_assert(std::is_same_v<std::remove_reference_t<decltype(get<mv_tag_b, 1>())>, double>);

static_assert(std::is_same_v<decltype(get<mv_tag_a, 0>()), int&>);
static_assert(std::is_same_v<decltype(get<mv_tag_a, 1>()), float&>);
static_assert(std::is_same_v<decltype(get<mv_tag_a, 2>()), double&>);
static_assert(std::is_same_v<decltype(get<mv_tag_b, 0>()), char&>);
static_assert(std::is_same_v<decltype(get<mv_tag_b, 1>()), double&>);

// --- same-type tests (standalone) ---
int STRUCTREG_VAR(same_a);
int STRUCTREG_VAR(same_b);

static_assert(structreg::count() == 9);
static_assert(std::is_same_v<decltype(get<structreg, 7>()), int&>);
static_assert(std::is_same_v<decltype(get<structreg, 8>()), int&>);

// --- same-type tests (struct) ---
struct {
  STRUCTREG_TAG(structreg);
  int STRUCTREG(st_a);
  int STRUCTREG(st_b);
} st_obj;
using st_tag = decltype(st_obj)::structreg;

static_assert(st_tag::count() == 2);
static_assert(std::is_same_v<std::remove_reference_t<decltype(st_obj.get<st_tag, 0>())>, int>);
static_assert(std::is_same_v<std::remove_reference_t<decltype(st_obj.get<st_tag, 1>())>, int>);

int main() {
  using tag_t = decltype(pile)::structreg;

  static_assert(tag_t::count() == 3);

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

  static_assert(tag_a::count() == 3);
  static_assert(tag_b::count() == 2);

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

  // --- standalone variable runtime tests (single-tag) ---
  get<structreg, 0>().x = 10;
  get<structreg, 0>().y = 3.14f;
  get<structreg, 1>().tag = 'A';
  get<structreg, 2>().d = 2.718;

  assert(sv0.x == 10);
  assert(sv0.y == 3.14f);
  assert(sv1.tag == 'A');
  assert(sv2.d == 2.718);

  sv0.x = 99;
  assert((get<structreg, 0>().x == 99));

  SV0& svref0 = get<structreg, 0>();
  SV1& svref1 = get<structreg, 1>();
  svref0.x = 7;
  svref0.y = 1.5f;
  svref1.tag = 'Z';
  assert(sv0.x == 7);
  assert(sv1.tag == 'Z');

  // --- standalone variable runtime tests (multi-tag) ---
  get<mv_tag_a, 0>() = 42;
  get<mv_tag_a, 1>() = 3.14f;
  get<mv_tag_b, 0>() = 'Q';

  assert(mv_x == 42);
  assert(mv_y == 3.14f);
  assert(mv_z == 'Q');

  get<mv_tag_a, 2>() = 2.718;
  assert(mv_w == 2.718);

  get<mv_tag_b, 1>() = 1.618;
  assert(mv_w == 1.618);

  // --- same-type runtime tests (standalone) ---
  get<structreg, 7>() = 100;
  get<structreg, 8>() = 200;
  assert(same_a == 100);
  assert(same_b == 200);

  // --- same-type runtime tests (struct) ---
  st_obj.get<st_tag, 0>() = 10;
  st_obj.get<st_tag, 1>() = 20;
  assert(st_obj.st_a == 10);
  assert(st_obj.st_b == 20);

  return 0;
}
