// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_LANG_GLSL_WRITER_OUTPUT_H_
#define SRC_TINT_LANG_GLSL_WRITER_OUTPUT_H_

#include <string>
#include <utility>
#include <vector>

#include "src/tint/api/options/texture_builtins_from_uniform.h"
#include "src/tint/lang/wgsl/ast/pipeline_stage.h"

namespace tint::glsl::writer {

/// The output produced when generating GLSL.
struct Output {
    /// Constructor
    Output();

    /// Destructor
    ~Output();

    /// Copy constructor
    Output(const Output&);

    /// The generated GLSL.
    std::string glsl = "";

    /// The list of entry points in the generated GLSL.
    std::vector<std::pair<std::string, ast::PipelineStage>> entry_points;

    /// True if the shader needs a UBO.
    bool needs_internal_uniform_buffer = false;

    /// Store a map of global texture variable binding points to the byte offset and data type to
    /// push into the internal uniform buffer.
    TextureBuiltinsFromUniformOptions::BindingPointToFieldAndOffset bindpoint_to_data;
};

}  // namespace tint::glsl::writer

#endif  // SRC_TINT_LANG_GLSL_WRITER_OUTPUT_H_
