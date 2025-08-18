#ifndef NEARGYE_MAGIC_ENUM_HPP
#define NEARGYE_MAGIC_ENUM_HPP

#define MAGIC_ENUM_VERSION_MAJOR 0
#define MAGIC_ENUM_VERSION_MINOR 9
#define MAGIC_ENUM_VERSION_PATCH 7

#ifndef MAGIC_ENUM_USE_STD_MODULE
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>
#endif

#if defined(MAGIC_ENUM_CONFIG_FILE)
#  include MAGIC_ENUM_CONFIG_FILE
#endif

#ifndef MAGIC_ENUM_USE_STD_MODULE
#if !defined(MAGIC_ENUM_USING_ALIAS_OPTIONAL)
#  include <optional>
#endif
#if !defined(MAGIC_ENUM_USING_ALIAS_STRING)
#  include <string>
#endif
#if !defined(MAGIC_ENUM_USING_ALIAS_STRING_VIEW)
#  include <string_view>
#endif
#endif

#if defined(MAGIC_ENUM_NO_ASSERT)
#  define MAGIC_ENUM_ASSERT(...) static_cast<void>(0)
#elif !defined(MAGIC_ENUM_ASSERT)
#  include <cassert>
#  define MAGIC_ENUM_ASSERT(...) assert((__VA_ARGS__))
#endif

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunknown-warning-option"
#  pragma clang diagnostic ignored "-Wenum-constexpr-conversion"
#  pragma clang diagnostic ignored "-Wuseless-cast" // suppresses 'static_cast<char_type>('\0')' for char_type = char (common on Linux).
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wmaybe-uninitialized" // May be used uninitialized 'return {};'.
#  pragma GCC diagnostic ignored "-Wuseless-cast" // suppresses 'static_cast<char_type>('\0')' for char_type = char (common on Linux).
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 26495) // Variable 'static_str<N>::chars_' is uninitialized.
#  pragma warning(disable : 28020) // Arithmetic overflow: Using operator '-' on a 4 byte value and then casting the result to a 8 byte value.
#  pragma warning(disable : 26451) // The expression '0<=_Param_(1)&&_Param_(1)<=1-1' is not true at this call.
#  pragma warning(disable : 4514) // Unreferenced inline function has been removed.
#endif

// Checks magic_enum compiler compatibility.
#if defined(__clang__) && __clang_major__ >= 5 || defined(__GNUC__) && __GNUC__ >= 9 || defined(_MSC_VER) && _MSC_VER >= 1910 || defined(__RESHARPER__)
#  undef  MAGIC_ENUM_SUPPORTED
#  define MAGIC_ENUM_SUPPORTED 1
#endif

// Checks magic_enum compiler aliases compatibility.
#if defined(__clang__) && __clang_major__ >= 5 || defined(__GNUC__) && __GNUC__ >= 9 || defined(_MSC_VER) && _MSC_VER >= 1920
#  undef  MAGIC_ENUM_SUPPORTED_ALIASES
#  define MAGIC_ENUM_SUPPORTED_ALIASES 1
#endif

// Specify the calling convention for compilers that need it in order to get reliable mangled names under different
// compiler flags. In particular, MSVC allows changing the default calling convention on x86.
#if defined(__clang__) || defined(__GNUC__)
#define MAGIC_ENUM_CALLING_CONVENTION
#elif defined(_MSC_VER)
#define MAGIC_ENUM_CALLING_CONVENTION __cdecl
#endif

// Enum value must be greater or equals than MAGIC_ENUM_RANGE_MIN. By default MAGIC_ENUM_RANGE_MIN = -128.
// If need another min range for all enum types by default, redefine the macro MAGIC_ENUM_RANGE_MIN.
#if !defined(MAGIC_ENUM_RANGE_MIN)
#  define MAGIC_ENUM_RANGE_MIN 0
#endif

// Enum value must be less or equals than MAGIC_ENUM_RANGE_MAX. By default MAGIC_ENUM_RANGE_MAX = 127.
// If need another max range for all enum types by default, redefine the macro MAGIC_ENUM_RANGE_MAX.
#if !defined(MAGIC_ENUM_RANGE_MAX)
#  define MAGIC_ENUM_RANGE_MAX 64
#endif

// Improve ReSharper C++ intellisense performance with builtins, avoiding unnecessary template instantiations.
#if defined(__RESHARPER__)
#  undef MAGIC_ENUM_GET_ENUM_NAME_BUILTIN
#  undef MAGIC_ENUM_GET_TYPE_NAME_BUILTIN
#  if __RESHARPER__ >= 20230100
#    define MAGIC_ENUM_GET_ENUM_NAME_BUILTIN(V) __rscpp_enumerator_name(V)
#    define MAGIC_ENUM_GET_TYPE_NAME_BUILTIN(T) __rscpp_type_name<T>()
#  else
#    define MAGIC_ENUM_GET_ENUM_NAME_BUILTIN(V) nullptr
#    define MAGIC_ENUM_GET_TYPE_NAME_BUILTIN(T) nullptr
#  endif
#endif

namespace magic_enum {

// If need another optional type, define the macro MAGIC_ENUM_USING_ALIAS_OPTIONAL.
#if defined(MAGIC_ENUM_USING_ALIAS_OPTIONAL)
MAGIC_ENUM_USING_ALIAS_OPTIONAL
#else
using std::optional;
#endif

// If need another string_view type, define the macro MAGIC_ENUM_USING_ALIAS_STRING_VIEW.
#if defined(MAGIC_ENUM_USING_ALIAS_STRING_VIEW)
MAGIC_ENUM_USING_ALIAS_STRING_VIEW
#else
using std::string_view;
#endif

// If need another string type, define the macro MAGIC_ENUM_USING_ALIAS_STRING.
#if defined(MAGIC_ENUM_USING_ALIAS_STRING)
MAGIC_ENUM_USING_ALIAS_STRING
#else
using std::string;
#endif

using char_type = string_view::value_type;
static_assert(std::is_same_v<string_view::value_type, string::value_type>, "magic_enum::customize requires same string_view::value_type and string::value_type");
static_assert([] {
  if constexpr (std::is_same_v<char_type, wchar_t>) {
    constexpr const char     c[] =  "abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789|";
    constexpr const wchar_t wc[] = L"abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789|";
    static_assert(std::size(c) == std::size(wc), "magic_enum::customize identifier characters are multichars in wchar_t.");

    for (std::size_t i = 0; i < std::size(c); ++i) {
      if (c[i] != wc[i]) {
        return false;
      }
    }
  }
  return true;
} (), "magic_enum::customize wchar_t is not compatible with ASCII.");

namespace detail {
    template<typename E, typename = void>
    constexpr inline bool has_is_flags_adl = false;

    template<typename E>
    constexpr inline bool has_is_flags_adl < E, std::void_t<decltype(decltype(adl_magic_enum_define_range(E{}))::is_flags) > > = decltype(adl_magic_enum_define_range(E{}))::is_flags;

