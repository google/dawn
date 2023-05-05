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

#ifndef SRC_TINT_UTILS_CASTABLE_H_
#define SRC_TINT_UTILS_CASTABLE_H_

#include <stdint.h>
#include <functional>
#include <tuple>
#include <utility>

#include "src/tint/utils/crc32.h"
#include "src/tint/utils/traits.h"

#if defined(__clang__)
/// Temporarily disable certain warnings when using Castable API
#define TINT_CASTABLE_PUSH_DISABLE_WARNINGS()                                 \
    _Pragma("clang diagnostic push")                                     /**/ \
        _Pragma("clang diagnostic ignored \"-Wundefined-var-template\"") /**/ \
        static_assert(true, "require extra semicolon")

/// Restore disabled warnings
#define TINT_CASTABLE_POP_DISABLE_WARNINGS() \
    _Pragma("clang diagnostic pop") /**/     \
        static_assert(true, "require extra semicolon")
#else
#define TINT_CASTABLE_PUSH_DISABLE_WARNINGS() static_assert(true, "require extra semicolon")
#define TINT_CASTABLE_POP_DISABLE_WARNINGS() static_assert(true, "require extra semicolon")
#endif

TINT_CASTABLE_PUSH_DISABLE_WARNINGS();

// Forward declarations
namespace tint::utils {
class CastableBase;

/// Ignore is used as a special type used for skipping over types for trait
/// helper functions.
class Ignore {};
}  // namespace tint::utils

namespace tint::utils::detail {
template <typename T>
struct TypeInfoOf;
}  // namespace tint::utils::detail

namespace tint::utils {

/// True if all template types that are not Ignore derive from CastableBase
template <typename... TYPES>
static constexpr bool IsCastable =
    ((utils::traits::IsTypeOrDerived<TYPES, CastableBase> || std::is_same_v<TYPES, Ignore>)&&...) &&
    !(std::is_same_v<TYPES, Ignore> && ...);

/// Helper macro to instantiate the TypeInfo<T> template for `CLASS`.
#define TINT_INSTANTIATE_TYPEINFO(CLASS)                                      \
    TINT_CASTABLE_PUSH_DISABLE_WARNINGS();                                    \
    template <>                                                               \
    const tint::utils::TypeInfo tint::utils::detail::TypeInfoOf<CLASS>::info{ \
        &tint::utils::detail::TypeInfoOf<CLASS::TrueBase>::info,              \
        #CLASS,                                                               \
        tint::utils::TypeInfo::HashCodeOf<CLASS>(),                           \
        tint::utils::TypeInfo::FullHashCodeOf<CLASS>(),                       \
    };                                                                        \
    TINT_CASTABLE_POP_DISABLE_WARNINGS()

/// Bit flags that can be passed to the template parameter `FLAGS` of Is() and As().
enum CastFlags {
    /// Disables the static_assert() inside Is(), that compile-time-verifies that the cast is
    /// possible. This flag may be useful for highly-generic template
    /// code that needs to compile for template permutations that generate
    /// impossible casts.
    kDontErrorOnImpossibleCast = 1,
};

/// The type of a hash code
using HashCode = uint64_t;

/// Maybe checks to see if an object with the full hashcode @p object_full_hashcode could
/// potentially be of, or derive from the type with the hashcode @p query_hashcode.
/// @param type_hashcode the hashcode of the type
/// @param object_full_hashcode the full hashcode of the object being queried
/// @returns true if the object with the given full hashcode could be one of the template types.
inline bool Maybe(HashCode type_hashcode, HashCode object_full_hashcode) {
    return (object_full_hashcode & type_hashcode) == type_hashcode;
}

/// MaybeAnyOf checks to see if an object with the full hashcode @p object_full_hashcode could
/// potentially be of, or derive from the types with the combined hashcode @p combined_hashcode.
/// @param combined_hashcode the bitwise OR'd hashcodes of the types
/// @param object_full_hashcode the full hashcode of the object being queried
/// @returns true if the object with the given full hashcode could be one of the template types.
inline bool MaybeAnyOf(HashCode combined_hashcode, HashCode object_full_hashcode) {
    // Compare the object's hashcode to the bitwise-or of all the tested type's hashcodes. If
    // there's no intersection of bits in the two masks, then we can guarantee that the type is not
    // in `TO`.
    HashCode mask = object_full_hashcode & combined_hashcode;
    // HashCodeOf() ensures that two bits are always set for every hash, so we can quickly
    // eliminate the bitmask where only one bit is set.
    HashCode two_bits = mask & (mask - 1);
    return two_bits != 0;
}

/// TypeInfo holds type information for a Castable type.
struct TypeInfo {
    /// The base class of this type
    const TypeInfo* base;
    /// The type name
    const char* name;
    /// The type hash code
    const HashCode hashcode;
    /// The type hash code bitwise-or'd with all ancestor's hashcodes.
    const HashCode full_hashcode;

