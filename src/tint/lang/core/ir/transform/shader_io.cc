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

#include "src/tint/lang/core/ir/transform/shader_io.h"

#include <memory>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/type/struct.h"

using namespace tint::core::fluent_types;  // NOLINT
using namespace tint::number_suffixes;     // NOLINT

namespace tint::ir::transform {

namespace {

core::BuiltinValue FunctionParamBuiltin(enum FunctionParam::Builtin builtin) {
    switch (builtin) {
        case FunctionParam::Builtin::kVertexIndex:
            return core::BuiltinValue::kVertexIndex;
        case FunctionParam::Builtin::kInstanceIndex:
            return core::BuiltinValue::kInstanceIndex;
        case FunctionParam::Builtin::kPosition:
            return core::BuiltinValue::kPosition;
        case FunctionParam::Builtin::kFrontFacing:
            return core::BuiltinValue::kFrontFacing;
        case FunctionParam::Builtin::kLocalInvocationId:
            return core::BuiltinValue::kLocalInvocationId;
        case FunctionParam::Builtin::kLocalInvocationIndex:
            return core::BuiltinValue::kLocalInvocationIndex;
        case FunctionParam::Builtin::kGlobalInvocationId:
            return core::BuiltinValue::kGlobalInvocationId;
        case FunctionParam::Builtin::kWorkgroupId:
            return core::BuiltinValue::kWorkgroupId;
        case FunctionParam::Builtin::kNumWorkgroups:
            return core::BuiltinValue::kNumWorkgroups;
        case FunctionParam::Builtin::kSampleIndex:
            return core::BuiltinValue::kSampleIndex;
        case FunctionParam::Builtin::kSampleMask:
            return core::BuiltinValue::kSampleMask;
    }
    return core::BuiltinValue::kUndefined;
}

core::BuiltinValue ReturnBuiltin(enum Function::ReturnBuiltin builtin) {
    switch (builtin) {
        case Function::ReturnBuiltin::kPosition:
            return core::BuiltinValue::kPosition;
        case Function::ReturnBuiltin::kFragDepth:
            return core::BuiltinValue::kFragDepth;
        case Function::ReturnBuiltin::kSampleMask:
            return core::BuiltinValue::kSampleMask;
    }
    return core::BuiltinValue::kUndefined;
}

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    Module* ir = nullptr;
    /// The IR builder.
    Builder b{*ir};
    /// The type manager.
    type::Manager& ty{ir->Types()};
    /// The set of struct members that need to have their IO attributes stripped.
    Hashset<const type::StructMember*, 8> members_to_strip;

    /// The entry point currently being processed.
    Function* func = nullptr;

    /// The backend state object for the current entry point.
    std::unique_ptr<ShaderIOBackendState> backend;

    /// Constructor
    /// @param mod the module
    explicit State(Module* mod) : ir(mod) {}

    /// Process an entry point.
    /// @param f the original entry point function
    /// @param bs the backend state object
    void Process(Function* f, std::unique_ptr<ShaderIOBackendState> bs) {
        TINT_SCOPED_ASSIGNMENT(func, f);
        backend = std::move(bs);
        TINT_DEFER(backend = nullptr);

        // Process the parameters and return value to prepare for building a wrapper function.
        GatherInputs();
        GatherOutput();
        auto new_params = backend->FinalizeInputs();
        auto* new_ret_val = backend->FinalizeOutputs();

        // Rename the old function and remove its pipeline stage and workgroup size, as we will be
        // wrapping it with a new entry point.
        auto name = ir->NameOf(func).Name();
        auto stage = func->Stage();
        auto wgsize = func->WorkgroupSize();
        ir->SetName(func, name + "_inner");
        func->SetStage(Function::PipelineStage::kUndefined);
        func->ClearWorkgroupSize();

        // Create the entry point wrapper function.
        auto* ep = b.Function(name, new_ret_val ? new_ret_val->Type() : ty.void_());
        ep->SetStage(stage);
        if (wgsize) {
            ep->SetWorkgroupSize((*wgsize)[0], (*wgsize)[1], (*wgsize)[2]);
        }
        auto wrapper = b.Append(ep->Block());

        // Call the original function, passing it the inputs and capturing its return value.
        auto inner_call_args = BuildInnerCallArgs(wrapper);
        auto* inner_result = wrapper.Call(func->ReturnType(), func, std::move(inner_call_args));
        SetOutputs(wrapper, inner_result->Result());

        // Return the new result.
        wrapper.Return(ep, new_ret_val);
    }

