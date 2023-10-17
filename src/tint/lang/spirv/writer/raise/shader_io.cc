// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/raise/shader_io.h"

#include <memory>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/transform/shader_io.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/array.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer::raise {

namespace {

/// PIMPL state for the parts of the shader IO transform specific to SPIR-V.
/// For SPIR-V, we declare a global variable for each input and output. The wrapper entry point then
/// loads from and stores to these variables. We also modify the type of the SampleMask builtin to
/// be an array, as required by Vulkan.
struct StateImpl : core::ir::transform::ShaderIOBackendState {
    /// The input variables.
    Vector<core::ir::Var*, 4> input_vars;
    /// The output variables.
    Vector<core::ir::Var*, 4> output_vars;

    /// The configuration options.
    const ShaderIOConfig& config;

    /// The frag_depth clamp arguments.
    core::ir::Value* frag_depth_clamp_args = nullptr;

    /// Constructor
    StateImpl(core::ir::Module& mod, core::ir::Function* f, const ShaderIOConfig& cfg)
        : ShaderIOBackendState(mod, f), config(cfg) {}

    /// Destructor
    ~StateImpl() override {}

    /// Declare a global variable for each IO entry listed in @p entries.
    /// @param vars the list of variables
    /// @param entries the entries to emit
    /// @param addrspace the address to use for the global variables
    /// @param access the access mode to use for the global variables
    /// @param name_suffix the suffix to add to struct and variable names
    void MakeVars(Vector<core::ir::Var*, 4>& vars,
                  Vector<core::type::Manager::StructMemberDesc, 4>& entries,
                  core::AddressSpace addrspace,
                  core::Access access,
                  const char* name_suffix) {
        for (auto io : entries) {
            StringStream name;
            name << ir.NameOf(func).Name();

            if (io.attributes.builtin) {
                // SampleMask must be an array for Vulkan.
                if (io.attributes.builtin.value() == core::BuiltinValue::kSampleMask) {
                    io.type = ty.array<u32, 1>();
                }
                name << "_" << io.attributes.builtin.value();

                // Vulkan requires that fragment integer builtin inputs be Flat decorated.
                if (func->Stage() == core::ir::Function::PipelineStage::kFragment &&
                    addrspace == core::AddressSpace::kIn &&
                    io.type->is_integer_scalar_or_vector()) {
                    io.attributes.interpolation = {core::InterpolationType::kFlat};
                }
            } else {
                name << "_loc" << io.attributes.location.value();
                if (io.attributes.index.has_value()) {
                    name << "_idx" << io.attributes.index.value();
                }
            }
            name << name_suffix;

            // Create an IO variable and add it to the root block.
            auto* ptr = ty.ptr(addrspace, io.type, access);
            auto* var = b.Var(name.str(), ptr);
            var->SetAttributes(core::ir::IOAttributes{
                io.attributes.location,
                io.attributes.index,
                io.attributes.builtin,
                io.attributes.interpolation,
                io.attributes.invariant,
            });
            ir.root_block->Append(var);
            vars.Push(var);
        }
    }

    /// @copydoc ShaderIO::BackendState::FinalizeInputs
    Vector<core::ir::FunctionParam*, 4> FinalizeInputs() override {
        MakeVars(input_vars, inputs, core::AddressSpace::kIn, core::Access::kRead, "_Input");
        return tint::Empty;
    }

    /// @copydoc ShaderIO::BackendState::FinalizeOutputs
    core::ir::Value* FinalizeOutputs() override {
        MakeVars(output_vars, outputs, core::AddressSpace::kOut, core::Access::kWrite, "_Output");
        return nullptr;
    }

