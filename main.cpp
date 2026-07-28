#include <cstdint>
#include <type_traits>
#include <utility>
#include "structreg.h"
#include <cstdio>
#include <cassert>

// --- struct pile: uids 0,1,2 ---
struct {
  struct{int x; float y;} STRUCTREG(st0);
  struct{char tag;} STRUCTREG(st1);
  struct{double d;} STRUCTREG(st2);
} pile;

constexpr auto pile_cnt = STRUCTREG_COUNT(pile);

// --- struct multi: fields with extra explicit tags ---
STRUCTREG_TAG(tag_a);
STRUCTREG_TAG(tag_b);
struct {
  int STRUCTREG(x, tag_a);
  float STRUCTREG(y, tag_a);
  char STRUCTREG(z, tag_b);
  double STRUCTREG(w, tag_a, tag_b);
} multi;

constexpr auto multi_cnt = STRUCTREG_COUNT(multi);

// --- standalone variable tests (single-tag) ---
struct{int x; float y;} STRUCTREG_VAR(sv0);
struct{char tag;} STRUCTREG_VAR(sv1);
struct{double d;} STRUCTREG_VAR(sv2);

static_assert(_structreg_::default_tag::count() == 3);

using SV0 = std::remove_reference_t<decltype(structreg_get<0>())>;
using SV1 = std::remove_reference_t<decltype(structreg_get<1>())>;
using SV2 = std::remove_reference_t<decltype(structreg_get<2>())>;

static_assert(std::is_same_v<SV0, decltype(sv0)>);
static_assert(std::is_same_v<SV1, decltype(sv1)>);
static_assert(std::is_same_v<SV2, decltype(sv2)>);

static_assert(std::is_same_v<decltype(structreg_get<0>()), SV0&>);
static_assert(std::is_same_v<decltype(structreg_get<1>()), SV1&>);
static_assert(std::is_same_v<decltype(structreg_get<2>()), SV2&>);

static_assert(std::is_same_v<decltype(structreg_get<0>().x), int>);
static_assert(std::is_same_v<decltype(structreg_get<0>().y), float>);
static_assert(std::is_same_v<decltype(structreg_get<1>().tag), char>);
static_assert(std::is_same_v<decltype(structreg_get<2>().d), double>);

// --- standalone variable tests (multi-tag) ---
STRUCTREG_TAG(mv_tag_a);
STRUCTREG_TAG(mv_tag_b);
int   STRUCTREG_VAR(mv_x, mv_tag_a);
float STRUCTREG_VAR(mv_y, mv_tag_a);
char  STRUCTREG_VAR(mv_z, mv_tag_b);
double STRUCTREG_VAR(mv_w, mv_tag_a, mv_tag_b);

static_assert(_structreg_::default_tag::count() == 7);
static_assert(mv_tag_a::count() == 3);
static_assert(mv_tag_b::count() == 2);

static_assert(structreg_convert_tag_index<mv_tag_a, 2, mv_tag_b> == 1);
static_assert(structreg_convert_tag_index<mv_tag_b, 1, mv_tag_a> == 2);


static_assert(std::is_same_v<std::remove_reference_t<decltype(structreg_get<0, mv_tag_a>())>, int>);
static_assert(std::is_same_v<std::remove_reference_t<decltype(structreg_get<1, mv_tag_a>())>, float>);
static_assert(std::is_same_v<std::remove_reference_t<decltype(structreg_get<2, mv_tag_a>())>, double>);
static_assert(std::is_same_v<std::remove_reference_t<decltype(structreg_get<0, mv_tag_b>())>, char>);
static_assert(std::is_same_v<std::remove_reference_t<decltype(structreg_get<1, mv_tag_b>())>, double>);