    /// @returns true if `type` derives from the class `TO`
    /// @param object the object type to test from, which must be, or derive from type `FROM`.
    /// @see CastFlags
    template <typename TO, typename FROM, int FLAGS = 0>
    static inline bool Is(const tint::utils::TypeInfo* object) {
        constexpr const bool downcast = std::is_base_of<FROM, TO>::value;
        constexpr const bool upcast = std::is_base_of<TO, FROM>::value;
        constexpr const bool nocast = std::is_same<FROM, TO>::value;
        constexpr const bool assert_is_castable = (FLAGS & kDontErrorOnImpossibleCast) == 0;

        static_assert(upcast || downcast || nocast || !assert_is_castable, "impossible cast");

        return upcast || nocast || object->Is<TO>();
    }

    /// @returns true if this type derives from the class `T`
    template <typename T>
    inline bool Is() const {
        auto* type = &Of<std::remove_cv_t<T>>();

        if constexpr (std::is_final_v<T>) {
            // T is final, so nothing can derive from T.
            // We do not need to check ancestors, only whether this type is equal to the type T.
            return type == this;
        } else {
            return Is(type);
        }
    }

    /// @param type the test type info
    /// @returns true if the class with this TypeInfo is of, or derives from the
    /// class with the given TypeInfo.
    inline bool Is(const tint::utils::TypeInfo* type) const {
        if (!Maybe(type->hashcode, full_hashcode)) {
            return false;
        }

        // Walk the base types, starting with this TypeInfo, to see if any of the pointers match
        // `type`.
        for (auto* ti = this; ti != nullptr; ti = ti->base) {
            if (ti == type) {
                return true;
            }
        }
        return false;
    }

    /// @returns the static TypeInfo for the type T
    template <typename T>
    static const TypeInfo& Of() {
        return detail::TypeInfoOf<std::remove_cv_t<T>>::info;
    }

    /// @returns a compile-time hashcode for the type `T`.
    /// @note the returned hashcode will have exactly 2 bits set, as the hashes are expected to be
    /// used in bloom-filters which will quickly saturate when multiple hashcodes are bitwise-or'd
    /// together.
    template <typename T>
    static constexpr HashCode HashCodeOf() {
        static_assert(IsCastable<T>, "T is not Castable");
        static_assert(std::is_same_v<T, std::remove_cv_t<T>>,
                      "Strip const / volatile decorations before calling HashCodeOf");
        /// Use the compiler's "pretty" function name, which includes the template
        /// type, to obtain a unique hash value.
#ifdef _MSC_VER
        constexpr uint32_t crc = utils::CRC32(__FUNCSIG__);
#else
        constexpr uint32_t crc = utils::CRC32(__PRETTY_FUNCTION__);
#endif
        constexpr uint32_t bit_a = (crc & 63);
        constexpr uint32_t bit_b = ((crc >> 6) & 63);
        constexpr uint32_t bit_c = (bit_a == bit_b) ? ((bit_a + 1) & 63) : bit_b;
        return (static_cast<HashCode>(1) << bit_a) | (static_cast<HashCode>(1) << bit_c);
    }

    /// @returns the hashcode of the given type, bitwise-or'd with the hashcodes of all base
    /// classes.
    template <typename T>
    static constexpr HashCode FullHashCodeOf() {
        if constexpr (std::is_same_v<T, CastableBase>) {
            return HashCodeOf<CastableBase>();
        } else {
            return HashCodeOf<T>() | FullHashCodeOf<typename T::TrueBase>();
        }
    }

