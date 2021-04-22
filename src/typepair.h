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
class Bool;
class F32;
class I32;
class Matrix;
class U32;
class Vector;
class Void;
}  // namespace ast

namespace sem {
class Bool;
class F32;
class I32;
class Matrix;
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
  AST const* const ast = nullptr;
  /// The sem::Type pointer
  SEM const* const sem = nullptr;

  /// Constructor
  TypePair() = default;
  /// Constructor
  /// @param a the ast::Type pointer
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
};

using Type = TypePair<ast::Type, sem::Type>;

using Bool = TypePair<ast::Bool, sem::Bool>;
using F32 = TypePair<ast::F32, sem::F32>;
using I32 = TypePair<ast::I32, sem::I32>;
using Matrix = TypePair<ast::Matrix, sem::Matrix>;
using U32 = TypePair<ast::U32, sem::U32>;
using Vector = TypePair<ast::Vector, sem::Vector>;
using Void = TypePair<ast::Void, sem::Void>;

}  // namespace typ

}  // namespace tint

#endif  // SRC_TYPEPAIR_H_
