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

#include "src/tint/writer/msl/generator.h"

#include <utility>

#include "src/tint/writer/msl/generator_impl.h"

namespace tint::writer::msl {

Options::Options() = default;
Options::~Options() = default;
Options::Options(const Options&) = default;
Options& Options::operator=(const Options&) = default;

Result::Result() = default;
Result::~Result() = default;
Result::Result(const Result&) = default;

Result Generate(const Program* program, const Options& options) {
    Result result;
    if (!program->IsValid()) {
        result.error = "input program is not valid";
        return result;
    }

    // Sanitize the program.
    auto sanitized_result = Sanitize(program, options);
    if (!sanitized_result.program.IsValid()) {
        result.success = false;
        result.error = sanitized_result.program.Diagnostics().str();
        return result;
    }
    result.needs_storage_buffer_sizes = sanitized_result.needs_storage_buffer_sizes;
    result.used_array_length_from_uniform_indices =
        std::move(sanitized_result.used_array_length_from_uniform_indices);

    // Generate the MSL code.
    auto impl = std::make_unique<GeneratorImpl>(&sanitized_result.program);
    result.success = impl->Generate();
    result.error = impl->Diagnostics().str();
    result.msl = impl->result();
    result.has_invariant_attribute = impl->HasInvariant();
    result.workgroup_allocations = impl->DynamicWorkgroupAllocations();

    return result;
}

}  // namespace tint::writer::msl
