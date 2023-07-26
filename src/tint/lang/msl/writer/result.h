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

#ifndef SRC_TINT_LANG_MSL_WRITER_RESULT_H_
#define SRC_TINT_LANG_MSL_WRITER_RESULT_H_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace tint::msl::writer {

/// The result produced when generating MSL.
struct Result {
    /// Constructor
    Result();

    /// Destructor
    ~Result();

    /// Copy constructor
    Result(const Result&);

    /// True if generation was successful.
    bool success = false;

    /// The errors generated during code generation, if any.
    std::string error;

    /// The generated MSL.
    std::string msl = "";

    /// True if the shader needs a UBO of buffer sizes.
    bool needs_storage_buffer_sizes = false;

    /// True if the generated shader uses the invariant attribute.
    bool has_invariant_attribute = false;

    /// A map from entry point name to a list of dynamic workgroup allocations.
    /// Each entry in the vector is the size of the workgroup allocation that
    /// should be created for that index.
    std::unordered_map<std::string, std::vector<uint32_t>> workgroup_allocations;

    /// Indices into the array_length_from_uniform binding that are statically
    /// used.
    std::unordered_set<uint32_t> used_array_length_from_uniform_indices;
};

}  // namespace tint::msl::writer

#endif  // SRC_TINT_LANG_MSL_WRITER_RESULT_H_
