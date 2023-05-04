// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_IR_FUNCTION_H_
#define SRC_TINT_IR_FUNCTION_H_

#include <array>
#include <optional>

#include "src/tint/ir/flow_node.h"
#include "src/tint/symbol.h"
#include "src/tint/type/type.h"

// Forward declarations
namespace tint::ir {
class Block;
class FunctionTerminator;
}  // namespace tint::ir

namespace tint::ir {

/// An IR representation of a function
class Function : public utils::Castable<Function, FlowNode> {
  public:
    /// The pipeline stage for an entry point
    enum class PipelineStage {
        /// Not a pipeline entry point
        kUndefined,
        /// Vertex
        kCompute,
        /// Fragment
        kFragment,
        /// Vertex
        kVertex,
    };

    /// Attributes attached to return types
    enum class ReturnAttribute {
        /// No return attribute
        kNone,
        /// Location attribute
        kLocation,
        /// Builtin Position attribute
        kPosition,
        /// Builtin FragDepth attribute
        kFragDepth,
        /// Builtin SampleMask
        kSampleMask,
        /// Invariant attribute
        kInvariant,
    };

    /// Constructor
    Function();
    ~Function() override;

    /// The function name
    Symbol name;

    /// The pipeline stage for the function, `kUndefined` if the function is not an entry point
    PipelineStage pipeline_stage = PipelineStage::kUndefined;

    /// If this is a `compute` entry point, holds the workgroup size information
    std::optional<std::array<uint32_t, 3>> workgroup_size;

    /// The function return type
    const type::Type* return_type = nullptr;
    /// The function return attributes if any
    utils::Vector<ReturnAttribute, 1> return_attributes;
    /// If the return attribute is `kLocation` this stores the location value.
    std::optional<uint32_t> return_location;

    /// The start target is the first block in a function.
    Block* start_target = nullptr;
    /// The end target is the end of the function. It is used as the branch target if a return is
    /// encountered in the function.
    FunctionTerminator* end_target = nullptr;
};

utils::StringStream& operator<<(utils::StringStream& out, Function::PipelineStage value);
utils::StringStream& operator<<(utils::StringStream& out, Function::ReturnAttribute value);

}  // namespace tint::ir

#endif  // SRC_TINT_IR_FUNCTION_H_