static_assert(std::is_same_v<decltype(structreg_get<0, mv_tag_a>()), int&>);
static_assert(std::is_same_v<decltype(structreg_get<1, mv_tag_a>()), float&>);
static_assert(std::is_same_v<decltype(structreg_get<2, mv_tag_a>()), double&>);
static_assert(std::is_same_v<decltype(structreg_get<0, mv_tag_b>()), char&>);
static_assert(std::is_same_v<decltype(structreg_get<1, mv_tag_b>()), double&>);

// --- same-type tests (standalone) ---
int STRUCTREG_VAR(same_a);
int STRUCTREG_VAR(same_b);

static_assert(_structreg_::default_tag::count() == 9);
static_assert(std::is_same_v<decltype(structreg_get<7>()), int&>);
static_assert(std::is_same_v<decltype(structreg_get<8>()), int&>);

// --- same-type tests (struct, uids 16,17) ---
struct {
  int STRUCTREG(st_a);
  int STRUCTREG(st_b);
} st_obj;

constexpr auto st_cnt = STRUCTREG_COUNT(st_obj);

static_assert(st_cnt == 2);
static_assert(std::is_same_v<std::remove_reference_t<decltype(st_obj.structreg_get<16>())>, int>);
static_assert(std::is_same_v<std::remove_reference_t<decltype(st_obj.structreg_get<17>())>, int>);

