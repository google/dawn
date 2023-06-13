// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_TYPE_MANAGER_H_
#define SRC_TINT_TYPE_MANAGER_H_

#include <utility>

#include "src/tint/builtin/access.h"
#include "src/tint/builtin/address_space.h"
#include "src/tint/number.h"
#include "src/tint/type/type.h"
#include "src/tint/type/unique_node.h"
#include "src/tint/utils/hash.h"
#include "src/tint/utils/unique_allocator.h"

// Forward declarations
namespace tint::type {
class AbstractFloat;
class AbstractInt;
class Array;
class Bool;
class F16;
class F32;
class I32;
class Matrix;
class Pointer;
class U32;
class Vector;
class Void;
}  // namespace tint::type

namespace tint::type {

template <typename T>
struct CppToType;

/// The type manager holds all the pointers to the known types.
class Manager final {
  public:
    /// Iterator is the type returned by begin() and end()
    using TypeIterator = utils::BlockAllocator<Type>::ConstIterator;

    /// Constructor
    Manager();

    /// Move constructor
    Manager(Manager&&);

    /// Move assignment operator
    /// @param rhs the Manager to move
    /// @return this Manager
    Manager& operator=(Manager&& rhs);

    /// Destructor
    ~Manager();

    /// Wrap returns a new Manager created with the types of `inner`.
    /// The Manager returned by Wrap is intended to temporarily extend the types
    /// of an existing immutable Manager.
    /// As the copied types are owned by `inner`, `inner` must not be destructed
    /// or assigned while using the returned Manager.
    /// TODO(bclayton) - Evaluate whether there are safer alternatives to this
    /// function. See crbug.com/tint/460.
    /// @param inner the immutable Manager to extend
    /// @return the Manager that wraps `inner`
    static Manager Wrap(const Manager& inner) {
        Manager out;
        out.types_.Wrap(inner.types_);
        out.unique_nodes_.Wrap(inner.unique_nodes_);
        return out;
    }

    /// Constructs or returns an existing type, unique node or node
    /// @param args the arguments used to construct the type, unique node or node.
    /// @tparam T a class deriving from type::Node, or a C-like type that's automatically translated
    /// to the equivalent type node type. For example `Get<i32>()` is equivalent to
    /// `Get<type::I32>()`
    /// @return a pointer to an instance of `T` with the provided arguments.
    /// If `T` derives from UniqueNode and an existing instance of `T` has been constructed, then
    /// the same pointer is returned.
    template <typename T, typename... ARGS>
    auto* Get(ARGS&&... args) {
        using N = ToType<T>;
        if constexpr (utils::traits::IsTypeOrDerived<N, Type>) {
            return types_.Get<N>(std::forward<ARGS>(args)...);
        } else if constexpr (utils::traits::IsTypeOrDerived<N, UniqueNode>) {
            return unique_nodes_.Get<T>(std::forward<ARGS>(args)...);
        } else {
            return nodes_.Create<T>(std::forward<ARGS>(args)...);
        }
    }

    /// @param args the arguments used to create the temporary used for the search.
    /// @return a pointer to an instance of `T` with the provided arguments, or nullptr if the item
    ///         was not found.
    template <typename TYPE,
              typename _ = std::enable_if<utils::traits::IsTypeOrDerived<TYPE, Type>>,
              typename... ARGS>
    auto* Find(ARGS&&... args) const {
        return types_.Find<ToType<TYPE>>(std::forward<ARGS>(args)...);
    }

    /// @returns a void type
    const type::Void* void_();

    /// @returns a bool type
    const type::Bool* bool_();

    /// @returns an i32 type
    const type::I32* i32();

    /// @returns a u32 type
    const type::U32* u32();

    /// @returns an f32 type
    const type::F32* f32();

    /// @returns an f16 type
    const type::F16* f16();

    /// @returns a abstract-float type
    const type::AbstractFloat* AFloat();

    /// @returns a abstract-int type
    const type::AbstractInt* AInt();

    /// @param inner the inner type
    /// @param size the vector size
    /// @returns the vector type
    const type::Vector* vec(const type::Type* inner, uint32_t size);

