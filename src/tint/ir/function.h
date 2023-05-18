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
#include <utility>

#include "src/tint/ir/flow_node.h"
#include "src/tint/ir/function_param.h"
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
    /// @param n the function name
    /// @param rt the function return type
    /// @param stage the function stage
    /// @param wg_size the workgroup_size
    Function(Symbol n,
             type::Type* rt,
             PipelineStage stage = PipelineStage::kUndefined,
             std::optional<std::array<uint32_t, 3>> wg_size = {});
    Function(Function&&) = delete;
    Function(const Function&) = delete;
    ~Function() override;

    Function& operator=(Function&&) = delete;
    Function& operator=(const Function&) = delete;

    /// @returns the function name
    Symbol Name() const { return name_; }

    /// Sets the function stage
    /// @param stage the stage to set
    void SetStage(PipelineStage stage) { pipeline_stage_ = stage; }

    /// @returns the function pipeline stage
    PipelineStage Stage() const { return pipeline_stage_; }

    /// Sets the workgroup size
    /// @param x the x size
    /// @param y the y size
    /// @param z the z size
    void SetWorkgroupSize(uint32_t x, uint32_t y, uint32_t z) { workgroup_size_ = {x, y, z}; }

    /// @returns the workgroup size information
    std::optional<std::array<uint32_t, 3>> WorkgroupSize() const { return workgroup_size_; }

    /// @returns the return type for the function
    const type::Type* ReturnType() const { return return_type_; }

    /// Sets the return attributes
    /// @param attrs the attributes to set
    void SetReturnAttributes(utils::VectorRef<ReturnAttribute> attrs) {
        return_attributes_ = std::move(attrs);
    }
    /// @returns the return attributes
    utils::VectorRef<ReturnAttribute> ReturnAttributes() const { return return_attributes_; }

    /// Sets the return location
    /// @param loc the location to set
    void SetReturnLocation(std::optional<uint32_t> loc) { return_location_ = loc; }
    /// @returns the return location
    std::optional<uint32_t> ReturnLocation() const { return return_location_; }

    /// Sets the function parameters
    /// @param params the function paramters
    void SetParams(utils::VectorRef<FunctionParam*> params) { params_ = std::move(params); }

    /// @returns the function parameters
    utils::VectorRef<FunctionParam*> Params() const { return params_; }

    /// Sets the start target for the function
    /// @param target the start target
    void SetStartTarget(Block* target) { start_target_ = target; }
    /// @returns the function start target
    Block* StartTarget() const { return start_target_; }

    /// Sets the end target for the function
    /// @param target the end target
    void SetEndTarget(FunctionTerminator* target) { end_target_ = target; }
    /// @returns the function end target
    FunctionTerminator* EndTarget() const { return end_target_; }

  private:
    Symbol name_;
    const type::Type* return_type_;
    PipelineStage pipeline_stage_;
    std::optional<std::array<uint32_t, 3>> workgroup_size_;

    utils::Vector<ReturnAttribute, 1> return_attributes_;
    std::optional<uint32_t> return_location_;

    utils::Vector<FunctionParam*, 1> params_;

    Block* start_target_ = nullptr;
    FunctionTerminator* end_target_ = nullptr;
};

utils::StringStream& operator<<(utils::StringStream& out, Function::PipelineStage value);
utils::StringStream& operator<<(utils::StringStream& out, Function::ReturnAttribute value);

}  // namespace tint::ir

#endif  // SRC_TINT_IR_FUNCTION_H_
