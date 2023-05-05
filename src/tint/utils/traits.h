// Copyright 2020 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_TINT_UTILS_TRAITS_H_
#define SRC_TINT_UTILS_TRAITS_H_

#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

namespace tint::utils::traits {

/// Convience type definition for std::decay<T>::type
template <typename T>
using Decay = typename std::decay<T>::type;

/// NthTypeOf returns the `N`th type in `Types`
template <int N, typename... Types>
using NthTypeOf = typename std::tuple_element<N, std::tuple<Types...>>::type;

/// Signature describes the signature of a function.
template <typename RETURN, typename... PARAMETERS>
struct Signature {
    /// The return type of the function signature
    using ret = RETURN;
    /// The parameters of the function signature held in a std::tuple
    using parameters = std::tuple<PARAMETERS...>;
    /// The type of the Nth parameter of function signature
    template <std::size_t N>
    using parameter = NthTypeOf<N, PARAMETERS...>;
    /// The total number of parameters
    static constexpr std::size_t parameter_count = sizeof...(PARAMETERS);
};

/// SignatureOf is a traits helper that infers the signature of the function,
/// method, static method, lambda, or function-like object `F`.
template <typename F>
struct SignatureOf {
    /// The signature of the function-like object `F`
    using type = typename SignatureOf<decltype(&F::operator())>::type;
};

/// SignatureOf specialization for a regular function or static method.
template <typename R, typename... ARGS>
struct SignatureOf<R (*)(ARGS...)> {
    /// The signature of the function-like object `F`
    using type = Signature<typename std::decay<R>::type, typename std::decay<ARGS>::type...>;
};

/// SignatureOf specialization for a non-static method.
template <typename R, typename C, typename... ARGS>
struct SignatureOf<R (C::*)(ARGS...)> {
    /// The signature of the function-like object `F`
    using type = Signature<typename std::decay<R>::type, typename std::decay<ARGS>::type...>;
};

/// SignatureOf specialization for a non-static, const method.
template <typename R, typename C, typename... ARGS>
struct SignatureOf<R (C::*)(ARGS...) const> {
    /// The signature of the function-like object `F`
    using type = Signature<typename std::decay<R>::type, typename std::decay<ARGS>::type...>;
};

/// SignatureOfT is an alias to `typename SignatureOf<F>::type`.
template <typename F>
using SignatureOfT = typename SignatureOf<Decay<F>>::type;

/// ParameterType is an alias to `typename SignatureOf<F>::type::parameter<N>`.
template <typename F, std::size_t N>
using ParameterType = typename SignatureOfT<Decay<F>>::template parameter<N>;

/// LastParameterType returns the type of the last parameter of `F`. `F` must have at least one
/// parameter.
template <typename F>
using LastParameterType = ParameterType<F, SignatureOfT<Decay<F>>::parameter_count - 1>;

/// ReturnType is an alias to `typename SignatureOf<F>::type::ret`.
template <typename F>
using ReturnType = typename SignatureOfT<Decay<F>>::ret;

/// Returns true iff decayed T and decayed U are the same.
template <typename T, typename U>
static constexpr bool IsType = std::is_same<Decay<T>, Decay<U>>::value;

/// IsTypeOrDerived<T, BASE> is true iff `T` is of type `BASE`, or derives from
/// `BASE`.
template <typename T, typename BASE>
static constexpr bool IsTypeOrDerived =
    std::is_base_of<BASE, Decay<T>>::value || std::is_same<BASE, Decay<T>>::value;

/// If `CONDITION` is true then EnableIf resolves to type T, otherwise an
/// invalid type.
template <bool CONDITION, typename T = void>
using EnableIf = typename std::enable_if<CONDITION, T>::type;

/// If `T` is of type `BASE`, or derives from `BASE`, then EnableIfIsType
/// resolves to type `T`, otherwise an invalid type.
template <typename T, typename BASE>
using EnableIfIsType = EnableIf<IsTypeOrDerived<T, BASE>, T>;

/// @returns the std::index_sequence with all the indices shifted by OFFSET.
template <std::size_t OFFSET, std::size_t... INDICES>
constexpr auto Shift(std::index_sequence<INDICES...>) {
    return std::integer_sequence<std::size_t, OFFSET + INDICES...>{};
}

/// @returns a std::integer_sequence with the integers `[OFFSET..OFFSET+COUNT)`
template <std::size_t OFFSET, std::size_t COUNT>
constexpr auto Range() {
    return Shift<OFFSET>(std::make_index_sequence<COUNT>{});
}

namespace detail {

/// @returns the tuple `t` swizzled by `INDICES`
template <typename TUPLE, std::size_t... INDICES>
constexpr auto Swizzle(TUPLE&& t, std::index_sequence<INDICES...>)
    -> std::tuple<std::tuple_element_t<INDICES, std::remove_reference_t<TUPLE>>...> {
    return {std::forward<std::tuple_element_t<INDICES, std::remove_reference_t<TUPLE>>>(
        std::get<INDICES>(std::forward<TUPLE>(t)))...};
}

/// @returns a nullptr of the tuple type `TUPLE` swizzled by `INDICES`.
/// @note: This function is intended to be used in a `decltype()` expression,
/// and returns a pointer-to-tuple as the tuple may hold non-constructable
/// types.
template <typename TUPLE, std::size_t... INDICES>
constexpr auto* SwizzlePtrTy(std::index_sequence<INDICES...>) {
    using Swizzled = std::tuple<std::tuple_element_t<INDICES, TUPLE>...>;
    return static_cast<Swizzled*>(nullptr);
}

}  // namespace detail

/// @returns the slice of the tuple `t` with the tuple elements
/// `[OFFSET..OFFSET+COUNT)`
template <std::size_t OFFSET, std::size_t COUNT, typename TUPLE>
constexpr auto Slice(TUPLE&& t) {
    return detail::Swizzle<TUPLE>(std::forward<TUPLE>(t), Range<OFFSET, COUNT>());
}

/// Resolves to the slice of the tuple `t` with the tuple elements
/// `[OFFSET..OFFSET+COUNT)`
template <std::size_t OFFSET, std::size_t COUNT, typename TUPLE>
using SliceTuple =
    std::remove_pointer_t<decltype(detail::SwizzlePtrTy<TUPLE>(Range<OFFSET, COUNT>()))>;

namespace detail {
/// Base template for IsTypeIn
template <typename T, typename TypeList>
struct IsTypeIn;

/// Specialization for IsTypeIn
template <typename T, template <typename...> typename TypeContainer, typename... Ts>
struct IsTypeIn<T, TypeContainer<Ts...>> : std::disjunction<std::is_same<T, Ts>...> {};
}  // namespace detail

/// Evaluates to true if T is one of the types in the TypeContainer's template arguments.
/// Works for std::variant, std::tuple, std::pair, or any typename template where all parameters are
/// types.
template <typename T, typename TypeContainer>
static constexpr bool IsTypeIn = detail::IsTypeIn<T, TypeContainer>::value;

/// Evaluates to the decayed pointer element type, or the decayed type T if T is not a pointer.
template <typename T>
using PtrElTy = Decay<std::remove_pointer_t<Decay<T>>>;

/// Evaluates to true if `T` decayed is a `std::string`, `std::string_view` or `const char*`
template <typename T>
static constexpr bool IsStringLike =
    std::is_same_v<Decay<T>, std::string> || std::is_same_v<Decay<T>, std::string_view> ||
    std::is_same_v<Decay<T>, const char*>;

namespace detail {
/// Helper for CharArrayToCharPtr
template <typename T>
struct CharArrayToCharPtrImpl {
    /// Evaluates to T
    using type = T;
};
/// Specialization of CharArrayToCharPtrImpl for `char[N]`
template <size_t N>
struct CharArrayToCharPtrImpl<char[N]> {
    /// Evaluates to `char*`
    using type = char*;
};
/// Specialization of CharArrayToCharPtrImpl for `const char[N]`
template <size_t N>
struct CharArrayToCharPtrImpl<const char[N]> {
    /// Evaluates to `const char*`
    using type = const char*;
};
}  // namespace detail

/// Evaluates to `char*` or `const char*` if `T` is `char[N]` or `const char[N]`, respectively,
/// otherwise T.
template <typename T>
using CharArrayToCharPtr = typename detail::CharArrayToCharPtrImpl<T>::type;

}  // namespace tint::utils::traits

#endif  // SRC_TINT_UTILS_TRAITS_H_
