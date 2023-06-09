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

#include "src/tint/ir/function_param.h"
#include "src/tint/ir/location.h"
#include "src/tint/ir/value.h"
#include "src/tint/type/type.h"

// Forward declarations
namespace tint::ir {
class Block;
class FunctionTerminator;
}  // namespace tint::ir

namespace tint::ir {

/// An IR representation of a function
class Function : public utils::Castable<Function, Value> {
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

    /// Builtin attached to return types
    enum class ReturnBuiltin {
        /// Builtin Position attribute
        kPosition,
        /// Builtin FragDepth attribute
        kFragDepth,
        /// Builtin SampleMask
        kSampleMask,
    };

    /// Constructor
    /// @param rt the function return type
    /// @param stage the function stage
    /// @param wg_size the workgroup_size
    Function(const type::Type* rt,
             PipelineStage stage = PipelineStage::kUndefined,
             std::optional<std::array<uint32_t, 3>> wg_size = {});
    ~Function() override;

    /// Sets the function stage
    /// @param stage the stage to set
    void SetStage(PipelineStage stage) { pipeline_stage_ = stage; }

    /// @returns the function pipeline stage
    PipelineStage Stage() { return pipeline_stage_; }

    /// Sets the workgroup size
    /// @param x the x size
    /// @param y the y size
    /// @param z the z size
    void SetWorkgroupSize(uint32_t x, uint32_t y, uint32_t z) { workgroup_size_ = {x, y, z}; }

    /// @returns the workgroup size information
    std::optional<std::array<uint32_t, 3>> WorkgroupSize() { return workgroup_size_; }

    /// @returns the return type for the function
    const type::Type* ReturnType() { return return_.type; }

    /// Sets the return attributes
    /// @param builtin the builtin to set
    void SetReturnBuiltin(ReturnBuiltin builtin) {
        TINT_ASSERT(IR, !return_.builtin.has_value());
        return_.builtin = builtin;
    }
    /// @returns the return builtin attribute
    std::optional<enum ReturnBuiltin> ReturnBuiltin() { return return_.builtin; }

    /// Sets the return location
    /// @param loc the location to set
    /// @param interp the interpolation
    void SetReturnLocation(uint32_t loc, std::optional<builtin::Interpolation> interp) {
        return_.location = {loc, interp};
    }
    /// @returns the return location
    std::optional<Location> ReturnLocation() { return return_.location; }

    /// Sets the return as invariant
    /// @param val the invariant value to set
    void SetReturnInvariant(bool val) { return_.invariant = val; }
    /// @returns the return invariant value
    bool ReturnInvariant() { return return_.invariant; }

    /// Sets the function parameters
    /// @param params the function parameters
    void SetParams(utils::VectorRef<FunctionParam*> params);

    /// Sets the function parameters
    /// @param params the function parameters
    void SetParams(std::initializer_list<FunctionParam*> params);

    /// @returns the function parameters
    const utils::VectorRef<FunctionParam*> Params() { return params_; }

    /// Sets the start target for the function
    /// @param target the start target
    void SetStartTarget(Block* target) {
        TINT_ASSERT(IR, target != nullptr);
        start_target_ = target;
    }
    /// @returns the function start target
    Block* StartTarget() { return start_target_; }

  private:
    PipelineStage pipeline_stage_;
    std::optional<std::array<uint32_t, 3>> workgroup_size_;

    struct {
        const type::Type* type = nullptr;
        std::optional<enum ReturnBuiltin> builtin;
        std::optional<Location> location;
        bool invariant = false;
    } return_;

    utils::Vector<FunctionParam*, 1> params_;
    Block* start_target_ = nullptr;
};

utils::StringStream& operator<<(utils::StringStream& out, Function::PipelineStage value);
utils::StringStream& operator<<(utils::StringStream& out, enum Function::ReturnBuiltin value);

}  // namespace tint::ir

#endif  // SRC_TINT_IR_FUNCTION_H_