    template<typename E, typename = void>
    constexpr inline auto has_minmax_adl = std::pair<int, int>(MAGIC_ENUM_RANGE_MIN, MAGIC_ENUM_RANGE_MAX);

    template<typename E>
    constexpr inline auto has_minmax_adl < E, std::void_t<decltype(decltype(adl_magic_enum_define_range(E{}))::max), decltype(decltype(adl_magic_enum_define_range(E{}))::max) >> =
        std::pair<int, int>(decltype(adl_magic_enum_define_range(E{}))::min, decltype(adl_magic_enum_define_range(E{}))::max);
}



namespace customize {

template<auto... Vs>
struct adl_info { static_assert(sizeof...(Vs) && !sizeof...(Vs), "adl_info parameter types must be either 2 ints exactly or 1 bool for the is_flgas"); };

template<int Min, int Max>
struct adl_info<Min, Max> {
    static constexpr int min = Min;
    static constexpr int max = Max;
};

template<bool IsFlags>
struct adl_info<IsFlags> {
    static constexpr bool is_flags = IsFlags;
};

// Enum value must be in range [MAGIC_ENUM_RANGE_MIN, MAGIC_ENUM_RANGE_MAX]. By default MAGIC_ENUM_RANGE_MIN = -128, MAGIC_ENUM_RANGE_MAX = 127.
// If need another range for all enum types by default, redefine the macro MAGIC_ENUM_RANGE_MIN and MAGIC_ENUM_RANGE_MAX.
// If need another range for specific enum type, add specialization enum_range for necessary enum type.
template <typename E, typename = void>
struct enum_range {
    static constexpr int min = MAGIC_ENUM_RANGE_MIN;
    static constexpr int max = MAGIC_ENUM_RANGE_MAX;
};

template <typename E>
struct enum_range < E, decltype(void(adl_magic_enum_define_range(E{}))) > {
    static constexpr int min = detail::has_minmax_adl<E>.first;
    static constexpr int max = detail::has_minmax_adl<E>.second;
    static constexpr bool is_flags = detail::has_is_flags_adl<E>;

    static_assert(is_flags || min != MAGIC_ENUM_RANGE_MIN || max != MAGIC_ENUM_RANGE_MAX, 
        "adl_magic_enum_define_range is declared for this enum but does not define any constants.\n"
        "be sure that the member names are static and are not mispelled.");
};

static_assert(MAGIC_ENUM_RANGE_MAX > MAGIC_ENUM_RANGE_MIN, "MAGIC_ENUM_RANGE_MAX must be greater than MAGIC_ENUM_RANGE_MIN.");

namespace detail {

enum class customize_tag {
  default_tag,
  invalid_tag,
  custom_tag
};

} // namespace magic_enum::customize::detail

class customize_t : public std::pair<detail::customize_tag, string_view> {
 public:
  constexpr customize_t(string_view srt) : std::pair<detail::customize_tag, string_view>{detail::customize_tag::custom_tag, srt} {}
  constexpr customize_t(const char_type* srt) : customize_t{string_view{srt}} {}
  constexpr customize_t(detail::customize_tag tag) : std::pair<detail::customize_tag, string_view>{tag, string_view{}} {
    MAGIC_ENUM_ASSERT(tag != detail::customize_tag::custom_tag);
  }
};

// Default customize.
inline constexpr auto default_tag = customize_t{detail::customize_tag::default_tag};
// Invalid customize.
inline constexpr auto invalid_tag = customize_t{detail::customize_tag::invalid_tag};

// If need custom names for enum, add specialization enum_name for necessary enum type.
template <typename E>
constexpr customize_t enum_name(E) noexcept {
  return default_tag;
}

} // namespace magic_enum::customize

namespace detail {

template <typename T>
struct supported
#if defined(MAGIC_ENUM_SUPPORTED) && MAGIC_ENUM_SUPPORTED || defined(MAGIC_ENUM_NO_CHECK_SUPPORT)
  : std::true_type {};
#else
  : std::false_type {};
#endif

template <auto V, typename E = std::decay_t<decltype(V)>, std::enable_if_t<std::is_enum_v<E>, int> = 0>
using enum_constant = std::integral_constant<E, V>;

template <typename... T>
inline constexpr bool always_false_v = false;

template <typename T, typename = void>
struct has_is_flags : std::false_type {};

template <typename T>
struct has_is_flags<T, std::void_t<decltype(customize::enum_range<T>::is_flags)>> : std::bool_constant<std::is_same_v<bool, std::decay_t<decltype(customize::enum_range<T>::is_flags)>>> {};

template <typename T, typename = void>
struct range_min : std::integral_constant<int, MAGIC_ENUM_RANGE_MIN> {};

template <typename T>
struct range_min<T, std::void_t<decltype(customize::enum_range<T>::min)>> : std::integral_constant<decltype(customize::enum_range<T>::min), customize::enum_range<T>::min> {};

template <typename T, typename = void>
struct range_max : std::integral_constant<int, MAGIC_ENUM_RANGE_MAX> {};

template <typename T>
struct range_max<T, std::void_t<decltype(customize::enum_range<T>::max)>> : std::integral_constant<decltype(customize::enum_range<T>::max), customize::enum_range<T>::max> {};

struct str_view {
  const char* str_ = nullptr;
  std::size_t size_ = 0;
};

template <std::uint16_t N>
class static_str {
 public:
  constexpr explicit static_str(str_view str) noexcept : static_str{str.str_, std::make_integer_sequence<std::uint16_t, N>{}} {
    MAGIC_ENUM_ASSERT(str.size_ == N);
  }

  constexpr explicit static_str(string_view str) noexcept : static_str{str.data(), std::make_integer_sequence<std::uint16_t, N>{}} {
    MAGIC_ENUM_ASSERT(str.size() == N);
  }

  constexpr const char_type* data() const noexcept { return chars_; }

  constexpr std::uint16_t size() const noexcept { return N; }

  constexpr operator string_view() const noexcept { return {data(), size()}; }

 private:
  template <std::uint16_t... I>
  constexpr static_str(const char* str, std::integer_sequence<std::uint16_t, I...>) noexcept : chars_{static_cast<char_type>(str[I])..., static_cast<char_type>('\0')} {}

  template <std::uint16_t... I>
  constexpr static_str(string_view str, std::integer_sequence<std::uint16_t, I...>) noexcept : chars_{str[I]..., static_cast<char_type>('\0')} {}

  char_type chars_[static_cast<std::size_t>(N) + 1];
};

template <>
class static_str<0> {
 public:
  constexpr explicit static_str() = default;

  constexpr explicit static_str(str_view) noexcept {}

  constexpr explicit static_str(string_view) noexcept {}

  constexpr const char_type* data() const noexcept { return nullptr; }

  constexpr std::uint16_t size() const noexcept { return 0; }