    /// @param inner the inner type
    /// @returns a vec2 type with the element type @p inner
    const type::Vector* vec2(const type::Type* inner);

    /// @param inner the inner type
    /// @returns a vec3 type with the element type @p inner
    const type::Vector* vec3(const type::Type* inner);

    /// @param inner the inner type
    /// @returns a vec4 type with the element type @p inner
    const type::Vector* vec4(const type::Type* inner);

    /// @tparam T the element type
    /// @tparam N the vector width
    /// @returns the vector type
    template <typename T, size_t N>
    const type::Vector* vec() {
        static_assert(N >= 2 && N <= 4);
        switch (N) {
            case 2:
                return vec2<T>();
            case 3:
                return vec3<T>();
            case 4:
                return vec4<T>();
        }
    }

    /// @tparam T the element type
    /// @returns a vec2 with the element type `T`
    template <typename T>
    const type::Vector* vec2() {
        return vec2(Get<T>());
    }

    /// @tparam T the element type
    /// @returns a vec2 with the element type `T`
    template <typename T>
    const type::Vector* vec3() {
        return vec3(Get<T>());
    }

    /// @tparam T the element type
    /// @returns a vec2 with the element type `T`
    template <typename T>
    const type::Vector* vec4() {
        return vec4(Get<T>());
    }

    /// @param inner the inner type
    /// @param cols the number of columns
    /// @param rows the number of rows
    /// @returns the matrix type
    const type::Matrix* mat(const type::Type* inner, uint32_t cols, uint32_t rows);

    /// @param inner the inner type
    /// @returns a mat2x2 with the element @p inner
    const type::Matrix* mat2x2(const type::Type* inner);

    /// @tparam T the element type
    /// @returns a mat2x2 with the element type `T`
    template <typename T>
    const type::Matrix* mat2x2() {
        return mat2x2(Get<T>());
    }

    /// @param inner the inner type
    /// @returns a mat2x3 with the element @p inner
    const type::Matrix* mat2x3(const type::Type* inner);

    /// @tparam T the element type
    /// @returns a mat2x3 with the element type `T`
    template <typename T>
    const type::Matrix* mat2x3() {
        return mat2x3(Get<T>());
    }

    /// @param inner the inner type
    /// @returns a mat2x4 with the element @p inner
    const type::Matrix* mat2x4(const type::Type* inner);

    /// @tparam T the element type
    /// @returns a mat2x4 with the element type `T`
    template <typename T>
    const type::Matrix* mat2x4() {
        return mat2x4(Get<T>());
    }

    /// @param inner the inner type
    /// @returns a mat3x2 with the element @p inner
    const type::Matrix* mat3x2(const type::Type* inner);

    /// @tparam T the element type
    /// @returns a mat3x2 with the element type `T`
    template <typename T>
    const type::Matrix* mat3x2() {
        return mat3x2(Get<T>());
    }

    /// @param inner the inner type
    /// @returns a mat3x3 with the element @p inner
    const type::Matrix* mat3x3(const type::Type* inner);

    /// @tparam T the element type
    /// @returns a mat3x3 with the element type `T`
    template <typename T>
    const type::Matrix* mat3x3() {
        return mat3x3(Get<T>());
    }

    /// @param inner the inner type
    /// @returns a mat3x4 with the element @p inner
    const type::Matrix* mat3x4(const type::Type* inner);

    /// @tparam T the element type
    /// @returns a mat3x4 with the element type `T`
    template <typename T>
    const type::Matrix* mat3x4() {
        return mat3x4(Get<T>());
    }

    /// @param inner the inner type
    /// @returns a mat4x2 with the element @p inner
    const type::Matrix* mat4x2(const type::Type* inner);

    /// @tparam T the element type
    /// @returns a mat4x2 with the element type `T`
    template <typename T>
    const type::Matrix* mat4x2() {
        return mat4x2(Get<T>());
    }

    /// @param inner the inner type
    /// @returns a mat4x3 with the element @p inner
    const type::Matrix* mat4x3(const type::Type* inner);

