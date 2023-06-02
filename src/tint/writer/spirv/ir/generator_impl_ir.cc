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

#include <utility>

#include "spirv/unified1/GLSL.std.450.h"
#include "spirv/unified1/spirv.h"
#include "src/tint/constant/scalar.h"
#include "src/tint/ir/binary.h"
#include "src/tint/ir/block.h"
#include "src/tint/ir/break_if.h"
#include "src/tint/ir/builtin.h"
#include "src/tint/ir/continue.h"
#include "src/tint/ir/exit_if.h"
#include "src/tint/ir/exit_loop.h"
#include "src/tint/ir/exit_switch.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/load.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/module.h"
#include "src/tint/ir/next_iteration.h"
#include "src/tint/ir/return.h"
#include "src/tint/ir/store.h"
#include "src/tint/ir/switch.h"
#include "src/tint/ir/transform/add_empty_entry_point.h"
#include "src/tint/ir/user_call.h"
#include "src/tint/ir/var.h"
#include "src/tint/switch.h"
#include "src/tint/transform/manager.h"
#include "src/tint/type/array.h"
#include "src/tint/type/bool.h"
#include "src/tint/type/f16.h"
#include "src/tint/type/f32.h"
#include "src/tint/type/i32.h"
#include "src/tint/type/matrix.h"
#include "src/tint/type/pointer.h"
#include "src/tint/type/struct.h"
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

    // Emit module-scope declarations.
    if (ir_->root_block) {
        EmitRootBlock(ir_->root_block);
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
            [&](const type::Matrix* mat) {
                OperandList operands = {Type(ty), id};
                for (uint32_t i = 0; i < mat->columns(); i++) {
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

uint32_t GeneratorImplIr::ConstantNull(const type::Type* type) {
    return constant_nulls_.GetOrCreate(type, [&]() {
        auto id = module_.NextId();
        module_.PushType(spv::Op::OpConstantNull, {Type(type), id});
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
            [&](const type::Matrix* mat) {
                module_.PushType(spv::Op::OpTypeMatrix,
                                 {id, Type(mat->ColumnType()), mat->columns()});
            },
            [&](const type::Array* arr) {
                if (arr->ConstantCount()) {
                    auto* count = ir_->constant_values.Get(u32(arr->ConstantCount().value()));
                    module_.PushType(spv::Op::OpTypeArray,
                                     {id, Type(arr->ElemType()), Constant(count)});
                } else {
                    TINT_ASSERT(Writer, arr->Count()->Is<type::RuntimeArrayCount>());
                    module_.PushType(spv::Op::OpTypeRuntimeArray, {id, Type(arr->ElemType())});
                }
                module_.PushAnnot(spv::Op::OpDecorate,
                                  {id, U32Operand(SpvDecorationArrayStride), arr->Stride()});
            },
            [&](const type::Pointer* ptr) {
                module_.PushType(
                    spv::Op::OpTypePointer,
                    {id, U32Operand(StorageClass(ptr->AddressSpace())), Type(ptr->StoreType())});
            },
            [&](const type::Struct* str) { EmitStructType(id, str); },
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
        [&](const ir::Value*) {
            return values_.GetOrCreate(value, [&] { return module_.NextId(); });
        });
}

uint32_t GeneratorImplIr::Label(const ir::Block* block) {
    return block_labels_.GetOrCreate(block, [&]() { return module_.NextId(); });
}

void GeneratorImplIr::EmitStructType(uint32_t id, const type::Struct* str) {
    // Helper to return `type` or a potentially nested array element type within `type` as a matrix
    // type, or nullptr if no such matrix type is present.
    auto get_nested_matrix_type = [&](const type::Type* type) {
        while (auto* arr = type->As<type::Array>()) {
            type = arr->ElemType();
        }
        return type->As<type::Matrix>();
    };

    OperandList operands = {id};
    for (auto* member : str->Members()) {
        operands.push_back(Type(member->Type()));

        // Generate struct member offset decoration.
        module_.PushAnnot(
            spv::Op::OpMemberDecorate,
            {operands[0], member->Index(), U32Operand(SpvDecorationOffset), member->Offset()});

        // Emit matrix layout decorations if necessary.
        if (auto* matrix_type = get_nested_matrix_type(member->Type())) {
            const uint32_t effective_row_count = (matrix_type->rows() == 2) ? 2 : 4;
            module_.PushAnnot(spv::Op::OpMemberDecorate,
                              {id, member->Index(), U32Operand(SpvDecorationColMajor)});
            module_.PushAnnot(spv::Op::OpMemberDecorate,
                              {id, member->Index(), U32Operand(SpvDecorationMatrixStride),
                               Operand(effective_row_count * matrix_type->type()->Size())});
        }

        if (member->Name().IsValid()) {
            module_.PushDebug(spv::Op::OpMemberName,
                              {operands[0], member->Index(), Operand(member->Name().Name())});
        }
    }
    module_.PushType(spv::Op::OpTypeStruct, std::move(operands));

    if (str->Name().IsValid()) {
        module_.PushDebug(spv::Op::OpName, {operands[0], Operand(str->Name().Name())});
    }
}

void GeneratorImplIr::EmitFunction(const ir::Function* func) {
    auto id = Value(func);

    // Emit the function name.
    module_.PushDebug(spv::Op::OpName, {id, Operand(ir_->NameOf(func).Name())});

    // Emit OpEntryPoint and OpExecutionMode declarations if needed.
    if (func->Stage() != ir::Function::PipelineStage::kUndefined) {
        EmitEntryPoint(func, id);
    }

    // Get the ID for the return type.
    auto return_type_id = Type(func->ReturnType());

    FunctionType function_type{return_type_id, {}};
    InstructionList params;

    // Generate function parameter declarations and add their type IDs to the function signature.
    for (auto* param : func->Params()) {
        auto param_type_id = Type(param->Type());
        auto param_id = Value(param);
        params.push_back(Instruction(spv::Op::OpFunctionParameter, {param_type_id, param_id}));
        function_type.param_type_ids.Push(param_type_id);
        if (auto name = ir_->NameOf(param)) {
            module_.PushDebug(spv::Op::OpName, {param_id, Operand(name.Name())});
        }
    }

    // Get the ID for the function type (creating it if needed).
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
    auto entry_block = module_.NextId();
    current_function_ = Function(decl, entry_block, std::move(params));
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
    module_.PushEntryPoint(spv::Op::OpEntryPoint,
                           {U32Operand(stage), id, ir_->NameOf(func).Name()});
}

void GeneratorImplIr::EmitRootBlock(const ir::Block* root_block) {
    for (auto* inst : *root_block) {
        Switch(
            inst,  //
            [&](const ir::Var* v) { return EmitVar(v); },
            [&](Default) {
                TINT_ICE(Writer, diagnostics_)
                    << "unimplemented root block instruction: " << inst->TypeInfo().name;
            });
    }
}

void GeneratorImplIr::EmitBlock(const ir::Block* block) {
    // Emit the label.
    // Skip if this is the function's entry block, as it will be emitted by the function object.
    if (!current_function_.instructions().empty()) {
        current_function_.push_inst(spv::Op::OpLabel, {Label(block)});
    }

    // If there are no instructions in the block, it's a dead end, so we shouldn't be able to get
    // here to begin with.
    if (block->IsEmpty()) {
        current_function_.push_inst(spv::Op::OpUnreachable, {});
        return;
    }

    // Emit Phi nodes for all the incoming block parameters
    for (size_t param_idx = 0; param_idx < block->Params().Length(); param_idx++) {
        auto* param = block->Params()[param_idx];
        OperandList ops{Type(param->Type()), Value(param)};

        for (auto* incoming : block->InboundBranches()) {
            auto* arg = incoming->Args()[param_idx];
            ops.push_back(Value(arg));
            ops.push_back(Label(incoming->Block()));
        }

        current_function_.push_inst(spv::Op::OpPhi, std::move(ops));
    }

    // Emit the instructions.
    for (auto* inst : *block) {
        Switch(
            inst,                                             //
            [&](const ir::Binary* b) { EmitBinary(b); },      //
            [&](const ir::Builtin* b) { EmitBuiltin(b); },    //
            [&](const ir::Load* l) { EmitLoad(l); },          //
            [&](const ir::Loop* l) { EmitLoop(l); },          //
            [&](const ir::Switch* sw) { EmitSwitch(sw); },    //
            [&](const ir::Store* s) { EmitStore(s); },        //
            [&](const ir::UserCall* c) { EmitUserCall(c); },  //
            [&](const ir::Var* v) { EmitVar(v); },            //
            [&](const ir::If* i) { EmitIf(i); },              //
            [&](const ir::Branch* b) { EmitBranch(b); },      //
            [&](Default) {
                TINT_ICE(Writer, diagnostics_)
                    << "unimplemented instruction: " << inst->TypeInfo().name;
            });
    }
}

void GeneratorImplIr::EmitBranch(const ir::Branch* b) {
    tint::Switch(  //
        b,         //
        [&](const ir::Return*) {
            if (!b->Args().IsEmpty()) {
                TINT_ASSERT(Writer, b->Args().Length() == 1u);
                OperandList operands;
                operands.push_back(Value(b->Args()[0]));
                current_function_.push_inst(spv::Op::OpReturnValue, operands);
            } else {
                current_function_.push_inst(spv::Op::OpReturn, {});
            }
            return;
        },
        [&](const ir::BreakIf* breakif) {
            current_function_.push_inst(spv::Op::OpBranchConditional,
                                        {
                                            Value(breakif->Condition()),
                                            Label(breakif->Loop()->Merge()),
                                            Label(breakif->Loop()->Body()),
                                        });
        },
        [&](const ir::Continue* cont) {
            current_function_.push_inst(spv::Op::OpBranch, {Label(cont->Loop()->Continuing())});
        },
        [&](const ir::ExitIf* if_) {
            current_function_.push_inst(spv::Op::OpBranch, {Label(if_->If()->Merge())});
        },
        [&](const ir::ExitLoop* loop) {
            current_function_.push_inst(spv::Op::OpBranch, {Label(loop->Loop()->Merge())});
        },
        [&](const ir::ExitSwitch* swtch) {
            current_function_.push_inst(spv::Op::OpBranch, {Label(swtch->Switch()->Merge())});
        },
        [&](const ir::NextIteration* loop) {
            current_function_.push_inst(spv::Op::OpBranch, {Label(loop->Loop()->Body())});
        },
        [&](Default) {
            TINT_ICE(Writer, diagnostics_) << "unimplemented branch: " << b->TypeInfo().name;
        });
}

void GeneratorImplIr::EmitIf(const ir::If* i) {
    auto* merge_block = i->Merge();
    auto* true_block = i->True();
    auto* false_block = i->False();

    // Generate labels for the blocks. We emit the true or false block if it:
    // 1. contains instructions other then the branch, or
    // 2. branches somewhere other then the Merge(), or
    // 3. the merge has input parameters
    // Otherwise we skip them and branch straight to the merge block.
    uint32_t merge_label = Label(merge_block);
    uint32_t true_label = merge_label;
    uint32_t false_label = merge_label;
    if (true_block->Length() > 1 || !merge_block->Params().IsEmpty() ||
        (true_block->HasBranchTarget() && !true_block->Branch()->Is<ir::ExitIf>())) {
        true_label = Label(true_block);
    }
    if (false_block->Length() > 1 || !merge_block->Params().IsEmpty() ||
        (false_block->HasBranchTarget() && !false_block->Branch()->Is<ir::ExitIf>())) {
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

void GeneratorImplIr::EmitBinary(const ir::Binary* binary) {
    auto id = Value(binary);
    auto* lhs_ty = binary->LHS()->Type();

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

        case ir::Binary::Kind::kAnd: {
            op = spv::Op::OpBitwiseAnd;
            break;
        }
        case ir::Binary::Kind::kOr: {
            op = spv::Op::OpBitwiseOr;
            break;
        }
        case ir::Binary::Kind::kXor: {
            op = spv::Op::OpBitwiseXor;
            break;
        }

        case ir::Binary::Kind::kEqual: {
            if (lhs_ty->is_bool_scalar_or_vector()) {
                op = spv::Op::OpLogicalEqual;
            } else if (lhs_ty->is_float_scalar_or_vector()) {
                op = spv::Op::OpFOrdEqual;
            } else if (lhs_ty->is_integer_scalar_or_vector()) {
                op = spv::Op::OpIEqual;
            }
            break;
        }
        case ir::Binary::Kind::kNotEqual: {
            if (lhs_ty->is_bool_scalar_or_vector()) {
                op = spv::Op::OpLogicalNotEqual;
            } else if (lhs_ty->is_float_scalar_or_vector()) {
                op = spv::Op::OpFOrdNotEqual;
            } else if (lhs_ty->is_integer_scalar_or_vector()) {
                op = spv::Op::OpINotEqual;
            }
            break;
        }
        case ir::Binary::Kind::kGreaterThan: {
            if (lhs_ty->is_float_scalar_or_vector()) {
                op = spv::Op::OpFOrdGreaterThan;
            } else if (lhs_ty->is_signed_integer_scalar_or_vector()) {
                op = spv::Op::OpSGreaterThan;
            } else if (lhs_ty->is_unsigned_integer_scalar_or_vector()) {
                op = spv::Op::OpUGreaterThan;
            }
            break;
        }
        case ir::Binary::Kind::kGreaterThanEqual: {
            if (lhs_ty->is_float_scalar_or_vector()) {
                op = spv::Op::OpFOrdGreaterThanEqual;
            } else if (lhs_ty->is_signed_integer_scalar_or_vector()) {
                op = spv::Op::OpSGreaterThanEqual;
            } else if (lhs_ty->is_unsigned_integer_scalar_or_vector()) {
                op = spv::Op::OpUGreaterThanEqual;
            }
            break;
        }
        case ir::Binary::Kind::kLessThan: {
            if (lhs_ty->is_float_scalar_or_vector()) {
                op = spv::Op::OpFOrdLessThan;
            } else if (lhs_ty->is_signed_integer_scalar_or_vector()) {
                op = spv::Op::OpSLessThan;
            } else if (lhs_ty->is_unsigned_integer_scalar_or_vector()) {
                op = spv::Op::OpULessThan;
            }
            break;
        }
        case ir::Binary::Kind::kLessThanEqual: {
            if (lhs_ty->is_float_scalar_or_vector()) {
                op = spv::Op::OpFOrdLessThanEqual;
            } else if (lhs_ty->is_signed_integer_scalar_or_vector()) {
                op = spv::Op::OpSLessThanEqual;
            } else if (lhs_ty->is_unsigned_integer_scalar_or_vector()) {
                op = spv::Op::OpULessThanEqual;
            }
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
}

void GeneratorImplIr::EmitBuiltin(const ir::Builtin* builtin) {
    auto* result_ty = builtin->Type();

    if (builtin->Func() == builtin::Function::kAbs &&
        result_ty->is_unsigned_integer_scalar_or_vector()) {
        // abs() is a no-op for unsigned integers.
        values_.Add(builtin, Value(builtin->Args()[0]));
        return;
    }

    auto id = Value(builtin);

    spv::Op op = spv::Op::Max;
    OperandList operands = {Type(result_ty), id};

    // Helper to set up the opcode and operand list for a GLSL extended instruction.
    auto glsl_ext_inst = [&](enum GLSLstd450 inst) {
        constexpr const char* kGLSLstd450 = "GLSL.std.450";
        op = spv::Op::OpExtInst;
        operands.push_back(imports_.GetOrCreate(kGLSLstd450, [&]() {
            // Import the instruction set the first time it is requested.
            auto import = module_.NextId();
            module_.PushExtImport(spv::Op::OpExtInstImport, {import, Operand(kGLSLstd450)});
            return import;
        }));
        operands.push_back(U32Operand(inst));
    };

    // Determine the opcode.
    switch (builtin->Func()) {
        case builtin::Function::kAbs:
            if (result_ty->is_float_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450FAbs);
            } else if (result_ty->is_signed_integer_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450SAbs);
            }
            break;
        case builtin::Function::kMax:
            if (result_ty->is_float_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450FMax);
            } else if (result_ty->is_signed_integer_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450SMax);
            } else if (result_ty->is_unsigned_integer_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450UMax);
            }
            break;
        case builtin::Function::kMin:
            if (result_ty->is_float_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450FMin);
            } else if (result_ty->is_signed_integer_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450SMin);
            } else if (result_ty->is_unsigned_integer_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450UMin);
            }
            break;
        default:
            TINT_ICE(Writer, diagnostics_) << "unimplemented builtin function: " << builtin->Func();
    }
    TINT_ASSERT(Writer, op != spv::Op::Max);

    // Add the arguments to the builtin call.
    for (auto* arg : builtin->Args()) {
        operands.push_back(Value(arg));
    }

    // Emit the instruction.
    current_function_.push_inst(op, operands);
}