  constexpr operator string_view() const noexcept { return {}; }
};

template <typename Op = std::equal_to<>>
class case_insensitive {
  static constexpr char_type to_lower(char_type c) noexcept {
    return (c >= static_cast<char_type>('A') && c <= static_cast<char_type>('Z')) ? static_cast<char_type>(c + (static_cast<char_type>('a') - static_cast<char_type>('A'))) : c;
  }

 public:
  template <typename L, typename R>
  constexpr auto operator()(L lhs, R rhs) const noexcept -> std::enable_if_t<std::is_same_v<std::decay_t<L>, char_type> && std::is_same_v<std::decay_t<R>, char_type>, bool> {
    return Op{}(to_lower(lhs), to_lower(rhs));
  }
};

constexpr std::size_t find(string_view str, char_type c) noexcept {
#if defined(__clang__) && __clang_major__ < 9 && defined(__GLIBCXX__) || defined(_MSC_VER) && _MSC_VER < 1920 && !defined(__clang__)
// https://stackoverflow.com/questions/56484834/constexpr-stdstring-viewfind-last-of-doesnt-work-on-clang-8-with-libstdc
// https://developercommunity.visualstudio.com/content/problem/360432/vs20178-regression-c-failed-in-test.html
  constexpr bool workaround = true;
#else
  constexpr bool workaround = false;
#endif

  if constexpr (workaround) {
    for (std::size_t i = 0; i < str.size(); ++i) {
      if (str[i] == c) {
        return i;
      }
    }

    return string_view::npos;
  } else {
    return str.find(c);
  }
}

template <typename BinaryPredicate>
inline constexpr bool is_default_predicate_v = std::is_same_v<std::decay_t<BinaryPredicate>, std::equal_to<string_view::value_type>> || std::is_same_v<std::decay_t<BinaryPredicate>, std::equal_to<>>;


template <typename BinaryPredicate>
inline constexpr bool is_nothrow_invocable_v = is_default_predicate_v<BinaryPredicate> || std::is_nothrow_invocable_r_v<bool, BinaryPredicate, char_type, char_type>;


template <typename BinaryPredicate>
constexpr bool cmp_equal(string_view lhs, string_view rhs, [[maybe_unused]] BinaryPredicate&& p) noexcept(is_nothrow_invocable_v<BinaryPredicate>) {
#if defined(_MSC_VER) && _MSC_VER < 1920 && !defined(__clang__)
  // https://developercommunity.visualstudio.com/content/problem/360432/vs20178-regression-c-failed-in-test.html
  // https://developercommunity.visualstudio.com/content/problem/232218/c-constexpr-string-view.html
  constexpr bool workaround = true;
#else
  constexpr bool workaround = false;
#endif

  if constexpr (!is_default_predicate_v<BinaryPredicate> || workaround) {
    if (lhs.size() != rhs.size()) {
      return false;
    }

    const auto size = lhs.size();
    for (std::size_t i = 0; i < size; ++i) {
      if (!p(lhs[i], rhs[i])) {
        return false;
      }
    }

    return true;
  } else {
    return lhs == rhs;
  }
}

template <typename L, typename R>
constexpr bool cmp_less(L lhs, R rhs) noexcept {
  static_assert(std::is_integral_v<L> && std::is_integral_v<R>, "magic_enum::detail::cmp_less requires integral type.");

  if constexpr (std::is_signed_v<L> == std::is_signed_v<R>) {
    // If same signedness (both signed or both unsigned).
    return lhs < rhs;
  } else if constexpr (std::is_same_v<L, bool>) { // bool special case
      return static_cast<R>(lhs) < rhs;
  } else if constexpr (std::is_same_v<R, bool>) { // bool special case
      return lhs < static_cast<L>(rhs);
  } else if constexpr (std::is_signed_v<R>) {
    // If 'right' is negative, then result is 'false', otherwise cast & compare.
    return rhs > 0 && lhs < static_cast<std::make_unsigned_t<R>>(rhs);
  } else {
    // If 'left' is negative, then result is 'true', otherwise cast & compare.
    return lhs < 0 || static_cast<std::make_unsigned_t<L>>(lhs) < rhs;
  }
}

template <typename I>
constexpr I log2(I value) noexcept {
  static_assert(std::is_integral_v<I>, "magic_enum::detail::log2 requires integral type.");

  if constexpr (std::is_same_v<I, bool>) { // bool special case
    return MAGIC_ENUM_ASSERT(false), value;
  } else {
    auto ret = I{0};
    for (; value > I{1}; value >>= I{1}, ++ret) {}

    return ret;
  }
}

#if defined(__cpp_lib_array_constexpr) && __cpp_lib_array_constexpr >= 201603L
#  define MAGIC_ENUM_ARRAY_CONSTEXPR 1
#else
template <typename T, std::size_t N, std::size_t... I>
constexpr std::array<std::remove_cv_t<T>, N> to_array(T (&a)[N], std::index_sequence<I...>) noexcept {
  return {{a[I]...}};
}
#endif

template <typename T>
inline constexpr bool is_enum_v = std::is_enum_v<T> && std::is_same_v<T, std::decay_t<T>>;

template <typename E>
constexpr auto MAGIC_ENUM_CALLING_CONVENTION n() noexcept {
  static_assert(is_enum_v<E>, "magic_enum::detail::n requires enum type.");

  if constexpr (supported<E>::value) {
#if defined(MAGIC_ENUM_GET_TYPE_NAME_BUILTIN)
    constexpr auto name_ptr = MAGIC_ENUM_GET_TYPE_NAME_BUILTIN(E);
    constexpr auto name = name_ptr ? str_view{name_ptr, std::char_traits<char>::length(name_ptr)} : str_view{};
#elif defined(__clang__)
    str_view name;
    if constexpr (sizeof(__PRETTY_FUNCTION__) == sizeof(__FUNCTION__)) {
      static_assert(always_false_v<E>, "magic_enum::detail::n requires __PRETTY_FUNCTION__.");
      return str_view{};
    } else {
      name.size_ = sizeof(__PRETTY_FUNCTION__) - 36;
      name.str_ = __PRETTY_FUNCTION__ + 34;
    }
#elif defined(__GNUC__)
    auto name = str_view{__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 1};
    if constexpr (sizeof(__PRETTY_FUNCTION__) == sizeof(__FUNCTION__)) {
      static_assert(always_false_v<E>, "magic_enum::detail::n requires __PRETTY_FUNCTION__.");
      return str_view{};
    } else if (name.str_[name.size_ - 1] == ']') {
      name.size_ -= 50;
      name.str_ += 49;
    } else {
      name.size_ -= 40;
      name.str_ += 37;
    }
#elif defined(_MSC_VER)
    // CLI/C++ workaround (see https://github.com/Neargye/magic_enum/issues/284).
    str_view name;
    name.str_ = __FUNCSIG__;
    name.str_ += 40;
    name.size_ += sizeof(__FUNCSIG__) - 57;
#else
    auto name = str_view{};
#endif
    std::size_t p = 0;
    for (std::size_t i = name.size_; i > 0; --i) {
      if (name.str_[i] == ':') {
        p = i + 1;
        break;
      }
    }
    if (p > 0) {
      name.size_ -= p;
      name.str_ += p;
    }
    return name;
  } else {
    return str_view{}; // Unsupported compiler or Invalid customize.
  }
}

template <auto V>
constexpr auto MAGIC_ENUM_CALLING_CONVENTION n() noexcept {
  static_assert(is_enum_v<decltype(V)>, "magic_enum::detail::n requires enum type.");

  if constexpr (supported<decltype(V)>::value) {
#if defined(MAGIC_ENUM_GET_ENUM_NAME_BUILTIN)
    constexpr auto name_ptr = MAGIC_ENUM_GET_ENUM_NAME_BUILTIN(V);
    auto name = name_ptr ? str_view{name_ptr, std::char_traits<char>::length(name_ptr)} : str_view{};
#elif defined(__clang__)
    str_view name;
    if constexpr (sizeof(__PRETTY_FUNCTION__) == sizeof(__FUNCTION__)) {
      static_assert(always_false_v<decltype(V)>, "magic_enum::detail::n requires __PRETTY_FUNCTION__.");
      return str_view{};
    } else {
      name.size_ = sizeof(__PRETTY_FUNCTION__) - 36;
      name.str_ = __PRETTY_FUNCTION__ + 34;
    }
    if (name.size_ > 22 && name.str_[0] == '(' && name.str_[1] == 'a' && name.str_[10] == ' ' && name.str_[22] == ':') {
      name.size_ -= 23;
      name.str_ += 23;
    }
    if (name.str_[0] == '(' || name.str_[0] == '-' || (name.str_[0] >= '0' && name.str_[0] <= '9')) {
      name = str_view{};
    }
#elif defined(__GNUC__)
    auto name = str_view{__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 1};
    if constexpr (sizeof(__PRETTY_FUNCTION__) == sizeof(__FUNCTION__)) {
      static_assert(always_false_v<decltype(V)>, "magic_enum::detail::n requires __PRETTY_FUNCTION__.");
      return str_view{};
    } else if (name.str_[name.size_ - 1] == ']') {
      name.size_ -= 55;
      name.str_ += 54;
    } else {
      name.size_ -= 40;
      name.str_ += 37;
    }
    if (name.str_[0] == '(') {
      name = str_view{};
    }
#elif defined(_MSC_VER)
    str_view name;
    if ((__FUNCSIG__[5] == '_' && __FUNCSIG__[35] != '(') || (__FUNCSIG__[5] == 'c' && __FUNCSIG__[41] != '(')) {
      // CLI/C++ workaround (see https://github.com/Neargye/magic_enum/issues/284).
      name.str_ = __FUNCSIG__;
      name.str_ += 35;
      name.size_ = sizeof(__FUNCSIG__) - 52;
    }
#else
    auto name = str_view{};
#endif
    std::size_t p = 0;
    for (std::size_t i = name.size_; i > 0; --i) {
      if (name.str_[i] == ':') {
        p = i + 1;
        break;
      }
    }
    if (p > 0) {
      name.size_ -= p;
      name.str_ += p;
    }
    return name;
  } else {
    return str_view{}; // Unsupported compiler or Invalid customize.
  }
}

#if defined(_MSC_VER) && !defined(__clang__) && _MSC_VER < 1920
#  define MAGIC_ENUM_VS_2017_WORKAROUND 1
#endif

#if defined(MAGIC_ENUM_VS_2017_WORKAROUND)
template <typename E, E V>
constexpr auto MAGIC_ENUM_CALLING_CONVENTION n() noexcept {
  static_assert(is_enum_v<E>, "magic_enum::detail::n requires enum type.");

#  if defined(MAGIC_ENUM_GET_ENUM_NAME_BUILTIN)
  constexpr auto name_ptr = MAGIC_ENUM_GET_ENUM_NAME_BUILTIN(V);
  auto name = name_ptr ? str_view{name_ptr, std::char_traits<char>::length(name_ptr)} : str_view{};
#  else
  // CLI/C++ workaround (see https://github.com/Neargye/magic_enum/issues/284).
  str_view name;
  name.str_ = __FUNCSIG__;
  name.size_ = sizeof(__FUNCSIG__) - 17;
  std::size_t p = 0;
  for (std::size_t i = name.size_; i > 0; --i) {
    if (name.str_[i] == ',' || name.str_[i] == ':') {
      p = i + 1;
      break;
    }
  }
  if (p > 0) {
    name.size_ -= p;
    name.str_ += p;
  }
  if (name.str_[0] == '(' || name.str_[0] == '-' || (name.str_[0] >= '0' && name.str_[0] <= '9')) {
    name = str_view{};
  }
  return name;
#  endif
}
#endif

template <typename E, E V>
constexpr auto enum_name() noexcept {
  [[maybe_unused]] constexpr auto custom = customize::enum_name<E>(V);
  static_assert(std::is_same_v<std::decay_t<decltype(custom)>, customize::customize_t>, "magic_enum::customize requires customize_t type.");
  if constexpr (custom.first == customize::detail::customize_tag::custom_tag) {
    constexpr auto name = custom.second;
    static_assert(!name.empty(), "magic_enum::customize requires not empty string.");
    return static_str<name.size()>{name};
  } else if constexpr (custom.first == customize::detail::customize_tag::invalid_tag) {
    return static_str<0>{};
  } else if constexpr (custom.first == customize::detail::customize_tag::default_tag) {
#if defined(MAGIC_ENUM_VS_2017_WORKAROUND)
    constexpr auto name = n<E, V>();
#else
    constexpr auto name = n<V>();   
#endif
    return static_str<name.size_>{name};
  } else {
    static_assert(always_false_v<E>, "magic_enum::customize invalid.");
  }
}

template <typename E, E V>
inline constexpr auto enum_name_v = enum_name<E, V>();

// CWG1766: Values outside the range of the values of an enumeration
// https://reviews.llvm.org/D130058, https://reviews.llvm.org/D131307
#if defined(__clang__) && __clang_major__ >= 16
template <typename E, auto V, typename = void>
inline constexpr bool is_enum_constexpr_static_cast_valid = false;
template <typename E, auto V>
inline constexpr bool is_enum_constexpr_static_cast_valid<E, V, std::void_t<std::integral_constant<E, static_cast<E>(V)>>> = true;
#else
template <typename, auto>
inline constexpr bool is_enum_constexpr_static_cast_valid = true;
#endif

template <typename E, auto V>
constexpr bool is_valid() noexcept {
  if constexpr (is_enum_constexpr_static_cast_valid<E, V>) {
    constexpr E v = static_cast<E>(V);
    [[maybe_unused]] constexpr auto custom = customize::enum_name<E>(v);
    static_assert(std::is_same_v<std::decay_t<decltype(custom)>, customize::customize_t>, "magic_enum::customize requires customize_t type.");
    if constexpr (custom.first == customize::detail::customize_tag::custom_tag) {
      constexpr auto name = custom.second;
      static_assert(!name.empty(), "magic_enum::customize requires not empty string.");
      return name.size() != 0;
    } else if constexpr (custom.first == customize::detail::customize_tag::default_tag) {
#if defined(MAGIC_ENUM_VS_2017_WORKAROUND)
      return n<E, v>().size_ != 0;
#else
      return n<v>().size_ != 0;
#endif
    } else {
      return false;
    }
  } else {
    return false;
  }
}

enum class enum_subtype {
  common,
  flags
};

template <typename E, int O, enum_subtype S, typename U = std::underlying_type_t<E>>
constexpr U ualue(std::size_t i) noexcept {
  if constexpr (std::is_same_v<U, bool>) { // bool special case
    static_assert(O == 0, "magic_enum::detail::ualue requires valid offset.");

    return static_cast<U>(i);
  } else if constexpr (S == enum_subtype::flags) {
    return static_cast<U>(U{1} << static_cast<U>(static_cast<int>(i) + O));
  } else {
    return static_cast<U>(static_cast<int>(i) + O);
  }
}

template <typename E, int O, enum_subtype S, typename U = std::underlying_type_t<E>>
constexpr E value(std::size_t i) noexcept {
  return static_cast<E>(ualue<E, O, S>(i));
}

template <typename E, enum_subtype S, typename U = std::underlying_type_t<E>>
constexpr int reflected_min() noexcept {
  if constexpr (S == enum_subtype::flags) {
    return 0;
  } else {
    constexpr auto lhs = range_min<E>::value;
    constexpr auto rhs = (std::numeric_limits<U>::min)();

    if constexpr (cmp_less(rhs, lhs)) {
      return lhs;
    } else {
      return rhs;
    }
  }
}

template <typename E, enum_subtype S, typename U = std::underlying_type_t<E>>
constexpr int reflected_max() noexcept {
  if constexpr (S == enum_subtype::flags) {
    return std::numeric_limits<U>::digits - 1;
  } else {
    constexpr auto lhs = range_max<E>::value;//std::integral_constant<int, MAGIC_ENUM_RANGE_MAX> {}
    constexpr auto rhs = (std::numeric_limits<U>::max)();

    if constexpr (cmp_less(lhs, rhs)) {
      return lhs;
    } else {
      return rhs;
    }
  }
}

#define MAGIC_ENUM_FOR_EACH_256(T)                                                                                                                                                                 \
  T(  0)T(  1)T(  2)T(  3)T(  4)T(  5)T(  6)T(  7)T(  8)T(  9)T( 10)T( 11)T( 12)T( 13)T( 14)T( 15)T( 16)T( 17)T( 18)T( 19)T( 20)T( 21)T( 22)T( 23)T( 24)T( 25)T( 26)T( 27)T( 28)T( 29)T( 30)T( 31) \
  T( 32)T( 33)T( 34)T( 35)T( 36)T( 37)T( 38)T( 39)T( 40)T( 41)T( 42)T( 43)T( 44)T( 45)T( 46)T( 47)T( 48)T( 49)T( 50)T( 51)T( 52)T( 53)T( 54)T( 55)T( 56)T( 57)T( 58)T( 59)T( 60)T( 61)T( 62)T( 63) \
  T( 64)T( 65)T( 66)T( 67)T( 68)T( 69)T( 70)T( 71)T( 72)T( 73)T( 74)T( 75)T( 76)T( 77)T( 78)T( 79)T( 80)T( 81)T( 82)T( 83)T( 84)T( 85)T( 86)T( 87)T( 88)T( 89)T( 90)T( 91)T( 92)T( 93)T( 94)T( 95) \
  T( 96)T( 97)T( 98)T( 99)T(100)T(101)T(102)T(103)T(104)T(105)T(106)T(107)T(108)T(109)T(110)T(111)T(112)T(113)T(114)T(115)T(116)T(117)T(118)T(119)T(120)T(121)T(122)T(123)T(124)T(125)T(126)T(127) \
  T(128)T(129)T(130)T(131)T(132)T(133)T(134)T(135)T(136)T(137)T(138)T(139)T(140)T(141)T(142)T(143)T(144)T(145)T(146)T(147)T(148)T(149)T(150)T(151)T(152)T(153)T(154)T(155)T(156)T(157)T(158)T(159) \
  T(160)T(161)T(162)T(163)T(164)T(165)T(166)T(167)T(168)T(169)T(170)T(171)T(172)T(173)T(174)T(175)T(176)T(177)T(178)T(179)T(180)T(181)T(182)T(183)T(184)T(185)T(186)T(187)T(188)T(189)T(190)T(191) \
  T(192)T(193)T(194)T(195)T(196)T(197)T(198)T(199)T(200)T(201)T(202)T(203)T(204)T(205)T(206)T(207)T(208)T(209)T(210)T(211)T(212)T(213)T(214)T(215)T(216)T(217)T(218)T(219)T(220)T(221)T(222)T(223) \
  T(224)T(225)T(226)T(227)T(228)T(229)T(230)T(231)T(232)T(233)T(234)T(235)T(236)T(237)T(238)T(239)T(240)T(241)T(242)T(243)T(244)T(245)T(246)T(247)T(248)T(249)T(250)T(251)T(252)T(253)T(254)T(255)

template <typename E, enum_subtype S, std::size_t Size, int Min, std::size_t I>
constexpr void valid_count(bool* valid, std::size_t& count) noexcept {
#define MAGIC_ENUM_V(O)                                     \
  if constexpr ((I + O) < Size) {                           \
    if constexpr (is_valid<E, ualue<E, Min, S>(I + O)>()) { \
      valid[I + O] = true;                                  \
      ++count;                                              \
    }                                                       \
  }

  MAGIC_ENUM_FOR_EACH_256(MAGIC_ENUM_V)

  if constexpr ((I + 256) < Size) {
    valid_count<E, S, Size, Min, I + 256>(valid, count);
  }
#undef MAGIC_ENUM_V
}

template <std::size_t N>
struct valid_count_t {
  std::size_t count = 0;
  bool valid[N] = {};
};

template <typename E, enum_subtype S, std::size_t Size, int Min>
constexpr auto valid_count() noexcept {
  valid_count_t<Size> vc;
  valid_count<E, S, Size, Min, 0>(vc.valid, vc.count);
  return vc;
}

template <typename E, enum_subtype S, std::size_t Size, int Min>
constexpr auto values() noexcept {
  constexpr auto vc = valid_count<E, S, Size, Min>();

  if constexpr (vc.count > 0) {
#if defined(MAGIC_ENUM_ARRAY_CONSTEXPR)
    std::array<E, vc.count> values = {};
#else
    E values[vc.count] = {};
#endif
    for (std::size_t i = 0, v = 0; v < vc.count; ++i) {
      if (vc.valid[i]) {
        values[v++] = value<E, Min, S>(i);
      }
    }
#if defined(MAGIC_ENUM_ARRAY_CONSTEXPR)
    return values;
#else
    return to_array(values, std::make_index_sequence<vc.count>{});
#endif
  } else {
    return std::array<E, 0>{};
  }
}

template <typename E, enum_subtype S, typename U = std::underlying_type_t<E>>
constexpr auto values() noexcept {
  constexpr auto min = reflected_min<E, S>();
  constexpr auto max = reflected_max<E, S>();
  constexpr auto range_size = max - min + 1;
  static_assert(range_size > 0, "magic_enum::enum_range requires valid size.");

  return values<E, S, range_size, min>();
}

template <typename E, typename U = std::underlying_type_t<E>>
constexpr enum_subtype subtype(std::true_type) noexcept {
  if constexpr (std::is_same_v<U, bool>) { // bool special case
    return enum_subtype::common;
  } else if constexpr (has_is_flags<E>::value) {
    return customize::enum_range<E>::is_flags ? enum_subtype::flags : enum_subtype::common;
  } else {
    return enum_subtype::common;
  }
}

template <typename T>
constexpr enum_subtype subtype(std::false_type) noexcept {
  // For non-enum type return default common subtype.
  return enum_subtype::common;
}

template <typename E, typename D = std::decay_t<E>>
inline constexpr auto subtype_v = subtype<D>(std::is_enum<D>{});

template <typename E, enum_subtype S>
inline constexpr auto values_v = values<E, S>();

template <typename E, enum_subtype S, typename D = std::decay_t<E>>
using values_t = decltype((values_v<D, S>));

template <typename E, enum_subtype S>
inline constexpr auto count_v = values_v<E, S>.size();

template <typename E, enum_subtype S, typename U = std::underlying_type_t<E>>
inline constexpr auto min_v = (count_v<E, S> > 0) ? static_cast<U>(values_v<E, S>.front()) : U{0};

template <typename E, enum_subtype S, typename U = std::underlying_type_t<E>>
inline constexpr auto max_v = (count_v<E, S> > 0) ? static_cast<U>(values_v<E, S>.back()) : U{0};

template <typename E, enum_subtype S, std::size_t... I>
constexpr auto names(std::index_sequence<I...>) noexcept {
  constexpr auto names = std::array<string_view, sizeof...(I)>{{enum_name_v<E, values_v<E, S>[I]>...}};
  return names;
}

template <typename E, enum_subtype S>
inline constexpr auto names_v = names<E, S>(std::make_index_sequence<count_v<E, S>>{});

template <typename E, enum_subtype S, typename D = std::decay_t<E>>
using names_t = decltype((names_v<D, S>));

template <typename E, enum_subtype S, std::size_t... I>
constexpr auto entries(std::index_sequence<I...>) noexcept {
  constexpr auto entries = std::array<std::pair<E, string_view>, sizeof...(I)>{{{values_v<E, S>[I], enum_name_v<E, values_v<E, S>[I]>}...}};
  return entries;
}

template <typename E, enum_subtype S>
inline constexpr auto entries_v = entries<E, S>(std::make_index_sequence<count_v<E, S>>{});

template <typename E, enum_subtype S, typename D = std::decay_t<E>>
using entries_t = decltype((entries_v<D, S>));

template <typename E, enum_subtype S, typename U = std::underlying_type_t<E>>
constexpr bool is_sparse() noexcept {
  if constexpr (count_v<E, S> == 0) {
    return false;
  } else if constexpr (std::is_same_v<U, bool>) { // bool special case
    return false;
  } else {
    constexpr auto max = (S == enum_subtype::flags) ? log2(max_v<E, S>) : max_v<E, S>;
    constexpr auto min = (S == enum_subtype::flags) ? log2(min_v<E, S>) : min_v<E, S>;
    constexpr auto range_size = max - min + 1;

    return range_size != count_v<E, S>;
  }
}

template <typename E, enum_subtype S = subtype_v<E>>
inline constexpr bool is_sparse_v = is_sparse<E, S>();

template <typename E, enum_subtype S>
struct is_reflected
#if defined(MAGIC_ENUM_NO_CHECK_REFLECTED_ENUM)
  : std::true_type {};
#else
  : std::bool_constant<std::is_enum_v<E> && (count_v<E, S> != 0)> {};
#endif

template <typename E, enum_subtype S>
inline constexpr bool is_reflected_v = is_reflected<std::decay_t<E>, S>{};

template <bool, typename R>
struct enable_if_enum {};

template <typename R>
struct enable_if_enum<true, R> {
  using type = R;
  static_assert(supported<R>::value, "magic_enum unsupported compiler (https://github.com/Neargye/magic_enum#compiler-compatibility).");
};

template <typename T, typename R, typename BinaryPredicate = std::equal_to<>, typename D = std::decay_t<T>>
using enable_if_t = typename enable_if_enum<std::is_enum_v<D> && std::is_invocable_r_v<bool, BinaryPredicate, char_type, char_type>, R>::type;

template <typename T, std::enable_if_t<std::is_enum_v<std::decay_t<T>>, int> = 0>
using enum_concept = T;

template <typename T, bool = std::is_enum_v<T>>
struct is_scoped_enum : std::false_type {};

template <typename T>
struct is_scoped_enum<T, true> : std::bool_constant<!std::is_convertible_v<T, std::underlying_type_t<T>>> {};

template <typename T, bool = std::is_enum_v<T>>
struct is_unscoped_enum : std::false_type {};

template <typename T>
struct is_unscoped_enum<T, true> : std::bool_constant<std::is_convertible_v<T, std::underlying_type_t<T>>> {};

template <typename T, bool = std::is_enum_v<std::decay_t<T>>>
struct underlying_type {};

template <typename T>
struct underlying_type<T, true> : std::underlying_type<std::decay_t<T>> {};

#if defined(MAGIC_ENUM_ENABLE_HASH) || defined(MAGIC_ENUM_ENABLE_HASH_SWITCH)

#endif

} // namespace magic_enum::detail

// Checks is magic_enum supported compiler.
inline constexpr bool is_magic_enum_supported = detail::supported<void>::value;

template <typename T>
using Enum = detail::enum_concept<T>;

// Checks whether T is an Unscoped enumeration type.
// Provides the member constant value which is equal to true, if T is an [Unscoped enumeration](https://en.cppreference.com/w/cpp/language/enum#Unscoped_enumeration) type. Otherwise, value is equal to false.
template <typename T>
struct is_unscoped_enum : detail::is_unscoped_enum<T> {};

template <typename T>
inline constexpr bool is_unscoped_enum_v = is_unscoped_enum<T>::value;

// Checks whether T is an Scoped enumeration type.
// Provides the member constant value which is equal to true, if T is an [Scoped enumeration](https://en.cppreference.com/w/cpp/language/enum#Scoped_enumerations) type. Otherwise, value is equal to false.
template <typename T>
struct is_scoped_enum : detail::is_scoped_enum<T> {};

template <typename T>
inline constexpr bool is_scoped_enum_v = is_scoped_enum<T>::value;

// If T is a complete enumeration type, provides a member typedef type that names the underlying type of T.
// Otherwise, if T is not an enumeration type, there is no member type. Otherwise (T is an incomplete enumeration type), the program is ill-formed.
template <typename T>
struct underlying_type : detail::underlying_type<T> {};

template <typename T>
using underlying_type_t = typename underlying_type<T>::type;

template <auto V>
using enum_constant = detail::enum_constant<V>;

// Returns enum value at specified index.
// No bounds checking is performed: the behavior is undefined if index >= number of enum values.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_value(std::size_t index) noexcept -> detail::enable_if_t<E, std::decay_t<E>> {
  using D = std::decay_t<E>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");

