#pragma once

#include <type_traits>

#if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wnon-template-friend"
  #pragma GCC diagnostic ignored "-Wsfinae-incomplete"
#endif

namespace _structreg_ {
  template <typename Tag, auto>
  struct nth {
    friend consteval auto get_type(nth);
  };
  template <typename Tag, auto N, class T>
  struct type_slot {
    friend consteval auto get_type(nth<Tag, N>) {
      return T{};
    }
  };

  template <class Tag, class T, std::size_t N = 0, auto Unique = []{}>
  consteval std::size_t reg_type() {
    if constexpr (requires { get_type(nth<Tag, N>{}); }) {
      if constexpr (std::is_same_v<
        decltype(get_type(nth<Tag, N>{})), T
      >) {
        return N;
      } else {
        return reg_type<Tag, T, N + 1, Unique>();
      }
    } else {
      (void)type_slot<Tag, N, T>{};
      return N;
    }
  }
}

template <class Tag, auto Unique = []{}, std::size_t N = 0>
consteval std::size_t type_count() {
  if constexpr (requires { get_type(_structreg_::nth<Tag, N>{}); }) {
    return type_count<Tag, Unique, N + 1>();
  }
  else{
    return N;
  }
}

#define STRUCTREG(name) \
  name; \
  static_assert((_structreg_::reg_type<structreg, decltype(name)>(), true)); \
  template <std::size_t I> \
  requires (I == _structreg_::reg_type<structreg, decltype(name)>()) \
  constexpr decltype(auto) get(this auto&& self){ \
    return (self.name); \
  }

#if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC diagnostic pop
#endif
