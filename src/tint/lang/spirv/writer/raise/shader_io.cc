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

#include "src/tint/lang/spirv/writer/raise/shader_io.h"

#include <memory>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/transform/shader_io.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/struct.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer::raise {

namespace {

/// PIMPL state for the parts of the shader IO transform specific to SPIR-V.
/// For SPIR-V, we split builtins and locations into two separate structures each for input and
/// output, and declare global variables for them. The wrapper entry point then loads from and
/// stores to these variables.
/// We also modify the type of the SampleMask builtin to be an array, as required by Vulkan.
struct StateImpl : core::ir::transform::ShaderIOBackendState {
    /// The global variable for input builtins.
    core::ir::Var* builtin_input_var = nullptr;
    /// The global variable for input locations.
    core::ir::Var* location_input_var = nullptr;
    /// The global variable for output builtins.
    core::ir::Var* builtin_output_var = nullptr;
    /// The global variable for output locations.
    core::ir::Var* location_output_var = nullptr;
    /// The member indices for inputs.
    Vector<uint32_t, 4> input_indices;
    /// The member indices for outputs.
    Vector<uint32_t, 4> output_indices;

    /// The configuration options.
    const ShaderIOConfig& config;

    /// The frag_depth clamp arguments.
    core::ir::Value* frag_depth_clamp_args = nullptr;

    /// Constructor
    StateImpl(core::ir::Module* mod, core::ir::Function* f, const ShaderIOConfig& cfg)
        : ShaderIOBackendState(mod, f), config(cfg) {}

    /// Destructor
    ~StateImpl() override {}

    /// Split the members listed in @p entries into two separate structures for builtins and
    /// locations, and make global variables for them. Record the new member indices in @p indices.
    /// @param builtin_var the generated global variable for builtins
    /// @param location_var the generated global variable for locations
    /// @param indices the new member indices
    /// @param entries the entries to split
    /// @param addrspace the address to use for the global variables
    /// @param access the access mode to use for the global variables
    /// @param name_suffix the suffix to add to struct and variable names
    void MakeStructs(core::ir::Var*& builtin_var,
                     core::ir::Var*& location_var,
                     Vector<uint32_t, 4>* indices,
                     Vector<core::type::Manager::StructMemberDesc, 4>& entries,
                     core::AddressSpace addrspace,
                     core::Access access,
                     const char* name_suffix) {
        // Build separate lists of builtin and location entries and record their new indices.
        uint32_t next_builtin_idx = 0;
        uint32_t next_location_idx = 0;
        Vector<core::type::Manager::StructMemberDesc, 4> builtin_members;
        Vector<core::type::Manager::StructMemberDesc, 4> location_members;
        for (auto io : entries) {
            if (io.attributes.builtin) {
                // SampleMask must be an array for Vulkan.
                if (io.attributes.builtin.value() == core::BuiltinValue::kSampleMask) {
                    io.type = ty.array<u32, 1>();
                }
                builtin_members.Push(io);
                indices->Push(next_builtin_idx++);
            } else {
                location_members.Push(io);
                indices->Push(next_location_idx++);
            }
        }

        // Declare the structs and variables if needed.
        auto make_struct = [&](auto& members, const char* iotype) {
            auto name = ir->NameOf(func).Name() + iotype + name_suffix;
            auto* str = ty.Struct(ir->symbols.New(name + "Struct"), std::move(members));
            auto* var = b.Var(name, ty.ptr(addrspace, str, access));
            str->SetStructFlag(core::type::kBlock);
            b.RootBlock()->Append(var);
            return var;
        };
        if (!builtin_members.IsEmpty()) {
            builtin_var = make_struct(builtin_members, "_Builtin");
        }
        if (!location_members.IsEmpty()) {
            location_var = make_struct(location_members, "_Location");
        }
    }

    /// @copydoc ShaderIO::BackendState::FinalizeInputs
    Vector<core::ir::FunctionParam*, 4> FinalizeInputs() override {
        MakeStructs(builtin_input_var, location_input_var, &input_indices, inputs,
                    core::AddressSpace::kIn, core::Access::kRead, "Inputs");
        return tint::Empty;
    }

