#pragma once

#include <type_traits>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
#pragma GCC diagnostic ignored "-Wsfinae-incomplete"
#endif

// --- Friend-injection machinery --------------------------------------------
namespace detail {

template <auto>
struct nth {
  friend consteval auto get_type(nth);
};

template <auto N, class T>
struct type_slot {
  friend consteval auto get_type(nth<N>) { return T{}; }
};

} // namespace detail

// --- Registration: assign next sequential index to a type -----------------
template <class T, std::size_t N = 0, auto Unique = []{}>
consteval std::size_t reg_type() {
  if constexpr (requires { get_type(detail::nth<N>{}); }) {
    if constexpr (std::is_same_v<
      decltype(get_type(detail::nth<N>{})), T
    >) {
      return N;
    } else {
      return reg_type<T, N + 1, Unique>();
    }
  } else {
    (void)detail::type_slot<N, T>{};
    return N;
  }
}

// --- Number of registered types -------------------------------------------
template <auto Unique = []{}, std::size_t N = 0>
consteval std::size_t type_count() {
  if constexpr (requires { get_type(detail::nth<N>{}); }) {
    return type_count<Unique, N + 1>();
  } else {
    return N;
  }
}

// --- Access type by compile-time index ------------------------------------
template <std::size_t I>
using type_at = decltype(get_type(detail::nth<I>{}));

// --- Macro: define struct instance + register its type ---------------------
// Works at both namespace scope and inside a class/struct body.
// Also adds a constrained get<I>() overload for compile-time indexed access.
// Variable access: direct (pile.st0) or indexed (pile.get<0>()).
//
// Note: get<I>() uses reg_type which assigns global indices.
// For a single struct at namespace scope, indices start at 0 transparently.
// For multiple structs, indices are global across all STRUCT calls.
#define STRUCT(name, ...)                                                    \
  struct { __VA_ARGS__; } name;                                              \
  static_assert((reg_type<decltype(name)>(), true));                           \
  template <std::size_t I>                                                     \
    requires (I == reg_type<decltype(name)>())                                  \
  constexpr decltype(auto) get(this auto&& self) {                              \
    return (self.name);                                                        \
  }

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