void GeneratorImplIr::EmitLoad(const ir::Load* load) {
    current_function_.push_inst(spv::Op::OpLoad,
                                {Type(load->Type()), Value(load), Value(load->From())});
}

void GeneratorImplIr::EmitLoop(const ir::Loop* loop) {
    auto header_label = module_.NextId();
    auto body_label = Label(loop->Body());
    auto continuing_label = Label(loop->Continuing());
    auto merge_label = Label(loop->Merge());

    // Branch to and emit the loop header, which contains OpLoopMerge and OpBranch instructions.
    current_function_.push_inst(spv::Op::OpBranch, {header_label});
    current_function_.push_inst(spv::Op::OpLabel, {header_label});
    current_function_.push_inst(
        spv::Op::OpLoopMerge, {merge_label, continuing_label, U32Operand(SpvLoopControlMaskNone)});
    current_function_.push_inst(spv::Op::OpBranch, {body_label});

    // Emit the loop body.
    EmitBlock(loop->Body());

    // Emit the loop continuing block.
    // The back-edge needs to go to the loop header, so update the label for the start block.
    block_labels_.Replace(loop->Body(), header_label);
    if (loop->Continuing()->HasBranchTarget()) {
        EmitBlock(loop->Continuing());
    } else {
        // We still need to emit a continuing block with a back-edge, even if it is unreachable.
        current_function_.push_inst(spv::Op::OpLabel, {continuing_label});
        current_function_.push_inst(spv::Op::OpBranch, {header_label});
    }

    // Emit the loop merge block.
    EmitBlock(loop->Merge());
}