    /// @copydoc ShaderIO::BackendState::GetInput
    core::ir::Value* GetInput(core::ir::Builder& builder, uint32_t idx) override {
        // Load the input from the global variable declared earlier.
        auto* ptr = ty.ptr(core::AddressSpace::kIn, inputs[idx].type, core::Access::kRead);
        auto* from = input_vars[idx]->Result();
        if (inputs[idx].attributes.builtin) {
            if (inputs[idx].attributes.builtin.value() == core::BuiltinValue::kSampleMask) {
                // SampleMask becomes an array for SPIR-V, so load from the first element.
                from = builder.Access(ptr, input_vars[idx], 0_u)->Result();
            }
        }
        return builder.Load(from)->Result();
    }

    /// @copydoc ShaderIO::BackendState::SetOutput
    void SetOutput(core::ir::Builder& builder, uint32_t idx, core::ir::Value* value) override {
        // Store the output to the global variable declared earlier.
        auto* ptr = ty.ptr(core::AddressSpace::kOut, outputs[idx].type, core::Access::kWrite);
        auto* to = output_vars[idx]->Result();
        if (outputs[idx].attributes.builtin) {
            if (outputs[idx].attributes.builtin.value() == core::BuiltinValue::kSampleMask) {
                // SampleMask becomes an array for SPIR-V, so store to the first element.
                to = builder.Access(ptr, to, 0_u)->Result();
            }

            // Clamp frag_depth values if necessary.
            if (outputs[idx].attributes.builtin.value() == core::BuiltinValue::kFragDepth) {
                value = ClampFragDepth(builder, value);
            }
        }
        builder.Store(to, value);
    }

    /// Clamp a frag_depth builtin value if necessary.
    /// @param builder the builder to use for new instructions
    /// @param frag_depth the incoming frag_depth value
    /// @returns the clamped value
    core::ir::Value* ClampFragDepth(core::ir::Builder& builder, core::ir::Value* frag_depth) {
        if (!config.clamp_frag_depth) {
            return frag_depth;
        }

        // Create the clamp args struct and variable.
        if (!frag_depth_clamp_args) {
            // Check that there are no push constants in the module already.
            for (auto* inst : *ir.root_block) {
                if (auto* var = inst->As<core::ir::Var>()) {
                    auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
                    if (ptr->AddressSpace() == core::AddressSpace::kPushConstant) {
                        TINT_ICE() << "cannot clamp frag_depth with pre-existing push constants";
                    }
                }
            }

            // Declare the struct.
            auto* str = ty.Struct(ir.symbols.Register("FragDepthClampArgs"),
                                  {
                                      {ir.symbols.Register("min"), ty.f32()},
                                      {ir.symbols.Register("max"), ty.f32()},
                                  });
            str->SetStructFlag(core::type::kBlock);

            // Declare the variable.
            auto* var = b.Var("tint_frag_depth_clamp_args", ty.ptr(push_constant, str));
            ir.root_block->Append(var);
            frag_depth_clamp_args = var->Result();
        }

        // Clamp the value.
        auto* args = builder.Load(frag_depth_clamp_args);
        auto* frag_depth_min = builder.Access(ty.f32(), args, 0_u);
        auto* frag_depth_max = builder.Access(ty.f32(), args, 1_u);
        return builder
            .Call(ty.f32(), core::BuiltinFn::kClamp, frag_depth, frag_depth_min, frag_depth_max)
            ->Result();
    }

    /// @copydoc ShaderIO::BackendState::NeedsVertexPointSize
    bool NeedsVertexPointSize() const override { return config.emit_vertex_point_size; }
};
}  // namespace

Result<SuccessType> ShaderIO(core::ir::Module& ir, const ShaderIOConfig& config) {
    auto result = ValidateAndDumpIfNeeded(ir, "ShaderIO transform");
    if (!result) {
        return result;
    }

    core::ir::transform::RunShaderIOBase(ir, [&](core::ir::Module& mod, core::ir::Function* func) {
        return std::make_unique<StateImpl>(mod, func, config);
    });

    return Success;
}

}  // namespace tint::spirv::writer::raise