    /// @copydoc ShaderIO::BackendState::FinalizeOutputs
    core::ir::Value* FinalizeOutputs() override {
        MakeStructs(builtin_output_var, location_output_var, &output_indices, outputs,
                    core::AddressSpace::kOut, core::Access::kWrite, "Outputs");
        return nullptr;
    }

    /// @copydoc ShaderIO::BackendState::GetInput
    core::ir::Value* GetInput(core::ir::Builder& builder, uint32_t idx) override {
        // Load the input from the global variable declared earlier.
        auto* ptr = ty.ptr(core::AddressSpace::kIn, inputs[idx].type, core::Access::kRead);
        core::ir::Access* from = nullptr;
        if (inputs[idx].attributes.builtin) {
            if (inputs[idx].attributes.builtin.value() == core::BuiltinValue::kSampleMask) {
                // SampleMask becomes an array for SPIR-V, so load from the first element.
                from = builder.Access(ptr, builtin_input_var, u32(input_indices[idx]), 0_u);
            } else {
                from = builder.Access(ptr, builtin_input_var, u32(input_indices[idx]));
            }
        } else {
            from = builder.Access(ptr, location_input_var, u32(input_indices[idx]));
        }
        return builder.Load(from)->Result();
    }

    /// @copydoc ShaderIO::BackendState::SetOutput
    void SetOutput(core::ir::Builder& builder, uint32_t idx, core::ir::Value* value) override {
        // Store the output to the global variable declared earlier.
        auto* ptr = ty.ptr(core::AddressSpace::kOut, outputs[idx].type, core::Access::kWrite);
        core::ir::Access* to = nullptr;
        if (outputs[idx].attributes.builtin) {
            if (outputs[idx].attributes.builtin.value() == core::BuiltinValue::kSampleMask) {
                // SampleMask becomes an array for SPIR-V, so store to the first element.
                to = builder.Access(ptr, builtin_output_var, u32(output_indices[idx]), 0_u);
            } else {
                to = builder.Access(ptr, builtin_output_var, u32(output_indices[idx]));
            }

            // Clamp frag_depth values if necessary.
            if (outputs[idx].attributes.builtin.value() == core::BuiltinValue::kFragDepth) {
                value = ClampFragDepth(builder, value);
            }
        } else {
            to = builder.Access(ptr, location_output_var, u32(output_indices[idx]));
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
            for (auto* inst : *b.RootBlock()) {
                if (auto* var = inst->As<core::ir::Var>()) {
                    auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
                    if (ptr->AddressSpace() == core::AddressSpace::kPushConstant) {
                        TINT_ICE() << "cannot clamp frag_depth with pre-existing push constants";
                    }
                }
            }

            // Declare the struct.
            auto* str = ty.Struct(ir->symbols.Register("FragDepthClampArgs"),
                                  {
                                      {ir->symbols.Register("min"), ty.f32()},
                                      {ir->symbols.Register("max"), ty.f32()},
                                  });
            str->SetStructFlag(core::type::kBlock);

            // Declare the variable.
            auto* var = b.Var("tint_frag_depth_clamp_args", ty.ptr(push_constant, str));
            b.RootBlock()->Append(var);
            frag_depth_clamp_args = var->Result();
        }

        // Clamp the value.
        auto* args = builder.Load(frag_depth_clamp_args);
        auto* frag_depth_min = builder.Access(ty.f32(), args, 0_u);
        auto* frag_depth_max = builder.Access(ty.f32(), args, 1_u);
        return builder
            .Call(ty.f32(), core::Function::kClamp, frag_depth, frag_depth_min, frag_depth_max)
            ->Result();
    }
};
}  // namespace

Result<SuccessType, std::string> ShaderIO(core::ir::Module* ir, const ShaderIOConfig& config) {
    auto result = ValidateAndDumpIfNeeded(*ir, "ShaderIO transform");
    if (!result) {
        return result;
    }

    core::ir::transform::RunShaderIOBase(ir, [&](core::ir::Module* mod, core::ir::Function* func) {
        return std::make_unique<StateImpl>(mod, func, config);
    });

    return Success;
}

}  // namespace tint::spirv::writer::raise
