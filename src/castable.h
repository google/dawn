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

class ClassID;

/// Helper macro to instantiate the TypeInfo<T> template for `CLASS`.
#define TINT_INSTANTIATE_CLASS_ID(CLASS)             \
  template <>                                        \
  const char tint::UniqueToken<CLASS>::token = 0;    \
  template <>                                        \
  const uintptr_t tint::TypeInfo<CLASS>::unique_id = \
      reinterpret_cast<uintptr_t>(&tint::UniqueToken<CLASS>::token)

/// TypeInfo holds type information for the type T.
/// TINT_INSTANTIATE_CLASS_ID() must be defined in a .cpp file for each type
/// `T`.
template <typename T>
struct TypeInfo {
  /// The unique identifier for the type T.
  static const uintptr_t unique_id;
};

/// UniqueToken holds a single static const char, which is uniquely declared for
/// each specialization of the template class.
/// Use by TINT_INSTANTIATE_CLASS_ID() to generate a unique pointer, which is in
/// turn used to generate TypeInfo<T>::unique_id.
template <typename T>
struct UniqueToken {
  /// A dummy static variable that is unique for the type T.
  static const char token;
};

/// ClassID represents a unique, comparable identifier for a C++ type.
class ClassID {
 public:
  /// @returns the unique ClassID for the type T.
  template <typename T>
  static inline ClassID Of() {
    return ClassID(TypeInfo<T>::unique_id);
  }

  /// Equality operator
  /// @param rhs the ClassID to compare against
  /// @returns true if this ClassID is equal to `rhs`
  inline bool operator==(const ClassID& rhs) const { return id == rhs.id; }

  /// @return the unique numerical identifier of this ClassID
  inline uintptr_t ID() const { return id; }

 private:
  inline explicit ClassID(uintptr_t v) : id(v) {}

  const uintptr_t id;
};

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

  /// @returns true if this object is of, or derives from a class with the
  /// ClassID `id`.
  /// @param id the ClassID to test for
  virtual bool Is(ClassID id) const;

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

    return Is(ClassID::Of<TO>());
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

  /// @returns true if this object is of, or derives from a class with the
  /// ClassID `id`.
  /// @param id the ClassID to test for
  inline bool Is(ClassID id) const override {
    return ClassID::Of<CLASS>() == id || BASE::Is(id);
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

    return Is(ClassID::Of<TO>());
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

namespace std {

/// Custom std::hash specialization for tint::ClassID so ClassID can be used as
/// keys for std::unordered_map and std::unordered_set.
template <>
class hash<tint::ClassID> {
 public:
  /// @param id the ClassID to hash
  /// @return the ClassID's internal numerical identifier
  inline std::size_t operator()(const tint::ClassID& id) const {
    return static_cast<std::size_t>(id.ID());
  }
};

}  // namespace std

#endif  // SRC_CASTABLE_H_
