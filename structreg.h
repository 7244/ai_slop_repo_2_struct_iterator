#pragma once

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

  template <class Tag, class T, uintptr_t N = 0, auto Unique = []{}>
  consteval uintptr_t reg_type() {
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

  template <class Tag, auto Unique = []{}, uintptr_t N = 0>
  consteval uintptr_t type_count() {
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
    static constexpr uintptr_t count() {
      return type_count<tag, Unique>();
    }
  };

  using default_tag = tag<[]{ }>;

  template <typename StructT, uintptr_t I>
  consteval bool _uid_exists() {
      if constexpr (requires { StructT{}.template structreg_get<I>(); }) {
      return true;
    }
    return false;
  }

  template <typename StructT, uintptr_t Start, uintptr_t End>
  consteval uintptr_t _count_range() {
    if constexpr (End - Start <= 512) {
      return []<uintptr_t... Is>(std::index_sequence<Is...>) {
        return (_uid_exists<StructT, Start + Is>() + ...);
      }(std::make_index_sequence<End - Start>{});
    } else {
      constexpr uintptr_t Mid = (Start + End) / 2;
      return _count_range<StructT, Start, Mid>() + _count_range<StructT, Mid, End>();
    }
  }

  template <typename StructT>
  consteval uintptr_t count_all() {
    return _count_range<StructT, 0, 4096>();
  }
}

#define STRUCTREG_TAG(name) \
  using name = _structreg_::tag<[]{ }>

#define _STRUCTREG_IMPL(name, uid, ...) \
  __VA_OPT__(static_assert((_structreg_::register_all<std::integral_constant<uintptr_t, uid>, __VA_ARGS__>(), true));) \
  template <uintptr_t I, typename Tag = _structreg_::default_tag> \
  requires (I == uid) \
  constexpr decltype(auto) structreg_get(this auto&& self){ \
    return (self.name); \
  }
#define STRUCTREG(name, ...) name; _STRUCTREG_IMPL(name, __COUNTER__, __VA_ARGS__)
#define STRUCTREG_EXIST(name, ...) _STRUCTREG_IMPL(name, __COUNTER__, __VA_ARGS__)

#define _STRUCTREG_VAR_IMPL(name, uid, ...) \
  static_assert((_structreg_::reg_type<_structreg_::default_tag, std::integral_constant<uintptr_t, uid>>(), true)); \
  __VA_OPT__(static_assert((_structreg_::register_all<std::integral_constant<uintptr_t, uid>, __VA_ARGS__>(), true));) \
  template <uintptr_t I, typename Tag = _structreg_::default_tag> \
  requires (I == _structreg_::reg_type<Tag, std::integral_constant<uintptr_t, uid>>()) \
  constexpr decltype(auto) structreg_get() { \
    return (name); \
  }
#define STRUCTREG_VAR(name, ...) name; _STRUCTREG_VAR_IMPL(name, __COUNTER__, __VA_ARGS__)
#define STRUCTREG_VAR_EXIST(name, ...) _STRUCTREG_VAR_IMPL(name, __COUNTER__, __VA_ARGS__)

#define STRUCTREG_COUNT(obj) \
  _structreg_::count_all<decltype(obj)>()

#if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC diagnostic pop
#endif
