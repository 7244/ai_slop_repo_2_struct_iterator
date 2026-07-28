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

  template <typename T, uintptr_t I>
  consteval bool _uid_exists() {
    if constexpr (requires { T{}.template structreg_get<I>(); }) return true;
    return false;
  }

  template <typename T, auto& O, auto& F, uintptr_t I>
  consteval bool _uid_addr_matches() {
    if constexpr (_uid_exists<T, I>())
      if constexpr (std::is_same_v<decltype(&O.template structreg_get<I>()), decltype(&F)>)
        return &O.template structreg_get<I>() == &F;
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
  _structreg_::count_all<decltype(obj)>()

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
    ((_structreg_::_uid_addr_matches<T, S, F, Is>()
      ? (f = Is, true) : false) || ...);
    return f;
  }(std::make_index_sequence<4096>{});
  static_assert(uid != ~uintptr_t{}, "structreg_get_tag_index: field not found in struct");
  constexpr auto r = _structreg_::try_lookup_type<Tag, std::integral_constant<uintptr_t, uid>>();
  static_assert(r != ~uintptr_t{} || std::is_same_v<Tag, _structreg_::default_tag>, "structreg_get_tag_index: field not registered under tag");
  return r != ~uintptr_t{} ? r : []<uintptr_t... Is>(std::index_sequence<Is...>) {
    return (uintptr_t{0} + ... + (_structreg_::_uid_exists<T, Is>() ? 1 : 0));
  }(std::make_index_sequence<uid>{});
}

#if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC diagnostic pop
#endif
