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

#ifndef SRC_WRITER_APPEND_VECTOR_H_
#define SRC_WRITER_APPEND_VECTOR_H_

#include "src/program_builder.h"

namespace tint {

namespace ast {
class Expression;
class TypeConstructorExpression;
}  // namespace ast

namespace writer {

/// A helper function used to append a vector with an additional scalar.
/// AppendVector is used to generate texture intrinsic function calls for
/// backends that expect the texture coordinates to be packed with an additional
/// mip-level or array-index parameter.
/// All types must have been assigned to the expressions and their child nodes
/// before calling.
/// @param builder the program builder.
/// @param vector the vector to be appended. May be a scalar, `vec2` or `vec3`.
/// @param scalar the scalar to append to the vector. Must be a scalar.
/// @returns a vector expression containing the elements of `vector` followed by
/// the single element of `scalar` cast to the `vector` element type.
ast::TypeConstructorExpression* AppendVector(ProgramBuilder* builder,
                                             ast::Expression* vector,
                                             ast::Expression* scalar);

}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_APPEND_VECTOR_H_
