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

#ifndef SRC_CASTABLE_H_
#define SRC_CASTABLE_H_

#include <utility>

#if defined(__clang__)
/// Temporarily disable certain warnings when using Castable API
#define TINT_CASTABLE_PUSH_DISABLE_WARNINGS()                               \
  _Pragma("clang diagnostic push")                                     /**/ \
      _Pragma("clang diagnostic ignored \"-Wundefined-var-template\"") /**/ \
      static_assert(true, "require extra semicolon")

/// Restore disabled warnings
#define TINT_CASTABLE_POP_DISABLE_WARNINGS() \
  _Pragma("clang diagnostic pop") /**/       \
      static_assert(true, "require extra semicolon")
#else
#define TINT_CASTABLE_PUSH_DISABLE_WARNINGS() \
  static_assert(true, "require extra semicolon")
#define TINT_CASTABLE_POP_DISABLE_WARNINGS() \
  static_assert(true, "require extra semicolon")
#endif

TINT_CASTABLE_PUSH_DISABLE_WARNINGS();

namespace tint {

namespace detail {
template <typename T>
struct TypeInfoOf;
}  // namespace detail

/// Helper macro to instantiate the TypeInfo<T> template for `CLASS`.
#define TINT_INSTANTIATE_TYPEINFO(CLASS)                      \
  TINT_CASTABLE_PUSH_DISABLE_WARNINGS();                      \
  template <>                                                 \
  const tint::TypeInfo tint::detail::TypeInfoOf<CLASS>::info{ \
      &tint::detail::TypeInfoOf<CLASS::TrueBase>::info,       \
      #CLASS,                                                 \
  };                                                          \
  TINT_CASTABLE_POP_DISABLE_WARNINGS()

/// TypeInfo holds type information for a Castable type.
struct TypeInfo {
  /// The base class of this type.
  const TypeInfo* base;
  /// The type name
  const char* name;

  /// @param type the test type info
  /// @returns true if the class with this TypeInfo is of, or derives from the
  /// class with the given TypeInfo.
  bool Is(const tint::TypeInfo& type) const;

  /// @returns the static TypeInfo for the type T
  template <typename T>
  static const TypeInfo& Of() {
    return detail::TypeInfoOf<T>::info;
  }
};

namespace detail {

/// TypeInfoOf contains a single TypeInfo field for the type T.
/// TINT_INSTANTIATE_TYPEINFO() must be defined in a .cpp file for each type
/// `T`.
template <typename T>
struct TypeInfoOf {
  /// The unique TypeInfo for the type T.
  static const TypeInfo info;
};

// Forward declaration
template <typename TO_FIRST, typename... TO_REST>
struct IsAnyOf;

}  // namespace detail

/// @returns true if `obj` is a valid pointer, and is of, or derives from the
/// class `TO`
/// @param obj the object to test from
template <typename TO, typename FROM>
inline bool Is(FROM* obj) {
  constexpr const bool downcast = std::is_base_of<FROM, TO>::value;
  constexpr const bool upcast = std::is_base_of<TO, FROM>::value;
  constexpr const bool nocast = std::is_same<FROM, TO>::value;
  static_assert(upcast || downcast || nocast, "impossible cast");

  if (obj == nullptr) {
    return false;
  }

  if (upcast || nocast) {
    return true;
  }

  return obj->TypeInfo().Is(TypeInfo::Of<std::remove_const_t<TO>>());
}

/// @returns true if `obj` is a valid pointer, and is of, or derives from the
/// class `TO`, and pred(const TO*) returns true
/// @param obj the object to test from
/// @param pred predicate function with signature `bool(const TO*)` called iff
/// object is of, or derives from the class `TO`.
template <typename TO, typename FROM, typename Pred>
inline bool Is(FROM* obj, Pred&& pred) {
  constexpr const bool downcast = std::is_base_of<FROM, TO>::value;
  constexpr const bool upcast = std::is_base_of<TO, FROM>::value;
  constexpr const bool nocast = std::is_same<FROM, TO>::value;
  static_assert(upcast || downcast || nocast, "impossible cast");

  if (obj == nullptr) {
    return false;
  }

  bool is_type = upcast || nocast ||
                 obj->TypeInfo().Is(TypeInfo::Of<std::remove_const_t<TO>>());

  return is_type && pred(static_cast<std::add_const_t<TO>*>(obj));
}

/// @returns true if `obj` is of, or derives from any of the `TO`
/// classes.
/// @param obj the object to cast from
template <typename... TO, typename FROM>
inline bool IsAnyOf(FROM* obj) {
  return detail::IsAnyOf<TO...>::Exec(obj);
}

/// @returns obj dynamically cast to the type `TO` or `nullptr` if
/// this object does not derive from `TO`.
/// @param obj the object to cast from
template <typename TO, typename FROM>
inline TO* As(FROM* obj) {
  return Is<TO>(obj) ? static_cast<TO*>(obj) : nullptr;
}

/// CastableBase is the base class for all Castable objects.
/// It is not encouraged to directly derive from CastableBase without using the
/// Castable helper template.
/// @see Castable
class CastableBase {
 public:
  /// Copy constructor
  CastableBase(const CastableBase&) = default;