void GeneratorImplIr::EmitSwitch(const ir::Switch* swtch) {
    // Find the default selector. There must be exactly one.
    uint32_t default_label = 0u;
    for (auto& c : swtch->Cases()) {
        for (auto& sel : c.selectors) {
            if (sel.IsDefault()) {
                default_label = Label(c.Start());
            }
        }
    }
    TINT_ASSERT(Writer, default_label != 0u);

    // Build the operands to the OpSwitch instruction.
    OperandList switch_operands = {Value(swtch->Condition()), default_label};
    for (auto& c : swtch->Cases()) {
        auto label = Label(c.Start());
        for (auto& sel : c.selectors) {
            if (sel.IsDefault()) {
                continue;
            }
            switch_operands.push_back(sel.val->Value()->ValueAs<uint32_t>());
            switch_operands.push_back(label);
        }
    }

    // Emit the OpSelectionMerge and OpSwitch instructions.
    current_function_.push_inst(spv::Op::OpSelectionMerge,
                                {Label(swtch->Merge()), U32Operand(SpvSelectionControlMaskNone)});
    current_function_.push_inst(spv::Op::OpSwitch, switch_operands);

    // Emit the cases.
    for (auto& c : swtch->Cases()) {
        EmitBlock(c.Start());
    }

    // Emit the switch merge block.
    EmitBlock(swtch->Merge());
}

