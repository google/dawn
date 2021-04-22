// Copyright 2021 The Tint Authors.
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

// This file contains temporary code to help implement the new `ast::Type`s.
// Once complete, this file should be completely removed.
// Bug: crbug.com/tint/724

#ifndef SRC_TYPEPAIR_H_
#define SRC_TYPEPAIR_H_

namespace tint {

namespace ast {
class AccessControl;
class Alias;
class Array;
class Bool;
class DepthTexture;
class F32;
class I32;
class Matrix;
class MultisampledTexture;
class Pointer;
class Sampler;
class SampledTexture;
class StorageTexture;
class Struct;
class U32;
class Vector;
class Void;
}  // namespace ast

namespace sem {
class AccessControl;
class Alias;
class ArrayType;
class Bool;
class DepthTexture;
class F32;
class I32;
class Matrix;
class MultisampledTexture;
class Pointer;
class Sampler;
class SampledTexture;
class StorageTexture;
class StructType;
class U32;
class Vector;
class Void;
}  // namespace sem

namespace typ {  //  type-pair

/// TypePair is a pair of ast::Type and sem::Type pointers used to simplify
/// migration to the new ast::Type nodes.
///
/// Type attempts to behave as either an ast::Type or sem::Type:
/// * Type has constructors that take either an ast::Type and sem::Type pointer
///   pair, and single-parameter implicit constructors for either a single
///   ast::Type or sem::Type pointer.
/// * Type also has user-defined conversion functions for returning either an
///   ast::Type or sem::Type pointer.
/// * operator->() returns the sem::Type pointer. Later in the migration this
///   will switch to returning the ast::Type pointer.
template <typename AST, typename SEM>
struct TypePair {
  /// The ast::Type pointer
  AST const* ast = nullptr;
  /// The sem::Type pointer
  SEM const* sem = nullptr;

  /// Constructor
  TypePair() = default;
  /// Copy constructor
  /// @param other the TypePair to copy
  template <typename OTHER_AST, typename OTHER_SEM>
  TypePair(const TypePair<OTHER_AST, OTHER_SEM>& other)
      : ast(static_cast<const AST*>(other.ast)),
        sem(static_cast<const SEM*>(other.sem)) {}
  /// Constructor
  /// @param a the ast::Type pointer
  template <typename U>
  TypePair(const AST* a) : ast(a) {}  // NOLINT: explicit
  /// Constructor
  /// @param s the sem::Type pointer
  TypePair(const SEM* s) : sem(s) {}  // NOLINT: explicit
  /// Constructor
  /// @param a the ast::Type pointer
  /// @param s the sem::Type pointer
  TypePair(const AST* a, const SEM* s) : ast(a), sem(s) {}

  /// @returns the ast::Type pointer
  operator AST*() const { return const_cast<AST*>(ast); }
  /// @returns the sem::Type pointer
  operator SEM*() const { return const_cast<SEM*>(sem); }
  /// @returns the sem::Type pointer
  SEM* operator->() const { return const_cast<SEM*>(sem); }

  /// @param ty the semantic type to compare against
  /// @returns true if the semantic type is equal to `ty`
  bool operator==(sem::Type* ty) const { return sem == ty; }

  /// @param other the TypePair to compare against
  /// @returns true if this TypePair is less than `other`
  template <typename OTHER_AST, typename OTHER_SEM>
  bool operator<(const TypePair<OTHER_AST, OTHER_SEM>& other) const {
    if (sem < other.sem) {
      return true;
    }
    if (sem > other.sem) {
      return false;
    }
    return ast < other.ast;
  }
};

using Type = TypePair<ast::Type, sem::Type>;

using AccessControl = TypePair<ast::AccessControl, sem::AccessControl>;
using Alias = TypePair<ast::Alias, sem::Alias>;
using Array = TypePair<ast::Array, sem::ArrayType>;
using Bool = TypePair<ast::Bool, sem::Bool>;
using DepthTexture = TypePair<ast::DepthTexture, sem::DepthTexture>;
using F32 = TypePair<ast::F32, sem::F32>;
using I32 = TypePair<ast::I32, sem::I32>;
using Matrix = TypePair<ast::Matrix, sem::Matrix>;
using MultisampledTexture =
    TypePair<ast::MultisampledTexture, sem::MultisampledTexture>;
using Pointer = TypePair<ast::Pointer, sem::Pointer>;
using Sampler = TypePair<ast::Sampler, sem::Sampler>;
using SampledTexture = TypePair<ast::SampledTexture, sem::SampledTexture>;
using StorageTexture = TypePair<ast::StorageTexture, sem::StorageTexture>;
using Struct = TypePair<ast::Struct, sem::StructType>;
using U32 = TypePair<ast::U32, sem::U32>;
using Vector = TypePair<ast::Vector, sem::Vector>;
using Void = TypePair<ast::Void, sem::Void>;

}  // namespace typ

}  // namespace tint

#endif  // SRC_TYPEPAIR_H_