  virtual ~CastableBase() = default;

  /// @returns the TypeInfo of the object
  virtual const tint::TypeInfo& TypeInfo() const = 0;

  /// @returns true if this object is of, or derives from the class `TO`
  template <typename TO>
  inline bool Is() const {
    return tint::Is<TO>(this);
  }

  /// @returns true if this object is of, or derives from the class `TO` and
  /// pred(const TO*) returns true
  /// @param pred predicate function with signature `bool(const TO*)` called iff
  /// object is of, or derives from the class `TO`.
  template <typename TO, typename Pred>
  inline bool Is(Pred&& pred) const {
    return tint::Is<TO>(this, std::forward<Pred>(pred));
  }

  /// @returns true if this object is of, or derives from any of the `TO`
  /// classes.
  template <typename... TO>
  inline bool IsAnyOf() const {
    return tint::IsAnyOf<TO...>(this);
  }

  /// @returns this object dynamically cast to the type `TO` or `nullptr` if
  /// this object does not derive from `TO`.
  template <typename TO>
  inline TO* As() {
    return tint::As<TO>(this);
  }

  /// @returns this object dynamically cast to the type `TO` or `nullptr` if
  /// this object does not derive from `TO`.
  template <typename TO>
  inline const TO* As() const {
    return tint::As<const TO>(this);
  }

 protected:
  CastableBase() = default;
};

/// Castable is a helper to derive `CLASS` from `BASE`, automatically
/// implementing the Is() and As() methods, along with a #Base type alias.
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
  // Inherit the `BASE` class constructors.
  using BASE::BASE;

  /// A type alias for `CLASS` to easily access the `BASE` class members.
  /// Base actually aliases to the Castable instead of `BASE` so that you can
  /// use Base in the `CLASS` constructor.
  using Base = Castable;

  /// A type alias for `BASE`.
  using TrueBase = BASE;

  /// @returns the TypeInfo of the object
  const tint::TypeInfo& TypeInfo() const override {
    return TypeInfo::Of<CLASS>();
  }

  /// @returns true if this object is of, or derives from the class `TO`
  template <typename TO>
  inline bool Is() const {
    return tint::Is<TO>(static_cast<const CLASS*>(this));
  }

  /// @returns true if this object is of, or derives from the class `TO` and
  /// pred(const TO*) returns true
  /// @param pred predicate function with signature `bool(const TO*)` called iff
  /// object is of, or derives from the class `TO`.
  template <typename TO, typename Pred>
  inline bool Is(Pred&& pred) const {
    return tint::Is<TO>(static_cast<const CLASS*>(this),
                        std::forward<Pred>(pred));
  }

  /// @returns true if this object is of, or derives from any of the `TO`
  /// classes.
  template <typename... TO>
  inline bool IsAnyOf() const {
    return tint::IsAnyOf<TO...>(static_cast<const CLASS*>(this));
  }

  /// @returns this object dynamically cast to the type `TO` or `nullptr` if
  /// this object does not derive from `TO`.
  template <typename TO>
  inline TO* As() {
    return tint::As<TO>(this);
  }

  /// @returns this object dynamically cast to the type `TO` or `nullptr` if
  /// this object does not derive from `TO`.
  template <typename TO>
  inline const TO* As() const {
    return tint::As<const TO>(this);
  }
};

namespace detail {
/// Helper for Castable::IsAnyOf
template <typename TO_FIRST, typename... TO_REST>
struct IsAnyOf {
  /// @param obj castable object to test
  /// @returns true if `obj` is of, or derives from any of `[TO_FIRST,
  /// ...TO_REST]`
  template <typename FROM>
  static bool Exec(FROM* obj) {
    return Is<TO_FIRST>(obj) || IsAnyOf<TO_REST...>::Exec(obj);
  }
};
/// Terminal specialization
template <typename TO>
struct IsAnyOf<TO> {
  /// @param obj castable object to test
  /// @returns true if `obj` is of, or derives from TO
  template <typename FROM>
  static bool Exec(FROM* obj) {
    return Is<TO>(obj);
  }
};
}  // namespace detail

}  // namespace tint

TINT_CASTABLE_POP_DISABLE_WARNINGS();

#endif  // SRC_CASTABLE_H_