    /// Gather the shader inputs.
    void GatherInputs() {
        for (auto* param : func->Params()) {
            if (auto* str = param->Type()->As<type::Struct>()) {
                for (auto* member : str->Members()) {
                    auto name = str->Name().Name() + "_" + member->Name().Name();
                    backend->AddInput(ir->symbols.Register(name), member->Type(),
                                      member->Attributes());
                    members_to_strip.Add(member);
                }
            } else {
                // Pull out the IO attributes and remove them from the parameter.
                type::StructMemberAttributes attributes;
                if (auto loc = param->Location()) {
                    attributes.location = loc->value;
                    if (loc->interpolation) {
                        attributes.interpolation = *loc->interpolation;
                    }
                    param->ClearLocation();
                } else if (auto builtin = param->Builtin()) {
                    attributes.builtin = FunctionParamBuiltin(*builtin);
                    param->ClearBuiltin();
                }
                attributes.invariant = param->Invariant();
                param->SetInvariant(false);

                auto name = ir->NameOf(param);
                backend->AddInput(name, param->Type(), std::move(attributes));
            }
        }
    }

    /// Gather the shader outputs.
    void GatherOutput() {
        if (func->ReturnType()->Is<type::Void>()) {
            return;
        }

        if (auto* str = func->ReturnType()->As<type::Struct>()) {
            for (auto* member : str->Members()) {
                auto name = str->Name().Name() + "_" + member->Name().Name();
                backend->AddOutput(ir->symbols.Register(name), member->Type(),
                                   member->Attributes());
                members_to_strip.Add(member);
            }
        } else {
            // Pull out the IO attributes and remove them from the original function.
            type::StructMemberAttributes attributes;
            if (auto loc = func->ReturnLocation()) {
                attributes.location = loc->value;
                func->ClearReturnLocation();
            } else if (auto builtin = func->ReturnBuiltin()) {
                attributes.builtin = ReturnBuiltin(*builtin);
                func->ClearReturnBuiltin();
            }
            attributes.invariant = func->ReturnInvariant();
            func->SetReturnInvariant(false);

            backend->AddOutput(ir->symbols.New(), func->ReturnType(), std::move(attributes));
        }
    }

    /// Build the argument list to call the original entry point function.
    /// @param builder the IR builder for new instructions
    /// @returns the argument list
    Vector<Value*, 4> BuildInnerCallArgs(Builder& builder) {
        uint32_t input_idx = 0;
        Vector<Value*, 4> args;
        for (auto* param : func->Params()) {
            if (auto* str = param->Type()->As<type::Struct>()) {
                Vector<Value*, 4> construct_args;
                for (uint32_t i = 0; i < str->Members().Length(); i++) {
                    construct_args.Push(backend->GetInput(builder, input_idx++));
                }
                args.Push(builder.Construct(param->Type(), construct_args)->Result());
            } else {
                args.Push(backend->GetInput(builder, input_idx++));
            }
        }

        return args;
    }

    /// Propagate outputs from the inner function call to their final destination.
    /// @param builder the IR builder for new instructions
    /// @param inner_result the return value from calling the original entry point function
    void SetOutputs(Builder& builder, Value* inner_result) {
        if (auto* str = inner_result->Type()->As<type::Struct>()) {
            for (auto* member : str->Members()) {
                Value* from =
                    builder.Access(member->Type(), inner_result, u32(member->Index()))->Result();
                backend->SetOutput(builder, member->Index(), from);
            }
        } else if (!inner_result->Type()->Is<type::Void>()) {
            backend->SetOutput(builder, 0u, inner_result);
        }
    }

    /// Finalize any state needed to complete the transform.
    void Finalize() {
        // Remove IO attributes from all structure members that had them prior to this transform.
        for (auto* member : members_to_strip) {
            // TODO(crbug.com/tint/745): Remove the const_cast.
            const_cast<type::StructMember*>(member)->SetAttributes({});
        }
    }
};

}  // namespace

void RunShaderIOBase(Module* module, std::function<MakeBackendStateFunc> make_backend_state) {
    State state(module);
    for (auto* func : module->functions) {
        // Only process entry points.
        if (func->Stage() == Function::PipelineStage::kUndefined) {
            continue;
        }

        // Skip entry points with no inputs or outputs.
        if (func->Params().IsEmpty() && func->ReturnType()->Is<type::Void>()) {
            continue;
        }

        state.Process(func, make_backend_state(module, func));
    }
    state.Finalize();
}

ShaderIOBackendState::~ShaderIOBackendState() = default;

}  // namespace tint::ir::transform
