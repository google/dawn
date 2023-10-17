// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_IR_FUNCTION_H_
#define SRC_TINT_LANG_CORE_IR_FUNCTION_H_

#include <array>
#include <optional>
#include <utility>

#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/location.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/ice/ice.h"

// Forward declarations
namespace tint::core::ir {
class Block;
class FunctionTerminator;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// An IR representation of a function
class Function : public Castable<Function, Value> {
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
    Function(const core::type::Type* rt,
             PipelineStage stage = PipelineStage::kUndefined,
             std::optional<std::array<uint32_t, 3>> wg_size = {});
    ~Function() override;

    /// @copydoc Instruction::Clone()
    Function* Clone(CloneContext& ctx) override;

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

    /// Clears the workgroup size.
    void ClearWorkgroupSize() { workgroup_size_ = {}; }

    /// @returns the workgroup size information
    std::optional<std::array<uint32_t, 3>> WorkgroupSize() { return workgroup_size_; }

    /// @returns the return type for the function
    const core::type::Type* ReturnType() { return return_.type; }

    /// Sets the return attributes
    /// @param builtin the builtin to set
    void SetReturnBuiltin(ReturnBuiltin builtin) {
        TINT_ASSERT(!return_.builtin.has_value());
        return_.builtin = builtin;
    }
    /// @returns the return builtin attribute
    std::optional<enum ReturnBuiltin> ReturnBuiltin() { return return_.builtin; }
    /// Clears the return builtin attribute.
    void ClearReturnBuiltin() { return_.builtin = {}; }

    /// Sets the return location
    /// @param loc the location to set
    /// @param interp the interpolation
    void SetReturnLocation(uint32_t loc, std::optional<core::Interpolation> interp) {
        return_.location = {loc, interp};
    }
    /// @returns the return location
    std::optional<Location> ReturnLocation() { return return_.location; }
    /// Clears the return location attribute.
    void ClearReturnLocation() { return_.location = {}; }

    /// Sets the return as invariant
    /// @param val the invariant value to set
    void SetReturnInvariant(bool val) { return_.invariant = val; }
    /// @returns the return invariant value
    bool ReturnInvariant() { return return_.invariant; }

    /// Sets the function parameters
    /// @param params the function parameters
    void SetParams(VectorRef<FunctionParam*> params);

    /// Sets the function parameters
    /// @param params the function parameters
    void SetParams(std::initializer_list<FunctionParam*> params);

    /// @returns the function parameters
    const VectorRef<FunctionParam*> Params() { return params_; }

    /// Sets the root block for the function
    /// @param target the root block
    void SetBlock(Block* target) {
        TINT_ASSERT(target != nullptr);
        block_ = target;
    }
    /// @returns the function root block
    ir::Block* Block() { return block_; }

    /// Destroys the function and all of its instructions.
    void Destroy() override;

  private:
    PipelineStage pipeline_stage_;
    std::optional<std::array<uint32_t, 3>> workgroup_size_;

    struct {
        const core::type::Type* type = nullptr;
        std::optional<enum ReturnBuiltin> builtin;
        std::optional<Location> location;
        bool invariant = false;
    } return_;

    Vector<FunctionParam*, 1> params_;
    ir::Block* block_ = nullptr;
};

/// @param value the enum value
/// @returns the string for the given enum value
std::string_view ToString(Function::PipelineStage value);

/// @param out the stream to write to
/// @param value the Function::PipelineStage
/// @returns @p out so calls can be chained
template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& out, Function::PipelineStage value) {
    return out << ToString(value);
}

/// @param value the enum value
/// @returns the string for the given enum value
std::string_view ToString(enum Function::ReturnBuiltin value);

/// @param out the stream to write to
/// @param value the Function::ReturnBuiltin
/// @returns @p out so calls can be chained
template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& out, enum Function::ReturnBuiltin value) {
    return out << ToString(value);
}

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_FUNCTION_H_