void GeneratorImplIr::EmitStore(const ir::Store* store) {
    current_function_.push_inst(spv::Op::OpStore, {Value(store->To()), Value(store->From())});
}

void GeneratorImplIr::EmitUserCall(const ir::UserCall* call) {
    auto id = Value(call);
    OperandList operands = {Type(call->Type()), id, Value(call->Func())};
    for (auto* arg : call->Args()) {
        operands.push_back(Value(arg));
    }
    current_function_.push_inst(spv::Op::OpFunctionCall, operands);
}

void GeneratorImplIr::EmitVar(const ir::Var* var) {
    auto id = Value(var);
    auto* ptr = var->Type()->As<type::Pointer>();
    TINT_ASSERT(Writer, ptr);
    auto ty = Type(ptr);

    switch (ptr->AddressSpace()) {
        case builtin::AddressSpace::kFunction: {
            TINT_ASSERT(Writer, current_function_);
            current_function_.push_var({ty, id, U32Operand(SpvStorageClassFunction)});
            if (var->Initializer()) {
                current_function_.push_inst(spv::Op::OpStore, {id, Value(var->Initializer())});
            }
            break;
        }
        case builtin::AddressSpace::kPrivate: {
            TINT_ASSERT(Writer, !current_function_);
            OperandList operands = {ty, id, U32Operand(SpvStorageClassPrivate)};
            if (var->Initializer()) {
                TINT_ASSERT(Writer, var->Initializer()->Is<ir::Constant>());
                operands.push_back(Value(var->Initializer()));
            }
            module_.PushType(spv::Op::OpVariable, operands);
            break;
        }
        case builtin::AddressSpace::kWorkgroup: {
            TINT_ASSERT(Writer, !current_function_);
            OperandList operands = {ty, id, U32Operand(SpvStorageClassWorkgroup)};
            if (zero_init_workgroup_memory_) {
                // If requested, use the VK_KHR_zero_initialize_workgroup_memory to zero-initialize
                // the workgroup variable using an null constant initializer.
                operands.push_back(ConstantNull(ptr->StoreType()));
            }
            module_.PushType(spv::Op::OpVariable, operands);
            break;
        }
        default: {
            TINT_ICE(Writer, diagnostics_)
                << "unimplemented variable address space " << ptr->AddressSpace();
        }
    }

    // Set the name if present.
    if (auto name = ir_->NameOf(var)) {
        module_.PushDebug(spv::Op::OpName, {id, Operand(name.Name())});
    }
}

}  // namespace tint::writer::spirv
