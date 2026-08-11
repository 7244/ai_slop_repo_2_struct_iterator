#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include "../structreg.h"
#include "../include_guard/header_a.h"
#include "../include_guard/header_b.h"

using _structreg_::path_capacity;
using _structreg_::path_id;
using _structreg_::path_value;

static_assert(_structreg_::include_guard_tag::count() == 2);

static_assert(std::is_same_v<path_id<path_value<path_capacity>("a/./b.h")>, path_id<path_value<path_capacity>("a/b.h")>>);
static_assert(std::is_same_v<path_id<path_value<path_capacity>("a/../b.h")>, path_id<path_value<path_capacity>("b.h")>>);
static_assert(std::is_same_v<path_id<path_value<path_capacity>("a//b.h")>, path_id<path_value<path_capacity>("a/b.h")>>);
static_assert(std::is_same_v<path_id<path_value<path_capacity>("/a/../b.h")>, path_id<path_value<path_capacity>("/b.h")>>);
static_assert(std::is_same_v<path_id<path_value<path_capacity>("a/b/..")>, path_id<path_value<path_capacity>("a")>>);
static_assert(std::is_same_v<path_id<path_value<path_capacity>("/a/..")>, path_id<path_value<path_capacity>("/")>>);
static_assert(std::is_same_v<path_id<path_value<path_capacity>("/..")>, path_id<path_value<path_capacity>("/")>>);
static_assert(std::is_same_v<path_id<path_value<path_capacity>("/../a")>, path_id<path_value<path_capacity>("/a")>>);
static_assert(!std::is_same_v<path_id<path_value<path_capacity>("../a.h")>, path_id<path_value<path_capacity>("a.h")>>);
static_assert(std::is_same_v<path_id<path_value<path_capacity>("../a.h")>, path_id<path_value<path_capacity>("../a.h")>>);
static_assert(std::is_same_v<path_id<path_value<path_capacity>("../..")>, path_id<path_value<path_capacity>("../..")>>);
static_assert(std::is_same_v<path_id<path_value<path_capacity>("a/../..")>, path_id<path_value<path_capacity>("..")>>);

int main() {
  return header_a_value + header_b_value != 3;
}
