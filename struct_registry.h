#pragma once

#include <type_traits>

#if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wnon-template-friend"
  #pragma GCC diagnostic ignored "-Wsfinae-incomplete"
#endif

// friend-injection machinery
namespace _struct_registry{
  template <auto>
  struct nth {
    friend consteval auto get_type(nth);
  };
  template <auto N, class T>
  struct type_slot {
    friend consteval auto get_type(nth<N>){
      return T{};
    }
  };
}

// registration: assign next sequential index to a type
template <class T, std::size_t N = 0, auto Unique = []{}>
consteval std::size_t reg_type() {
  if constexpr (requires { get_type(_struct_registry::nth<N>{}); }) {
    if constexpr (std::is_same_v<
      decltype(get_type(_struct_registry::nth<N>{})), T
    >) {
      return N;
    } else {
      return reg_type<T, N + 1, Unique>();
    }
  } else {
    (void)_struct_registry::type_slot<N, T>{};
    return N;
  }
}

template <auto Unique = []{}, std::size_t N = 0>
consteval std::size_t type_count() {
  if constexpr (requires { get_type(_struct_registry::nth<N>{}); }) {
    return type_count<Unique, N + 1>();
  } else {
    return N;
  }
}

// access type by compile-time index
template <std::size_t I>
using type_at = decltype(get_type(_struct_registry::nth<I>{}));

#define STRUCT(name, ...) \
  struct { __VA_ARGS__; } name; \
  static_assert((reg_type<decltype(name)>(), true)); \
  template <std::size_t I> \
  requires (I == reg_type<decltype(name)>()) \
  constexpr decltype(auto) get(this auto&& self){ \
    return (self.name); \
  }

#if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC diagnostic pop
#endif
