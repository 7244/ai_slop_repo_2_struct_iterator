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

  template <typename T, auto>
  struct var_entry {};

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

  template <typename T, typename... Tags>
  consteval void register_all() {
    ((void)reg_type<Tags, T>(), ...);
  }

  template <class Tag, auto Unique = []{}, std::size_t N = 0>
  consteval std::size_t type_count() {
    if constexpr (requires { get_type(nth<Tag, N>{}); }) {
      return type_count<Tag, Unique, N + 1>();
    }
    else{
      return N;
    }
  }

  template <auto>
  struct tag {
    template <auto Unique = []{}>
    static constexpr std::size_t count() {
      return type_count<tag, Unique>();
    }
  };
}

#define STRUCTREG_TAG(name) \
  using name = _structreg_::tag<[]{}>

#define STRUCTREG(name, ...) STRUCTREG_IMPL(name, __COUNTER__, __VA_ARGS__)
#define STRUCTREG_IMPL(name, uid, ...) \
  name; \
  static_assert((_structreg_::reg_type<structreg, _structreg_::var_entry<decltype(name), uid>>(), true)); \
  __VA_OPT__(static_assert((_structreg_::register_all<_structreg_::var_entry<decltype(name), uid>, __VA_ARGS__>(), true));) \
  template <typename Tag, std::size_t I> \
  requires (I == _structreg_::reg_type<Tag, _structreg_::var_entry<decltype(name), uid>>()) \
  constexpr decltype(auto) get(this auto&& self){ \
    return (self.name); \
  }

#define STRUCTREG_VAR(name, ...) STRUCTREG_VAR_IMPL(name, __COUNTER__, __VA_ARGS__)
#define STRUCTREG_VAR_IMPL(name, uid, ...) \
  name; \
  static_assert((_structreg_::reg_type<structreg, _structreg_::var_entry<decltype(name), uid>>(), true)); \
  __VA_OPT__(static_assert((_structreg_::register_all<_structreg_::var_entry<decltype(name), uid>, __VA_ARGS__>(), true));) \
  template <typename Tag, std::size_t I> \
  requires (I == _structreg_::reg_type<Tag, _structreg_::var_entry<decltype(name), uid>>()) \
  constexpr decltype(auto) get() { \
    return (name); \
  }

#if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC diagnostic pop
#endif
