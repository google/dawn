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

#include "src/tint/writer/spirv/ir/generator_impl_ir.h"

#include "spirv/unified1/spirv.h"
#include "src/tint/ir/binary.h"
#include "src/tint/ir/block.h"
#include "src/tint/ir/function_terminator.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/load.h"
#include "src/tint/ir/module.h"
#include "src/tint/ir/store.h"
#include "src/tint/ir/transform/add_empty_entry_point.h"
#include "src/tint/ir/var.h"
#include "src/tint/switch.h"
#include "src/tint/transform/manager.h"
#include "src/tint/type/bool.h"
#include "src/tint/type/f16.h"
#include "src/tint/type/f32.h"
#include "src/tint/type/i32.h"
#include "src/tint/type/pointer.h"
#include "src/tint/type/type.h"
#include "src/tint/type/u32.h"
#include "src/tint/type/vector.h"
#include "src/tint/type/void.h"
#include "src/tint/writer/spirv/generator.h"
#include "src/tint/writer/spirv/module.h"

namespace tint::writer::spirv {

namespace {

void Sanitize(ir::Module* module) {
    transform::Manager manager;
    transform::DataMap data;

    manager.Add<ir::transform::AddEmptyEntryPoint>();

    transform::DataMap outputs;
    manager.Run(module, data, outputs);
}

SpvStorageClass StorageClass(builtin::AddressSpace addrspace) {
    switch (addrspace) {
        case builtin::AddressSpace::kFunction:
            return SpvStorageClassFunction;
        case builtin::AddressSpace::kPrivate:
            return SpvStorageClassPrivate;
        case builtin::AddressSpace::kStorage:
            return SpvStorageClassStorageBuffer;
        case builtin::AddressSpace::kUniform:
            return SpvStorageClassUniform;
        case builtin::AddressSpace::kWorkgroup:
            return SpvStorageClassWorkgroup;
        default:
            return SpvStorageClassMax;
    }
}

}  // namespace

GeneratorImplIr::GeneratorImplIr(ir::Module* module, bool zero_init_workgroup_mem)
    : ir_(module), zero_init_workgroup_memory_(zero_init_workgroup_mem) {}

bool GeneratorImplIr::Generate() {
    // Run the IR transformations to prepare for SPIR-V emission.
    Sanitize(ir_);

    // TODO(crbug.com/tint/1906): Check supported extensions.

    module_.PushCapability(SpvCapabilityShader);
    module_.PushMemoryModel(spv::Op::OpMemoryModel, {U32Operand(SpvAddressingModelLogical),
                                                     U32Operand(SpvMemoryModelGLSL450)});

    // TODO(crbug.com/tint/1906): Emit extensions.

    // TODO(crbug.com/tint/1906): Emit variables.
    (void)zero_init_workgroup_memory_;
    if (ir_->root_block) {
        TINT_ICE(Writer, diagnostics_) << "root block is unimplemented";
        return false;
    }

    // Emit functions.
    for (auto* func : ir_->functions) {
        EmitFunction(func);
    }

    if (diagnostics_.contains_errors()) {
        return false;
    }

    // Serialize the module into binary SPIR-V.
    writer_.WriteHeader(module_.IdBound());
    writer_.WriteModule(&module_);

    return true;
}

uint32_t GeneratorImplIr::Constant(const ir::Constant* constant) {
    return Constant(constant->Value());
}

uint32_t GeneratorImplIr::Constant(const constant::Value* constant) {
    return constants_.GetOrCreate(constant, [&]() {
        auto id = module_.NextId();
        auto* ty = constant->Type();
        Switch(
            ty,  //
            [&](const type::Bool*) {
                module_.PushType(
                    constant->ValueAs<bool>() ? spv::Op::OpConstantTrue : spv::Op::OpConstantFalse,
                    {Type(ty), id});
            },
            [&](const type::I32*) {
                module_.PushType(spv::Op::OpConstant, {Type(ty), id, constant->ValueAs<u32>()});
            },
            [&](const type::U32*) {
                module_.PushType(spv::Op::OpConstant,
                                 {Type(ty), id, U32Operand(constant->ValueAs<i32>())});
            },
            [&](const type::F32*) {
                module_.PushType(spv::Op::OpConstant, {Type(ty), id, constant->ValueAs<f32>()});
            },
            [&](const type::F16*) {
                module_.PushType(
                    spv::Op::OpConstant,
                    {Type(ty), id, U32Operand(constant->ValueAs<f16>().BitsRepresentation())});
            },
            [&](const type::Vector* vec) {
                OperandList operands = {Type(ty), id};
                for (uint32_t i = 0; i < vec->Width(); i++) {
                    operands.push_back(Constant(constant->Index(i)));
                }
                module_.PushType(spv::Op::OpConstantComposite, operands);
            },
            [&](Default) {
                TINT_ICE(Writer, diagnostics_) << "unhandled constant type: " << ty->FriendlyName();
            });
        return id;
    });
}

uint32_t GeneratorImplIr::Type(const type::Type* ty) {
    return types_.GetOrCreate(ty, [&]() {
        auto id = module_.NextId();
        Switch(
            ty,  //
            [&](const type::Void*) { module_.PushType(spv::Op::OpTypeVoid, {id}); },
            [&](const type::Bool*) { module_.PushType(spv::Op::OpTypeBool, {id}); },
            [&](const type::I32*) {
                module_.PushType(spv::Op::OpTypeInt, {id, 32u, 1u});
            },
            [&](const type::U32*) {
                module_.PushType(spv::Op::OpTypeInt, {id, 32u, 0u});
            },
            [&](const type::F32*) {
                module_.PushType(spv::Op::OpTypeFloat, {id, 32u});
            },
            [&](const type::F16*) {
                module_.PushType(spv::Op::OpTypeFloat, {id, 16u});
            },
            [&](const type::Vector* vec) {
                module_.PushType(spv::Op::OpTypeVector, {id, Type(vec->type()), vec->Width()});
            },
            [&](const type::Pointer* ptr) {
                module_.PushType(
                    spv::Op::OpTypePointer,
                    {id, U32Operand(StorageClass(ptr->AddressSpace())), Type(ptr->StoreType())});
            },
            [&](Default) {
                TINT_ICE(Writer, diagnostics_) << "unhandled type: " << ty->FriendlyName();
            });
        return id;
    });
}

uint32_t GeneratorImplIr::Value(const ir::Value* value) {
    return Switch(
        value,  //
        [&](const ir::Constant* constant) { return Constant(constant); },
        [&](const ir::Instruction* inst) {
            auto id = instructions_.Find(inst);
            if (TINT_UNLIKELY(!id)) {
                TINT_ICE(Writer, diagnostics_) << "missing instruction result";
                return 0u;
            }
            return *id;
        },
        [&](Default) {
            TINT_ICE(Writer, diagnostics_) << "unhandled value node: " << value->TypeInfo().name;
            return 0u;
        });
}

uint32_t GeneratorImplIr::Label(const ir::Block* block) {
    return block_labels_.GetOrCreate(block, [&]() { return module_.NextId(); });
}

void GeneratorImplIr::EmitFunction(const ir::Function* func) {
    // Make an ID for the function.
    auto id = module_.NextId();

    // Emit the function name.
    module_.PushDebug(spv::Op::OpName, {id, Operand(func->Name().Name())});

    // Emit OpEntryPoint and OpExecutionMode declarations if needed.
    if (func->Stage() != ir::Function::PipelineStage::kUndefined) {
        EmitEntryPoint(func, id);
    }

    // Get the ID for the return type.
    auto return_type_id = Type(func->ReturnType());

    // Get the ID for the function type (creating it if needed).
    // TODO(jrprice): Add the parameter types when they are supported in the IR.
    FunctionType function_type{return_type_id, {}};
    auto function_type_id = function_types_.GetOrCreate(function_type, [&]() {
        auto func_ty_id = module_.NextId();
        OperandList operands = {func_ty_id, return_type_id};
        operands.insert(operands.end(), function_type.param_type_ids.begin(),
                        function_type.param_type_ids.end());
        module_.PushType(spv::Op::OpTypeFunction, operands);
        return func_ty_id;
    });

    // Declare the function.
    auto decl =
        Instruction{spv::Op::OpFunction,
                    {return_type_id, id, U32Operand(SpvFunctionControlMaskNone), function_type_id}};

    // Create a function that we will add instructions to.
    // TODO(jrprice): Add the parameter declarations when they are supported in the IR.
    auto entry_block = module_.NextId();
    current_function_ = Function(decl, entry_block, {});
    TINT_DEFER(current_function_ = Function());

    // Emit the body of the function.
    EmitBlock(func->StartTarget());

    // Add the function to the module.
    module_.PushFunction(current_function_);
}

void GeneratorImplIr::EmitEntryPoint(const ir::Function* func, uint32_t id) {
    SpvExecutionModel stage = SpvExecutionModelMax;
    switch (func->Stage()) {
        case ir::Function::PipelineStage::kCompute: {
            stage = SpvExecutionModelGLCompute;
            module_.PushExecutionMode(
                spv::Op::OpExecutionMode,
                {id, U32Operand(SpvExecutionModeLocalSize), func->WorkgroupSize()->at(0),
                 func->WorkgroupSize()->at(1), func->WorkgroupSize()->at(2)});
            break;
        }
        case ir::Function::PipelineStage::kFragment: {
            stage = SpvExecutionModelFragment;
            module_.PushExecutionMode(spv::Op::OpExecutionMode,
                                      {id, U32Operand(SpvExecutionModeOriginUpperLeft)});
            // TODO(jrprice): Add DepthReplacing execution mode if FragDepth is used.
            break;
        }
        case ir::Function::PipelineStage::kVertex: {
            stage = SpvExecutionModelVertex;
            break;
        }
        case ir::Function::PipelineStage::kUndefined:
            TINT_ICE(Writer, diagnostics_) << "undefined pipeline stage for entry point";
            return;
    }

    // TODO(jrprice): Add the interface list of all referenced global variables.
    module_.PushEntryPoint(spv::Op::OpEntryPoint, {U32Operand(stage), id, func->Name().Name()});
}

void GeneratorImplIr::EmitBlock(const ir::Block* block) {
    // Emit the label.
    // Skip if this is the function's entry block, as it will be emitted by the function object.
    if (!current_function_.instructions().empty()) {
        current_function_.push_inst(spv::Op::OpLabel, {Label(block)});
    }

    // If there are no instructions in the block, it's a dead end, so we shouldn't be able to get
    // here to begin with.
    if (block->Instructions().IsEmpty()) {
        current_function_.push_inst(spv::Op::OpUnreachable, {});
        return;
    }

    // Emit the instructions.
    for (auto* inst : block->Instructions()) {
        auto result = Switch(
            inst,  //
            [&](const ir::Binary* b) { return EmitBinary(b); },
            [&](const ir::Load* l) { return EmitLoad(l); },
            [&](const ir::Store* s) {
                EmitStore(s);
                return 0u;
            },
            [&](const ir::Var* v) { return EmitVar(v); },
            [&](const ir::If* i) {
                EmitIf(i);
                return 0u;
            },
            [&](const ir::Branch* b) {
                EmitBranch(b);
                return 0u;
            },
            [&](Default) {
                TINT_ICE(Writer, diagnostics_)
                    << "unimplemented instruction: " << inst->TypeInfo().name;
                return 0u;
            });
        instructions_.Add(inst, result);
    }
}

void GeneratorImplIr::EmitBranch(const ir::Branch* b) {
    Switch(
        b->To(),
        [&](const ir::Block* blk) { current_function_.push_inst(spv::Op::OpBranch, {Label(blk)}); },
        [&](const ir::FunctionTerminator*) {
            // TODO(jrprice): Handle the return value, which will be a branch argument.
            if (!b->Args().IsEmpty()) {
                TINT_ICE(Writer, diagnostics_) << "unimplemented return value";
            }
            current_function_.push_inst(spv::Op::OpReturn, {});
        },
        [&](Default) {
            // A block may not have an outward branch (e.g. an unreachable merge
            // block).
            current_function_.push_inst(spv::Op::OpUnreachable, {});
        });
}

void GeneratorImplIr::EmitIf(const ir::If* i) {
    auto* merge_block = i->Merge();
    auto* true_block = i->True();
    auto* false_block = i->False();

    // Generate labels for the blocks. We emit the true or false block if it:
    // 1. contains instructions other then the branch, or
    // 2. branches somewhere other then the Merge().
    // Otherwise we skip them and branch straight to the merge block.
    uint32_t merge_label = Label(merge_block);
    uint32_t true_label = merge_label;
    uint32_t false_label = merge_label;
    if (true_block->Instructions().Length() > 1 || true_block->Branch()->To() != merge_block) {
        true_label = Label(true_block);
    }
    if (false_block->Instructions().Length() > 1 || false_block->Branch()->To() != merge_block) {
        false_label = Label(false_block);
    }

    // Emit the OpSelectionMerge and OpBranchConditional instructions.
    current_function_.push_inst(spv::Op::OpSelectionMerge,
                                {merge_label, U32Operand(SpvSelectionControlMaskNone)});
    current_function_.push_inst(spv::Op::OpBranchConditional,
                                {Value(i->Condition()), true_label, false_label});

    // Emit the `true` and `false` blocks, if they're not being skipped.
    if (true_label != merge_label) {
        EmitBlock(true_block);
    }
    if (false_label != merge_label) {
        EmitBlock(false_block);
    }

    // Emit the merge block.
    EmitBlock(merge_block);
}

uint32_t GeneratorImplIr::EmitBinary(const ir::Binary* binary) {
    auto id = module_.NextId();

    // Determine the opcode.
    spv::Op op = spv::Op::Max;
    switch (binary->Kind()) {
        case ir::Binary::Kind::kAdd: {
            op = binary->Type()->is_integer_scalar_or_vector() ? spv::Op::OpIAdd : spv::Op::OpFAdd;
            break;
        }
        case ir::Binary::Kind::kSubtract: {
            op = binary->Type()->is_integer_scalar_or_vector() ? spv::Op::OpISub : spv::Op::OpFSub;
            break;
        }
        default: {
            TINT_ICE(Writer, diagnostics_)
                << "unimplemented binary instruction: " << static_cast<uint32_t>(binary->Kind());
        }
    }

    // Emit the instruction.
    current_function_.push_inst(
        op, {Type(binary->Type()), id, Value(binary->LHS()), Value(binary->RHS())});

    return id;
}

uint32_t GeneratorImplIr::EmitLoad(const ir::Load* load) {
    auto id = module_.NextId();
    current_function_.push_inst(spv::Op::OpLoad, {Type(load->Type()), id, Value(load->From())});
    return id;
}

void GeneratorImplIr::EmitStore(const ir::Store* store) {
    current_function_.push_inst(spv::Op::OpStore, {Value(store->To()), Value(store->From())});
}

uint32_t GeneratorImplIr::EmitVar(const ir::Var* var) {
    auto id = module_.NextId();
    auto* ptr = var->Type()->As<type::Pointer>();
    TINT_ASSERT(Writer, ptr);
    auto ty = Type(ptr);

    if (ptr->AddressSpace() == builtin::AddressSpace::kFunction) {
        TINT_ASSERT(Writer, current_function_);
        current_function_.push_var({ty, id, U32Operand(SpvStorageClassFunction)});
        if (var->Initializer()) {
            current_function_.push_inst(spv::Op::OpStore, {id, Value(var->Initializer())});
        }
    } else {
        TINT_ICE(Writer, diagnostics_)
            << "unimplemented variable address space " << ptr->AddressSpace();
        return 0u;
    }

    // Set the name if present.
    if (auto name = ir_->NameOf(var)) {
        module_.PushDebug(spv::Op::OpName, {id, Operand(name.Name())});
    }

    return id;
}

}  // namespace tint::writer::spirv
