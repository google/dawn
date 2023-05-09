// Copyright 2020 The Tint Authors.  //
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

#include "src/tint/writer/spirv/builder.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "spirv/unified1/GLSL.std.450.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/internal_attribute.h"
#include "src/tint/ast/traverse_expressions.h"
#include "src/tint/constant/value.h"
#include "src/tint/sem/builtin.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/load.h"
#include "src/tint/sem/materialize.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/module.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/struct.h"
#include "src/tint/sem/switch_statement.h"
#include "src/tint/sem/value_constructor.h"
#include "src/tint/sem/value_conversion.h"
#include "src/tint/sem/variable.h"
#include "src/tint/transform/add_block_attribute.h"
#include "src/tint/type/array.h"
#include "src/tint/type/atomic.h"
#include "src/tint/type/depth_multisampled_texture.h"
#include "src/tint/type/depth_texture.h"
#include "src/tint/type/multisampled_texture.h"
#include "src/tint/type/reference.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/texture_dimension.h"
#include "src/tint/type/vector.h"
#include "src/tint/utils/compiler_macros.h"
#include "src/tint/utils/defer.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/string_stream.h"
#include "src/tint/writer/append_vector.h"
#include "src/tint/writer/check_supported_extensions.h"

namespace tint::writer::spirv {
namespace {

const char kGLSLstd450[] = "GLSL.std.450";

uint32_t pipeline_stage_to_execution_model(ast::PipelineStage stage) {
    SpvExecutionModel model = SpvExecutionModelVertex;

    switch (stage) {
        case ast::PipelineStage::kFragment:
            model = SpvExecutionModelFragment;
            break;
        case ast::PipelineStage::kVertex:
            model = SpvExecutionModelVertex;
            break;
        case ast::PipelineStage::kCompute:
            model = SpvExecutionModelGLCompute;
            break;
        case ast::PipelineStage::kNone:
            model = SpvExecutionModelMax;
            break;
    }
    return model;
}

/// Returns the matrix type that is `type` or that is wrapped by
/// one or more levels of an arrays inside of `type`.
/// @param type the given type, which must not be null
/// @returns the nested matrix type, or nullptr if none
const type::Matrix* GetNestedMatrixType(const type::Type* type) {
    while (auto* arr = type->As<type::Array>()) {
        type = arr->ElemType();
    }
    return type->As<type::Matrix>();
}

uint32_t builtin_to_glsl_method(const sem::Builtin* builtin) {
    switch (builtin->Type()) {
        case builtin::Function::kAcos:
            return GLSLstd450Acos;
        case builtin::Function::kAcosh:
            return GLSLstd450Acosh;
        case builtin::Function::kAsin:
            return GLSLstd450Asin;
        case builtin::Function::kAsinh:
            return GLSLstd450Asinh;
        case builtin::Function::kAtan:
            return GLSLstd450Atan;
        case builtin::Function::kAtan2:
            return GLSLstd450Atan2;
        case builtin::Function::kAtanh:
            return GLSLstd450Atanh;
        case builtin::Function::kCeil:
            return GLSLstd450Ceil;
        case builtin::Function::kClamp:
            if (builtin->ReturnType()->is_float_scalar_or_vector()) {
                return GLSLstd450NClamp;
            } else if (builtin->ReturnType()->is_unsigned_integer_scalar_or_vector()) {
                return GLSLstd450UClamp;
            } else {
                return GLSLstd450SClamp;
            }
        case builtin::Function::kCos:
            return GLSLstd450Cos;
        case builtin::Function::kCosh:
            return GLSLstd450Cosh;
        case builtin::Function::kCross:
            return GLSLstd450Cross;
        case builtin::Function::kDegrees:
            return GLSLstd450Degrees;
        case builtin::Function::kDeterminant:
            return GLSLstd450Determinant;
        case builtin::Function::kDistance:
            return GLSLstd450Distance;
        case builtin::Function::kExp:
            return GLSLstd450Exp;
        case builtin::Function::kExp2:
            return GLSLstd450Exp2;
        case builtin::Function::kFaceForward:
            return GLSLstd450FaceForward;
        case builtin::Function::kFloor:
            return GLSLstd450Floor;
        case builtin::Function::kFma:
            return GLSLstd450Fma;
        case builtin::Function::kFract:
            return GLSLstd450Fract;
        case builtin::Function::kFrexp:
            return GLSLstd450FrexpStruct;
        case builtin::Function::kInverseSqrt:
            return GLSLstd450InverseSqrt;
        case builtin::Function::kLdexp:
            return GLSLstd450Ldexp;
        case builtin::Function::kLength:
            return GLSLstd450Length;
        case builtin::Function::kLog:
            return GLSLstd450Log;
        case builtin::Function::kLog2:
            return GLSLstd450Log2;
        case builtin::Function::kMax:
            if (builtin->ReturnType()->is_float_scalar_or_vector()) {
                return GLSLstd450NMax;
            } else if (builtin->ReturnType()->is_unsigned_integer_scalar_or_vector()) {
                return GLSLstd450UMax;
            } else {
                return GLSLstd450SMax;
            }
        case builtin::Function::kMin:
            if (builtin->ReturnType()->is_float_scalar_or_vector()) {
                return GLSLstd450NMin;
            } else if (builtin->ReturnType()->is_unsigned_integer_scalar_or_vector()) {
                return GLSLstd450UMin;
            } else {
                return GLSLstd450SMin;
            }
        case builtin::Function::kMix:
            return GLSLstd450FMix;
        case builtin::Function::kModf:
            return GLSLstd450ModfStruct;
        case builtin::Function::kNormalize:
            return GLSLstd450Normalize;
        case builtin::Function::kPack4X8Snorm:
            return GLSLstd450PackSnorm4x8;
        case builtin::Function::kPack4X8Unorm:
            return GLSLstd450PackUnorm4x8;
        case builtin::Function::kPack2X16Snorm:
            return GLSLstd450PackSnorm2x16;
        case builtin::Function::kPack2X16Unorm:
            return GLSLstd450PackUnorm2x16;
        case builtin::Function::kPack2X16Float:
            return GLSLstd450PackHalf2x16;
        case builtin::Function::kPow:
            return GLSLstd450Pow;
        case builtin::Function::kRadians:
            return GLSLstd450Radians;
        case builtin::Function::kReflect:
            return GLSLstd450Reflect;
        case builtin::Function::kRefract:
            return GLSLstd450Refract;
        case builtin::Function::kRound:
            return GLSLstd450RoundEven;
        case builtin::Function::kSign:
            if (builtin->ReturnType()->is_signed_integer_scalar_or_vector()) {
                return GLSLstd450SSign;
            } else {
                return GLSLstd450FSign;
            }
        case builtin::Function::kSin:
            return GLSLstd450Sin;
        case builtin::Function::kSinh:
            return GLSLstd450Sinh;
        case builtin::Function::kSmoothstep:
            return GLSLstd450SmoothStep;
        case builtin::Function::kSqrt:
            return GLSLstd450Sqrt;
        case builtin::Function::kStep:
            return GLSLstd450Step;
        case builtin::Function::kTan:
            return GLSLstd450Tan;
        case builtin::Function::kTanh:
            return GLSLstd450Tanh;
        case builtin::Function::kTrunc:
            return GLSLstd450Trunc;
        case builtin::Function::kUnpack4X8Snorm:
            return GLSLstd450UnpackSnorm4x8;
        case builtin::Function::kUnpack4X8Unorm:
            return GLSLstd450UnpackUnorm4x8;
        case builtin::Function::kUnpack2X16Snorm:
            return GLSLstd450UnpackSnorm2x16;
        case builtin::Function::kUnpack2X16Unorm:
            return GLSLstd450UnpackUnorm2x16;
        case builtin::Function::kUnpack2X16Float:
            return GLSLstd450UnpackHalf2x16;
        default:
            break;
    }
    return 0;
}

/// @return the vector element type if ty is a vector, otherwise return ty.
const type::Type* ElementTypeOf(const type::Type* ty) {
    if (auto* v = ty->As<type::Vector>()) {
        return v->type();
    }
    return ty;
}

}  // namespace

Builder::AccessorInfo::AccessorInfo() : source_id(0), source_type(nullptr) {}

Builder::AccessorInfo::~AccessorInfo() {}

Builder::Builder(const Program* program, bool zero_initialize_workgroup_memory)
    : builder_(ProgramBuilder::Wrap(program)),
      scope_stack_{Scope{}},
      zero_initialize_workgroup_memory_(zero_initialize_workgroup_memory) {}

Builder::~Builder() = default;

bool Builder::Build() {
    if (!CheckSupportedExtensions("SPIR-V", builder_.AST(), builder_.Diagnostics(),
                                  utils::Vector{
                                      builtin::Extension::kChromiumDisableUniformityAnalysis,
                                      builtin::Extension::kChromiumExperimentalDp4A,
                                      builtin::Extension::kChromiumExperimentalFullPtrParameters,
                                      builtin::Extension::kChromiumExperimentalPushConstant,
                                      builtin::Extension::kF16,
                                  })) {
        return false;
    }

    module_.PushCapability(SpvCapabilityShader);

    module_.PushMemoryModel(spv::Op::OpMemoryModel, {U32Operand(SpvAddressingModelLogical),
                                                     U32Operand(SpvMemoryModelGLSL450)});

    for (auto ext : builder_.Sem().Module()->Extensions()) {
        GenerateExtension(ext);
    }

    for (auto* var : builder_.AST().GlobalVariables()) {
        if (!GenerateGlobalVariable(var)) {
            return false;
        }
    }

    auto* mod = builder_.Sem().Module();
    for (auto* decl : mod->DependencyOrderedDeclarations()) {
        if (auto* func = decl->As<ast::Function>()) {
            if (!GenerateFunction(func)) {
                return false;
            }
        }
    }

    return true;
}

void Builder::RegisterVariable(const sem::Variable* var, uint32_t id) {
    var_to_id_.emplace(var, id);
    id_to_var_.emplace(id, var);
}

uint32_t Builder::LookupVariableID(const sem::Variable* var) {
    auto it = var_to_id_.find(var);
    if (it == var_to_id_.end()) {
        TINT_ICE(Writer, builder_.Diagnostics())
            << "unable to find ID for variable: " + var->Declaration()->name->symbol.Name();
        return 0;
    }
    return it->second;
}

void Builder::PushScope() {
    // Push a new scope, by copying the top-most stack
    scope_stack_.push_back(scope_stack_.back());
}

void Builder::PopScope() {
    scope_stack_.pop_back();
}

Operand Builder::result_op() {
    return Operand(module_.NextId());
}

bool Builder::GenerateExtension(builtin::Extension extension) {
    switch (extension) {
        case builtin::Extension::kChromiumExperimentalDp4A:
            module_.PushExtension("SPV_KHR_integer_dot_product");
            module_.PushCapability(SpvCapabilityDotProductKHR);
            module_.PushCapability(SpvCapabilityDotProductInput4x8BitPackedKHR);
            break;
        case builtin::Extension::kF16:
            module_.PushCapability(SpvCapabilityFloat16);
            module_.PushCapability(SpvCapabilityUniformAndStorageBuffer16BitAccess);
            module_.PushCapability(SpvCapabilityStorageBuffer16BitAccess);
            module_.PushCapability(SpvCapabilityStorageInputOutput16);
            break;
        default:
            return false;
    }

    return true;
}

bool Builder::GenerateLabel(uint32_t id) {
    if (!push_function_inst(spv::Op::OpLabel, {Operand(id)})) {
        return false;
    }
    current_label_id_ = id;
    return true;
}

bool Builder::GenerateAssignStatement(const ast::AssignmentStatement* assign) {
    if (assign->lhs->Is<ast::PhonyExpression>()) {
        if (builder_.Sem().GetVal(assign->rhs)->ConstantValue()) {
            // RHS of phony assignment is constant.
            // Constants can't have side-effects, so just drop this.
            return true;
        }
        auto rhs_id = GenerateExpression(assign->rhs);
        if (rhs_id == 0) {
            return false;
        }
        return true;
    } else {
        auto lhs_id = GenerateExpression(assign->lhs);
        if (lhs_id == 0) {
            return false;
        }
        auto rhs_id = GenerateExpression(assign->rhs);
        if (rhs_id == 0) {
            return false;
        }
        return GenerateStore(lhs_id, rhs_id);
    }
}

bool Builder::GenerateBreakStatement(const ast::BreakStatement*) {
    if (merge_stack_.empty()) {
        TINT_ICE(Writer, builder_.Diagnostics()) << "Attempted to break without a merge block";
        return false;
    }
    if (!push_function_inst(spv::Op::OpBranch, {Operand(merge_stack_.back())})) {
        return false;
    }
    return true;
}

bool Builder::GenerateBreakIfStatement(const ast::BreakIfStatement* stmt) {
    TINT_ASSERT(Writer, !backedge_stack_.empty());
    const auto cond_id = GenerateExpression(stmt->condition);
    if (!cond_id) {
        return false;
    }
    const ContinuingInfo& ci = continuing_stack_.back();
    backedge_stack_.back() =
        Backedge(spv::Op::OpBranchConditional,
                 {Operand(cond_id), Operand(ci.break_target_id), Operand(ci.loop_header_id)});
    return true;
}

bool Builder::GenerateContinueStatement(const ast::ContinueStatement*) {
    if (continue_stack_.empty()) {
        TINT_ICE(Writer, builder_.Diagnostics())
            << "Attempted to continue without a continue block";
        return false;
    }
    if (!push_function_inst(spv::Op::OpBranch, {Operand(continue_stack_.back())})) {
        return false;
    }
    return true;
}

// TODO(dsinclair): This is generating an OpKill but the semantics of kill
// haven't been defined for WGSL yet. So, this may need to change.
// https://github.com/gpuweb/gpuweb/issues/676
bool Builder::GenerateDiscardStatement(const ast::DiscardStatement*) {
    if (!push_function_inst(spv::Op::OpKill, {})) {
        return false;
    }
    return true;
}

bool Builder::GenerateEntryPoint(const ast::Function* func, uint32_t id) {
    auto stage = pipeline_stage_to_execution_model(func->PipelineStage());
    if (stage == SpvExecutionModelMax) {
        TINT_ICE(Writer, builder_.Diagnostics()) << "Unknown pipeline stage provided";
        return false;
    }

    OperandList operands = {Operand(stage), Operand(id), Operand(func->name->symbol.Name())};

    auto* func_sem = builder_.Sem().Get(func);
    for (const auto* var : func_sem->TransitivelyReferencedGlobals()) {
        // For SPIR-V 1.3 we only output Input/output variables. If we update to
        // SPIR-V 1.4 or later this should be all variables.
        if (var->AddressSpace() != builtin::AddressSpace::kIn &&
            var->AddressSpace() != builtin::AddressSpace::kOut) {
            continue;
        }

        uint32_t var_id = LookupVariableID(var);
        if (var_id == 0) {
            TINT_ICE(Writer, builder_.Diagnostics()) << "unable to find ID for global variable: " +
                                                            var->Declaration()->name->symbol.Name();
            return false;
        }

        operands.push_back(Operand(var_id));
    }
    module_.PushEntryPoint(spv::Op::OpEntryPoint, operands);

    return true;
}

bool Builder::GenerateExecutionModes(const ast::Function* func, uint32_t id) {
    auto* func_sem = builder_.Sem().Get(func);

    // WGSL fragment shader origin is upper left
    if (func->PipelineStage() == ast::PipelineStage::kFragment) {
        module_.PushExecutionMode(spv::Op::OpExecutionMode,
                                  {Operand(id), U32Operand(SpvExecutionModeOriginUpperLeft)});
    } else if (func->PipelineStage() == ast::PipelineStage::kCompute) {
        auto& wgsize = func_sem->WorkgroupSize();

        // Check if the workgroup_size uses pipeline-overridable constants.
        if (!wgsize[0].has_value() || !wgsize[1].has_value() || !wgsize[2].has_value()) {
            TINT_ICE(Writer, builder_.Diagnostics())
                << "override-expressions should have been removed with the SubstituteOverride "
                   "transform";
            return false;
        }
        module_.PushExecutionMode(
            spv::Op::OpExecutionMode,
            {Operand(id), U32Operand(SpvExecutionModeLocalSize),  //
             Operand(wgsize[0].value()), Operand(wgsize[1].value()), Operand(wgsize[2].value())});
    }

    for (auto it : func_sem->TransitivelyReferencedBuiltinVariables()) {
        auto builtin = builder_.Sem().Get(it.second)->Value();
        if (builtin == builtin::BuiltinValue::kFragDepth) {
            module_.PushExecutionMode(spv::Op::OpExecutionMode,
                                      {Operand(id), U32Operand(SpvExecutionModeDepthReplacing)});
        }
    }

    return true;
}

uint32_t Builder::GenerateExpression(const sem::Expression* expr) {
    if (auto* val_expr = expr->As<sem::ValueExpression>()) {
        if (auto* constant = val_expr->ConstantValue()) {
            return GenerateConstantIfNeeded(constant);
        }
    }
    if (auto* load = expr->As<sem::Load>()) {
        auto ref_id = GenerateExpression(load->Reference());
        if (ref_id == 0) {
            return 0;
        }
        return GenerateLoad(load->ReferenceType(), ref_id);
    }
    return Switch(
        expr->Declaration(),  //
        [&](const ast::AccessorExpression* a) { return GenerateAccessorExpression(a); },
        [&](const ast::BinaryExpression* b) { return GenerateBinaryExpression(b); },
        [&](const ast::BitcastExpression* b) { return GenerateBitcastExpression(b); },
        [&](const ast::CallExpression* c) { return GenerateCallExpression(c); },
        [&](const ast::IdentifierExpression* i) { return GenerateIdentifierExpression(i); },
        [&](const ast::LiteralExpression* l) { return GenerateLiteralIfNeeded(l); },
        [&](const ast::UnaryOpExpression* u) { return GenerateUnaryOpExpression(u); },
        [&](Default) {
            TINT_ICE(Writer, builder_.Diagnostics())
                << "unknown expression type: " + std::string(expr->TypeInfo().name);
            return 0;
        });
}

uint32_t Builder::GenerateExpression(const ast::Expression* expr) {
    return GenerateExpression(builder_.Sem().Get(expr));
}

bool Builder::GenerateFunction(const ast::Function* func_ast) {
    auto* func = builder_.Sem().Get(func_ast);

    uint32_t func_type_id = GenerateFunctionTypeIfNeeded(func);
    if (func_type_id == 0) {
        return false;
    }

    auto func_op = result_op();
    auto func_id = std::get<uint32_t>(func_op);

    module_.PushDebug(spv::Op::OpName, {Operand(func_id), Operand(func_ast->name->symbol.Name())});

    auto ret_id = GenerateTypeIfNeeded(func->ReturnType());
    if (ret_id == 0) {
        return false;
    }

    PushScope();
    TINT_DEFER(PopScope());

    auto definition_inst = Instruction{
        spv::Op::OpFunction,
        {Operand(ret_id), func_op, U32Operand(SpvFunctionControlMaskNone), Operand(func_type_id)}};

    InstructionList params;
    for (auto* param : func->Parameters()) {
        auto param_op = result_op();
        auto param_id = std::get<uint32_t>(param_op);

        auto param_type_id = GenerateTypeIfNeeded(param->Type());
        if (param_type_id == 0) {
            return false;
        }

        module_.PushDebug(spv::Op::OpName,
                          {Operand(param_id), Operand(param->Declaration()->name->symbol.Name())});
        params.push_back(
            Instruction{spv::Op::OpFunctionParameter, {Operand(param_type_id), param_op}});

        RegisterVariable(param, param_id);
    }

    // Start a new function.
    current_function_ = Function{definition_inst, result_op(), std::move(params)};
    current_label_id_ = current_function_.label_id();
    TINT_DEFER(current_function_ = Function());

    for (auto* stmt : func_ast->body->statements) {
        if (!GenerateStatement(stmt)) {
            return false;
        }
    }

    if (InsideBasicBlock()) {
        if (func->ReturnType()->Is<type::Void>()) {
            push_function_inst(spv::Op::OpReturn, {});
        } else {
            auto zero = GenerateConstantNullIfNeeded(func->ReturnType());
            push_function_inst(spv::Op::OpReturnValue, {Operand(zero)});
        }
    }

    if (func_ast->IsEntryPoint()) {
        if (!GenerateEntryPoint(func_ast, func_id)) {
            return false;
        }
        if (!GenerateExecutionModes(func_ast, func_id)) {
            return false;
        }
    }

    func_symbol_to_id_[func_ast->name->symbol] = func_id;

    // Add the function to the module.
    module_.PushFunction(std::move(current_function_));

    return true;
}

uint32_t Builder::GenerateFunctionTypeIfNeeded(const sem::Function* func) {
    return utils::GetOrCreate(func_sig_to_id_, func->Signature(), [&]() -> uint32_t {
        auto func_op = result_op();
        auto func_type_id = std::get<uint32_t>(func_op);

        auto ret_id = GenerateTypeIfNeeded(func->ReturnType());
        if (ret_id == 0) {
            return 0;
        }

        OperandList ops = {func_op, Operand(ret_id)};
        for (auto* param : func->Parameters()) {
            auto param_type_id = GenerateTypeIfNeeded(param->Type());
            if (param_type_id == 0) {
                return 0;
            }
            ops.push_back(Operand(param_type_id));
        }

        module_.PushType(spv::Op::OpTypeFunction, std::move(ops));
        return func_type_id;
    });
}

bool Builder::GenerateFunctionVariable(const ast::Variable* v) {
    if (v->Is<ast::Const>()) {
        // Constants are generated at their use. This is required as the 'const' declaration may be
        // abstract-numeric, which has no SPIR-V type.
        return true;
    }

    uint32_t init_id = 0;
    if (v->initializer) {
        init_id = GenerateExpression(v->initializer);
        if (init_id == 0) {
            return false;
        }
    }

    auto* sem = builder_.Sem().Get(v);

    if (v->Is<ast::Let>()) {
        if (!v->initializer) {
            TINT_ICE(Writer, builder_.Diagnostics()) << "missing initializer for let";
            return false;
        }
        RegisterVariable(sem, init_id);
        return true;
    }

    auto result = result_op();
    auto var_id = std::get<uint32_t>(result);
    auto sc = builtin::AddressSpace::kFunction;
    auto* type = sem->Type();
    auto type_id = GenerateTypeIfNeeded(type);
    if (type_id == 0) {
        return false;
    }

    module_.PushDebug(spv::Op::OpName, {Operand(var_id), Operand(v->name->symbol.Name())});

    // TODO(dsinclair) We could detect if the initializer is fully const and emit
    // an initializer value for the variable instead of doing the OpLoad.
    auto null_id = GenerateConstantNullIfNeeded(type->UnwrapRef());
    if (null_id == 0) {
        return 0;
    }
    push_function_var(
        {Operand(type_id), result, U32Operand(ConvertAddressSpace(sc)), Operand(null_id)});

    if (v->initializer) {
        if (!GenerateStore(var_id, init_id)) {
            return false;
        }
    }

    RegisterVariable(sem, var_id);

    return true;
}

bool Builder::GenerateStore(uint32_t to, uint32_t from) {
    return push_function_inst(spv::Op::OpStore, {Operand(to), Operand(from)});
}

bool Builder::GenerateGlobalVariable(const ast::Variable* v) {
    if (v->Is<ast::Const>()) {
        // Constants are generated at their use. This is required as the 'const' declaration may be
        // abstract-numeric, which has no SPIR-V type.
        return true;
    }

    auto* sem = builder_.Sem().Get<sem::GlobalVariable>(v);
    if (TINT_UNLIKELY(!sem)) {
        TINT_ICE(Writer, builder_.Diagnostics())
            << "attempted to generate a global from a non-global variable";
        return false;
    }
    auto* type = sem->Type()->UnwrapRef();

    uint32_t init_id = 0;
    if (auto* ctor = v->initializer) {
        init_id = GenerateConstructorExpression(v, ctor);
        if (init_id == 0) {
            return false;
        }
    }

    auto result = result_op();
    auto var_id = std::get<uint32_t>(result);

    auto sc = sem->AddressSpace() == builtin::AddressSpace::kUndefined
                  ? builtin::AddressSpace::kPrivate
                  : sem->AddressSpace();

    auto type_id = GenerateTypeIfNeeded(sem->Type());
    if (type_id == 0) {
        return false;
    }

    module_.PushDebug(spv::Op::OpName, {Operand(var_id), Operand(v->name->symbol.Name())});

    OperandList ops = {Operand(type_id), result, U32Operand(ConvertAddressSpace(sc))};

    if (v->initializer) {
        ops.push_back(Operand(init_id));
    } else {
        auto* st = type->As<type::StorageTexture>();
        if (st || type->Is<type::Struct>()) {
            // type is a type::Struct or a type::StorageTexture
            auto access = st ? st->access() : sem->Access();
            switch (access) {
                case builtin::Access::kWrite:
                    module_.PushAnnot(spv::Op::OpDecorate,
                                      {Operand(var_id), U32Operand(SpvDecorationNonReadable)});
                    break;
                case builtin::Access::kRead:
                    module_.PushAnnot(spv::Op::OpDecorate,
                                      {Operand(var_id), U32Operand(SpvDecorationNonWritable)});
                    break;
                case builtin::Access::kUndefined:
                case builtin::Access::kReadWrite:
                    break;
            }
        }
        if (!type->Is<type::Sampler>()) {
            // If we don't have a initializer and we're an Output or Private
            // variable, then WGSL requires that we zero-initialize.
            // If we're a Workgroup variable, and the
            // VK_KHR_zero_initialize_workgroup_memory extension is enabled, we should
            // also zero-initialize.
            if (sem->AddressSpace() == builtin::AddressSpace::kPrivate ||
                sem->AddressSpace() == builtin::AddressSpace::kOut ||
                (zero_initialize_workgroup_memory_ &&
                 sem->AddressSpace() == builtin::AddressSpace::kWorkgroup)) {
                init_id = GenerateConstantNullIfNeeded(type);
                if (init_id == 0) {
                    return 0;
                }
                ops.push_back(Operand(init_id));
            }
        }
    }

    module_.PushType(spv::Op::OpVariable, std::move(ops));

    for (auto* attr : v->attributes) {
        bool ok = Switch(
            attr,
            [&](const ast::BuiltinAttribute* builtin_attr) {
                auto builtin = builder_.Sem().Get(builtin_attr)->Value();
                module_.PushAnnot(spv::Op::OpDecorate,
                                  {Operand(var_id), U32Operand(SpvDecorationBuiltIn),
                                   U32Operand(ConvertBuiltin(builtin, sem->AddressSpace()))});
                return true;
            },
            [&](const ast::LocationAttribute*) {
                module_.PushAnnot(spv::Op::OpDecorate,
                                  {Operand(var_id), U32Operand(SpvDecorationLocation),
                                   Operand(sem->Location().value())});
                return true;
            },
            [&](const ast::InterpolateAttribute* interpolate) {
                auto& s = builder_.Sem();
                auto i_type =
                    s.Get<sem::BuiltinEnumExpression<builtin::InterpolationType>>(interpolate->type)
                        ->Value();

                auto i_smpl = builtin::InterpolationSampling::kUndefined;
                if (interpolate->sampling) {
                    i_smpl = s.Get<sem::BuiltinEnumExpression<builtin::InterpolationSampling>>(
                                  interpolate->sampling)
                                 ->Value();
                }

                AddInterpolationDecorations(var_id, i_type, i_smpl);
                return true;
            },
            [&](const ast::InvariantAttribute*) {
                module_.PushAnnot(spv::Op::OpDecorate,
                                  {Operand(var_id), U32Operand(SpvDecorationInvariant)});
                return true;
            },
            [&](const ast::BindingAttribute*) {
                auto bp = sem->BindingPoint();
                module_.PushAnnot(
                    spv::Op::OpDecorate,
                    {Operand(var_id), U32Operand(SpvDecorationBinding), Operand(bp->binding)});
                return true;
            },
            [&](const ast::GroupAttribute*) {
                auto bp = sem->BindingPoint();
                module_.PushAnnot(
                    spv::Op::OpDecorate,
                    {Operand(var_id), U32Operand(SpvDecorationDescriptorSet), Operand(bp->group)});
                return true;
            },
            [&](const ast::IdAttribute*) {
                return true;  // Spec constants are handled elsewhere
            },
            [&](const ast::InternalAttribute*) {
                return true;  // ignored
            },
            [&](Default) {
                TINT_ICE(Writer, builder_.Diagnostics()) << "unknown attribute";
                return false;
            });
        if (!ok) {
            return false;
        }
    }

    RegisterVariable(sem, var_id);
    return true;
}

bool Builder::GenerateIndexAccessor(const ast::IndexAccessorExpression* expr, AccessorInfo* info) {
    auto idx_id = GenerateExpression(expr->index);
    if (idx_id == 0) {
        return 0;
    }

    // If the source is a reference, we access chain into it.
    // In the future, pointers may support access-chaining.
    // See https://github.com/gpuweb/gpuweb/pull/1580
    if (info->source_type->Is<type::Reference>()) {
        info->access_chain_indices.push_back(idx_id);
        info->source_type = builder_.Sem().Get(expr)->UnwrapLoad()->Type();
        return true;
    }

    auto result_type_id = GenerateTypeIfNeeded(TypeOf(expr));
    if (result_type_id == 0) {
        return false;
    }

    // We don't have a pointer, so we can just directly extract the value.
    auto extract = result_op();
    auto extract_id = std::get<uint32_t>(extract);

    // If the index is compile-time constant, we use OpCompositeExtract.
    auto* idx = builder_.Sem().GetVal(expr->index);
    if (auto idx_constval = idx->ConstantValue()) {
        if (!push_function_inst(spv::Op::OpCompositeExtract,
                                {
                                    Operand(result_type_id),
                                    extract,
                                    Operand(info->source_id),
                                    Operand(idx_constval->ValueAs<u32>()),
                                })) {
            return false;
        }

        info->source_id = extract_id;
        info->source_type = TypeOf(expr);

        return true;
    }

    // If the source is a vector, we use OpVectorExtractDynamic.
    if (TINT_LIKELY(info->source_type->Is<type::Vector>())) {
        if (!push_function_inst(
                spv::Op::OpVectorExtractDynamic,
                {Operand(result_type_id), extract, Operand(info->source_id), Operand(idx_id)})) {
            return false;
        }

        info->source_id = extract_id;
        info->source_type = TypeOf(expr);

        return true;
    }

    TINT_ICE(Writer, builder_.Diagnostics()) << "unsupported index accessor expression";
    return false;
}

bool Builder::GenerateMemberAccessor(const ast::MemberAccessorExpression* expr,
                                     AccessorInfo* info) {
    auto* expr_sem = builder_.Sem().Get(expr)->UnwrapLoad();
    auto* expr_type = expr_sem->Type();

    return Switch(
        expr_sem,  //
        [&](const sem::StructMemberAccess* access) {
            uint32_t idx = access->Member()->Index();

            if (info->source_type->Is<type::Reference>()) {
                auto idx_id = GenerateConstantIfNeeded(ScalarConstant::U32(idx));
                if (TINT_UNLIKELY(idx_id == 0)) {
                    return false;
                }
                info->access_chain_indices.push_back(idx_id);
                info->source_type = expr_type;
            } else {
                auto result_type_id = GenerateTypeIfNeeded(expr_type);
                if (TINT_UNLIKELY(result_type_id == 0)) {
                    return false;
                }

                auto extract = result_op();
                auto extract_id = std::get<uint32_t>(extract);
                if (!push_function_inst(spv::Op::OpCompositeExtract,
                                        {Operand(result_type_id), extract, Operand(info->source_id),
                                         Operand(idx)})) {
                    return false;
                }

                info->source_id = extract_id;
                info->source_type = expr_type;
            }

            return true;
        },
        [&](const sem::Swizzle* swizzle) {
            // Single element swizzle is either an access chain or a composite extract
            auto& indices = swizzle->Indices();
            if (indices.Length() == 1) {
                if (info->source_type->Is<type::Reference>()) {
                    auto idx_id = GenerateConstantIfNeeded(ScalarConstant::U32(indices[0]));
                    if (TINT_UNLIKELY(idx_id == 0)) {
                        return false;
                    }
                    info->access_chain_indices.push_back(idx_id);
                } else {
                    auto result_type_id = GenerateTypeIfNeeded(expr_type);
                    if (TINT_UNLIKELY(result_type_id == 0)) {
                        return false;
                    }

                    auto extract = result_op();
                    auto extract_id = std::get<uint32_t>(extract);
                    if (!push_function_inst(spv::Op::OpCompositeExtract,
                                            {Operand(result_type_id), extract,
                                             Operand(info->source_id), Operand(indices[0])})) {
                        return false;
                    }

                    info->source_id = extract_id;
                    info->source_type = expr_type;
                }
                return true;
            }

            // Store the type away as it may change if we run the access chain
            auto* incoming_type = info->source_type;

            // Multi-item extract is a VectorShuffle. We have to emit any existing
            // access chain data, then load the access chain and shuffle that.
            if (!info->access_chain_indices.empty()) {
                auto result_type_id = GenerateTypeIfNeeded(info->source_type);
                if (TINT_UNLIKELY(result_type_id == 0)) {
                    return false;
                }
                auto extract = result_op();
                auto extract_id = std::get<uint32_t>(extract);

                OperandList ops = {Operand(result_type_id), extract, Operand(info->source_id)};
                for (auto id : info->access_chain_indices) {
                    ops.push_back(Operand(id));
                }

                if (!push_function_inst(spv::Op::OpAccessChain, ops)) {
                    return false;
                }

                info->source_id = GenerateLoadIfNeeded(expr_type, extract_id);
                info->source_type = expr_type->UnwrapRef();
                info->access_chain_indices.clear();
            }

            auto result_type_id = GenerateTypeIfNeeded(expr_type);
            if (TINT_UNLIKELY(result_type_id == 0)) {
                return false;
            }

            auto vec_id = GenerateLoadIfNeeded(incoming_type, info->source_id);

            auto result = result_op();
            auto result_id = std::get<uint32_t>(result);

            OperandList ops = {Operand(result_type_id), result, Operand(vec_id), Operand(vec_id)};

            for (auto idx : indices) {
                ops.push_back(Operand(idx));
            }

            if (!push_function_inst(spv::Op::OpVectorShuffle, ops)) {
                return false;
            }
            info->source_id = result_id;
            info->source_type = expr_type;
            return true;
        },
        [&](Default) {
            TINT_ICE(Writer, builder_.Diagnostics())
                << "unhandled member index type: " << expr_sem->TypeInfo().name;
            return false;
        });
}

uint32_t Builder::GenerateAccessorExpression(const ast::AccessorExpression* expr) {
    // Gather a list of all the member and index accessors that are in this chain.
    // The list is built in reverse order as that's the order we need to access
    // the chain.
    std::vector<const ast::Expression*> accessors;
    const ast::Expression* source = expr;
    while (true) {
        if (auto* array = source->As<ast::IndexAccessorExpression>()) {
            accessors.insert(accessors.begin(), source);
            source = array->object;
        } else if (auto* member = source->As<ast::MemberAccessorExpression>()) {
            accessors.insert(accessors.begin(), source);
            source = member->object;
        } else {
            break;
        }

        // Stop traversing if we've hit a constant source expression.
        if (builder_.Sem().GetVal(source)->ConstantValue()) {
            break;
        }
    }

    AccessorInfo info;
    info.source_id = GenerateExpression(source);
    if (info.source_id == 0) {
        return 0;
    }
    info.source_type = TypeOf(source);

    // Note: Dynamic index on array and matrix values (lets) should have been
    // promoted to storage with the VarForDynamicIndex transform.

    for (auto* accessor : accessors) {
        bool ok = Switch(
            accessor,
            [&](const ast::IndexAccessorExpression* array) {
                return GenerateIndexAccessor(array, &info);
            },
            [&](const ast::MemberAccessorExpression* member) {
                return GenerateMemberAccessor(member, &info);
            },
            [&](Default) {
                TINT_ICE(Writer, builder_.Diagnostics())
                    << "invalid accessor in list: " + std::string(accessor->TypeInfo().name);
                return false;
            });
        if (!ok) {
            return false;
        }
    }

    if (!info.access_chain_indices.empty()) {
        auto* type = builder_.Sem().Get(expr)->UnwrapLoad()->Type();
        auto result_type_id = GenerateTypeIfNeeded(type);
        if (result_type_id == 0) {
            return 0;
        }

        auto result = result_op();
        auto result_id = std::get<uint32_t>(result);

        OperandList ops = {Operand(result_type_id), result, Operand(info.source_id)};
        for (auto id : info.access_chain_indices) {
            ops.push_back(Operand(id));
        }

        if (!push_function_inst(spv::Op::OpAccessChain, ops)) {
            return false;
        }
        info.source_id = result_id;
    }

    return info.source_id;
}

uint32_t Builder::GenerateIdentifierExpression(const ast::IdentifierExpression* expr) {
    if (auto* sem = builder_.Sem().GetVal(expr); sem) {
        if (auto* user = sem->UnwrapLoad()->As<sem::VariableUser>()) {
            return LookupVariableID(user->Variable());
        }
    }
    TINT_ICE(Writer, builder_.Diagnostics())
        << "identifier '" + expr->identifier->symbol.Name() + "' does not resolve to a variable";
    return 0;
}

uint32_t Builder::GenerateLoad(const type::Reference* type, uint32_t id) {
    auto type_id = GenerateTypeIfNeeded(type->StoreType());
    auto result = result_op();
    auto result_id = std::get<uint32_t>(result);
    if (!push_function_inst(spv::Op::OpLoad, {Operand(type_id), result, Operand(id)})) {
        return 0;
    }
    return result_id;
}

uint32_t Builder::GenerateLoadIfNeeded(const type::Type* type, uint32_t id) {
    if (auto* ref = type->As<type::Reference>()) {
        return GenerateLoad(ref, id);
    }
    return id;
}

uint32_t Builder::GenerateUnaryOpExpression(const ast::UnaryOpExpression* expr) {
    auto result = result_op();
    auto result_id = std::get<uint32_t>(result);

    spv::Op op = spv::Op::OpNop;
    switch (expr->op) {
        case ast::UnaryOp::kComplement:
            op = spv::Op::OpNot;
            break;
        case ast::UnaryOp::kNegation:
            if (TypeOf(expr)->is_float_scalar_or_vector()) {
                op = spv::Op::OpFNegate;
            } else {
                op = spv::Op::OpSNegate;
            }
            break;
        case ast::UnaryOp::kNot:
            op = spv::Op::OpLogicalNot;
            break;
        case ast::UnaryOp::kAddressOf:
        case ast::UnaryOp::kIndirection:
            // Address-of converts a reference to a pointer, and dereference converts
            // a pointer to a reference. These are the same thing in SPIR-V, so this
            // is a no-op.
            return GenerateExpression(expr->expr);
    }

    auto val_id = GenerateExpression(expr->expr);
    if (val_id == 0) {
        return 0;
    }

    auto type_id = GenerateTypeIfNeeded(TypeOf(expr));
    if (type_id == 0) {
        return 0;
    }

    if (!push_function_inst(op, {Operand(type_id), result, Operand(val_id)})) {
        return false;
    }

    return result_id;
}

uint32_t Builder::GetGLSLstd450Import() {
    auto where = import_name_to_id_.find(kGLSLstd450);
    if (where != import_name_to_id_.end()) {
        return where->second;
    }

    // It doesn't exist yet. Generate it.
    auto result = result_op();
    auto id = std::get<uint32_t>(result);

    module_.PushExtImport(spv::Op::OpExtInstImport, {result, Operand(kGLSLstd450)});

    // Remember it for later.
    import_name_to_id_[kGLSLstd450] = id;
    return id;
}

uint32_t Builder::GenerateConstructorExpression(const ast::Variable* var,
                                                const ast::Expression* expr) {
    if (auto* sem = builder_.Sem().GetVal(expr)) {
        if (auto constant = sem->ConstantValue()) {
            return GenerateConstantIfNeeded(constant);
        }
    }
    if (auto* call = builder_.Sem().Get<sem::Call>(expr)) {
        if (call->Target()->IsAnyOf<sem::ValueConstructor, sem::ValueConversion>()) {
            return GenerateValueConstructorOrConversion(call, var);
        }
    }
    TINT_ICE(Writer, builder_.Diagnostics()) << "unknown constructor expression";
    return 0;
}

bool Builder::IsConstructorConst(const ast::Expression* expr) {
    bool is_const = true;
    ast::TraverseExpressions(expr, builder_.Diagnostics(), [&](const ast::Expression* e) {
        if (e->Is<ast::LiteralExpression>()) {
            return ast::TraverseAction::Descend;
        }
        if (auto* ce = e->As<ast::CallExpression>()) {
            auto* sem = builder_.Sem().Get(ce);
            if (sem->Is<sem::Materialize>()) {
                // Materialize can only occur on compile time expressions, so this sub-tree must be
                // constant.
                return ast::TraverseAction::Skip;
            }
            auto* call = sem->As<sem::Call>();
            if (call->Target()->Is<sem::ValueConstructor>()) {
                return ast::TraverseAction::Descend;
            }
        }

        is_const = false;
        return ast::TraverseAction::Stop;
    });
    return is_const;
}

uint32_t Builder::GenerateValueConstructorOrConversion(const sem::Call* call,
                                                       const ast::Variable* var) {
    auto& args = call->Arguments();
    auto* global_var = builder_.Sem().Get<sem::GlobalVariable>(var);
    auto* result_type = call->Type();

    // Generate the zero constructor if there are no values provided.
    if (args.IsEmpty()) {
        return GenerateConstantNullIfNeeded(result_type->UnwrapRef());
    }

    result_type = result_type->UnwrapRef();
    bool constructor_is_const = IsConstructorConst(call->Declaration());
    if (has_error()) {
        return 0;
    }

    bool can_cast_or_copy = result_type->is_scalar();

    if (auto* res_vec = result_type->As<type::Vector>()) {
        if (res_vec->type()->is_scalar()) {
            auto* value_type = args[0]->Type()->UnwrapRef();
            if (auto* val_vec = value_type->As<type::Vector>()) {
                if (val_vec->type()->is_scalar()) {
                    can_cast_or_copy = res_vec->Width() == val_vec->Width();
                }
            }
        }
    }

    if (auto* res_mat = result_type->As<type::Matrix>()) {
        auto* value_type = args[0]->Type()->UnwrapRef();
        if (auto* val_mat = value_type->As<type::Matrix>()) {
            // Generate passthrough for matrices of the same type
            can_cast_or_copy = res_mat == val_mat;
        }
    }

    if (can_cast_or_copy) {
        return GenerateCastOrCopyOrPassthrough(result_type, args[0]->Declaration(), global_var);
    }

    auto type_id = GenerateTypeIfNeeded(result_type);
    if (type_id == 0) {
        return 0;
    }

    bool result_is_constant_composite = constructor_is_const;
    bool result_is_spec_composite = false;

    if (auto* vec = result_type->As<type::Vector>()) {
        result_type = vec->type();
    }

    OperandList ops;
    static constexpr size_t kOpsResultIdx = 1;
    static constexpr size_t kOpsFirstValueIdx = 2;
    ops.reserve(8);
    ops.push_back(Operand(type_id));
    ops.push_back(Operand(0u));  // Placeholder for the result ID

    for (auto* e : args) {
        uint32_t id = 0;
        id = GenerateExpression(e);
        if (id == 0) {
            return 0;
        }

        auto* value_type = e->Type()->UnwrapRef();
        // If the result and value types are the same we can just use the object.
        // If the result is not a vector then we should have validated that the
        // value type is a correctly sized vector so we can just use it directly.
        if (result_type == value_type || result_type->Is<type::Matrix>() ||
            result_type->Is<type::Array>() || result_type->Is<type::Struct>()) {
            ops.push_back(Operand(id));
            continue;
        }

        // Both scalars, but not the same type so we need to generate a conversion
        // of the value.
        if (value_type->is_scalar() && result_type->is_scalar()) {
            id = GenerateCastOrCopyOrPassthrough(result_type, args[0]->Declaration(), global_var);
            ops.push_back(Operand(id));
            continue;
        }

        // When handling vectors as the values there a few cases to take into
        // consideration:
        //  1. Module scoped vec3<f32>(vec2<f32>(1, 2), 3)  -> OpSpecConstantOp
        //  2. Function scoped vec3<f32>(vec2<f32>(1, 2), 3) ->  OpCompositeExtract
        //  3. Either array<vec3<f32>, 1>(vec3<f32>(1, 2, 3))  -> use the ID.
        //       -> handled above
        //
        // For cases 1 and 2, if the type is different we also may need to insert
        // a type cast.
        if (auto* vec = value_type->As<type::Vector>()) {
            auto* vec_type = vec->type();

            auto value_type_id = GenerateTypeIfNeeded(vec_type);
            if (value_type_id == 0) {
                return 0;
            }

            for (uint32_t i = 0; i < vec->Width(); ++i) {
                auto extract = result_op();
                auto extract_id = std::get<uint32_t>(extract);

                if (!global_var) {
                    // A non-global initializer. Case 2.
                    if (!push_function_inst(
                            spv::Op::OpCompositeExtract,
                            {Operand(value_type_id), extract, Operand(id), Operand(i)})) {
                        return false;
                    }

                    // We no longer have a constant composite, but have to do a
                    // composite construction as these calls are inside a function.
                    result_is_constant_composite = false;
                } else {
                    // A global initializer, must use OpSpecConstantOp. Case 1.
                    auto idx_id = GenerateConstantIfNeeded(ScalarConstant::U32(i));
                    if (idx_id == 0) {
                        return 0;
                    }
                    module_.PushType(spv::Op::OpSpecConstantOp, {Operand(value_type_id), extract,
                                                                 U32Operand(SpvOpCompositeExtract),
                                                                 Operand(id), Operand(idx_id)});

                    result_is_spec_composite = true;
                }

                ops.push_back(Operand(extract_id));
            }
        } else {
            TINT_ICE(Writer, builder_.Diagnostics()) << "Unhandled type cast value type";
            return 0;
        }
    }

    // For a single-value vector initializer, splat the initializer value.
    auto* const init_result_type = call->Type()->UnwrapRef();
    if (args.Length() == 1 && init_result_type->is_scalar_vector() &&
        args[0]->Type()->UnwrapRef()->is_scalar()) {
        size_t vec_size = init_result_type->As<type::Vector>()->Width();
        for (size_t i = 0; i < (vec_size - 1); ++i) {
            ops.push_back(ops[kOpsFirstValueIdx]);
        }
    }

    auto& stack = (result_is_spec_composite || result_is_constant_composite)
                      ? scope_stack_[0]       // Global scope
                      : scope_stack_.back();  // Lexical scope

    return utils::GetOrCreate(stack.type_init_to_id_, OperandListKey{ops}, [&]() -> uint32_t {
        auto result = result_op();
        ops[kOpsResultIdx] = result;

        if (result_is_spec_composite) {
            module_.PushType(spv::Op::OpSpecConstantComposite, ops);
        } else if (result_is_constant_composite) {
            module_.PushType(spv::Op::OpConstantComposite, ops);
        } else {
            if (!push_function_inst(spv::Op::OpCompositeConstruct, ops)) {
                return 0;
            }
        }

        return std::get<uint32_t>(result);
    });
}

uint32_t Builder::GenerateCastOrCopyOrPassthrough(const type::Type* to_type,
                                                  const ast::Expression* from_expr,
                                                  bool is_global_init) {
    // This should not happen as we rely on constant folding to obviate
    // casts/conversions for module-scope variables
    if (TINT_UNLIKELY(is_global_init)) {
        TINT_ICE(Writer, builder_.Diagnostics())
            << "Module-level conversions are not supported. Conversions should "
               "have already been constant-folded by the FoldConstants transform.";
        return 0;
    }

    auto elem_type_of = [](const type::Type* t) -> const type::Type* {
        if (t->is_scalar()) {
            return t;
        }
        if (auto* v = t->As<type::Vector>()) {
            return v->type();
        }
        return nullptr;
    };

    auto result = result_op();
    auto result_id = std::get<uint32_t>(result);

    auto result_type_id = GenerateTypeIfNeeded(to_type);
    if (result_type_id == 0) {
        return 0;
    }

    auto val_id = GenerateExpression(from_expr);
    if (val_id == 0) {
        return 0;
    }

    auto* from_type = TypeOf(from_expr)->UnwrapRef();

    spv::Op op = spv::Op::OpNop;
    if ((from_type->Is<type::I32>() && to_type->is_float_scalar()) ||
        (from_type->is_signed_integer_vector() && to_type->is_float_vector())) {
        op = spv::Op::OpConvertSToF;
    } else if ((from_type->Is<type::U32>() && to_type->is_float_scalar()) ||
               (from_type->is_unsigned_integer_vector() && to_type->is_float_vector())) {
        op = spv::Op::OpConvertUToF;
    } else if ((from_type->is_float_scalar() && to_type->Is<type::I32>()) ||
               (from_type->is_float_vector() && to_type->is_signed_integer_vector())) {
        op = spv::Op::OpConvertFToS;
    } else if ((from_type->is_float_scalar() && to_type->Is<type::U32>()) ||
               (from_type->is_float_vector() && to_type->is_unsigned_integer_vector())) {
        op = spv::Op::OpConvertFToU;
    } else if (from_type->IsAnyOf<type::Bool, type::F32, type::I32, type::U32, type::F16,
                                  type::Vector>() &&
               from_type == to_type) {
        // Identity initializer for scalar and vector types
        return val_id;
    } else if ((from_type->is_float_scalar() && to_type->is_float_scalar()) ||
               (from_type->is_float_vector() && to_type->is_float_vector() &&
                from_type->As<type::Vector>()->Width() == to_type->As<type::Vector>()->Width())) {
        // Convert between f32 and f16 types.
        // OpFConvert requires the scalar component types to be different, and the case of from_type
        // and to_type being the same floating point scalar or vector type, i.e. identity
        // initializer, is already handled in the previous else-if clause.
        op = spv::Op::OpFConvert;
    } else if ((from_type->Is<type::I32>() && to_type->Is<type::U32>()) ||
               (from_type->Is<type::U32>() && to_type->Is<type::I32>()) ||
               (from_type->is_signed_integer_vector() && to_type->is_unsigned_integer_vector()) ||
               (from_type->is_unsigned_integer_vector() &&
                to_type->is_integer_scalar_or_vector())) {
        op = spv::Op::OpBitcast;
    } else if ((from_type->is_numeric_scalar() && to_type->Is<type::Bool>()) ||
               (from_type->is_numeric_vector() && to_type->is_bool_vector())) {
        // Convert scalar (vector) to bool (vector)

        // Return the result of comparing from_expr with zero
        uint32_t zero = GenerateConstantNullIfNeeded(from_type);
        const auto* from_elem_type = elem_type_of(from_type);
        op = from_elem_type->is_integer_scalar() ? spv::Op::OpINotEqual : spv::Op::OpFUnordNotEqual;
        if (!push_function_inst(op, {Operand(result_type_id), Operand(result_id), Operand(val_id),
                                     Operand(zero)})) {
            return 0;
        }

        return result_id;
    } else if (from_type->is_bool_scalar_or_vector() && to_type->is_numeric_scalar_or_vector()) {
        // Convert bool scalar/vector to numeric scalar/vector.
        // Use the bool to select between 1 (if true) and 0 (if false).

        const auto* to_elem_type = elem_type_of(to_type);
        uint32_t one_id;
        uint32_t zero_id;
        if (to_elem_type->Is<type::F32>()) {
            zero_id = GenerateConstantIfNeeded(ScalarConstant::F32(0));
            one_id = GenerateConstantIfNeeded(ScalarConstant::F32(1));
        } else if (to_elem_type->Is<type::F16>()) {
            zero_id = GenerateConstantIfNeeded(ScalarConstant::F16(0));
            one_id = GenerateConstantIfNeeded(ScalarConstant::F16(1));
        } else if (to_elem_type->Is<type::U32>()) {
            zero_id = GenerateConstantIfNeeded(ScalarConstant::U32(0));
            one_id = GenerateConstantIfNeeded(ScalarConstant::U32(1));
        } else if (to_elem_type->Is<type::I32>()) {
            zero_id = GenerateConstantIfNeeded(ScalarConstant::I32(0));
            one_id = GenerateConstantIfNeeded(ScalarConstant::I32(1));
        } else {
            TINT_ICE(Writer, builder_.Diagnostics())
                << "invalid destination type for bool conversion";
            return false;
        }
        if (auto* to_vec = to_type->As<type::Vector>()) {
            // Splat the scalars into vectors.
            zero_id = GenerateConstantVectorSplatIfNeeded(to_vec, zero_id);
            one_id = GenerateConstantVectorSplatIfNeeded(to_vec, one_id);
        }
        if (!one_id || !zero_id) {
            return false;
        }

        op = spv::Op::OpSelect;
        if (!push_function_inst(op, {Operand(result_type_id), Operand(result_id), Operand(val_id),
                                     Operand(one_id), Operand(zero_id)})) {
            return 0;
        }

        return result_id;
    } else if (TINT_LIKELY(from_type->Is<type::Matrix>() && to_type->Is<type::Matrix>())) {
        // SPIRV does not support matrix conversion, the only valid case is matrix identity
        // initializer. Matrix conversion between f32 and f16 should be transformed into vector
        // conversions for each column vectors by VectorizeMatrixConversions.
        auto* from_mat = from_type->As<type::Matrix>();
        auto* to_mat = to_type->As<type::Matrix>();
        if (TINT_LIKELY(from_mat == to_mat)) {
            return val_id;
        }
        TINT_ICE(Writer, builder_.Diagnostics())
            << "matrix conversion is not supported and should have been handled by "
               "VectorizeMatrixConversions";
    } else {
        TINT_ICE(Writer, builder_.Diagnostics()) << "Invalid from_type";
    }

    if (op == spv::Op::OpNop) {
        TINT_ICE(Writer, builder_.Diagnostics())
            << "unable to determine conversion type for cast, from: " + from_type->FriendlyName() +
                   " to: " + to_type->FriendlyName();
        return 0;
    }

    if (!push_function_inst(op, {Operand(result_type_id), result, Operand(val_id)})) {
        return 0;
    }

    return result_id;
}

uint32_t Builder::GenerateLiteralIfNeeded(const ast::LiteralExpression* lit) {
    ScalarConstant constant;
    Switch(
        lit,
        [&](const ast::BoolLiteralExpression* l) {
            constant.kind = ScalarConstant::Kind::kBool;
            constant.value.b = l->value;
        },
        [&](const ast::IntLiteralExpression* i) {
            switch (i->suffix) {
                case ast::IntLiteralExpression::Suffix::kNone:
                case ast::IntLiteralExpression::Suffix::kI:
                    constant.kind = ScalarConstant::Kind::kI32;
                    constant.value.i32 = static_cast<int32_t>(i->value);
                    return;
                case ast::IntLiteralExpression::Suffix::kU:
                    constant.kind = ScalarConstant::Kind::kU32;
                    constant.value.u32 = static_cast<uint32_t>(i->value);
                    return;
            }
        },
        [&](const ast::FloatLiteralExpression* f) {
            switch (f->suffix) {
                case ast::FloatLiteralExpression::Suffix::kNone:
                case ast::FloatLiteralExpression::Suffix::kF:
                    constant.kind = ScalarConstant::Kind::kF32;
                    constant.value.f32 = static_cast<float>(f->value);
                    return;
                case ast::FloatLiteralExpression::Suffix::kH:
                    constant.kind = ScalarConstant::Kind::kF16;
                    constant.value.f16 = {f16(static_cast<float>(f->value)).BitsRepresentation()};
                    return;
            }
        },
        [&](Default) { TINT_ICE(Writer, builder_.Diagnostics()) << "unknown literal type"; });

    if (has_error()) {
        return false;
    }

    return GenerateConstantIfNeeded(constant);
}

uint32_t Builder::GenerateConstantIfNeeded(const constant::Value* constant) {
    if (constant->AllZero()) {
        return GenerateConstantNullIfNeeded(constant->Type());
    }
    auto* ty = constant->Type();

    auto composite = [&](size_t el_count) -> uint32_t {
        auto type_id = GenerateTypeIfNeeded(ty);
        if (!type_id) {
            return 0;
        }

        static constexpr size_t kOpsResultIdx = 1;  // operand index of the result

        std::vector<Operand> ops;
        ops.reserve(el_count + 2);
        ops.emplace_back(type_id);
        ops.push_back(Operand(0u));  // Placeholder for the result ID

        for (size_t i = 0; i < el_count; i++) {
            auto id = GenerateConstantIfNeeded(constant->Index(i));
            if (!id) {
                return 0;
            }
            ops.emplace_back(id);
        }

        auto& global_scope = scope_stack_[0];
        return utils::GetOrCreate(
            global_scope.type_init_to_id_, OperandListKey{ops}, [&]() -> uint32_t {
                auto result = result_op();
                ops[kOpsResultIdx] = result;
                module_.PushType(spv::Op::OpConstantComposite, std::move(ops));
                return std::get<uint32_t>(result);
            });
    };

    return Switch(
        ty,  //
        [&](const type::Bool*) {
            bool val = constant->ValueAs<bool>();
            return GenerateConstantIfNeeded(ScalarConstant::Bool(val));
        },
        [&](const type::F32*) {
            auto val = constant->ValueAs<f32>();
            return GenerateConstantIfNeeded(ScalarConstant::F32(val.value));
        },
        [&](const type::F16*) {
            auto val = constant->ValueAs<f16>();
            return GenerateConstantIfNeeded(ScalarConstant::F16(val.value));
        },
        [&](const type::I32*) {
            auto val = constant->ValueAs<i32>();
            return GenerateConstantIfNeeded(ScalarConstant::I32(val.value));
        },
        [&](const type::U32*) {
            auto val = constant->ValueAs<u32>();
            return GenerateConstantIfNeeded(ScalarConstant::U32(val.value));
        },
        [&](const type::Vector* v) { return composite(v->Width()); },
        [&](const type::Matrix* m) { return composite(m->columns()); },
        [&](const type::Array* a) {
            auto count = a->ConstantCount();
            if (!count) {
                TINT_ICE(Writer, builder_.Diagnostics()) << type::Array::kErrExpectedConstantCount;
                return static_cast<uint32_t>(0);
            }
            return composite(count.value());
        },
        [&](const type::Struct* s) { return composite(s->Members().Length()); },
        [&](Default) {
            TINT_ICE(Writer, builder_.Diagnostics())
                << "unhandled constant type: " + ty->FriendlyName();
            return 0;
        });
}

uint32_t Builder::GenerateConstantIfNeeded(const ScalarConstant& constant) {
    auto it = const_to_id_.find(constant);
    if (it != const_to_id_.end()) {
        return it->second;
    }

    uint32_t type_id = 0;

    switch (constant.kind) {
        case ScalarConstant::Kind::kU32: {
            type_id = GenerateTypeIfNeeded(builder_.create<type::U32>());
            break;
        }
        case ScalarConstant::Kind::kI32: {
            type_id = GenerateTypeIfNeeded(builder_.create<type::I32>());
            break;
        }
        case ScalarConstant::Kind::kF32: {
            type_id = GenerateTypeIfNeeded(builder_.create<type::F32>());
            break;
        }
        case ScalarConstant::Kind::kF16: {
            type_id = GenerateTypeIfNeeded(builder_.create<type::F16>());
            break;
        }
        case ScalarConstant::Kind::kBool: {
            type_id = GenerateTypeIfNeeded(builder_.create<type::Bool>());
            break;
        }
    }

    if (type_id == 0) {
        return 0;
    }

    auto result = result_op();
    auto result_id = std::get<uint32_t>(result);

    switch (constant.kind) {
        case ScalarConstant::Kind::kU32: {
            module_.PushType(spv::Op::OpConstant,
                             {Operand(type_id), result, Operand(constant.value.u32)});
            break;
        }
        case ScalarConstant::Kind::kI32: {
            module_.PushType(spv::Op::OpConstant,
                             {Operand(type_id), result, U32Operand(constant.value.i32)});
            break;
        }
        case ScalarConstant::Kind::kF32: {
            module_.PushType(spv::Op::OpConstant,
                             {Operand(type_id), result, Operand(constant.value.f32)});
            break;
        }
        case ScalarConstant::Kind::kF16: {
            module_.PushType(
                spv::Op::OpConstant,
                {Operand(type_id), result, U32Operand(constant.value.f16.bits_representation)});
            break;
        }
        case ScalarConstant::Kind::kBool: {
            if (constant.value.b) {
                module_.PushType(spv::Op::OpConstantTrue, {Operand(type_id), result});
            } else {
                module_.PushType(spv::Op::OpConstantFalse, {Operand(type_id), result});
            }
            break;
        }
    }

    const_to_id_[constant] = result_id;
    return result_id;
}

uint32_t Builder::GenerateConstantNullIfNeeded(const type::Type* type) {
    auto type_id = GenerateTypeIfNeeded(type);
    if (type_id == 0) {
        return 0;
    }

    return utils::GetOrCreate(const_null_to_id_, type, [&] {
        auto result = result_op();

        module_.PushType(spv::Op::OpConstantNull, {Operand(type_id), result});

        return std::get<uint32_t>(result);
    });
}

uint32_t Builder::GenerateConstantVectorSplatIfNeeded(const type::Vector* type, uint32_t value_id) {
    auto type_id = GenerateTypeIfNeeded(type);
    if (type_id == 0 || value_id == 0) {
        return 0;
    }

    uint64_t key = (static_cast<uint64_t>(type->Width()) << 32) + value_id;
    return utils::GetOrCreate(const_splat_to_id_, key, [&] {
        auto result = result_op();
        auto result_id = std::get<uint32_t>(result);

        OperandList ops;
        ops.push_back(Operand(type_id));
        ops.push_back(result);
        for (uint32_t i = 0; i < type->Width(); i++) {
            ops.push_back(Operand(value_id));
        }
        module_.PushType(spv::Op::OpConstantComposite, ops);

        const_splat_to_id_[key] = result_id;
        return result_id;
    });
}

uint32_t Builder::GenerateShortCircuitBinaryExpression(const ast::BinaryExpression* expr) {
    auto lhs_id = GenerateExpression(expr->lhs);
    if (lhs_id == 0) {
        return false;
    }

    // Get the ID of the basic block where control flow will diverge. It's the
    // last basic block generated for the left-hand-side of the operator.
    auto original_label_id = current_label_id_;

    auto type_id = GenerateTypeIfNeeded(TypeOf(expr));
    if (type_id == 0) {
        return 0;
    }

    auto merge_block = result_op();
    auto merge_block_id = std::get<uint32_t>(merge_block);

    auto block = result_op();
    auto block_id = std::get<uint32_t>(block);

    auto true_block_id = block_id;
    auto false_block_id = merge_block_id;

    // For a logical or we want to only check the RHS if the LHS is failed.
    if (expr->IsLogicalOr()) {
        std::swap(true_block_id, false_block_id);
    }

    if (!push_function_inst(spv::Op::OpSelectionMerge,
                            {Operand(merge_block_id), U32Operand(SpvSelectionControlMaskNone)})) {
        return 0;
    }
    if (!push_function_inst(spv::Op::OpBranchConditional,
                            {Operand(lhs_id), Operand(true_block_id), Operand(false_block_id)})) {
        return 0;
    }

    // Output block to check the RHS
    if (!GenerateLabel(block_id)) {
        return 0;
    }
    auto rhs_id = GenerateExpression(expr->rhs);
    if (rhs_id == 0) {
        return 0;
    }

    // Get the block ID of the last basic block generated for the right-hand-side
    // expression. That block will be an immediate predecessor to the merge block.
    auto rhs_block_id = current_label_id_;
    if (!push_function_inst(spv::Op::OpBranch, {Operand(merge_block_id)})) {
        return 0;
    }

    // Output the merge block
    if (!GenerateLabel(merge_block_id)) {
        return 0;
    }

    auto result = result_op();
    auto result_id = std::get<uint32_t>(result);

    if (!push_function_inst(spv::Op::OpPhi,
                            {Operand(type_id), result, Operand(lhs_id), Operand(original_label_id),
                             Operand(rhs_id), Operand(rhs_block_id)})) {
        return 0;
    }

    return result_id;
}

uint32_t Builder::GenerateSplat(uint32_t scalar_id, const type::Type* vec_type) {
    // Create a new vector to splat scalar into
    auto splat_vector = result_op();
    auto* splat_vector_type = builder_.create<type::Pointer>(
        vec_type, builtin::AddressSpace::kFunction, builtin::Access::kReadWrite);
    push_function_var({Operand(GenerateTypeIfNeeded(splat_vector_type)), splat_vector,
                       U32Operand(ConvertAddressSpace(builtin::AddressSpace::kFunction)),
                       Operand(GenerateConstantNullIfNeeded(vec_type))});

    // Splat scalar into vector
    auto splat_result = result_op();
    OperandList ops;
    ops.push_back(Operand(GenerateTypeIfNeeded(vec_type)));
    ops.push_back(splat_result);
    for (size_t i = 0; i < vec_type->As<type::Vector>()->Width(); ++i) {
        ops.push_back(Operand(scalar_id));
    }
    if (!push_function_inst(spv::Op::OpCompositeConstruct, ops)) {
        return 0;
    }

    return std::get<uint32_t>(splat_result);
}

uint32_t Builder::GenerateMatrixAddOrSub(uint32_t lhs_id,
                                         uint32_t rhs_id,
                                         const type::Matrix* type,
                                         spv::Op op) {
    // Example addition of two matrices:
    // %31 = OpLoad %mat3v4float %m34
    // %32 = OpLoad %mat3v4float %m34
    // %33 = OpCompositeExtract %v4float %31 0
    // %34 = OpCompositeExtract %v4float %32 0
    // %35 = OpFAdd %v4float %33 %34
    // %36 = OpCompositeExtract %v4float %31 1
    // %37 = OpCompositeExtract %v4float %32 1
    // %38 = OpFAdd %v4float %36 %37
    // %39 = OpCompositeExtract %v4float %31 2
    // %40 = OpCompositeExtract %v4float %32 2
    // %41 = OpFAdd %v4float %39 %40
    // %42 = OpCompositeConstruct %mat3v4float %35 %38 %41

    auto* column_type = builder_.create<type::Vector>(type->type(), type->rows());
    auto column_type_id = GenerateTypeIfNeeded(column_type);

    OperandList ops;

    for (uint32_t i = 0; i < type->columns(); ++i) {
        // Extract column `i` from lhs mat
        auto lhs_column_id = result_op();
        if (!push_function_inst(
                spv::Op::OpCompositeExtract,
                {Operand(column_type_id), lhs_column_id, Operand(lhs_id), Operand(i)})) {
            return 0;
        }

        // Extract column `i` from rhs mat
        auto rhs_column_id = result_op();
        if (!push_function_inst(
                spv::Op::OpCompositeExtract,
                {Operand(column_type_id), rhs_column_id, Operand(rhs_id), Operand(i)})) {
            return 0;
        }

        // Add or subtract the two columns
        auto result = result_op();
        if (!push_function_inst(op,
                                {Operand(column_type_id), result, lhs_column_id, rhs_column_id})) {
            return 0;
        }

        ops.push_back(result);
    }

    // Create the result matrix from the added/subtracted column vectors
    auto result_mat_id = result_op();
    ops.insert(ops.begin(), result_mat_id);
    ops.insert(ops.begin(), Operand(GenerateTypeIfNeeded(type)));
    if (!push_function_inst(spv::Op::OpCompositeConstruct, ops)) {
        return 0;
    }

    return std::get<uint32_t>(result_mat_id);
}

uint32_t Builder::GenerateBinaryExpression(const ast::BinaryExpression* expr) {
    // There is special logic for short circuiting operators.
    if (expr->IsLogicalAnd() || expr->IsLogicalOr()) {
        return GenerateShortCircuitBinaryExpression(expr);
    }

    auto lhs_id = GenerateExpression(expr->lhs);
    if (lhs_id == 0) {
        return 0;
    }

    auto rhs_id = GenerateExpression(expr->rhs);
    if (rhs_id == 0) {
        return 0;
    }

    auto result = result_op();
    auto result_id = std::get<uint32_t>(result);

    auto type_id = GenerateTypeIfNeeded(TypeOf(expr));
    if (type_id == 0) {
        return 0;
    }

    // Handle int and float and the vectors of those types. Other types
    // should have been rejected by validation.
    auto* lhs_type = TypeOf(expr->lhs)->UnwrapRef();
    auto* rhs_type = TypeOf(expr->rhs)->UnwrapRef();

    // Handle matrix-matrix addition and subtraction
    if ((expr->IsAdd() || expr->IsSubtract()) && lhs_type->is_float_matrix() &&
        rhs_type->is_float_matrix()) {
        auto* lhs_mat = lhs_type->As<type::Matrix>();
        auto* rhs_mat = rhs_type->As<type::Matrix>();

        // This should already have been validated by resolver
        if (lhs_mat->rows() != rhs_mat->rows() || lhs_mat->columns() != rhs_mat->columns()) {
            TINT_ICE(Writer, builder_.Diagnostics())
                << "matrices must have same dimensionality for add or subtract";
            return 0;
        }

        return GenerateMatrixAddOrSub(lhs_id, rhs_id, lhs_mat,
                                      expr->IsAdd() ? spv::Op::OpFAdd : spv::Op::OpFSub);
    }

    // For vector-scalar arithmetic operations, splat scalar into a vector. We
    // skip this for multiply as we can use OpVectorTimesScalar.
    const bool is_float_scalar_vector_multiply =
        expr->IsMultiply() && ((lhs_type->is_float_scalar() && rhs_type->is_float_vector()) ||
                               (lhs_type->is_float_vector() && rhs_type->is_float_scalar()));

    if (expr->IsArithmetic() && !is_float_scalar_vector_multiply) {
        if (lhs_type->Is<type::Vector>() && rhs_type->is_numeric_scalar()) {
            uint32_t splat_vector_id = GenerateSplat(rhs_id, lhs_type);
            if (splat_vector_id == 0) {
                return 0;
            }
            rhs_id = splat_vector_id;
            rhs_type = lhs_type;

        } else if (lhs_type->is_numeric_scalar() && rhs_type->Is<type::Vector>()) {
            uint32_t splat_vector_id = GenerateSplat(lhs_id, rhs_type);
            if (splat_vector_id == 0) {
                return 0;
            }
            lhs_id = splat_vector_id;
            lhs_type = rhs_type;
        }
    }

    bool lhs_is_float_or_vec = lhs_type->is_float_scalar_or_vector();
    bool lhs_is_bool_or_vec = lhs_type->is_bool_scalar_or_vector();
    bool lhs_is_integer_or_vec = lhs_type->is_integer_scalar_or_vector();
    bool lhs_is_unsigned = lhs_type->is_unsigned_integer_scalar_or_vector();

    spv::Op op = spv::Op::OpNop;
    if (expr->IsAnd()) {
        if (lhs_is_integer_or_vec) {
            op = spv::Op::OpBitwiseAnd;
        } else if (lhs_is_bool_or_vec) {
            op = spv::Op::OpLogicalAnd;
        } else {
            TINT_ICE(Writer, builder_.Diagnostics()) << "invalid and expression";
            return 0;
        }
    } else if (expr->IsAdd()) {
        op = lhs_is_float_or_vec ? spv::Op::OpFAdd : spv::Op::OpIAdd;
    } else if (expr->IsDivide()) {
        if (lhs_is_float_or_vec) {
            op = spv::Op::OpFDiv;
        } else if (lhs_is_unsigned) {
            op = spv::Op::OpUDiv;
        } else {
            op = spv::Op::OpSDiv;
        }
    } else if (expr->IsEqual()) {
        if (lhs_is_float_or_vec) {
            op = spv::Op::OpFOrdEqual;
        } else if (lhs_is_bool_or_vec) {
            op = spv::Op::OpLogicalEqual;
        } else if (lhs_is_integer_or_vec) {
            op = spv::Op::OpIEqual;
        } else {
            TINT_ICE(Writer, builder_.Diagnostics()) << "invalid equal expression";
            return 0;
        }
    } else if (expr->IsGreaterThan()) {
        if (lhs_is_float_or_vec) {
            op = spv::Op::OpFOrdGreaterThan;
        } else if (lhs_is_unsigned) {
            op = spv::Op::OpUGreaterThan;
        } else {
            op = spv::Op::OpSGreaterThan;
        }
    } else if (expr->IsGreaterThanEqual()) {
        if (lhs_is_float_or_vec) {
            op = spv::Op::OpFOrdGreaterThanEqual;
        } else if (lhs_is_unsigned) {
            op = spv::Op::OpUGreaterThanEqual;
        } else {
            op = spv::Op::OpSGreaterThanEqual;
        }
    } else if (expr->IsLessThan()) {
        if (lhs_is_float_or_vec) {
            op = spv::Op::OpFOrdLessThan;
        } else if (lhs_is_unsigned) {
            op = spv::Op::OpULessThan;
        } else {
            op = spv::Op::OpSLessThan;
        }
    } else if (expr->IsLessThanEqual()) {
        if (lhs_is_float_or_vec) {
            op = spv::Op::OpFOrdLessThanEqual;
        } else if (lhs_is_unsigned) {
            op = spv::Op::OpULessThanEqual;
        } else {
            op = spv::Op::OpSLessThanEqual;
        }
    } else if (expr->IsModulo()) {
        if (lhs_is_float_or_vec) {
            op = spv::Op::OpFRem;
        } else if (lhs_is_unsigned) {
            op = spv::Op::OpUMod;
        } else {
            op = spv::Op::OpSRem;
        }
    } else if (expr->IsMultiply()) {
        if (lhs_type->is_integer_scalar_or_vector()) {
            // If the left hand side is an integer then this _has_ to be OpIMul as
            // there there is no other integer multiplication.
            op = spv::Op::OpIMul;
        } else if (lhs_type->is_float_scalar() && rhs_type->is_float_scalar()) {
            // Float scalars multiply with OpFMul
            op = spv::Op::OpFMul;
        } else if (lhs_type->is_float_vector() && rhs_type->is_float_vector()) {
            // Float vectors must be validated to be the same size and then use OpFMul
            op = spv::Op::OpFMul;
        } else if (lhs_type->is_float_scalar() && rhs_type->is_float_vector()) {
            // Scalar * Vector we need to flip lhs and rhs types
            // because OpVectorTimesScalar expects <vector>, <scalar>
            std::swap(lhs_id, rhs_id);
            op = spv::Op::OpVectorTimesScalar;
        } else if (lhs_type->is_float_vector() && rhs_type->is_float_scalar()) {
            // float vector * scalar
            op = spv::Op::OpVectorTimesScalar;
        } else if (lhs_type->is_float_scalar() && rhs_type->is_float_matrix()) {
            // Scalar * Matrix we need to flip lhs and rhs types because
            // OpMatrixTimesScalar expects <matrix>, <scalar>
            std::swap(lhs_id, rhs_id);
            op = spv::Op::OpMatrixTimesScalar;
        } else if (lhs_type->is_float_matrix() && rhs_type->is_float_scalar()) {
            // float matrix * scalar
            op = spv::Op::OpMatrixTimesScalar;
        } else if (lhs_type->is_float_vector() && rhs_type->is_float_matrix()) {
            // float vector * matrix
            op = spv::Op::OpVectorTimesMatrix;
        } else if (lhs_type->is_float_matrix() && rhs_type->is_float_vector()) {
            // float matrix * vector
            op = spv::Op::OpMatrixTimesVector;
        } else if (lhs_type->is_float_matrix() && rhs_type->is_float_matrix()) {
            // float matrix * matrix
            op = spv::Op::OpMatrixTimesMatrix;
        } else {
            TINT_ICE(Writer, builder_.Diagnostics()) << "invalid multiply expression";
            return 0;
        }
    } else if (expr->IsNotEqual()) {
        if (lhs_is_float_or_vec) {
            op = spv::Op::OpFOrdNotEqual;
        } else if (lhs_is_bool_or_vec) {
            op = spv::Op::OpLogicalNotEqual;
        } else if (lhs_is_integer_or_vec) {
            op = spv::Op::OpINotEqual;
        } else {
            TINT_ICE(Writer, builder_.Diagnostics()) << "invalid not-equal expression";
            return 0;
        }
    } else if (expr->IsOr()) {
        if (lhs_is_integer_or_vec) {
            op = spv::Op::OpBitwiseOr;
        } else if (lhs_is_bool_or_vec) {
            op = spv::Op::OpLogicalOr;
        } else {
            TINT_ICE(Writer, builder_.Diagnostics()) << "invalid and expression";
            return 0;
        }
    } else if (expr->IsShiftLeft()) {
        op = spv::Op::OpShiftLeftLogical;
    } else if (expr->IsShiftRight() && lhs_type->is_signed_integer_scalar_or_vector()) {
        // A shift right with a signed LHS is an arithmetic shift.
        op = spv::Op::OpShiftRightArithmetic;
    } else if (expr->IsShiftRight()) {
        op = spv::Op::OpShiftRightLogical;
    } else if (expr->IsSubtract()) {
        op = lhs_is_float_or_vec ? spv::Op::OpFSub : spv::Op::OpISub;
    } else if (expr->IsXor()) {
        op = spv::Op::OpBitwiseXor;
    } else {
        TINT_ICE(Writer, builder_.Diagnostics()) << "unknown binary expression";
        return 0;
    }

    if (!push_function_inst(op, {Operand(type_id), result, Operand(lhs_id), Operand(rhs_id)})) {
        return 0;
    }
    return result_id;
}

bool Builder::GenerateBlockStatement(const ast::BlockStatement* stmt) {
    PushScope();
    TINT_DEFER(PopScope());
    return GenerateBlockStatementWithoutScoping(stmt);
}

bool Builder::GenerateBlockStatementWithoutScoping(const ast::BlockStatement* stmt) {
    for (auto* block_stmt : stmt->statements) {
        if (!GenerateStatement(block_stmt)) {
            return false;
        }
    }
    return true;
}

uint32_t Builder::GenerateCallExpression(const ast::CallExpression* expr) {
    auto* call = builder_.Sem().Get<sem::Call>(expr);
    auto* target = call->Target();
    return Switch(
        target,  //
        [&](const sem::Function* func) { return GenerateFunctionCall(call, func); },
        [&](const sem::Builtin* builtin) { return GenerateBuiltinCall(call, builtin); },
        [&](const sem::ValueConversion*) {
            return GenerateValueConstructorOrConversion(call, nullptr);
        },
        [&](const sem::ValueConstructor*) {
            return GenerateValueConstructorOrConversion(call, nullptr);
        },
        [&](Default) {
            TINT_ICE(Writer, builder_.Diagnostics())
                << "unhandled call target: " << target->TypeInfo().name;
            return 0;
        });
}

uint32_t Builder::GenerateFunctionCall(const sem::Call* call, const sem::Function* fn) {
    auto* expr = call->Declaration();
    auto* ident = fn->Declaration()->name;

    auto type_id = GenerateTypeIfNeeded(call->Type());
    if (type_id == 0) {
        return 0;
    }

    auto result = result_op();
    auto result_id = std::get<uint32_t>(result);

    OperandList ops = {Operand(type_id), result};

    auto func_id = func_symbol_to_id_[ident->symbol];
    if (func_id == 0) {
        TINT_ICE(Writer, builder_.Diagnostics())
            << "unable to find called function: " + ident->symbol.Name();
        return 0;
    }
    ops.push_back(Operand(func_id));

    for (auto* arg : expr->args) {
        auto id = GenerateExpression(arg);
        if (id == 0) {
            return 0;
        }
        ops.push_back(Operand(id));
    }

    if (!push_function_inst(spv::Op::OpFunctionCall, std::move(ops))) {
        return 0;
    }

    return result_id;
}

uint32_t Builder::GenerateBuiltinCall(const sem::Call* call, const sem::Builtin* builtin) {
    auto result = result_op();
    auto result_id = std::get<uint32_t>(result);

    auto result_type_id = GenerateTypeIfNeeded(builtin->ReturnType());
    if (result_type_id == 0) {
        return 0;
    }

    if (builtin->IsFineDerivative() || builtin->IsCoarseDerivative()) {
        module_.PushCapability(SpvCapabilityDerivativeControl);
    }

    if (builtin->IsImageQuery()) {
        module_.PushCapability(SpvCapabilityImageQuery);
    }

    if (builtin->IsTexture()) {
        if (!GenerateTextureBuiltin(call, builtin, Operand(result_type_id), result)) {
            return 0;
        }
        return result_id;
    }

    if (builtin->IsBarrier()) {
        if (!GenerateControlBarrierBuiltin(builtin)) {
            return 0;
        }
        return result_id;
    }

    if (builtin->IsAtomic()) {
        if (!GenerateAtomicBuiltin(call, builtin, Operand(result_type_id), result)) {
            return 0;
        }
        return result_id;
    }

    // Generates the SPIR-V ID for the expression for the indexed call argument,
    // and loads it if necessary. Returns 0 on error.
    auto get_arg_as_value_id = [&](size_t i, bool generate_load = true) -> uint32_t {
        auto* arg = call->Arguments()[i];
        auto* param = builtin->Parameters()[i];
        auto val_id = GenerateExpression(arg->Declaration());
        if (val_id == 0) {
            return 0;
        }

        if (generate_load && !param->Type()->Is<type::Pointer>()) {
            val_id = GenerateLoadIfNeeded(arg->Type(), val_id);
        }
        return val_id;
    };

    OperandList params = {Operand(result_type_id), result};
    spv::Op op = spv::Op::OpNop;

    // Pushes the arguments for a GlslStd450 extended instruction, and sets op
    // to OpExtInst.
    auto glsl_std450 = [&](uint32_t inst_id) {
        auto set_id = GetGLSLstd450Import();
        params.push_back(Operand(set_id));
        params.push_back(Operand(inst_id));
        op = spv::Op::OpExtInst;
    };

    switch (builtin->Type()) {
        case builtin::Function::kAny:
            if (builtin->Parameters()[0]->Type()->Is<type::Bool>()) {
                // any(v: bool) just resolves to v.
                return get_arg_as_value_id(0);
            }
            op = spv::Op::OpAny;
            break;
        case builtin::Function::kAll:
            if (builtin->Parameters()[0]->Type()->Is<type::Bool>()) {
                // all(v: bool) just resolves to v.
                return get_arg_as_value_id(0);
            }
            op = spv::Op::OpAll;
            break;
        case builtin::Function::kArrayLength: {
            auto* address_of = call->Arguments()[0]->Declaration()->As<ast::UnaryOpExpression>();
            if (!address_of || address_of->op != ast::UnaryOp::kAddressOf) {
                TINT_ICE(Writer, builder_.Diagnostics())
                    << "arrayLength() expected pointer to member access, got " +
                           std::string(address_of->TypeInfo().name);
                return 0;
            }
            auto* array_expr = address_of->expr;

            auto* accessor = array_expr->As<ast::MemberAccessorExpression>();
            if (!accessor) {
                TINT_ICE(Writer, builder_.Diagnostics())
                    << "arrayLength() expected pointer to member access, got pointer to " +
                           std::string(array_expr->TypeInfo().name);
                return 0;
            }

            auto struct_id = GenerateExpression(accessor->object);
            if (struct_id == 0) {
                return 0;
            }
            params.push_back(Operand(struct_id));

            auto* type = TypeOf(accessor->object)->UnwrapRef();
            if (!type->Is<type::Struct>()) {
                TINT_ICE(Writer, builder_.Diagnostics())
                    << "invalid type (" + type->FriendlyName() + ") for runtime array length";
                return 0;
            }
            // Runtime array must be the last member in the structure
            params.push_back(Operand(uint32_t(type->As<type::Struct>()->Members().Length() - 1)));

            if (!push_function_inst(spv::Op::OpArrayLength, params)) {
                return 0;
            }
            return result_id;
        }
        case builtin::Function::kCountOneBits:
            op = spv::Op::OpBitCount;
            break;
        case builtin::Function::kDot: {
            op = spv::Op::OpDot;
            auto* vec_ty = builtin->Parameters()[0]->Type()->As<type::Vector>();
            if (vec_ty->type()->is_integer_scalar()) {
                // TODO(crbug.com/tint/1267): OpDot requires floating-point types, but
                // WGSL also supports integer types. SPV_KHR_integer_dot_product adds
                // support for integer vectors. Use it if it is available.
                auto el_ty = Operand(GenerateTypeIfNeeded(vec_ty->type()));
                auto vec_a = Operand(get_arg_as_value_id(0));
                auto vec_b = Operand(get_arg_as_value_id(1));
                if (std::get<uint32_t>(vec_a) == 0 || std::get<uint32_t>(vec_b) == 0) {
                    return 0;
                }

                auto sum = Operand(0u);
                for (uint32_t i = 0; i < vec_ty->Width(); i++) {
                    auto a = result_op();
                    auto b = result_op();
                    auto mul = result_op();
                    if (!push_function_inst(spv::Op::OpCompositeExtract,
                                            {el_ty, a, vec_a, Operand(i)}) ||
                        !push_function_inst(spv::Op::OpCompositeExtract,
                                            {el_ty, b, vec_b, Operand(i)}) ||
                        !push_function_inst(spv::Op::OpIMul, {el_ty, mul, a, b})) {
                        return 0;
                    }
                    if (i == 0) {
                        sum = mul;
                    } else {
                        auto prev_sum = sum;
                        auto is_last_el = i == (vec_ty->Width() - 1);
                        sum = is_last_el ? Operand(result_id) : result_op();
                        if (!push_function_inst(spv::Op::OpIAdd, {el_ty, sum, prev_sum, mul})) {
                            return 0;
                        }
                    }
                }
                return result_id;
            }
            break;
        }
        case builtin::Function::kDpdx:
            op = spv::Op::OpDPdx;
            break;
        case builtin::Function::kDpdxCoarse:
            op = spv::Op::OpDPdxCoarse;
            break;
        case builtin::Function::kDpdxFine:
            op = spv::Op::OpDPdxFine;
            break;
        case builtin::Function::kDpdy:
            op = spv::Op::OpDPdy;
            break;
        case builtin::Function::kDpdyCoarse:
            op = spv::Op::OpDPdyCoarse;
            break;
        case builtin::Function::kDpdyFine:
            op = spv::Op::OpDPdyFine;
            break;
        case builtin::Function::kExtractBits:
            op = builtin->Parameters()[0]->Type()->is_unsigned_integer_scalar_or_vector()
                     ? spv::Op::OpBitFieldUExtract
                     : spv::Op::OpBitFieldSExtract;
            break;
        case builtin::Function::kFwidth:
            op = spv::Op::OpFwidth;
            break;
        case builtin::Function::kFwidthCoarse:
            op = spv::Op::OpFwidthCoarse;
            break;
        case builtin::Function::kFwidthFine:
            op = spv::Op::OpFwidthFine;
            break;
        case builtin::Function::kInsertBits:
            op = spv::Op::OpBitFieldInsert;
            break;
        case builtin::Function::kMix: {
            auto std450 = Operand(GetGLSLstd450Import());

            auto a_id = get_arg_as_value_id(0);
            auto b_id = get_arg_as_value_id(1);
            auto f_id = get_arg_as_value_id(2);
            if (!a_id || !b_id || !f_id) {
                return 0;
            }

            // If the interpolant is scalar but the objects are vectors, we need to
            // splat the interpolant into a vector of the same size.
            auto* result_vector_type = builtin->ReturnType()->As<type::Vector>();
            if (result_vector_type && builtin->Parameters()[2]->Type()->is_scalar()) {
                f_id = GenerateSplat(f_id, builtin->Parameters()[0]->Type());
                if (f_id == 0) {
                    return 0;
                }
            }

            if (!push_function_inst(spv::Op::OpExtInst, {Operand(result_type_id), result, std450,
                                                         U32Operand(GLSLstd450FMix), Operand(a_id),
                                                         Operand(b_id), Operand(f_id)})) {
                return 0;
            }
            return result_id;
        }
        case builtin::Function::kQuantizeToF16:
            op = spv::Op::OpQuantizeToF16;
            break;
        case builtin::Function::kReverseBits:
            op = spv::Op::OpBitReverse;
            break;
        case builtin::Function::kSelect: {
            // Note: Argument order is different in WGSL and SPIR-V
            auto cond_id = get_arg_as_value_id(2);
            auto true_id = get_arg_as_value_id(1);
            auto false_id = get_arg_as_value_id(0);
            if (!cond_id || !true_id || !false_id) {
                return 0;
            }

            // If the condition is scalar but the objects are vectors, we need to
            // splat the condition into a vector of the same size.
            // TODO(jrprice): If we're targeting SPIR-V 1.4, we don't need to do this.
            auto* result_vector_type = builtin->ReturnType()->As<type::Vector>();
            if (result_vector_type && builtin->Parameters()[2]->Type()->is_scalar()) {
                auto* bool_vec_ty = builder_.create<type::Vector>(builder_.create<type::Bool>(),
                                                                  result_vector_type->Width());
                if (!GenerateTypeIfNeeded(bool_vec_ty)) {
                    return 0;
                }
                cond_id = GenerateSplat(cond_id, bool_vec_ty);
                if (cond_id == 0) {
                    return 0;
                }
            }

            if (!push_function_inst(spv::Op::OpSelect,
                                    {Operand(result_type_id), result, Operand(cond_id),
                                     Operand(true_id), Operand(false_id)})) {
                return 0;
            }
            return result_id;
        }
        case builtin::Function::kTranspose:
            op = spv::Op::OpTranspose;
            break;
        case builtin::Function::kAbs:
            if (builtin->ReturnType()->is_unsigned_integer_scalar_or_vector()) {
                // abs() only operates on *signed* integers.
                // This is a no-op for unsigned integers.
                return get_arg_as_value_id(0);
            }
            if (builtin->ReturnType()->is_float_scalar_or_vector()) {
                glsl_std450(GLSLstd450FAbs);
            } else {
                glsl_std450(GLSLstd450SAbs);
            }
            break;
        case builtin::Function::kDot4I8Packed: {
            auto first_param_id = get_arg_as_value_id(0);
            auto second_param_id = get_arg_as_value_id(1);
            if (!push_function_inst(spv::Op::OpSDotKHR,
                                    {Operand(result_type_id), result, Operand(first_param_id),
                                     Operand(second_param_id),
                                     Operand(static_cast<uint32_t>(
                                         spv::PackedVectorFormat::PackedVectorFormat4x8BitKHR))})) {
                return 0;
            }
            return result_id;
        }
        case builtin::Function::kDot4U8Packed: {
            auto first_param_id = get_arg_as_value_id(0);
            auto second_param_id = get_arg_as_value_id(1);
            if (!push_function_inst(spv::Op::OpUDotKHR,
                                    {Operand(result_type_id), result, Operand(first_param_id),
                                     Operand(second_param_id),
                                     Operand(static_cast<uint32_t>(
                                         spv::PackedVectorFormat::PackedVectorFormat4x8BitKHR))})) {
                return 0;
            }
            return result_id;
        }
        default: {
            auto inst_id = builtin_to_glsl_method(builtin);
            if (inst_id == 0) {
                TINT_ICE(Writer, builder_.Diagnostics())
                    << "unknown method " + std::string(builtin->str());
                return 0;
            }
            glsl_std450(inst_id);
            break;
        }
    }

    if (op == spv::Op::OpNop) {
        TINT_ICE(Writer, builder_.Diagnostics())
            << "unable to determine operator for: " + std::string(builtin->str());
        return 0;
    }

    for (size_t i = 0; i < call->Arguments().Length(); i++) {
        if (auto val_id = get_arg_as_value_id(i)) {
            params.emplace_back(Operand(val_id));
        } else {
            return 0;
        }
    }

    if (!push_function_inst(op, params)) {
        return 0;
    }

    return result_id;
}

bool Builder::GenerateTextureBuiltin(const sem::Call* call,
                                     const sem::Builtin* builtin,
                                     Operand result_type,
                                     Operand result_id) {
    using Usage = sem::ParameterUsage;

    auto& signature = builtin->Signature();
    auto& arguments = call->Arguments();

    // Generates the given expression, returning the operand ID
    auto gen = [&](const sem::ValueExpression* expr) { return Operand(GenerateExpression(expr)); };

    // Returns the argument with the given usage
    auto arg = [&](Usage usage) {
        int idx = signature.IndexOf(usage);
        return (idx >= 0) ? arguments[static_cast<size_t>(idx)] : nullptr;
    };

    // Generates the argument with the given usage, returning the operand ID
    auto gen_arg = [&](Usage usage) {
        auto* argument = arg(usage);
        if (TINT_UNLIKELY(!argument)) {
            TINT_ICE(Writer, builder_.Diagnostics())
                << "missing argument " << static_cast<int>(usage);
        }
        return gen(argument);
    };

    auto* texture = arg(Usage::kTexture);
    if (TINT_UNLIKELY(!texture)) {
        TINT_ICE(Writer, builder_.Diagnostics()) << "missing texture argument";
    }

    auto* texture_type = texture->Type()->UnwrapRef()->As<type::Texture>();

    auto op = spv::Op::OpNop;

    // Custom function to call after the texture-builtin op has been generated.
    std::function<bool()> post_emission = [] { return true; };

    // Populate the spirv_params with common parameters
    OperandList spirv_params;
    spirv_params.reserve(8);  // Enough to fit most parameter lists

    // Extra image operands, appended to spirv_params.
    struct ImageOperand {
        SpvImageOperandsMask mask;
        Operand operand;
    };
    std::vector<ImageOperand> image_operands;
    image_operands.reserve(4);  // Enough to fit most parameter lists

    // Appends `result_type` and `result_id` to `spirv_params`
    auto append_result_type_and_id_to_spirv_params = [&]() {
        spirv_params.emplace_back(std::move(result_type));
        spirv_params.emplace_back(std::move(result_id));
    };

    // Appends a result type and id to `spirv_params`, possibly adding a
    // post_emission step.
    //
    // If the texture is a depth texture, then this function wraps the result of
    // the op with a OpCompositeExtract to evaluate to the first element of the
    // returned vector. This is done as the WGSL texture reading functions for
    // depths return a single float scalar instead of a vector.
    //
    // If the texture is not a depth texture, then this function simply delegates
    // to calling append_result_type_and_id_to_spirv_params().
    auto append_result_type_and_id_to_spirv_params_for_read = [&]() {
        if (texture_type->IsAnyOf<type::DepthTexture, type::DepthMultisampledTexture>()) {
            auto* f32 = builder_.create<type::F32>();
            auto* spirv_result_type = builder_.create<type::Vector>(f32, 4u);
            auto spirv_result = result_op();
            post_emission = [=] {
                return push_function_inst(spv::Op::OpCompositeExtract,
                                          {result_type, result_id, spirv_result, Operand(0u)});
            };
            auto spirv_result_type_id = GenerateTypeIfNeeded(spirv_result_type);
            if (spirv_result_type_id == 0) {
                return false;
            }
            spirv_params.emplace_back(Operand(spirv_result_type_id));
            spirv_params.emplace_back(spirv_result);
            return true;
        }

        append_result_type_and_id_to_spirv_params();
        return true;
    };

    // Appends a result type and id to `spirv_params`, by first swizzling the
    // result of the op with `swizzle`.
    auto append_result_type_and_id_to_spirv_params_swizzled = [&](uint32_t spirv_result_width,
                                                                  std::vector<uint32_t> swizzle) {
        if (swizzle.empty()) {
            append_result_type_and_id_to_spirv_params();
        } else {
            // Assign post_emission to swizzle the result of the call to
            // OpImageQuerySize[Lod].
            auto* element_type = ElementTypeOf(call->Type());
            auto spirv_result = result_op();
            auto* spirv_result_type =
                builder_.create<type::Vector>(element_type, spirv_result_width);
            if (swizzle.size() > 1) {
                post_emission = [=] {
                    OperandList operands{
                        result_type,
                        result_id,
                        spirv_result,
                        spirv_result,
                    };
                    for (auto idx : swizzle) {
                        operands.emplace_back(Operand(idx));
                    }
                    return push_function_inst(spv::Op::OpVectorShuffle, operands);
                };
            } else {
                post_emission = [=] {
                    return push_function_inst(
                        spv::Op::OpCompositeExtract,
                        {result_type, result_id, spirv_result, Operand(swizzle[0])});
                };
            }
            auto spirv_result_type_id = GenerateTypeIfNeeded(spirv_result_type);
            if (spirv_result_type_id == 0) {
                return false;
            }
            spirv_params.emplace_back(Operand(spirv_result_type_id));
            spirv_params.emplace_back(spirv_result);
        }
        return true;
    };

    auto append_coords_to_spirv_params = [&]() -> bool {
        if (auto* array_index = arg(Usage::kArrayIndex)) {
            // Array index needs to be appended to the coordinates.
            auto* packed = AppendVector(&builder_, arg(Usage::kCoords)->Declaration(),
                                        array_index->Declaration());
            auto param = GenerateExpression(packed);
            if (param == 0) {
                return false;
            }
            spirv_params.emplace_back(Operand(param));
        } else {
            spirv_params.emplace_back(gen_arg(Usage::kCoords));  // coordinates
        }
        return true;
    };

    auto append_image_and_coords_to_spirv_params = [&]() -> bool {
        auto sampler_param = gen_arg(Usage::kSampler);
        auto texture_param = gen_arg(Usage::kTexture);
        auto sampled_image = GenerateSampledImage(texture_type, texture_param, sampler_param);

        // Populate the spirv_params with the common parameters
        spirv_params.emplace_back(Operand(sampled_image));  // sampled image
        return append_coords_to_spirv_params();
    };

    switch (builtin->Type()) {
        case builtin::Function::kTextureDimensions: {
            // Number of returned elements from OpImageQuerySize[Lod] may not match
            // those of textureDimensions().
            // This might be due to an extra vector scalar describing the number of
            // array elements or textureDimensions() returning a vec3 for cubes
            // when only width / height is returned by OpImageQuerySize[Lod]
            // (see https://github.com/gpuweb/gpuweb/issues/1345).
            // Handle these mismatches by swizzling the returned vector.
            std::vector<uint32_t> swizzle;
            uint32_t spirv_dims = 0;
            switch (texture_type->dim()) {
                case type::TextureDimension::kNone:
                    TINT_ICE(Writer, builder_.Diagnostics()) << "texture dimension is kNone";
                    return false;
                case type::TextureDimension::k1d:
                case type::TextureDimension::k2d:
                case type::TextureDimension::k3d:
                case type::TextureDimension::kCube:
                    break;  // No swizzle needed
                case type::TextureDimension::kCubeArray:
                case type::TextureDimension::k2dArray:
                    swizzle = {0, 1};  // Strip array index
                    spirv_dims = 3;    // [width, height, array_count]
                    break;
            }

            if (!append_result_type_and_id_to_spirv_params_swizzled(spirv_dims, swizzle)) {
                return false;
            }

            spirv_params.emplace_back(gen_arg(Usage::kTexture));
            if (texture_type->IsAnyOf<type::MultisampledTexture,       //
                                      type::DepthMultisampledTexture,  //
                                      type::StorageTexture>()) {
                op = spv::Op::OpImageQuerySize;
            } else if (auto* level = arg(Usage::kLevel)) {
                op = spv::Op::OpImageQuerySizeLod;
                spirv_params.emplace_back(gen(level));
            } else {
                op = spv::Op::OpImageQuerySizeLod;
                spirv_params.emplace_back(
                    Operand(GenerateConstantIfNeeded(ScalarConstant::I32(0))));
            }
            break;
        }
        case builtin::Function::kTextureNumLayers: {
            uint32_t spirv_dims = 0;
            switch (texture_type->dim()) {
                default:
                    TINT_ICE(Writer, builder_.Diagnostics()) << "texture is not arrayed";
                    return false;
                case type::TextureDimension::k2dArray:
                case type::TextureDimension::kCubeArray:
                    spirv_dims = 3;
                    break;
            }

            // OpImageQuerySize[Lod] packs the array count as the last element of the
            // returned vector. Extract this.
            if (!append_result_type_and_id_to_spirv_params_swizzled(spirv_dims, {spirv_dims - 1})) {
                return false;
            }

            spirv_params.emplace_back(gen_arg(Usage::kTexture));

            if (texture_type->Is<type::MultisampledTexture>() ||
                texture_type->Is<type::StorageTexture>()) {
                op = spv::Op::OpImageQuerySize;
            } else {
                op = spv::Op::OpImageQuerySizeLod;
                spirv_params.emplace_back(
                    Operand(GenerateConstantIfNeeded(ScalarConstant::I32(0))));
            }
            break;
        }
        case builtin::Function::kTextureNumLevels: {
            op = spv::Op::OpImageQueryLevels;
            append_result_type_and_id_to_spirv_params();
            spirv_params.emplace_back(gen_arg(Usage::kTexture));
            break;
        }
        case builtin::Function::kTextureNumSamples: {
            op = spv::Op::OpImageQuerySamples;
            append_result_type_and_id_to_spirv_params();
            spirv_params.emplace_back(gen_arg(Usage::kTexture));
            break;
        }
        case builtin::Function::kTextureLoad: {
            op = texture_type->Is<type::StorageTexture>() ? spv::Op::OpImageRead
                                                          : spv::Op::OpImageFetch;
            append_result_type_and_id_to_spirv_params_for_read();
            spirv_params.emplace_back(gen_arg(Usage::kTexture));
            if (!append_coords_to_spirv_params()) {
                return false;
            }

            if (auto* level = arg(Usage::kLevel)) {
                image_operands.emplace_back(ImageOperand{SpvImageOperandsLodMask, gen(level)});
            }

            if (auto* sample_index = arg(Usage::kSampleIndex)) {
                image_operands.emplace_back(
                    ImageOperand{SpvImageOperandsSampleMask, gen(sample_index)});
            }

            break;
        }
        case builtin::Function::kTextureStore: {
            op = spv::Op::OpImageWrite;
            spirv_params.emplace_back(gen_arg(Usage::kTexture));
            if (!append_coords_to_spirv_params()) {
                return false;
            }
            spirv_params.emplace_back(gen_arg(Usage::kValue));
            break;
        }
        case builtin::Function::kTextureGather: {
            op = spv::Op::OpImageGather;
            append_result_type_and_id_to_spirv_params();
            if (!append_image_and_coords_to_spirv_params()) {
                return false;
            }
            if (signature.IndexOf(Usage::kComponent) < 0) {
                spirv_params.emplace_back(
                    Operand(GenerateConstantIfNeeded(ScalarConstant::I32(0))));
            } else {
                spirv_params.emplace_back(gen_arg(Usage::kComponent));
            }
            break;
        }
        case builtin::Function::kTextureGatherCompare: {
            op = spv::Op::OpImageDrefGather;
            append_result_type_and_id_to_spirv_params();
            if (!append_image_and_coords_to_spirv_params()) {
                return false;
            }
            spirv_params.emplace_back(gen_arg(Usage::kDepthRef));
            break;
        }
        case builtin::Function::kTextureSample: {
            op = spv::Op::OpImageSampleImplicitLod;
            append_result_type_and_id_to_spirv_params_for_read();
            if (!append_image_and_coords_to_spirv_params()) {
                return false;
            }
            break;
        }
        case builtin::Function::kTextureSampleBias: {
            op = spv::Op::OpImageSampleImplicitLod;
            append_result_type_and_id_to_spirv_params_for_read();
            if (!append_image_and_coords_to_spirv_params()) {
                return false;
            }
            image_operands.emplace_back(
                ImageOperand{SpvImageOperandsBiasMask, gen_arg(Usage::kBias)});
            break;
        }
        case builtin::Function::kTextureSampleLevel: {
            op = spv::Op::OpImageSampleExplicitLod;
            append_result_type_and_id_to_spirv_params_for_read();
            if (!append_image_and_coords_to_spirv_params()) {
                return false;
            }
            auto level = Operand(0u);
            if (arg(Usage::kLevel)->Type()->UnwrapRef()->IsAnyOf<type::I32, type::U32>()) {
                // Depth textures have i32 or u32 parameters for the level, but SPIR-V expects f32.
                // Cast.
                auto f32_type_id = GenerateTypeIfNeeded(builder_.create<type::F32>());
                if (f32_type_id == 0) {
                    return 0;
                }
                level = result_op();
                if (!push_function_inst(spv::Op::OpConvertSToF,
                                        {Operand(f32_type_id), level, gen_arg(Usage::kLevel)})) {
                    return 0;
                }
            } else {
                level = gen_arg(Usage::kLevel);
            }
            image_operands.emplace_back(ImageOperand{SpvImageOperandsLodMask, level});
            break;
        }
        case builtin::Function::kTextureSampleGrad: {
            op = spv::Op::OpImageSampleExplicitLod;
            append_result_type_and_id_to_spirv_params_for_read();
            if (!append_image_and_coords_to_spirv_params()) {
                return false;
            }
            image_operands.emplace_back(
                ImageOperand{SpvImageOperandsGradMask, gen_arg(Usage::kDdx)});
            image_operands.emplace_back(
                ImageOperand{SpvImageOperandsGradMask, gen_arg(Usage::kDdy)});
            break;
        }
        case builtin::Function::kTextureSampleCompare: {
            op = spv::Op::OpImageSampleDrefImplicitLod;
            append_result_type_and_id_to_spirv_params();
            if (!append_image_and_coords_to_spirv_params()) {
                return false;
            }
            spirv_params.emplace_back(gen_arg(Usage::kDepthRef));
            break;
        }
        case builtin::Function::kTextureSampleCompareLevel: {
            op = spv::Op::OpImageSampleDrefExplicitLod;
            append_result_type_and_id_to_spirv_params();
            if (!append_image_and_coords_to_spirv_params()) {
                return false;
            }
            spirv_params.emplace_back(gen_arg(Usage::kDepthRef));

            image_operands.emplace_back(
                ImageOperand{SpvImageOperandsLodMask,
                             Operand(GenerateConstantIfNeeded(ScalarConstant::F32(0.0)))});
            break;
        }
        default:
            TINT_UNREACHABLE(Writer, builder_.Diagnostics());
            return false;
    }

    if (auto* offset = arg(Usage::kOffset)) {
        image_operands.emplace_back(ImageOperand{SpvImageOperandsConstOffsetMask, gen(offset)});
    }

    if (!image_operands.empty()) {
        std::sort(image_operands.begin(), image_operands.end(),
                  [](auto& a, auto& b) { return a.mask < b.mask; });
        uint32_t mask = 0;
        for (auto& image_operand : image_operands) {
            mask |= image_operand.mask;
        }
        spirv_params.emplace_back(Operand(mask));
        for (auto& image_operand : image_operands) {
            spirv_params.emplace_back(image_operand.operand);
        }
    }

    if (op == spv::Op::OpNop) {
        TINT_ICE(Writer, builder_.Diagnostics())
            << "unable to determine operator for: " + std::string(builtin->str());
        return false;
    }

    if (!push_function_inst(op, spirv_params)) {
        return false;
    }

    return post_emission();
}

bool Builder::GenerateControlBarrierBuiltin(const sem::Builtin* builtin) {
    auto const op = spv::Op::OpControlBarrier;
    uint32_t execution = 0;
    uint32_t memory = 0;
    uint32_t semantics = 0;

    // TODO(crbug.com/tint/661): Combine sequential barriers to a single
    // instruction.
    if (builtin->Type() == builtin::Function::kWorkgroupBarrier) {
        execution = static_cast<uint32_t>(spv::Scope::Workgroup);
        memory = static_cast<uint32_t>(spv::Scope::Workgroup);
        semantics = static_cast<uint32_t>(spv::MemorySemanticsMask::AcquireRelease) |
                    static_cast<uint32_t>(spv::MemorySemanticsMask::WorkgroupMemory);
    } else if (builtin->Type() == builtin::Function::kStorageBarrier) {
        execution = static_cast<uint32_t>(spv::Scope::Workgroup);
        memory = static_cast<uint32_t>(spv::Scope::Workgroup);
        semantics = static_cast<uint32_t>(spv::MemorySemanticsMask::AcquireRelease) |
                    static_cast<uint32_t>(spv::MemorySemanticsMask::UniformMemory);
    } else {
        TINT_ICE(Writer, builder_.Diagnostics())
            << "unexpected barrier builtin type " << builtin::str(builtin->Type());
        return false;
    }

    auto execution_id = GenerateConstantIfNeeded(ScalarConstant::U32(execution));
    auto memory_id = GenerateConstantIfNeeded(ScalarConstant::U32(memory));
    auto semantics_id = GenerateConstantIfNeeded(ScalarConstant::U32(semantics));
    if (execution_id == 0 || memory_id == 0 || semantics_id == 0) {
        return false;
    }

    return push_function_inst(op, {
                                      Operand(execution_id),
                                      Operand(memory_id),
                                      Operand(semantics_id),
                                  });
}

bool Builder::GenerateAtomicBuiltin(const sem::Call* call,
                                    const sem::Builtin* builtin,
                                    Operand result_type,
                                    Operand result_id) {
    auto is_value_signed = [&] { return builtin->Parameters()[1]->Type()->Is<type::I32>(); };

    auto address_space = builtin->Parameters()[0]->Type()->As<type::Pointer>()->AddressSpace();

    uint32_t memory_id = 0;
    switch (builtin->Parameters()[0]->Type()->As<type::Pointer>()->AddressSpace()) {
        case builtin::AddressSpace::kWorkgroup:
            memory_id = GenerateConstantIfNeeded(
                ScalarConstant::U32(static_cast<uint32_t>(spv::Scope::Workgroup)));
            break;
        case builtin::AddressSpace::kStorage:
            memory_id = GenerateConstantIfNeeded(
                ScalarConstant::U32(static_cast<uint32_t>(spv::Scope::Device)));
            break;
        default:
            TINT_UNREACHABLE(Writer, builder_.Diagnostics())
                << "unhandled atomic address space " << address_space;
            return false;
    }
    if (memory_id == 0) {
        return false;
    }

    uint32_t semantics_id = GenerateConstantIfNeeded(
        ScalarConstant::U32(static_cast<uint32_t>(spv::MemorySemanticsMask::MaskNone)));
    if (semantics_id == 0) {
        return false;
    }

    uint32_t pointer_id = GenerateExpression(call->Arguments()[0]);
    if (pointer_id == 0) {
        return false;
    }

    uint32_t value_id = 0;
    if (call->Arguments().Length() > 1) {
        value_id = GenerateExpression(call->Arguments().Back());
        if (value_id == 0) {
            return false;
        }
    }

    Operand pointer = Operand(pointer_id);
    Operand value = Operand(value_id);
    Operand memory = Operand(memory_id);
    Operand semantics = Operand(semantics_id);

    switch (builtin->Type()) {
        case builtin::Function::kAtomicLoad:
            return push_function_inst(spv::Op::OpAtomicLoad, {
                                                                 result_type,
                                                                 result_id,
                                                                 pointer,
                                                                 memory,
                                                                 semantics,
                                                             });
        case builtin::Function::kAtomicStore:
            return push_function_inst(spv::Op::OpAtomicStore, {
                                                                  pointer,
                                                                  memory,
                                                                  semantics,
                                                                  value,
                                                              });
        case builtin::Function::kAtomicAdd:
            return push_function_inst(spv::Op::OpAtomicIAdd, {
                                                                 result_type,
                                                                 result_id,
                                                                 pointer,
                                                                 memory,
                                                                 semantics,
                                                                 value,
                                                             });
        case builtin::Function::kAtomicSub:
            return push_function_inst(spv::Op::OpAtomicISub, {
                                                                 result_type,
                                                                 result_id,
                                                                 pointer,
                                                                 memory,
                                                                 semantics,
                                                                 value,
                                                             });
        case builtin::Function::kAtomicMax:
            return push_function_inst(
                is_value_signed() ? spv::Op::OpAtomicSMax : spv::Op::OpAtomicUMax, {
                                                                                       result_type,
                                                                                       result_id,
                                                                                       pointer,
                                                                                       memory,
                                                                                       semantics,
                                                                                       value,
                                                                                   });
        case builtin::Function::kAtomicMin:
            return push_function_inst(
                is_value_signed() ? spv::Op::OpAtomicSMin : spv::Op::OpAtomicUMin, {
                                                                                       result_type,
                                                                                       result_id,
                                                                                       pointer,
                                                                                       memory,
                                                                                       semantics,
                                                                                       value,
                                                                                   });
        case builtin::Function::kAtomicAnd:
            return push_function_inst(spv::Op::OpAtomicAnd, {
                                                                result_type,
                                                                result_id,
                                                                pointer,
                                                                memory,
                                                                semantics,
                                                                value,
                                                            });
        case builtin::Function::kAtomicOr:
            return push_function_inst(spv::Op::OpAtomicOr, {
                                                               result_type,
                                                               result_id,
                                                               pointer,
                                                               memory,
                                                               semantics,
                                                               value,
                                                           });
        case builtin::Function::kAtomicXor:
            return push_function_inst(spv::Op::OpAtomicXor, {
                                                                result_type,
                                                                result_id,
                                                                pointer,
                                                                memory,
                                                                semantics,
                                                                value,
                                                            });
        case builtin::Function::kAtomicExchange:
            return push_function_inst(spv::Op::OpAtomicExchange, {
                                                                     result_type,
                                                                     result_id,
                                                                     pointer,
                                                                     memory,
                                                                     semantics,
                                                                     value,
                                                                 });
        case builtin::Function::kAtomicCompareExchangeWeak: {
            auto comparator = GenerateExpression(call->Arguments()[1]);
            if (comparator == 0) {
                return false;
            }

            auto* value_sem_type = call->Target()->Signature().parameters[2]->Type();

            auto value_type = GenerateTypeIfNeeded(value_sem_type);
            if (value_type == 0) {
                return false;
            }

            auto* bool_sem_ty = builder_.create<type::Bool>();
            auto bool_type = GenerateTypeIfNeeded(bool_sem_ty);
            if (bool_type == 0) {
                return false;
            }

            // original_value := OpAtomicCompareExchange(pointer, memory, semantics,
            //                                           semantics, value, comparator)
            auto original_value = result_op();
            if (!push_function_inst(spv::Op::OpAtomicCompareExchange, {
                                                                          Operand(value_type),
                                                                          original_value,
                                                                          pointer,
                                                                          memory,
                                                                          semantics,
                                                                          semantics,
                                                                          value,
                                                                          Operand(comparator),
                                                                      })) {
                return false;
            }

            // https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#OpAtomicCompareExchange
            // According to SPIR-V spec, during the atomic steps of OpAtomicCompareExchange, the new
            // value will be stored only if original value equals to comparator, and the result of
            // OpAtomicCompareExchange is the original value. Therefore to check if the exchanging
            // has been executed, we should compare the result original_value to comparator.

            // values_equal := original_value == comparator
            auto values_equal = result_op();
            if (!push_function_inst(spv::Op::OpIEqual, {
                                                           Operand(bool_type),
                                                           values_equal,
                                                           original_value,
                                                           Operand(comparator),
                                                       })) {
                return false;
            }

            // result := __atomic_compare_exchange_result<T>(original_value, values_equal)
            return push_function_inst(spv::Op::OpCompositeConstruct, {
                                                                         result_type,
                                                                         result_id,
                                                                         original_value,
                                                                         values_equal,
                                                                     });
        }
        default:
            TINT_UNREACHABLE(Writer, builder_.Diagnostics())
                << "unhandled atomic builtin " << builtin->Type();
            return false;
    }
}

uint32_t Builder::GenerateSampledImage(const type::Type* texture_type,
                                       Operand texture_operand,
                                       Operand sampler_operand) {
    // DepthTexture is always declared as SampledTexture.
    // The Vulkan spec says: The "Depth" operand of OpTypeImage is ignored.
    // In SPIRV, 0 means not depth, 1 means depth, and 2 means unknown.
    // Using anything other than 0 is problematic on various Vulkan drivers.
    if (auto* depthTextureType = texture_type->As<type::DepthTexture>()) {
        texture_type = builder_.create<type::SampledTexture>(depthTextureType->dim(),
                                                             builder_.create<type::F32>());
    }

    uint32_t sampled_image_type_id =
        utils::GetOrCreate(texture_type_to_sampled_image_type_id_, texture_type, [&] {
            // We need to create the sampled image type and cache the result.
            auto sampled_image_type = result_op();
            auto texture_type_id = GenerateTypeIfNeeded(texture_type);
            module_.PushType(spv::Op::OpTypeSampledImage,
                             {sampled_image_type, Operand(texture_type_id)});
            return std::get<uint32_t>(sampled_image_type);
        });

    auto sampled_image = result_op();
    if (!push_function_inst(spv::Op::OpSampledImage, {Operand(sampled_image_type_id), sampled_image,
                                                      texture_operand, sampler_operand})) {
        return 0;
    }

    return std::get<uint32_t>(sampled_image);
}

uint32_t Builder::GenerateBitcastExpression(const ast::BitcastExpression* expr) {
    auto result = result_op();
    auto result_id = std::get<uint32_t>(result);

    auto result_type_id = GenerateTypeIfNeeded(TypeOf(expr));
    if (result_type_id == 0) {
        return 0;
    }

    auto val_id = GenerateExpression(expr->expr);
    if (val_id == 0) {
        return 0;
    }

    // Bitcast does not allow same types, just emit a CopyObject
    auto* to_type = TypeOf(expr)->UnwrapRef();
    auto* from_type = TypeOf(expr->expr)->UnwrapRef();
    if (to_type == from_type) {
        if (!push_function_inst(spv::Op::OpCopyObject,
                                {Operand(result_type_id), result, Operand(val_id)})) {
            return 0;
        }
        return result_id;
    }

    if (!push_function_inst(spv::Op::OpBitcast,
                            {Operand(result_type_id), result, Operand(val_id)})) {
        return 0;
    }

    return result_id;
}

bool Builder::GenerateConditionalBlock(const ast::Expression* cond,
                                       const ast::BlockStatement* true_body,
                                       const ast::Statement* else_stmt) {
    auto cond_id = GenerateExpression(cond);
    if (cond_id == 0) {
        return false;
    }

    auto merge_block = result_op();
    auto merge_block_id = std::get<uint32_t>(merge_block);

    if (!push_function_inst(spv::Op::OpSelectionMerge,
                            {Operand(merge_block_id), U32Operand(SpvSelectionControlMaskNone)})) {
        return false;
    }

    auto true_block = result_op();
    auto true_block_id = std::get<uint32_t>(true_block);

    // if there are no more else statements we branch on false to the merge
    // block otherwise we branch to the false block
    auto false_block_id = else_stmt ? module_.NextId() : merge_block_id;

    if (!push_function_inst(spv::Op::OpBranchConditional,
                            {Operand(cond_id), Operand(true_block_id), Operand(false_block_id)})) {
        return false;
    }

    // Output true block
    if (!GenerateLabel(true_block_id)) {
        return false;
    }
    if (!GenerateBlockStatement(true_body)) {
        return false;
    }
    // We only branch if the last element of the body didn't already branch.
    if (InsideBasicBlock()) {
        if (!push_function_inst(spv::Op::OpBranch, {Operand(merge_block_id)})) {
            return false;
        }
    }

    // Start the false block if needed
    if (false_block_id != merge_block_id) {
        if (!GenerateLabel(false_block_id)) {
            return false;
        }

        // Handle the else case by just outputting the statements.
        if (auto* block = else_stmt->As<ast::BlockStatement>()) {
            if (!GenerateBlockStatement(block)) {
                return false;
            }
        } else {
            auto* elseif = else_stmt->As<ast::IfStatement>();
            if (!GenerateConditionalBlock(elseif->condition, elseif->body,
                                          elseif->else_statement)) {
                return false;
            }
        }
        if (InsideBasicBlock()) {
            if (!push_function_inst(spv::Op::OpBranch, {Operand(merge_block_id)})) {
                return false;
            }
        }
    }

    // Output the merge block
    return GenerateLabel(merge_block_id);
}

bool Builder::GenerateIfStatement(const ast::IfStatement* stmt) {
    if (!GenerateConditionalBlock(stmt->condition, stmt->body, stmt->else_statement)) {
        return false;
    }
    return true;
}

bool Builder::GenerateSwitchStatement(const ast::SwitchStatement* stmt) {
    auto merge_block = result_op();
    auto merge_block_id = std::get<uint32_t>(merge_block);

    merge_stack_.push_back(merge_block_id);

    auto cond_id = GenerateExpression(stmt->condition);
    if (cond_id == 0) {
        return false;
    }

    auto default_block = result_op();
    auto default_block_id = std::get<uint32_t>(default_block);

    OperandList params = {Operand(cond_id), Operand(default_block_id)};

    std::vector<uint32_t> case_ids;
    for (const auto* item : stmt->body) {
        auto block_id = default_block_id;
        if (!item->ContainsDefault()) {
            auto block = result_op();
            block_id = std::get<uint32_t>(block);
        }
        case_ids.push_back(block_id);

        // If this case statement is only a default selector skip adding the block
        // as it will be done below.
        if (item->selectors.Length() == 1 && item->ContainsDefault()) {
            continue;
        }

        auto* sem = builder_.Sem().Get<sem::CaseStatement>(item);
        for (auto* selector : sem->Selectors()) {
            if (selector->IsDefault()) {
                continue;
            }

            params.push_back(Operand(selector->Value()->ValueAs<u32>()));
            params.push_back(Operand(block_id));
        }
    }

    if (!push_function_inst(spv::Op::OpSelectionMerge,
                            {Operand(merge_block_id), U32Operand(SpvSelectionControlMaskNone)})) {
        return false;
    }
    if (!push_function_inst(spv::Op::OpSwitch, params)) {
        return false;
    }

    bool generated_default = false;
    auto& body = stmt->body;
    // We output the case statements in order they were entered in the original
    // source. The branch is to the merge block which comes after the switch statement.
    for (uint32_t i = 0; i < body.Length(); i++) {
        auto* item = body[i];

        if (item->ContainsDefault()) {
            generated_default = true;
        }

        if (!GenerateLabel(case_ids[i])) {
            return false;
        }
        if (!GenerateBlockStatement(item->body)) {
            return false;
        }
        if (InsideBasicBlock()) {
            if (!push_function_inst(spv::Op::OpBranch, {Operand(merge_block_id)})) {
                return false;
            }
        }
    }

    if (!generated_default) {
        if (!GenerateLabel(default_block_id)) {
            return false;
        }
        if (!push_function_inst(spv::Op::OpBranch, {Operand(merge_block_id)})) {
            return false;
        }
    }

    merge_stack_.pop_back();

    return GenerateLabel(merge_block_id);
}

bool Builder::GenerateReturnStatement(const ast::ReturnStatement* stmt) {
    if (stmt->value) {
        auto val_id = GenerateExpression(stmt->value);
        if (val_id == 0) {
            return false;
        }
        if (!push_function_inst(spv::Op::OpReturnValue, {Operand(val_id)})) {
            return false;
        }
    } else {
        if (!push_function_inst(spv::Op::OpReturn, {})) {
            return false;
        }
    }

    return true;
}

bool Builder::GenerateLoopStatement(const ast::LoopStatement* stmt) {
    auto loop_header = result_op();
    auto loop_header_id = std::get<uint32_t>(loop_header);
    if (!push_function_inst(spv::Op::OpBranch, {Operand(loop_header_id)})) {
        return false;
    }
    if (!GenerateLabel(loop_header_id)) {
        return false;
    }

    auto merge_block = result_op();
    auto merge_block_id = std::get<uint32_t>(merge_block);
    auto continue_block = result_op();
    auto continue_block_id = std::get<uint32_t>(continue_block);

    auto body_block = result_op();
    auto body_block_id = std::get<uint32_t>(body_block);

    if (!push_function_inst(spv::Op::OpLoopMerge,
                            {Operand(merge_block_id), Operand(continue_block_id),
                             U32Operand(SpvLoopControlMaskNone)})) {
        return false;
    }

    continue_stack_.push_back(continue_block_id);
    merge_stack_.push_back(merge_block_id);

    // Usually, the backedge is a simple branch.  This will be modified if the
    // backedge block in the continuing construct has an exiting edge.
    backedge_stack_.emplace_back(spv::Op::OpBranch, OperandList{Operand(loop_header_id)});

    if (!push_function_inst(spv::Op::OpBranch, {Operand(body_block_id)})) {
        return false;
    }
    if (!GenerateLabel(body_block_id)) {
        return false;
    }

    // We need variables from the body to be visible in the continuing block, so
    // manage scope outside of GenerateBlockStatement.
    {
        PushScope();
        TINT_DEFER(PopScope());

        if (!GenerateBlockStatementWithoutScoping(stmt->body)) {
            return false;
        }

        // We only branch if the last element of the body didn't already branch.
        if (InsideBasicBlock()) {
            if (!push_function_inst(spv::Op::OpBranch, {Operand(continue_block_id)})) {
                return false;
            }
        }

        if (!GenerateLabel(continue_block_id)) {
            return false;
        }
        if (stmt->continuing && !stmt->continuing->Empty()) {
            continuing_stack_.emplace_back(stmt->continuing->Last(), loop_header_id,
                                           merge_block_id);
            if (!GenerateBlockStatementWithoutScoping(stmt->continuing)) {
                return false;
            }
            continuing_stack_.pop_back();
        }
    }

    // Generate the backedge.
    TINT_ASSERT(Writer, !backedge_stack_.empty());
    const Backedge& backedge = backedge_stack_.back();
    if (!push_function_inst(backedge.opcode, backedge.operands)) {
        return false;
    }
    backedge_stack_.pop_back();

    merge_stack_.pop_back();
    continue_stack_.pop_back();

    return GenerateLabel(merge_block_id);
}

bool Builder::GenerateStatement(const ast::Statement* stmt) {
    return Switch(
        stmt, [&](const ast::AssignmentStatement* a) { return GenerateAssignStatement(a); },
        [&](const ast::BlockStatement* b) { return GenerateBlockStatement(b); },
        [&](const ast::BreakStatement* b) { return GenerateBreakStatement(b); },
        [&](const ast::BreakIfStatement* b) { return GenerateBreakIfStatement(b); },
        [&](const ast::CallStatement* c) { return GenerateCallExpression(c->expr) != 0; },
        [&](const ast::ContinueStatement* c) { return GenerateContinueStatement(c); },
        [&](const ast::DiscardStatement* d) { return GenerateDiscardStatement(d); },
        [&](const ast::IfStatement* i) { return GenerateIfStatement(i); },
        [&](const ast::LoopStatement* l) { return GenerateLoopStatement(l); },
        [&](const ast::ReturnStatement* r) { return GenerateReturnStatement(r); },
        [&](const ast::SwitchStatement* s) { return GenerateSwitchStatement(s); },
        [&](const ast::VariableDeclStatement* v) { return GenerateVariableDeclStatement(v); },
        [&](const ast::ConstAssert*) {
            return true;  // Not emitted
        },
        [&](Default) {
            TINT_ICE(Writer, builder_.Diagnostics())
                << "unknown statement type: " + std::string(stmt->TypeInfo().name);
            return false;
        });
}

bool Builder::GenerateVariableDeclStatement(const ast::VariableDeclStatement* stmt) {
    return GenerateFunctionVariable(stmt->variable);
}

uint32_t Builder::GenerateTypeIfNeeded(const type::Type* type) {
    if (type == nullptr) {
        TINT_ICE(Writer, builder_.Diagnostics()) << "attempting to generate type from null type";
        return 0;
    }

    // Atomics are a type in WGSL, but aren't a distinct type in SPIR-V.
    // Just emit the type inside the atomic.
    if (auto* atomic = type->As<type::Atomic>()) {
        return GenerateTypeIfNeeded(atomic->Type());
    }

    // DepthTexture is always declared as SampledTexture.
    // The Vulkan spec says: The "Depth" operand of OpTypeImage is ignored.
    // In SPIRV, 0 means not depth, 1 means depth, and 2 means unknown.
    // Using anything other than 0 is problematic on various Vulkan drivers.
    if (auto* depthTextureType = type->As<type::DepthTexture>()) {
        type = builder_.create<type::SampledTexture>(depthTextureType->dim(),
                                                     builder_.create<type::F32>());
    } else if (auto* multisampledDepthTextureType = type->As<type::DepthMultisampledTexture>()) {
        type = builder_.create<type::MultisampledTexture>(multisampledDepthTextureType->dim(),
                                                          builder_.create<type::F32>());
    }

    // Pointers and references with differing accesses should not result in a
    // different SPIR-V types, so we explicitly ignore the access.
    // Pointers and References both map to a SPIR-V pointer type.
    // Transform a Reference to a Pointer to prevent these having duplicated
    // definitions in the generated SPIR-V. Note that nested pointers and
    // references are not legal in WGSL, so only considering the top-level type is
    // fine.
    if (auto* ptr = type->As<type::Pointer>()) {
        type = builder_.create<type::Pointer>(ptr->StoreType(), ptr->AddressSpace(),
                                              builtin::Access::kReadWrite);
    } else if (auto* ref = type->As<type::Reference>()) {
        type = builder_.create<type::Pointer>(ref->StoreType(), ref->AddressSpace(),
                                              builtin::Access::kReadWrite);
    }

    return utils::GetOrCreate(type_to_id_, type, [&]() -> uint32_t {
        auto result = result_op();
        auto id = std::get<uint32_t>(result);
        bool ok = Switch(
            type,
            [&](const type::Array* arr) {  //
                return GenerateArrayType(arr, result);
            },
            [&](const type::Bool*) {
                module_.PushType(spv::Op::OpTypeBool, {result});
                return true;
            },
            [&](const type::F32*) {
                module_.PushType(spv::Op::OpTypeFloat, {result, Operand(32u)});
                return true;
            },
            [&](const type::F16*) {
                module_.PushType(spv::Op::OpTypeFloat, {result, Operand(16u)});
                return true;
            },
            [&](const type::I32*) {
                module_.PushType(spv::Op::OpTypeInt, {result, Operand(32u), Operand(1u)});
                return true;
            },
            [&](const type::Matrix* mat) {  //
                return GenerateMatrixType(mat, result);
            },
            [&](const type::Pointer* ptr) {  //
                return GeneratePointerType(ptr, result);
            },
            [&](const type::Reference* ref) {  //
                return GenerateReferenceType(ref, result);
            },
            [&](const type::Struct* str) {  //
                return GenerateStructType(str, result);
            },
            [&](const type::U32*) {
                module_.PushType(spv::Op::OpTypeInt, {result, Operand(32u), Operand(0u)});
                return true;
            },
            [&](const type::Vector* vec) {  //
                return GenerateVectorType(vec, result);
            },
            [&](const type::Void*) {
                module_.PushType(spv::Op::OpTypeVoid, {result});
                return true;
            },
            [&](const type::StorageTexture* tex) {
                if (!GenerateTextureType(tex, result)) {
                    return false;
                }

                // Register all three access types of StorageTexture names. In
                // SPIR-V, we must output a single type, while the variable is
                // annotated with the access type. Doing this ensures we de-dupe.
                type_to_id_[builder_.create<type::StorageTexture>(
                    tex->dim(), tex->texel_format(), builtin::Access::kRead, tex->type())] = id;
                type_to_id_[builder_.create<type::StorageTexture>(
                    tex->dim(), tex->texel_format(), builtin::Access::kWrite, tex->type())] = id;
                type_to_id_[builder_.create<type::StorageTexture>(
                    tex->dim(), tex->texel_format(), builtin::Access::kReadWrite, tex->type())] =
                    id;
                return true;
            },
            [&](const type::Texture* tex) { return GenerateTextureType(tex, result); },
            [&](const type::Sampler* s) {
                module_.PushType(spv::Op::OpTypeSampler, {result});

                // Register both of the sampler type names. In SPIR-V they're the same
                // sampler type, so we need to match that when we do the dedup check.
                if (s->kind() == type::SamplerKind::kSampler) {
                    type_to_id_[builder_.create<type::Sampler>(
                        type::SamplerKind::kComparisonSampler)] = id;
                } else {
                    type_to_id_[builder_.create<type::Sampler>(type::SamplerKind::kSampler)] = id;
                }
                return true;
            },
            [&](Default) {
                TINT_ICE(Writer, builder_.Diagnostics())
                    << "unable to convert type: " + type->FriendlyName();
                return false;
            });

        if (!ok) {
            return 0;
        }

        return id;
    });
}

bool Builder::GenerateTextureType(const type::Texture* texture, const Operand& result) {
    if (TINT_UNLIKELY(texture->Is<type::ExternalTexture>())) {
        TINT_ICE(Writer, builder_.Diagnostics())
            << "Multiplanar external texture transform was not run.";
        return false;
    }

    uint32_t array_literal = 0u;
    const auto dim = texture->dim();
    if (dim == type::TextureDimension::k2dArray || dim == type::TextureDimension::kCubeArray) {
        array_literal = 1u;
    }

    uint32_t dim_literal = SpvDim2D;
    if (dim == type::TextureDimension::k1d) {
        dim_literal = SpvDim1D;
        if (texture->Is<type::SampledTexture>()) {
            module_.PushCapability(SpvCapabilitySampled1D);
        } else if (texture->Is<type::StorageTexture>()) {
            module_.PushCapability(SpvCapabilityImage1D);
        }
    }
    if (dim == type::TextureDimension::k3d) {
        dim_literal = SpvDim3D;
    }
    if (dim == type::TextureDimension::kCube || dim == type::TextureDimension::kCubeArray) {
        dim_literal = SpvDimCube;
    }

    uint32_t ms_literal = 0u;
    if (texture->IsAnyOf<type::MultisampledTexture, type::DepthMultisampledTexture>()) {
        ms_literal = 1u;
    }

    uint32_t depth_literal = 0u;
    // The Vulkan spec says: The "Depth" operand of OpTypeImage is ignored.
    // In SPIRV, 0 means not depth, 1 means depth, and 2 means unknown.
    // Using anything other than 0 is problematic on various Vulkan drivers.

    uint32_t sampled_literal = 2u;
    if (texture->IsAnyOf<type::MultisampledTexture, type::SampledTexture, type::DepthTexture,
                         type::DepthMultisampledTexture>()) {
        sampled_literal = 1u;
    }

    if (dim == type::TextureDimension::kCubeArray) {
        if (texture->IsAnyOf<type::SampledTexture, type::DepthTexture>()) {
            module_.PushCapability(SpvCapabilitySampledCubeArray);
        }
    }

    uint32_t type_id = Switch(
        texture,
        [&](const type::DepthTexture*) {
            return GenerateTypeIfNeeded(builder_.create<type::F32>());
        },
        [&](const type::DepthMultisampledTexture*) {
            return GenerateTypeIfNeeded(builder_.create<type::F32>());
        },
        [&](const type::SampledTexture* t) { return GenerateTypeIfNeeded(t->type()); },
        [&](const type::MultisampledTexture* t) { return GenerateTypeIfNeeded(t->type()); },
        [&](const type::StorageTexture* t) { return GenerateTypeIfNeeded(t->type()); },
        [&](Default) { return 0u; });
    if (type_id == 0u) {
        return false;
    }

    uint32_t format_literal = SpvImageFormat_::SpvImageFormatUnknown;
    if (auto* t = texture->As<type::StorageTexture>()) {
        format_literal = convert_texel_format_to_spv(t->texel_format());
    }

    module_.PushType(spv::Op::OpTypeImage,
                     {result, Operand(type_id), Operand(dim_literal), Operand(depth_literal),
                      Operand(array_literal), Operand(ms_literal), Operand(sampled_literal),
                      Operand(format_literal)});

    return true;
}

bool Builder::GenerateArrayType(const type::Array* arr, const Operand& result) {
    auto elem_type = GenerateTypeIfNeeded(arr->ElemType());
    if (elem_type == 0) {
        return false;
    }

    auto result_id = std::get<uint32_t>(result);
    if (arr->Count()->Is<type::RuntimeArrayCount>()) {
        module_.PushType(spv::Op::OpTypeRuntimeArray, {result, Operand(elem_type)});
    } else {
        auto count = arr->ConstantCount();
        if (!count) {
            TINT_ICE(Writer, builder_.Diagnostics()) << type::Array::kErrExpectedConstantCount;
            return static_cast<uint32_t>(0);
        }

        auto len_id = GenerateConstantIfNeeded(ScalarConstant::U32(count.value()));
        if (len_id == 0) {
            return false;
        }

        module_.PushType(spv::Op::OpTypeArray, {result, Operand(elem_type), Operand(len_id)});
    }

    module_.PushAnnot(
        spv::Op::OpDecorate,
        {Operand(result_id), U32Operand(SpvDecorationArrayStride), Operand(arr->Stride())});
    return true;
}

bool Builder::GenerateMatrixType(const type::Matrix* mat, const Operand& result) {
    auto* col_type = builder_.create<type::Vector>(mat->type(), mat->rows());
    auto col_type_id = GenerateTypeIfNeeded(col_type);
    if (has_error()) {
        return false;
    }

    module_.PushType(spv::Op::OpTypeMatrix,
                     {result, Operand(col_type_id), Operand(mat->columns())});
    return true;
}

bool Builder::GeneratePointerType(const type::Pointer* ptr, const Operand& result) {
    auto subtype_id = GenerateTypeIfNeeded(ptr->StoreType());
    if (subtype_id == 0) {
        return false;
    }

    auto stg_class = ConvertAddressSpace(ptr->AddressSpace());
    if (stg_class == SpvStorageClassMax) {
        TINT_ICE(Writer, builder_.Diagnostics()) << "invalid address space for pointer";
        return false;
    }

    module_.PushType(spv::Op::OpTypePointer, {result, U32Operand(stg_class), Operand(subtype_id)});

    return true;
}

bool Builder::GenerateReferenceType(const type::Reference* ref, const Operand& result) {
    auto subtype_id = GenerateTypeIfNeeded(ref->StoreType());
    if (subtype_id == 0) {
        return false;
    }

    auto stg_class = ConvertAddressSpace(ref->AddressSpace());
    if (stg_class == SpvStorageClassMax) {
        TINT_ICE(Writer, builder_.Diagnostics()) << "invalid address space for reference";
        return false;
    }

    module_.PushType(spv::Op::OpTypePointer, {result, U32Operand(stg_class), Operand(subtype_id)});

    return true;
}

bool Builder::GenerateStructType(const type::Struct* struct_type, const Operand& result) {
    auto struct_id = std::get<uint32_t>(result);

    if (struct_type->Name().IsValid()) {
        module_.PushDebug(spv::Op::OpName,
                          {Operand(struct_id), Operand(struct_type->Name().Name())});
    }

    OperandList ops;
    ops.push_back(result);

    if (auto* sem_str = struct_type->As<sem::Struct>()) {
        auto* decl = sem_str->Declaration();
        if (ast::HasAttribute<transform::AddBlockAttribute::BlockAttribute>(decl->attributes)) {
            module_.PushAnnot(spv::Op::OpDecorate,
                              {Operand(struct_id), U32Operand(SpvDecorationBlock)});
        }
    }

    for (uint32_t i = 0; i < struct_type->Members().Length(); ++i) {
        auto mem_id = GenerateStructMember(struct_id, i, struct_type->Members()[i]);
        if (mem_id == 0) {
            return false;
        }

        ops.push_back(Operand(mem_id));
    }

    module_.PushType(spv::Op::OpTypeStruct, std::move(ops));
    return true;
}

uint32_t Builder::GenerateStructMember(uint32_t struct_id,
                                       uint32_t idx,
                                       const type::StructMember* member) {
    module_.PushDebug(spv::Op::OpMemberName,
                      {Operand(struct_id), Operand(idx), Operand(member->Name().Name())});

    // Note: This will generate layout annotations for *all* structs, whether or
    // not they are used in host-shareable variables. This is officially ok in
    // SPIR-V 1.0 through 1.3. If / when we migrate to using SPIR-V 1.4 we'll have
    // to only generate the layout info for structs used for certain storage
    // classes.

    module_.PushAnnot(spv::Op::OpMemberDecorate,
                      {Operand(struct_id), Operand(idx), U32Operand(SpvDecorationOffset),
                       Operand(member->Offset())});

    // Infer and emit matrix layout.
    auto* matrix_type = GetNestedMatrixType(member->Type());
    if (matrix_type) {
        module_.PushAnnot(spv::Op::OpMemberDecorate,
                          {Operand(struct_id), Operand(idx), U32Operand(SpvDecorationColMajor)});
        const uint32_t scalar_elem_size = matrix_type->type()->Size();
        const uint32_t effective_row_count = (matrix_type->rows() == 2) ? 2 : 4;
        module_.PushAnnot(spv::Op::OpMemberDecorate,
                          {Operand(struct_id), Operand(idx), U32Operand(SpvDecorationMatrixStride),
                           Operand(effective_row_count * scalar_elem_size)});
    }

    return GenerateTypeIfNeeded(member->Type());
}

bool Builder::GenerateVectorType(const type::Vector* vec, const Operand& result) {
    auto type_id = GenerateTypeIfNeeded(vec->type());
    if (has_error()) {
        return false;
    }

    module_.PushType(spv::Op::OpTypeVector, {result, Operand(type_id), Operand(vec->Width())});
    return true;
}

SpvStorageClass Builder::ConvertAddressSpace(builtin::AddressSpace klass) const {
    switch (klass) {
        case builtin::AddressSpace::kIn:
            return SpvStorageClassInput;
        case builtin::AddressSpace::kOut:
            return SpvStorageClassOutput;
        case builtin::AddressSpace::kUniform:
            return SpvStorageClassUniform;
        case builtin::AddressSpace::kWorkgroup:
            return SpvStorageClassWorkgroup;
        case builtin::AddressSpace::kPushConstant:
            return SpvStorageClassPushConstant;
        case builtin::AddressSpace::kHandle:
            return SpvStorageClassUniformConstant;
        case builtin::AddressSpace::kStorage:
            return SpvStorageClassStorageBuffer;
        case builtin::AddressSpace::kPrivate:
            return SpvStorageClassPrivate;
        case builtin::AddressSpace::kFunction:
            return SpvStorageClassFunction;
        case builtin::AddressSpace::kUndefined:
            break;
    }
    return SpvStorageClassMax;
}

SpvBuiltIn Builder::ConvertBuiltin(builtin::BuiltinValue builtin, builtin::AddressSpace storage) {
    switch (builtin) {
        case builtin::BuiltinValue::kPosition:
            if (storage == builtin::AddressSpace::kIn) {
                return SpvBuiltInFragCoord;
            } else if (TINT_LIKELY(storage == builtin::AddressSpace::kOut)) {
                return SpvBuiltInPosition;
            } else {
                TINT_ICE(Writer, builder_.Diagnostics()) << "invalid address space for builtin";
                break;
            }
        case builtin::BuiltinValue::kVertexIndex:
            return SpvBuiltInVertexIndex;
        case builtin::BuiltinValue::kInstanceIndex:
            return SpvBuiltInInstanceIndex;
        case builtin::BuiltinValue::kFrontFacing:
            return SpvBuiltInFrontFacing;
        case builtin::BuiltinValue::kFragDepth:
            return SpvBuiltInFragDepth;
        case builtin::BuiltinValue::kLocalInvocationId:
            return SpvBuiltInLocalInvocationId;
        case builtin::BuiltinValue::kLocalInvocationIndex:
            return SpvBuiltInLocalInvocationIndex;
        case builtin::BuiltinValue::kGlobalInvocationId:
            return SpvBuiltInGlobalInvocationId;
        case builtin::BuiltinValue::kPointSize:
            return SpvBuiltInPointSize;
        case builtin::BuiltinValue::kWorkgroupId:
            return SpvBuiltInWorkgroupId;
        case builtin::BuiltinValue::kNumWorkgroups:
            return SpvBuiltInNumWorkgroups;
        case builtin::BuiltinValue::kSampleIndex:
            module_.PushCapability(SpvCapabilitySampleRateShading);
            return SpvBuiltInSampleId;
        case builtin::BuiltinValue::kSampleMask:
            return SpvBuiltInSampleMask;
        case builtin::BuiltinValue::kUndefined:
            break;
    }
    return SpvBuiltInMax;
}

void Builder::AddInterpolationDecorations(uint32_t id,
                                          builtin::InterpolationType type,
                                          builtin::InterpolationSampling sampling) {
    switch (type) {
        case builtin::InterpolationType::kLinear:
            module_.PushAnnot(spv::Op::OpDecorate,
                              {Operand(id), U32Operand(SpvDecorationNoPerspective)});
            break;
        case builtin::InterpolationType::kFlat:
            module_.PushAnnot(spv::Op::OpDecorate, {Operand(id), U32Operand(SpvDecorationFlat)});
            break;
        case builtin::InterpolationType::kPerspective:
        case builtin::InterpolationType::kUndefined:
            break;
    }
    switch (sampling) {
        case builtin::InterpolationSampling::kCentroid:
            module_.PushAnnot(spv::Op::OpDecorate,
                              {Operand(id), U32Operand(SpvDecorationCentroid)});
            break;
        case builtin::InterpolationSampling::kSample:
            module_.PushCapability(SpvCapabilitySampleRateShading);
            module_.PushAnnot(spv::Op::OpDecorate, {Operand(id), U32Operand(SpvDecorationSample)});
            break;
        case builtin::InterpolationSampling::kCenter:
        case builtin::InterpolationSampling::kUndefined:
            break;
    }
}

SpvImageFormat Builder::convert_texel_format_to_spv(const builtin::TexelFormat format) {
    switch (format) {
        case builtin::TexelFormat::kBgra8Unorm:
            TINT_ICE(Writer, builder_.Diagnostics())
                << "bgra8unorm should have been polyfilled to rgba8unorm";
            return SpvImageFormatUnknown;
        case builtin::TexelFormat::kR32Uint:
            return SpvImageFormatR32ui;
        case builtin::TexelFormat::kR32Sint:
            return SpvImageFormatR32i;
        case builtin::TexelFormat::kR32Float:
            return SpvImageFormatR32f;
        case builtin::TexelFormat::kRgba8Unorm:
            return SpvImageFormatRgba8;
        case builtin::TexelFormat::kRgba8Snorm:
            return SpvImageFormatRgba8Snorm;
        case builtin::TexelFormat::kRgba8Uint:
            return SpvImageFormatRgba8ui;
        case builtin::TexelFormat::kRgba8Sint:
            return SpvImageFormatRgba8i;
        case builtin::TexelFormat::kRg32Uint:
            module_.PushCapability(SpvCapabilityStorageImageExtendedFormats);
            return SpvImageFormatRg32ui;
        case builtin::TexelFormat::kRg32Sint:
            module_.PushCapability(SpvCapabilityStorageImageExtendedFormats);
            return SpvImageFormatRg32i;
        case builtin::TexelFormat::kRg32Float:
            module_.PushCapability(SpvCapabilityStorageImageExtendedFormats);
            return SpvImageFormatRg32f;
        case builtin::TexelFormat::kRgba16Uint:
            return SpvImageFormatRgba16ui;
        case builtin::TexelFormat::kRgba16Sint:
            return SpvImageFormatRgba16i;
        case builtin::TexelFormat::kRgba16Float:
            return SpvImageFormatRgba16f;
        case builtin::TexelFormat::kRgba32Uint:
            return SpvImageFormatRgba32ui;
        case builtin::TexelFormat::kRgba32Sint:
            return SpvImageFormatRgba32i;
        case builtin::TexelFormat::kRgba32Float:
            return SpvImageFormatRgba32f;
        case builtin::TexelFormat::kUndefined:
            return SpvImageFormatUnknown;
    }
    return SpvImageFormatUnknown;
}

bool Builder::push_function_inst(spv::Op op, const OperandList& operands) {
    if (!current_function_) {
        utils::StringStream ss;
        ss << "Internal error: trying to add SPIR-V instruction " << int(op)
           << " outside a function";
        TINT_ICE(Writer, builder_.Diagnostics()) << ss.str();
        return false;
    }
    current_function_.push_inst(op, operands);
    return true;
}

bool Builder::InsideBasicBlock() const {
    if (!current_function_) {
        return false;
    }
    const auto& instructions = current_function_.instructions();
    if (instructions.empty()) {
        // The Function object does not explicitly represent its entry block
        // label.  So return *true* because an empty list means the only
        // thing in the function is that entry block label.
        return true;
    }
    const auto& inst = instructions.back();
    switch (inst.opcode()) {
        case spv::Op::OpBranch:
        case spv::Op::OpBranchConditional:
        case spv::Op::OpSwitch:
        case spv::Op::OpReturn:
        case spv::Op::OpReturnValue:
        case spv::Op::OpUnreachable:
        case spv::Op::OpKill:
        case spv::Op::OpTerminateInvocation:
            return false;
        default:
            break;
    }
    return true;
}

Builder::ContinuingInfo::ContinuingInfo(const ast::Statement* the_last_statement,
                                        uint32_t loop_id,
                                        uint32_t break_id)
    : last_statement(the_last_statement), loop_header_id(loop_id), break_target_id(break_id) {
    TINT_ASSERT(Writer, last_statement != nullptr);
    TINT_ASSERT(Writer, loop_header_id != 0u);
    TINT_ASSERT(Writer, break_target_id != 0u);
}

Builder::Backedge::Backedge(spv::Op the_opcode, OperandList the_operands)
    : opcode(the_opcode), operands(the_operands) {}

Builder::Backedge::Backedge(const Builder::Backedge& other) = default;
Builder::Backedge& Builder::Backedge::operator=(const Builder::Backedge& other) = default;
Builder::Backedge::~Backedge() = default;

Builder::Scope::Scope() = default;
Builder::Scope::Scope(const Scope&) = default;
Builder::Scope::~Scope() = default;

}  // namespace tint::writer::spirv