  if constexpr (detail::is_sparse_v<D, S>) {
    return MAGIC_ENUM_ASSERT(index < detail::count_v<D, S>), detail::values_v<D, S>[index];
  } else {
    constexpr auto min = (S == detail::enum_subtype::flags) ? detail::log2(detail::min_v<D, S>) : detail::min_v<D, S>;

    return MAGIC_ENUM_ASSERT(index < detail::count_v<D, S>), detail::value<D, min, S>(index);
  }
}

// Returns enum value at specified index.
template <typename E, std::size_t I, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_value() noexcept -> detail::enable_if_t<E, std::decay_t<E>> {
  using D = std::decay_t<E>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");
  static_assert(I < detail::count_v<D, S>, "magic_enum::enum_value out of range.");

  return enum_value<D, S>(I);
}

// Returns std::array with enum values, sorted by enum value.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_values() noexcept -> detail::enable_if_t<E, detail::values_t<E, S>> {
  using D = std::decay_t<E>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");

  return detail::values_v<D, S>;
}

// Returns integer value from enum value.
template <typename E>
[[nodiscard]] constexpr auto enum_integer(E value) noexcept -> detail::enable_if_t<E, underlying_type_t<E>> {
  return static_cast<underlying_type_t<E>>(value);
}

// Returns underlying value from enum value.
template <typename E>
[[nodiscard]] constexpr auto enum_underlying(E value) noexcept -> detail::enable_if_t<E, underlying_type_t<E>> {
  return static_cast<underlying_type_t<E>>(value);
}