    /// @returns the bitwise-or'd hashcodes of all the types of the tuple `TUPLE`.
    /// @see HashCodeOf
    template <typename TUPLE>
    static constexpr HashCode CombinedHashCodeOfTuple() {
        constexpr auto kCount = std::tuple_size_v<TUPLE>;
        if constexpr (kCount == 0) {
            return 0;
        } else if constexpr (kCount == 1) {
            return HashCodeOf<std::remove_cv_t<std::tuple_element_t<0, TUPLE>>>();
        } else {
            constexpr auto kMid = kCount / 2;
            return CombinedHashCodeOfTuple<utils::traits::SliceTuple<0, kMid, TUPLE>>() |
                   CombinedHashCodeOfTuple<utils::traits::SliceTuple<kMid, kCount - kMid, TUPLE>>();
        }
    }

    /// @returns the bitwise-or'd hashcodes of all the template parameter types.
    /// @see HashCodeOf
    template <typename... TYPES>
    static constexpr HashCode CombinedHashCodeOf() {
        return CombinedHashCodeOfTuple<std::tuple<TYPES...>>();
    }

    /// @returns true if this TypeInfo is of, or derives from any of the types in `TUPLE`.
    template <typename TUPLE>
    inline bool IsAnyOfTuple() const {
        constexpr auto kCount = std::tuple_size_v<TUPLE>;
        if constexpr (kCount == 0) {
            return false;
        } else if constexpr (kCount == 1) {
            return Is(&Of<std::tuple_element_t<0, TUPLE>>());
        } else {
            if (MaybeAnyOf(TypeInfo::CombinedHashCodeOfTuple<TUPLE>(), full_hashcode)) {
                // Possibly one of the types in `TUPLE`.
                // Split the search in two, and scan each block.
                static constexpr auto kMid = kCount / 2;
                return IsAnyOfTuple<utils::traits::SliceTuple<0, kMid, TUPLE>>() ||
                       IsAnyOfTuple<utils::traits::SliceTuple<kMid, kCount - kMid, TUPLE>>();
            }
            return false;
        }
    }

    /// @returns true if this TypeInfo is of, or derives from any of the types in `TYPES`.
    template <typename... TYPES>
    inline bool IsAnyOf() const {
        return IsAnyOfTuple<std::tuple<TYPES...>>();
    }
};

namespace detail {

/// TypeInfoOf contains a single TypeInfo field for the type T.
/// TINT_INSTANTIATE_TYPEINFO() must be defined in a .cpp file for each type `T`.
template <typename T>
struct TypeInfoOf {
    /// The unique TypeInfo for the type T.
    static const TypeInfo info;
};

/// A placeholder structure used for template parameters that need a default type, but can always be
/// automatically inferred.
struct Infer;

}  // namespace detail

/// @returns true if `obj` is a valid pointer, and is of, or derives from the class `TO`
/// @param obj the object to test from
/// @see CastFlags
template <typename TO, int FLAGS = 0, typename FROM = detail::Infer>
inline bool Is(FROM* obj) {
    if (obj == nullptr) {
        return false;
    }
    return TypeInfo::Is<TO, FROM, FLAGS>(&obj->TypeInfo());
}

/// @returns true if `obj` is a valid pointer, and is of, or derives from the type `TYPE`, and
/// pred(const TYPE*) returns true
/// @param obj the object to test from
/// @param pred predicate function with signature `bool(const TYPE*)` called iff object is of, or
/// derives from the class `TYPE`.
/// @see CastFlags
template <typename TYPE, int FLAGS = 0, typename OBJ = detail::Infer, typename Pred = detail::Infer>
inline bool Is(OBJ* obj, Pred&& pred) {
    return Is<TYPE, FLAGS, OBJ>(obj) && pred(static_cast<std::add_const_t<TYPE>*>(obj));
}

/// @returns true if `obj` is a valid pointer, and is of, or derives from any of the types in
/// `TYPES`.
/// @param obj the object to query.
template <typename... TYPES, typename OBJ>
inline bool IsAnyOf(OBJ* obj) {
    if (!obj) {
        return false;
    }
    return obj->TypeInfo().template IsAnyOf<TYPES...>();
}

/// @returns obj dynamically cast to the type `TO` or `nullptr` if this object does not derive from
/// `TO`.
/// @param obj the object to cast from
/// @see CastFlags
template <typename TO, int FLAGS = 0, typename FROM = detail::Infer>
inline TO* As(FROM* obj) {
    auto* as_castable = static_cast<CastableBase*>(obj);
    return Is<TO, FLAGS>(obj) ? static_cast<TO*>(as_castable) : nullptr;
}

/// @returns obj dynamically cast to the type `TO` or `nullptr` if this object does not derive from
/// `TO`.
/// @param obj the object to cast from
/// @see CastFlags
template <typename TO, int FLAGS = 0, typename FROM = detail::Infer>
inline const TO* As(const FROM* obj) {
    auto* as_castable = static_cast<const CastableBase*>(obj);
    return Is<TO, FLAGS>(obj) ? static_cast<const TO*>(as_castable) : nullptr;
}

/// CastableBase is the base class for all Castable objects.
/// It is not encouraged to directly derive from CastableBase without using the Castable helper
/// template.
/// @see Castable
class CastableBase {
  public:
    /// Copy constructor
    CastableBase(const CastableBase&);