    /// @tparam T the element type
    /// @returns a mat4x3 with the element type `T`
    template <typename T>
    const type::Matrix* mat4x3() {
        return mat4x3(Get<T>());
    }

    /// @param inner the inner type
    /// @returns a mat4x4 with the element @p inner
    const type::Matrix* mat4x4(const type::Type* inner);

    /// @tparam T the element type
    /// @returns a mat4x4 with the element type `T`
    template <typename T>
    const type::Matrix* mat4x4() {
        return mat4x4(Get<T>());
    }

    /// @param elem_ty the array element type
    /// @param count the array element count
    /// @param stride the optional array element stride
    /// @returns the array type
    const type::Array* array(const type::Type* elem_ty, uint32_t count, uint32_t stride = 0);

    /// @param elem_ty the array element type
    /// @param stride the optional array element stride
    /// @returns the runtime array type
    const type::Array* runtime_array(const type::Type* elem_ty, uint32_t stride = 0);

    /// @returns an array type with the element type `T` and size `N`.
    /// @tparam T the element type
    /// @tparam N the array length. If zero, then constructs a runtime-sized array.
    /// @param stride the optional array element stride
    template <typename T, size_t N = 0>
    const type::Array* array(uint32_t stride = 0) {
        if constexpr (N == 0) {
            return runtime_array(Get<T>(), stride);
        } else {
            return array(Get<T>(), N, stride);
        }
    }

    /// @param address_space the address space
    /// @param subtype the pointer subtype
    /// @param access the access settings
    /// @returns the pointer type
    const type::Pointer* ptr(builtin::AddressSpace address_space,
                             const type::Type* subtype,
                             builtin::Access access);

    /// @tparam SPACE the address space
    /// @tparam T the storage type
    /// @tparam ACCESS the access mode
    /// @returns the pointer type with the templated address space, storage type and access.
    template <builtin::AddressSpace SPACE,
              typename T,
              builtin::Access ACCESS = builtin::Access::kReadWrite>
    const type::Pointer* ptr() {
        return ptr(SPACE, Get<T>(), ACCESS);
    }

    /// @returns an iterator to the beginning of the types
    TypeIterator begin() const { return types_.begin(); }
    /// @returns an iterator to the end of the types
    TypeIterator end() const { return types_.end(); }

  private:
    /// ToType<T> is specialized for various `T` types and each specialization contains a single
    /// `type` alias to the corresponding type deriving from `type::Type`.
    template <typename T>
    struct ToTypeImpl {
        using type = T;
    };

    template <typename T>
    using ToType = typename ToTypeImpl<T>::type;

    /// Unique types owned by the manager
    utils::UniqueAllocator<Type> types_;
    /// Unique nodes (excluding types) owned by the manager
    utils::UniqueAllocator<UniqueNode> unique_nodes_;
    /// Non-unique nodes owned by the manager
    utils::BlockAllocator<Node> nodes_;
};

//! @cond Doxygen_Suppress
// Various template specializations for Manager::ToTypeImpl.
template <>
struct Manager::ToTypeImpl<AInt> {
    using type = type::AbstractInt;
};
template <>
struct Manager::ToTypeImpl<AFloat> {
    using type = type::AbstractFloat;
};
template <>
struct Manager::ToTypeImpl<i32> {
    using type = type::I32;
};
template <>
struct Manager::ToTypeImpl<u32> {
    using type = type::U32;
};
template <>
struct Manager::ToTypeImpl<f32> {
    using type = type::F32;
};
template <>
struct Manager::ToTypeImpl<f16> {
    using type = type::F16;
};
template <>
struct Manager::ToTypeImpl<bool> {
    using type = type::Bool;
};
template <typename T>
struct Manager::ToTypeImpl<const T> {
    using type = const Manager::ToType<T>;
};
template <typename T>
struct Manager::ToTypeImpl<T*> {
    using type = Manager::ToType<T>*;
};
//! @endcond

}  // namespace tint::type

#endif  // SRC_TINT_TYPE_MANAGER_H_