// Obtains index in enum values from enum value.
// Returns optional with index.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_index(E value) noexcept -> detail::enable_if_t<E, optional<std::size_t>> {
  using D = std::decay_t<E>;
  using U = underlying_type_t<D>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");

  if constexpr (detail::is_sparse_v<D, S> || (S == detail::enum_subtype::flags)) {
#if defined(MAGIC_ENUM_ENABLE_HASH)
    return detail::constexpr_switch<&detail::values_v<D, S>, detail::case_call_t::index>(
        [](std::size_t i) { return optional<std::size_t>{i}; },
        value,
        detail::default_result_type_lambda<optional<std::size_t>>);
#else
    for (std::size_t i = 0; i < detail::count_v<D, S>; ++i) {
      if (enum_value<D, S>(i) == value) {
        return i;
      }
    }
    return {}; // Invalid value or out of range.
#endif
  } else {
    const auto v = static_cast<U>(value);
    if (v >= detail::min_v<D, S> && v <= detail::max_v<D, S>) {
      return static_cast<std::size_t>(v - detail::min_v<D, S>);
    }
    return {}; // Invalid value or out of range.
  }
}

// Obtains index in enum values from enum value.
// Returns optional with index.
template <detail::enum_subtype S, typename E>
[[nodiscard]] constexpr auto enum_index(E value) noexcept -> detail::enable_if_t<E, optional<std::size_t>> {
  using D = std::decay_t<E>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");

  return enum_index<D, S>(value);
}

