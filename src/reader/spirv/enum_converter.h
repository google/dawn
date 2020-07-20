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

#ifndef SRC_READER_SPIRV_ENUM_CONVERTER_H_
#define SRC_READER_SPIRV_ENUM_CONVERTER_H_

#include "spirv/unified1/spirv.h"
#include "src/ast/builtin.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/storage_class.h"
#include "src/reader/spirv/fail_stream.h"

namespace tint {
namespace reader {
namespace spirv {

/// A converter from SPIR-V enums to Tint AST enums.
class EnumConverter {
 public:
  /// Creates a new enum converter.
  /// @param fail_stream the error reporting stream.
  explicit EnumConverter(const FailStream& fail_stream);
  /// Destructor
  ~EnumConverter();

  /// Converts a SPIR-V execution model to a Tint pipeline stage.
  /// On failure, logs an error and returns kNone
  /// @param model the SPIR-V entry point execution model
  /// @returns a Tint AST pipeline stage
  ast::PipelineStage ToPipelineStage(SpvExecutionModel model);

  /// Converts a SPIR-V storage class to a Tint storage class.
  /// On failure, logs an error and returns kNone
  /// @param sc the SPIR-V storage class
  /// @returns a Tint AST storage class
  ast::StorageClass ToStorageClass(const SpvStorageClass sc);

  /// Converts a SPIR-V Builtin value a Tint Builtin.
  /// On failure, logs an error and returns kNone
  /// @param b the SPIR-V builtin
  /// @returns a Tint AST builtin
  ast::Builtin ToBuiltin(SpvBuiltIn b);

 private:
  /// Registers a failure and returns a stream for log diagnostics.
  /// @returns a failure stream
  FailStream Fail() { return fail_stream_.Fail(); }

  FailStream fail_stream_;
};

}  // namespace spirv
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_SPIRV_ENUM_CONVERTER_H_
