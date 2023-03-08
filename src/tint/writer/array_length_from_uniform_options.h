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

#ifndef SRC_TINT_WRITER_ARRAY_LENGTH_FROM_UNIFORM_OPTIONS_H_
#define SRC_TINT_WRITER_ARRAY_LENGTH_FROM_UNIFORM_OPTIONS_H_

#include <unordered_map>

#include "src/tint/writer/binding_point.h"

namespace tint::writer {

/// Options used to specify a mapping of binding points to indices into a UBO
/// from which to load buffer sizes.
struct ArrayLengthFromUniformOptions {
    /// Constructor
    ArrayLengthFromUniformOptions();
    /// Destructor
    ~ArrayLengthFromUniformOptions();
    /// Copy constructor
    ArrayLengthFromUniformOptions(const ArrayLengthFromUniformOptions&);
    /// Copy assignment
    /// @returns this ArrayLengthFromUniformOptions
    ArrayLengthFromUniformOptions& operator=(const ArrayLengthFromUniformOptions&);
    /// Move constructor
    ArrayLengthFromUniformOptions(ArrayLengthFromUniformOptions&&);

    /// The binding point to use to generate a uniform buffer from which to read
    /// buffer sizes.
    BindingPoint ubo_binding;
    /// The mapping from storage buffer binding points to the index into the
    /// uniform buffer where the length of the buffer is stored.
    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_size_index;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(ubo_binding, bindpoint_to_size_index);
};

}  // namespace tint::writer

#endif  // SRC_TINT_WRITER_ARRAY_LENGTH_FROM_UNIFORM_OPTIONS_H_