// Obtains index in enum values from static storage enum variable.
template <auto V, detail::enum_subtype S = detail::subtype_v<std::decay_t<decltype(V)>>>
[[nodiscard]] constexpr auto enum_index() noexcept -> detail::enable_if_t<decltype(V), std::size_t> {\
  using D = std::decay_t<decltype(V)>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");
  constexpr auto index = enum_index<D, S>(V);
  static_assert(index, "magic_enum::enum_index enum value does not have a index.");

  return *index;
}

// Returns name from static storage enum variable.
// This version is much lighter on the compile times and is not restricted to the enum_range limitation.
template <auto V>
[[nodiscard]] constexpr auto enum_name() noexcept -> detail::enable_if_t<decltype(V), string_view> {
  constexpr string_view name = detail::enum_name_v<std::decay_t<decltype(V)>, V>;
  static_assert(!name.empty(), "magic_enum::enum_name enum value does not have a name.");

  return name;
}

// Returns name from enum value.
// If enum value does not have name or value out of range, returns empty string.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_name(E value) noexcept -> detail::enable_if_t<E, string_view> {
  using D = std::decay_t<E>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");


  if (const auto i = enum_index<D, S>(value)) {
    return detail::names_v<D, S>[*i];
  }
  return {};
}

// Returns name from enum value.
// If enum value does not have name or value out of range, returns empty string.
template <detail::enum_subtype S, typename E>
[[nodiscard]] constexpr auto enum_name(E value) -> detail::enable_if_t<E, string_view> {
  using D = std::decay_t<E>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");

  return enum_name<D, S>(value);
}

