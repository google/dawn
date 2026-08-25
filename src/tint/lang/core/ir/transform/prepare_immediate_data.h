// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_PREPARE_IMMEDIATE_DATA_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_PREPARE_IMMEDIATE_DATA_H_

#include <map>

#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/reflection/reflection.h"
#include "src/tint/utils/result.h"
#include "src/tint/utils/symbol/symbol.h"

// Forward declarations.
namespace tint::core::ir {
class Builder;
class Module;
class Value;
class Var;
}  // namespace tint::core::ir
namespace tint::core::type {
class Type;
}

namespace tint::core {

enum InternalImmediate {
    kStorageBufferSizes,
    kStorageBufferOffsets,
    kFirstInstanceOffset,
    kFirstVertexOffset,
    kNumWorkgroups,
    kFragDepthMin,
    kFragDepthMax,
    kNonConstantZero,
};

namespace ir::transform {

/// A descriptor for an internal immediate.
struct InternalImmediateData {
    InternalImmediate immediate;
    Symbol name;
    const core::type::Type* type = nullptr;
};

/// A struct that describes the layout of the generated immediate data structure.
struct ImmediateDataLayout {
    /// The immediate data variable.
    core::ir::Var* var = nullptr;

    /// A map from immediate to member index.
    Hashmap<InternalImmediate, uint32_t, 6> immediate_to_index;

    /// @returns true if the immediate data contains the specified immediate
    bool HasImmediate(InternalImmediate immediate) const {
        return immediate_to_index.Contains(immediate);
    }

    /// @returns a pointer to the specified immediate
    Value* GetPointer(Builder& b, InternalImmediate immediate) const;

    /// @returns the value of the specified immediate
    Value* GetValue(Builder& b, InternalImmediate immediate) const;
};

/// The internally created immediate data members.
struct PrepareImmediateDataConfig {
    /// Add an internal immediate data to the map.
    Result<SuccessType> AddInternalImmediateData(InternalImmediate immediate,
                                                 uint32_t offset,
                                                 Symbol name,
                                                 const core::type::Type* type) {
        auto res =
            internal_immediate_data.emplace(offset, InternalImmediateData{immediate, name, type});
        if (!res.second) {
            return Failure("multiple internal immediates created at offset " +
                           std::to_string(offset));
        }
        return Success;
    }

    /// The ordered map from offset to internally used constant descriptor.
    std::map<uint32_t, InternalImmediateData> internal_immediate_data{};

    /// Reflection for this class.
    TINT_REFLECT(PrepareImmediateDataConfig, internal_immediate_data);
};

/// PrepareImmediateData is a transform that sets up the structure and variable used for immediate
/// data to combine both user-defined and internally used immediate data values.
/// @param module the module to transform
/// @param config the transform config
/// @returns the generated immediate layout or failure
Result<ImmediateDataLayout> PrepareImmediateData(Module& module,
                                                 const PrepareImmediateDataConfig& config);

}  // namespace ir::transform
}  // namespace tint::core

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_PREPARE_IMMEDIATE_DATA_H_