int main() {
  static_assert(pile_cnt == 3);

  using T0 = std::remove_reference_t<decltype(pile.structreg_get<0>())>;
  using T1 = std::remove_reference_t<decltype(pile.structreg_get<1>())>;
  using T2 = std::remove_reference_t<decltype(pile.structreg_get<2>())>;

  static_assert(std::is_same_v<T0, decltype(pile.st0)>);
  static_assert(std::is_same_v<T1, decltype(pile.st1)>);
  static_assert(std::is_same_v<T2, decltype(pile.st2)>);

  pile.structreg_get<0>().x = 10;
  pile.structreg_get<0>().y = 3.14f;
  pile.structreg_get<1>().tag = 'A';
  pile.structreg_get<2>().d = 2.718;

  assert(pile.st0.x == 10);
  assert(pile.st0.y == 3.14f);
  assert(pile.st1.tag == 'A');
  assert(pile.st2.d == 2.718);

  pile.st0.x = 99;
  assert((pile.structreg_get<0>().x == 99));

  static_assert(std::is_same_v<decltype(pile.structreg_get<0>()), T0&>);
  static_assert(std::is_same_v<decltype(pile.structreg_get<1>()), T1&>);
  static_assert(std::is_same_v<decltype(pile.structreg_get<2>()), T2&>);

  static_assert(std::is_same_v<decltype(pile.structreg_get<0>().x), int>);
  static_assert(std::is_same_v<decltype(pile.structreg_get<0>().y), float>);
  static_assert(std::is_same_v<decltype(pile.structreg_get<1>().tag), char>);
  static_assert(std::is_same_v<decltype(pile.structreg_get<2>().d), double>);

  T0& ref0 = pile.structreg_get<0>();
  T1& ref1 = pile.structreg_get<1>();
  ref0.x = 7;
  ref0.y = 1.5f;
  ref1.tag = 'Z';
  assert(pile.st0.x == 7);
  assert(pile.st1.tag == 'Z');

  pile.structreg_get<0>().x = 1; pile.structreg_get<0>().y = 2.5f;
  std::printf("pile.structreg_get<0> same as pile.st0: %s\n",
    &pile.structreg_get<0>() == &pile.st0 ? "yes" : "no");
  std::printf("pile.st0: x=%d, y=%.2f\n", pile.st0.x, double(pile.st0.y));
  std::printf("pile.st1: tag=%c\n", pile.st1.tag);
  std::printf("pile.st2: d=%.3f\n", pile.st2.d);

  static_assert(multi_cnt == 4);
  static_assert(tag_a::count() == 3);
  static_assert(tag_b::count() == 2);
  static_assert(structreg_convert_tag_index<tag_a, 2, tag_b> == 1);
  static_assert(structreg_convert_tag_index<tag_b, 1, tag_a> == 2);


  static_assert(std::is_same_v<
    std::remove_reference_t<decltype(multi.structreg_get<3>())>, int>);
  static_assert(std::is_same_v<
    std::remove_reference_t<decltype(multi.structreg_get<4>())>, float>);
  static_assert(std::is_same_v<
    std::remove_reference_t<decltype(multi.structreg_get<6>())>, double>);
  static_assert(std::is_same_v<
    std::remove_reference_t<decltype(multi.structreg_get<5>())>, char>);
  static_assert(std::is_same_v<
    std::remove_reference_t<decltype(multi.structreg_get<6>())>, double>);

  multi.structreg_get<3>() = 42;
  multi.structreg_get<4>() = 3.14f;
  multi.structreg_get<5>() = 'Q';

  assert(multi.x == 42);
  assert(multi.y == 3.14f);
  assert(multi.z == 'Q');

  multi.structreg_get<6>() = 2.718;
  assert(multi.w == 2.718);

  multi.structreg_get<6>() = 1.618;
  assert(multi.w == 1.618);

  static_assert(std::is_same_v<decltype(multi.structreg_get<3>()), int&>);
  static_assert(std::is_same_v<decltype(multi.structreg_get<4>()), float&>);
  static_assert(std::is_same_v<decltype(multi.structreg_get<6>()), double&>);
  static_assert(std::is_same_v<decltype(multi.structreg_get<5>()), char&>);
  static_assert(std::is_same_v<decltype(multi.structreg_get<6>()), double&>);

  std::printf("multi.x=%d, multi.y=%.2f, multi.w=%.3f, multi.z=%c\n",
    multi.x, double(multi.y), multi.w, multi.z);

  // --- standalone variable runtime tests (single-tag) ---
  structreg_get<0>().x = 10;
  structreg_get<0>().y = 3.14f;
  structreg_get<1>().tag = 'A';
  structreg_get<2>().d = 2.718;

  assert(sv0.x == 10);
  assert(sv0.y == 3.14f);
  assert(sv1.tag == 'A');
  assert(sv2.d == 2.718);

  sv0.x = 99;
  assert((structreg_get<0>().x == 99));

  SV0& svref0 = structreg_get<0>();
  SV1& svref1 = structreg_get<1>();
  svref0.x = 7;
  svref0.y = 1.5f;
  svref1.tag = 'Z';
  assert(sv0.x == 7);
  assert(sv1.tag == 'Z');

  // --- standalone variable runtime tests (multi-tag) ---
  structreg_get<0, mv_tag_a>() = 42;
  structreg_get<1, mv_tag_a>() = 3.14f;
  structreg_get<0, mv_tag_b>() = 'Q';

  assert(mv_x == 42);
  assert(mv_y == 3.14f);
  assert(mv_z == 'Q');

  structreg_get<2, mv_tag_a>() = 2.718;
  assert(mv_w == 2.718);

  structreg_get<1, mv_tag_b>() = 1.618;
  assert(mv_w == 1.618);

  // --- cross-tag access via convert_tag_index ---
  constexpr auto mv_w_in_b = structreg_convert_tag_index<mv_tag_a, 2, mv_tag_b>;
  structreg_get<mv_w_in_b, mv_tag_b>() = 3.14;
  assert(mv_w == 3.14);

  constexpr auto mv_w_in_a = structreg_convert_tag_index<mv_tag_b, 1, mv_tag_a>;
  structreg_get<mv_w_in_a, mv_tag_a>() = 2.71;
  assert(mv_w == 2.71);

  // --- same-type runtime tests (standalone) ---
  structreg_get<7>() = 100;
  structreg_get<8>() = 200;
  assert(same_a == 100);
  assert(same_b == 200);

  // --- same-type runtime tests (struct, uids 16,17) ---
  st_obj.structreg_get<16>() = 10;
  st_obj.structreg_get<17>() = 20;
  assert(st_obj.st_a == 10);
  assert(st_obj.st_b == 20);

  return 0;
}