// Returns std::array with names, sorted by enum value.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_names() noexcept{
// [[nodiscard]] constexpr auto enum_names() noexcept -> detail::enable_if_t<E, detail::names_t<E, S>> {
  using D = std::decay_t<E>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");

  return detail::names_v<D, S>;
}

template <bool AsFlags = true>
inline constexpr auto as_flags = AsFlags ? detail::enum_subtype::flags : detail::enum_subtype::common;

template <bool AsFlags = true>
inline constexpr auto as_common = AsFlags ? detail::enum_subtype::common : detail::enum_subtype::flags;

namespace bitwise_operators {

template <typename E, detail::enable_if_t<E, int> = 0>
constexpr E operator~(E rhs) noexcept {
  return static_cast<E>(~static_cast<underlying_type_t<E>>(rhs));
}

template <typename E, detail::enable_if_t<E, int> = 0>
constexpr E operator|(E lhs, E rhs) noexcept {
  return static_cast<E>(static_cast<underlying_type_t<E>>(lhs) | static_cast<underlying_type_t<E>>(rhs));
}

template <typename E, detail::enable_if_t<E, int> = 0>
constexpr E operator&(E lhs, E rhs) noexcept {
  return static_cast<E>(static_cast<underlying_type_t<E>>(lhs) & static_cast<underlying_type_t<E>>(rhs));
}

template <typename E, detail::enable_if_t<E, int> = 0>
constexpr E operator^(E lhs, E rhs) noexcept {
  return static_cast<E>(static_cast<underlying_type_t<E>>(lhs) ^ static_cast<underlying_type_t<E>>(rhs));
}

template <typename E, detail::enable_if_t<E, int> = 0>
constexpr E& operator|=(E& lhs, E rhs) noexcept {
  return lhs = (lhs | rhs);
}

template <typename E, detail::enable_if_t<E, int> = 0>
constexpr E& operator&=(E& lhs, E rhs) noexcept {
  return lhs = (lhs & rhs);
}

template <typename E, detail::enable_if_t<E, int> = 0>
constexpr E& operator^=(E& lhs, E rhs) noexcept {
  return lhs = (lhs ^ rhs);
}

} // namespace magic_enum::bitwise_operators

} // namespace magic_enum

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif

