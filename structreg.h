#pragma once

#if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wnon-template-friend"
  #pragma GCC diagnostic ignored "-Wsfinae-incomplete"
  #pragma GCC diagnostic ignored "-Wtemplate-body"
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
  consteval bool register_all() {
    ((void)reg_type<Tags, T>(), ...);
    return true;
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

  template <class FromTag, uintptr_t FromIndex, class ToTag>
  consteval uintptr_t convert_tag_index() {
    using T = decltype(get_type(nth<FromTag, FromIndex>{}));
    return reg_type<ToTag, T>();
  }

  template <class Tag, class T, uintptr_t N = 0>
  consteval uintptr_t try_lookup_type() {
    if constexpr (requires { get_type(nth<Tag, N>{}); })
      if constexpr (std::is_same_v<decltype(get_type(nth<Tag, N>{})), T>)
        return N;
      else
        return try_lookup_type<Tag, T, N + 1>();
    else
      return ~uintptr_t{};
  }

  template <auto&>
  struct var_to_uid;

  template <typename StructT, uintptr_t Start, uintptr_t End>
  consteval uintptr_t _count_range() {
    if constexpr (Start >= End) return 0;
    else return []<uintptr_t... Is>(std::index_sequence<Is...>) {
      return (requires { StructT{}.template structreg_get<Start + Is>(); } + ...);
    }(std::make_index_sequence<End - Start>{});
  }
}

#define STRUCTREG_TAG(name) \
  using name = _structreg_::tag<[]{ }>

#define _STRUCTREG_IMPL(name, uid, ...) \
  __VA_OPT__(static_assert(_structreg_::register_all<std::integral_constant<uintptr_t, uid>, __VA_ARGS__>());) \
  template <uintptr_t I, typename Tag = _structreg_::default_tag> \
  requires (I == uid) \
  constexpr decltype(auto) structreg_get(this auto&& self){ \
    return (self.name); \
  }
#define STRUCTREG(name, ...) name; _STRUCTREG_IMPL(name, __COUNTER__, __VA_ARGS__)
#define STRUCTREG_EXIST(name, ...) _STRUCTREG_IMPL(name, __COUNTER__, __VA_ARGS__)

#define _STRUCTREG_VAR_IMPL(name, uid, ...) \
  static_assert(_structreg_::register_all<std::integral_constant<uintptr_t, uid>, _structreg_::default_tag __VA_OPT__(,) __VA_ARGS__>()); \
  template <> \
  struct _structreg_::var_to_uid<name> { \
    static constexpr uintptr_t value = uid; \
  }; \
  template <uintptr_t I, typename Tag = _structreg_::default_tag> \
  requires (I == _structreg_::reg_type<Tag, std::integral_constant<uintptr_t, uid>>()) \
  constexpr decltype(auto) structreg_get() { \
    return (name); \
  }
#define STRUCTREG_VAR(name, ...) name; _STRUCTREG_VAR_IMPL(name, __COUNTER__, __VA_ARGS__)
#define STRUCTREG_VAR_EXIST(name, ...) _STRUCTREG_VAR_IMPL(name, __COUNTER__, __VA_ARGS__)

#define STRUCTREG_COUNT(obj) \
  _structreg_::_count_range<decltype(obj), 0, 1024>()

template <class FromTag, uintptr_t FromIndex, class ToTag>
inline constexpr uintptr_t structreg_convert_tag_index =
  _structreg_::convert_tag_index<FromTag, FromIndex, ToTag>();

template <auto& Field, typename Tag = _structreg_::default_tag>
consteval uintptr_t structreg_get_tag_index() {
  constexpr auto uid = _structreg_::var_to_uid<Field>::value;
  constexpr auto r = _structreg_::try_lookup_type<Tag,
    std::integral_constant<uintptr_t, uid>>();
  static_assert(r != ~uintptr_t{}, "structreg_get_tag_index: field not registered under tag");
  return r;
}

template <auto& S, auto& F, typename Tag = _structreg_::default_tag>
consteval uintptr_t structreg_get_tag_index() {
  using T = std::remove_reference_t<decltype(S)>;
  constexpr auto uid = []<uintptr_t... Is>(std::index_sequence<Is...>) {
    uintptr_t f = ~uintptr_t{};
    (([]<uintptr_t I> consteval {
      if constexpr (requires { &S.template structreg_get<I>() == &F; })
        return &S.template structreg_get<I>() == &F;
      return false;
    }.template operator()<Is>() ? (f = Is) || true : false) || ...);
    return f;
  }(std::make_index_sequence<1024>{});
  static_assert(uid != ~uintptr_t{}, "structreg_get_tag_index: field not found in struct");
  constexpr auto r = _structreg_::try_lookup_type<Tag, std::integral_constant<uintptr_t, uid>>();
  static_assert(r != ~uintptr_t{} || std::is_same_v<Tag, _structreg_::default_tag>, "structreg_get_tag_index: field not registered under tag");
  return r != ~uintptr_t{} ? r : _structreg_::_count_range<T, 0, uid>();
}

#if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC diagnostic pop
#endif