    /// Destructor
    virtual ~CastableBase();

    /// Copy assignment
    /// @param other the CastableBase to copy
    /// @returns the new CastableBase
    CastableBase& operator=(const CastableBase& other) = default;

    /// @returns the TypeInfo of the object
    inline const tint::utils::TypeInfo& TypeInfo() const { return *type_info_; }

    /// @returns true if this object is of, or derives from the class `TO`
    template <typename TO>
    inline bool Is() const {
        return tint::utils::Is<TO>(this);
    }

    /// @returns true if this object is of, or derives from the class `TO` and pred(const TO*)
    /// returns true
    /// @param pred predicate function with signature `bool(const TO*)` called iff object is of, or
    /// derives from the class `TO`.
    template <typename TO, int FLAGS = 0, typename Pred = detail::Infer>
    inline bool Is(Pred&& pred) const {
        return tint::utils::Is<TO, FLAGS>(this, std::forward<Pred>(pred));
    }

    /// @returns true if this object is of, or derives from any of the `TO` classes.
    template <typename... TO>
    inline bool IsAnyOf() const {
        return tint::utils::IsAnyOf<TO...>(this);
    }

    /// @returns this object dynamically cast to the type `TO` or `nullptr` if this object does not
    /// derive from `TO`.
    /// @see CastFlags
    template <typename TO, int FLAGS = 0>
    inline TO* As() {
        return tint::utils::As<TO, FLAGS>(this);
    }

    /// @returns this object dynamically cast to the type `TO` or `nullptr` if this object does not
    /// derive from `TO`.
    /// @see CastFlags
    template <typename TO, int FLAGS = 0>
    inline const TO* As() const {
        return tint::utils::As<const TO, FLAGS>(this);
    }

  protected:
    CastableBase() = default;

    /// The type information for the object
    const tint::utils::TypeInfo* type_info_ = nullptr;
};

/// Castable is a helper to derive `CLASS` from `BASE`, automatically implementing the Is() and As()
/// methods, along with a #Base type alias.
///
/// Example usage:
///
/// ```
/// class Animal : public Castable<Animal> {};
///
/// class Sheep : public Castable<Sheep, Animal> {};
///
/// Sheep* cast_to_sheep(Animal* animal) {
///    // You can query whether a Castable is of the given type with Is<T>():
///    printf("animal is a sheep? %s", animal->Is<Sheep>() ? "yes" : "no");
///
///    // You can always just try the cast with As<T>().
///    // If the object is not of the correct type, As<T>() will return nullptr:
///    return animal->As<Sheep>();
/// }
/// ```
template <typename CLASS, typename BASE = CastableBase>
class Castable : public BASE {
  public:
    /// A type alias for `CLASS` to easily access the `BASE` class members.
    /// Base actually aliases to the Castable instead of `BASE` so that you can
    /// use Base in the `CLASS` constructor.
    using Base = Castable;

    /// A type alias for `BASE`.
    using TrueBase = BASE;