#undef MAGIC_ENUM_GET_ENUM_NAME_BUILTIN
#undef MAGIC_ENUM_GET_TYPE_NAME_BUILTIN
#undef MAGIC_ENUM_VS_2017_WORKAROUND
#undef MAGIC_ENUM_ARRAY_CONSTEXPR
#undef MAGIC_ENUM_FOR_EACH_256

#endif // NEARGYE_MAGIC_ENUM_HPP

/*****************************************************************************************
	Author:xw  2025-8-16
******************************************************************************************/
// #define READ_ENUM_FROM_FILE
#define SAVE_ENUM
#ifndef READ_ENUM_FROM_FILE
#define ENUMSTR(format) std::string(magic_enum_simple::EnumName(format)).c_str()
namespace magic_enum_simple
{
  constexpr std::size_t kEnumMaxIndex = 32;
  constexpr std::size_t kEnumMinIndex = 0;

  constexpr auto EnumDefaultSize()
  {
    return kEnumMaxIndex - kEnumMinIndex;
  }

  template <auto V>
  constexpr auto sig()
  {
    return __PRETTY_FUNCTION__;
  }

  template <auto V>
  constexpr std::string_view EnumName()
  {
    std::string_view name = sig<V>();

    std::size_t end = 0;
    std::size_t start = 0;
    for (std::size_t i = name.size(); i > 0; --i)
    {
      if (end != 0 && name[i] == '=')
      {
        start = i + 2;
        break;
      }
      if (name[i] == ']')
      {
        end = i;
      }
    }

    //末尾不是字母说明这个不是枚举量
    char end_char = *(name.data() + end - 1);
    if ('0' <= end_char and end_char <= '9')
      return {};

    return std::string_view(name.data() + start, end - start);
  }

  template <class E>
  constexpr auto EnumNames()
  {
    constexpr auto sz = EnumDefaultSize();
    std::array<std::string_view, sz> arr;
    [&arr]<std::size_t... I>(std::index_sequence<I...>)
    {
      ((arr[I] = EnumName<static_cast<E>(I + kEnumMinIndex)>()), ...);
    }
    (std::make_index_sequence<sz>{});

    return arr;
  }

#ifdef SAVE_ENUM
  static int delete_older_file = 0;
  static std::vector<std::string> enum_typename;

  template <typename E>
  constexpr auto GetHeader(std::vector<std::string_view> &valid_names)
  {
    std::string_view header = __PRETTY_FUNCTION__;
    auto start = header.rfind('=') + 2;
    valid_names.push_back(std::string_view(header.data() + start, header.size() - start - 1));
  }

  template <typename E>
  constexpr auto GetEnumNames(const E v)
  {
    auto names = EnumNames<E>();
    std::vector<std::string_view> valid_names;
    GetHeader<E>(valid_names);

    for (auto name : names)
    {
      if (name.empty())
        continue;
      valid_names.push_back(name);
    }

    return valid_names;
  }

  template <typename E>
  void SaveEnumNamesYAML(E v)
  {
    auto names = GetEnumNames(v);
    std::string key_name(names[0]);

    std::vector<std::string> values(names.begin() + 1, names.end());
    std::string output_filename = "/home/devel/mstf/modules/master_pnc/pnc_interface/enumStrs.h"; // /userdata/LOGS/enumStrs.yaml

    if (delete_older_file == 0)
    {
      std::string cmd = "rm " + output_filename;
      int ret = system(cmd.c_str());
    }

    if (std::find(enum_typename.begin(), enum_typename.end(), key_name) != enum_typename.end())
    {
      return;
    }
    else
    {
      enum_typename.push_back(key_name);
    }

    std::ofstream file(output_filename, std::ios::app);
    std::string code = "std::string_view enum_to_string(" + key_name + " value) {\n";
    code += "  switch(value) {\n";
    for (int i = 0; i < values.size(); i++)
    {
      code += "    case " + std::to_string(i) + ": return \"" + values[i] + "\";\n";
    }
    code += "    default: return \"UNKNOWN\";\n  }\n}\n";
    file << code;
    file.close();

    delete_older_file++;
  }
#endif

  template <typename E>
  constexpr std::string_view EnumName(const E &v)
  {
#ifdef SAVE_ENUM
    SaveEnumNamesYAML(v);
#endif
    return EnumNames<E>()[static_cast<std::size_t>(v) - kEnumMinIndex];
  }
}
#endif

#ifdef READ_ENUM_FROM_FILE
#define ENUMSTR(format) std::string(EnumName(format)).c_str()
static std::map<std::string, std::vector<std::string>> enum_map;

template <typename E>
constexpr auto GetHeader(std::string_view &key_name)
{
  std::string_view header = __PRETTY_FUNCTION__;
  auto start = header.find('=') + 2;
  int end = start;
  for (size_t i = start; i < header.size(); i++)
  {
    if (header[i] == ';')
    {
      end = i;
      break;
    }
  }
  key_name = std::string_view(header.data() + start, end - start);
}

std::map<std::string, std::vector<std::string>> EnumStrsFromFile()
{
  std::string in_filename = "/home/devel/mstf/modules/master_pnc/pnc_interface/enumStrs.yaml";
  std::ifstream fs(in_filename);
  std::string line_str, key;
  std::map<std::string, std::vector<std::string>> enum_map;
  bool key_receive = false;

  while (getline(fs, line_str))
  {
    std::stringstream ss(line_str);
    key_receive = false;

    while (!ss.eof())
    {
      std::string x;
      ss >> x;
      if (!key_receive)
      {
        key_receive = true;
        key = x;
        continue;
      }
      enum_map[key].push_back(x);
    }
  }
  return enum_map;
}

template <typename E>
std::string EnumName(const E &v)
{
  std::string_view key_name;
  GetHeader<E>(key_name);
  if (enum_map.empty())
  {
    enum_map = EnumStrsFromFile();
  }

  if (enum_map.count(std::string(key_name)))
    return enum_map[std::string(key_name)][static_cast<std::size_t>(v)];
  else
    return {};
}
#endif //define read enum from file
