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

#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>

namespace tint {

namespace detail {
template <typename T>
struct TypeInfoOf;
}  // namespace detail

/// Helper macro to instantiate the TypeInfo<T> template for `CLASS`.
#define TINT_INSTANTIATE_CLASS_ID(CLASS)                       \
  template <>                                                  \
  const tint::TypeInfo tint::detail::TypeInfoOf<CLASS>::info { \
    &tint::detail::TypeInfoOf<CLASS::TrueBase>::info, #CLASS,  \
  }

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
/// TINT_INSTANTIATE_CLASS_ID() must be defined in a .cpp file for each type
/// `T`.
template <typename T>
struct TypeInfoOf {
  /// The unique TypeInfo for the type T.
  static const TypeInfo info;
};

}  // namespace detail

/// CastableBase is the base class for all Castable objects.
/// It is not encouraged to directly derive from CastableBase without using the
/// Castable helper template.
/// @see Castable
class CastableBase {
 public:
  /// Copy constructor
  CastableBase(const CastableBase&) = default;

  /// Move constructor
  CastableBase(CastableBase&&) = default;

  virtual ~CastableBase() = default;

  /// @returns the TypeInfo of the object
  virtual const tint::TypeInfo& TypeInfo() const = 0;

  /// @returns true if this object is of, or derives from the class `TO`
  template <typename TO>
  inline bool Is() const {
    using FROM = CastableBase;
    constexpr const bool downcast = std::is_base_of<FROM, TO>::value;
    constexpr const bool upcast = std::is_base_of<TO, FROM>::value;
    constexpr const bool nocast = std::is_same<FROM, TO>::value;
    static_assert(upcast || downcast || nocast, "impossible cast");

    if (upcast || nocast) {
      return true;
    }

    return TypeInfo().Is(TypeInfo::Of<TO>());
  }

  /// @returns this object dynamically cast to the type `TO` or `nullptr` if
  /// this object does not derive from `TO`.
  template <typename TO>
  inline TO* As() {
    return Is<TO>() ? static_cast<TO*>(this) : nullptr;
  }

  /// @returns this object dynamically cast to the type `TO` or `nullptr` if
  /// this object does not derive from `TO`.
  template <typename TO>
  inline const TO* As() const {
    return Is<TO>() ? static_cast<const TO*>(this) : nullptr;
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
    using FROM = Castable;
    constexpr const bool downcast = std::is_base_of<FROM, TO>::value;
    constexpr const bool upcast = std::is_base_of<TO, FROM>::value;
    constexpr const bool nocast = std::is_same<FROM, TO>::value;
    static_assert(upcast || downcast || nocast, "impossible cast");

    if (upcast || nocast) {
      return true;
    }

    return TypeInfo().Is(TypeInfo::Of<TO>());
  }

  /// @returns this object dynamically cast to the type `TO` or `nullptr` if
  /// this object does not derive from `TO`.
  template <typename TO>
  inline TO* As() {
    return Is<TO>() ? static_cast<TO*>(this) : nullptr;
  }

  /// @returns this object dynamically cast to the type `TO` or `nullptr` if
  /// this object does not derive from `TO`.
  template <typename TO>
  inline const TO* As() const {
    return Is<TO>() ? static_cast<const TO*>(this) : nullptr;
  }
};

/// As() dynamically casts `obj` to the target type `TO`.
/// @returns the cast object, or nullptr if `obj` is `nullptr` or not of the
/// type `TO`.
/// @param obj the object to cast
template <typename TO, typename FROM>
inline TO* As(FROM* obj) {
  if (obj == nullptr) {
    return nullptr;
  }
  return obj->template As<TO>();
}

}  // namespace tint

#endif  // SRC_CASTABLE_H_
