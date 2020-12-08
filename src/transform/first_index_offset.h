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

#ifndef SRC_TRANSFORM_FIRST_INDEX_OFFSET_H_
#define SRC_TRANSFORM_FIRST_INDEX_OFFSET_H_

#include <string>

#include "src/ast/module.h"
#include "src/transform/transform.h"

namespace tint {
namespace transform {

/// Adds firstVertex/Instance (injected via root constants) to
/// vertex/instance_idx builtins.
///
/// This transform assumes that Name transform has been run before.
///
/// Unlike other APIs, D3D always starts vertex and instance numbering at 0,
/// regardless of the firstVertex/Instance value specified. This transformer
/// adds the value of firstVertex/Instance to each builtin. This action is
/// performed by adding a new constant equal to original builtin +
/// firstVertex/Instance to each function that references one of these builtins.
///
/// Note that D3D does not have any semantics for firstVertex/Instance.
/// Therefore, these values must by passed to the shader.
///
/// Before:
///   [[builtin(vertex_index)]] var<in> vert_idx : u32;
///   fn func() -> u32 {
///     return vert_idx;
///   }
///
/// After:
///   [[block]]
///   struct TintFirstIndexOffsetData {
///     [[offset(0)]] tint_first_vertex_index : u32;
///     [[offset(4)]] tint_first_instance_index : u32;
///   };
///   [[builtin(vertex_index)]] var<in> tint_first_index_offset_vert_idx : u32;
///   [[binding(N), set(M)]] var<uniform> tint_first_index_data :
///                                                    TintFirstIndexOffsetData;
///   fn func() -> u32 {
///     const vert_idx = (tint_first_index_offset_vert_idx +
///                       tint_first_index_data.tint_first_vertex_index);
///     return vert_idx;
///   }
///
class FirstIndexOffset : public Transform {
 public:
  /// Constructor
  /// @param binding the binding() for firstVertex/Instance uniform
  /// @param set the set() for firstVertex/Instance uniform
  FirstIndexOffset(uint32_t binding, uint32_t set);
  ~FirstIndexOffset() override;

  /// Runs the transform on `module`, returning the transformation result.
  /// @note Users of Tint should register the transform with transform manager
  /// and invoke its Run(), instead of directly calling the transform's Run().
  /// Calling Run() directly does not perform module state cleanup operations.
  /// @param module the source module to transform
  /// @returns the transformation result
  Output Run(ast::Module* module) override;

  /// @returns whether shader uses vertex_index
  bool HasVertexIndex();

  /// @returns whether shader uses instance_index
  bool HasInstanceIndex();

  /// @returns offset of firstVertex into constant buffer
  uint32_t GetFirstVertexOffset();

  /// @returns offset of firstInstance into constant buffer
  uint32_t GetFirstInstanceOffset();

 private:
  /// Adds uniform buffer with firstVertex/Instance to module
  /// @returns variable of new uniform buffer
  ast::Variable* AddUniformBuffer(ast::Module* mod);
  /// Adds constant with modified original_name builtin to func
  /// @param original_name the name of the original builtin used in function
  /// @param field_name name of field in firstVertex/Instance buffer
  /// @param buffer_var variable of firstVertex/Instance buffer
  /// @param func function to modify
  /// @returns true if it was able to add constant to function
  void AddFirstIndexOffset(const std::string& original_name,
                           const std::string& field_name,
                           ast::Variable* buffer_var,
                           ast::Function* func,
                           ast::Module* module);

  uint32_t binding_;
  uint32_t set_;

  bool has_vertex_index_ = false;
  bool has_instance_index_ = false;

  uint32_t vertex_index_offset_ = 0;
  uint32_t instance_index_offset_ = 0;
};
}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_FIRST_INDEX_OFFSET_H_
