#pragma once

#if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wnon-template-friend"
  #pragma GCC diagnostic ignored "-Wsfinae-incomplete"
  #pragma GCC diagnostic ignored "-Wtemplate-body"
#endif

namespace _structreg_ {
  template <typename T, T... Is>
  struct integer_sequence {};

  template <uintptr_t N>
  using _make_index_sequence = __make_integer_seq<integer_sequence, uintptr_t, N>;

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

  template <class Tag, class T, uintptr_t N = 0>
  consteval uintptr_t reg_type() {
    if constexpr (requires { get_type(nth<Tag, N>{}); }) {
      if constexpr (std::is_same_v<
        decltype(get_type(nth<Tag, N>{})), T
      >) {
        return N;
      } else {
        return reg_type<Tag, T, N + 1>();
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
  using include_guard_tag = tag<[]{ }>;

  inline constexpr uintptr_t path_capacity = 1024;

  // Normalization is defensive: clang records the first spelling of a file per
  // TU, so __FILE__ is already spelling-deduped there. It matters on compilers
  // that keep include spellings as-written.
  template <uintptr_t Max>
  struct path_value {
    char data[Max]{};
    template <uintptr_t N>
    consteval path_value(const char (&s)[N]) {
      static_assert(N <= Max, "path_value: path exceeds capacity");
      uintptr_t seg_end[Max] = {};
      bool normal[Max] = {};
      uintptr_t depth = 0;
      const bool absolute = s[0] == '/';
      uintptr_t out = absolute ? 1 : 0;
      if (absolute) { data[0] = '/'; }
      uintptr_t i = out;
      while (s[i]) {
        if (s[i] == '/') { ++i; continue; }
        const uintptr_t start = i;
        while (s[i] && s[i] != '/') { ++i; }
        const uintptr_t len = i - start;
        if (len == 1 && s[start] == '.') { continue; }
        if (len == 2 && s[start] == '.' && s[start + 1] == '.') {
          if (depth > 0 && normal[depth - 1]) {
            const uintptr_t new_out = seg_end[--depth];
            for (uintptr_t j = new_out; j < out; ++j) { data[j] = '\0'; }
            out = new_out;
          } else if (!absolute) {
            seg_end[depth] = out;
            normal[depth++] = false;
            if (out > 0) { data[out++] = '/'; }
            data[out++] = '.';
            data[out++] = '.';
          }
          continue;
        }
        seg_end[depth] = out;
        normal[depth++] = true;
        if (out > (absolute ? 1u : 0u)) { data[out++] = '/'; }
        for (uintptr_t j = 0; j < len; ++j) { data[out + j] = s[start + j]; }
        out += len;
      }
      data[out] = '\0';
    }
    friend consteval bool operator==(const path_value&, const path_value&) = default;
  };

  template <path_value<path_capacity> P>
  struct path_id {};

  template <class FromTag, uintptr_t FromIndex, class ToTag>
  consteval uintptr_t convert_tag_index() {
    using T = decltype(get_type(nth<FromTag, FromIndex>{}));
    return reg_type<ToTag, T>();
  }

  // clang quirk: evaluating try_lookup_type/reg_type at an index whose type_slot
  // is not yet defined (requires probe fails, friend return type undeduced)
  // poisons later scans of that index for try_lookup_type — the slot reports as
  // unregistered forever. type_count (requires-only) and reg_type
  // (scan-then-define) are immune. Never use try_lookup_type to check a
  // registration before reg_type defines it (pre-registration duplicate
  // checks); use the count-before/count-after pattern (STRICT_INCLUDE_GUARD).
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

  template <uintptr_t I>
  consteval bool field_addr_match(const auto& Sref, const auto& Fref) {
    if constexpr (requires { &Sref.template structreg_get<I>() == &Fref; })
      return &Sref.template structreg_get<I>() == &Fref;
    return false;
  }

  template <typename StructT, uintptr_t Start, uintptr_t End>
  consteval uintptr_t _count_range() {
    if constexpr (Start >= End) return 0;
    else return []<uintptr_t... Is>(integer_sequence<uintptr_t, Is...>) {
      return (requires { StructT{}.template structreg_get<Start + Is>(); } + ...);
    }(_make_index_sequence<End - Start>{});
  }
}

#define STRUCTREG_TAG(name) \
  using name = _structreg_::tag<[]{ }>

#define _STRUCTREG_IMPL(name, uid, ...) \
  __VA_OPT__(static_assert(_structreg_::register_all<std::integral_constant<uintptr_t, uid>, __VA_ARGS__>());) \
  template <uintptr_t I, typename Tag = _structreg_::default_tag> \
  constexpr decltype(auto) structreg_get(this auto&& self) \
    requires (I == _structreg_::reg_type<std::conditional_t<std::is_same_v<Tag, _structreg_::default_tag>, std::remove_reference_t<decltype(self)>, Tag>, std::integral_constant<uintptr_t, uid>>()) \
  { \
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
  _structreg_::_count_range<std::remove_reference_t<decltype(obj)>, 0, 1024>()

#ifndef STRICT_INCLUDE_GUARD
  #define STRICT_INCLUDE_GUARD static_assert([]{ \
      using id = _structreg_::path_id<_structreg_::path_value<_structreg_::path_capacity>(__FILE__)>; \
      constexpr auto before = _structreg_::type_count<_structreg_::include_guard_tag>(); \
      constexpr auto idx = _structreg_::reg_type<_structreg_::include_guard_tag, id>(); \
      (void)idx; \
      constexpr auto after = _structreg_::type_count<_structreg_::include_guard_tag>(); \
      return after == before + 1; \
    }(), "duplicate include: " __FILE__);
#endif

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
  constexpr auto uid = []<uintptr_t... Is>(_structreg_::integer_sequence<uintptr_t, Is...>) {
    uintptr_t f = ~uintptr_t{};
    ((_structreg_::field_addr_match<Is>(S, F) ? (f = Is) || true : false) || ...);
    return f;
  }(_structreg_::_make_index_sequence<1024>{});
  static_assert(uid != ~uintptr_t{}, "structreg_get_tag_index: field not found in struct");
  using UidType = decltype(get_type(_structreg_::nth<T, uid>{}));
  constexpr auto r = _structreg_::try_lookup_type<Tag, UidType>();
  static_assert(r != ~uintptr_t{} || std::is_same_v<Tag, _structreg_::default_tag>, "structreg_get_tag_index: field not registered under tag");
  return r != ~uintptr_t{} ? r : uid;
}

#if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC diagnostic pop
#endif