    /// Constructor
    /// @param arguments the arguments to forward to the base class.
    template <typename... ARGS>
    inline explicit Castable(ARGS&&... arguments) : TrueBase(std::forward<ARGS>(arguments)...) {
        this->type_info_ = &TypeInfo::Of<CLASS>();
    }

    /// @returns true if this object is of, or derives from the class `TO`
    /// @see CastFlags
    template <typename TO, int FLAGS = 0>
    inline bool Is() const {
        return tint::utils::Is<TO, FLAGS>(static_cast<const CLASS*>(this));
    }

    /// @returns true if this object is of, or derives from the class `TO` and
    /// pred(const TO*) returns true
    /// @param pred predicate function with signature `bool(const TO*)` called iff
    /// object is of, or derives from the class `TO`.
    template <int FLAGS = 0, typename Pred = detail::Infer>
    inline bool Is(Pred&& pred) const {
        using TO = typename std::remove_pointer<utils::traits::ParameterType<Pred, 0>>::type;
        return tint::utils::Is<TO, FLAGS>(static_cast<const CLASS*>(this),
                                          std::forward<Pred>(pred));
    }

    /// @returns true if this object is of, or derives from any of the `TO`
    /// classes.
    template <typename... TO>
    inline bool IsAnyOf() const {
        return tint::utils::IsAnyOf<TO...>(static_cast<const CLASS*>(this));
    }

    /// @returns this object dynamically cast to the type `TO` or `nullptr` if
    /// this object does not derive from `TO`.
    /// @see CastFlags
    template <typename TO, int FLAGS = 0>
    inline TO* As() {
        return tint::utils::As<TO, FLAGS>(this);
    }

    /// @returns this object dynamically cast to the type `TO` or `nullptr` if
    /// this object does not derive from `TO`.
    /// @see CastFlags
    template <typename TO, int FLAGS = 0>
    inline const TO* As() const {
        return tint::utils::As<const TO, FLAGS>(this);
    }
};

namespace detail {
/// <code>typename CastableCommonBaseImpl<TYPES>::type</code> resolves to the common base class for
/// all of TYPES.
template <typename... TYPES>
struct CastableCommonBaseImpl {};

/// Alias to typename CastableCommonBaseImpl<TYPES>::type
template <typename... TYPES>
using CastableCommonBase = typename CastableCommonBaseImpl<TYPES...>::type;

/// CastableCommonBaseImpl template specialization for a single type
template <typename T>
struct CastableCommonBaseImpl<T> {
    /// Common base class of a single type is itself
    using type = T;
};

/// CastableCommonBaseImpl A <-> CastableBase specialization
template <typename A>
struct CastableCommonBaseImpl<A, CastableBase> {
    /// Common base class for A and CastableBase is CastableBase
    using type = CastableBase;
};

/// CastableCommonBaseImpl T <-> Ignore specialization
template <typename T>
struct CastableCommonBaseImpl<T, Ignore> {
    /// Resolves to T as the other type is ignored
    using type = T;
};

/// CastableCommonBaseImpl Ignore <-> T specialization
template <typename T>
struct CastableCommonBaseImpl<Ignore, T> {
    /// Resolves to T as the other type is ignored
    using type = T;
};

/// CastableCommonBaseImpl A <-> B specialization
template <typename A, typename B>
struct CastableCommonBaseImpl<A, B> {
    /// The common base class for A, B and OTHERS
    using type = std::conditional_t<utils::traits::IsTypeOrDerived<A, B>,
                                    B,  // A derives from B
                                    CastableCommonBase<A, typename B::TrueBase>>;
};

/// CastableCommonBaseImpl 3+ types specialization
template <typename A, typename B, typename... OTHERS>
struct CastableCommonBaseImpl<A, B, OTHERS...> {
    /// The common base class for A, B and OTHERS
    using type = CastableCommonBase<CastableCommonBase<A, B>, OTHERS...>;
};

}  // namespace detail

/// Resolves to the common most derived type that each of the types in `TYPES` derives from.
template <typename... TYPES>
using CastableCommonBase = detail::CastableCommonBase<TYPES...>;

}  // namespace tint::utils

namespace tint {

using utils::As;
using utils::Is;

}  // namespace tint

TINT_CASTABLE_POP_DISABLE_WARNINGS();

#endif  // SRC_TINT_UTILS_CASTABLE_H_
