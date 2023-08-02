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

#ifndef SRC_TINT_LANG_HLSL_WRITER_OUTPUT_H_
#define SRC_TINT_LANG_HLSL_WRITER_OUTPUT_H_

#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/lang/wgsl/ast/pipeline_stage.h"

namespace tint::hlsl::writer {

/// The output produced when generating HLSL.
struct Output {
    /// Constructor
    Output();

    /// Destructor
    ~Output();

    /// Copy constructor
    Output(const Output&);

    /// The generated HLSL.
    std::string hlsl = "";

    /// The list of entry points in the generated HLSL.
    std::vector<std::pair<std::string, ast::PipelineStage>> entry_points;

    /// Indices into the array_length_from_uniform binding that are statically
    /// used.
    std::unordered_set<uint32_t> used_array_length_from_uniform_indices;
};

}  // namespace tint::hlsl::writer

#endif  // SRC_TINT_LANG_HLSL_WRITER_OUTPUT_H_
