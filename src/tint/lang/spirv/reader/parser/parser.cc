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

#include "src/tint/lang/spirv/reader/parser/parser.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

TINT_BEGIN_DISABLE_WARNING(NEWLINE_EOF);
TINT_BEGIN_DISABLE_WARNING(OLD_STYLE_CAST);
TINT_BEGIN_DISABLE_WARNING(SIGN_CONVERSION);
TINT_BEGIN_DISABLE_WARNING(WEAK_VTABLES);
#include "source/opt/build_module.h"
TINT_END_DISABLE_WARNING(WEAK_VTABLES);
TINT_END_DISABLE_WARNING(SIGN_CONVERSION);
TINT_END_DISABLE_WARNING(OLD_STYLE_CAST);
TINT_END_DISABLE_WARNING(NEWLINE_EOF);

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/spirv/validate/validate.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::spirv::reader {

namespace {

/// The SPIR-V environment that we validate against.
constexpr auto kTargetEnv = SPV_ENV_VULKAN_1_1;

/// PIMPL class for SPIR-V parser.
/// Validates the SPIR-V module and then parses it to produce a Tint IR module.
class Parser {
  public:
    /// @param spirv the SPIR-V binary data
    /// @returns the generated SPIR-V IR module on success, or failure
    Result<core::ir::Module> Run(Slice<const uint32_t> spirv) {
        // Validate the incoming SPIR-V binary.
        auto result = validate::Validate(spirv, kTargetEnv);
        if (result != Success) {
            return result.Failure();
        }

        // Build the SPIR-V tools internal representation of the SPIR-V module.
        spvtools::Context context(kTargetEnv);
        spirv_context_ =
            spvtools::BuildModule(kTargetEnv, context.CContext()->consumer, spirv.data, spirv.len);
        if (!spirv_context_) {
            return Failure("failed to build the internal representation of the module");
        }

        EmitModuleScopeVariables();

        EmitFunctions();

        EmitEntryPoints();

        // TODO(crbug.com/tint/1907): Handle annotation instructions.
        // TODO(crbug.com/tint/1907): Handle names.

        return std::move(ir_);
    }

    /// @param sc a SPIR-V storage class
    /// @returns the Tint address space for a SPIR-V storage class
    core::AddressSpace AddressSpace(spv::StorageClass sc) {
        switch (sc) {
            case spv::StorageClass::Function:
                return core::AddressSpace::kFunction;
            case spv::StorageClass::Private:
                return core::AddressSpace::kPrivate;
            case spv::StorageClass::Uniform:
                return core::AddressSpace::kUniform;
            default:
                TINT_UNIMPLEMENTED()
                    << "unhandled SPIR-V storage class: " << static_cast<uint32_t>(sc);
                return core::AddressSpace::kUndefined;
        }
    }

    /// @param type a SPIR-V type object
    /// @returns a Tint type object
    const core::type::Type* Type(const spvtools::opt::analysis::Type* type) {
        return types_.GetOrCreate(type, [&]() -> const core::type::Type* {
            switch (type->kind()) {
                case spvtools::opt::analysis::Type::kVoid:
                    return ty_.void_();
                case spvtools::opt::analysis::Type::kBool:
                    return ty_.bool_();
                case spvtools::opt::analysis::Type::kInteger: {
                    auto* int_ty = type->AsInteger();
                    TINT_ASSERT_OR_RETURN_VALUE(int_ty->width() == 32, ty_.void_());
                    if (int_ty->IsSigned()) {
                        return ty_.i32();
                    } else {
                        return ty_.u32();
                    }
                }
                case spvtools::opt::analysis::Type::kFloat: {
                    auto* float_ty = type->AsFloat();
                    if (float_ty->width() == 16) {
                        return ty_.f16();
                    } else if (float_ty->width() == 32) {
                        return ty_.f32();
                    } else {
                        TINT_UNREACHABLE()
                            << "unsupported floating point type width: " << float_ty->width();
                        return ty_.void_();
                    }
                }
                case spvtools::opt::analysis::Type::kVector: {
                    auto* vec_ty = type->AsVector();
                    TINT_ASSERT_OR_RETURN_VALUE(vec_ty->element_count() <= 4, ty_.void_());
                    return ty_.vec(Type(vec_ty->element_type()), vec_ty->element_count());
                }
                case spvtools::opt::analysis::Type::kMatrix: {
                    auto* mat_ty = type->AsMatrix();
                    TINT_ASSERT_OR_RETURN_VALUE(mat_ty->element_count() <= 4, ty_.void_());
                    return ty_.mat(As<core::type::Vector>(Type(mat_ty->element_type())),
                                   mat_ty->element_count());
                }
                case spvtools::opt::analysis::Type::kArray:
                    return EmitArray(type->AsArray());
                case spvtools::opt::analysis::Type::kStruct:
                    return EmitStruct(type->AsStruct());
                case spvtools::opt::analysis::Type::kPointer: {
                    auto* ptr_ty = type->AsPointer();
                    return ty_.ptr(AddressSpace(ptr_ty->storage_class()),
                                   Type(ptr_ty->pointee_type()));
                }
                default:
                    TINT_UNIMPLEMENTED() << "unhandled SPIR-V type: " << type->str();
                    return ty_.void_();
            }
        });
    }

    /// @param id a SPIR-V result ID for a type declaration instruction
    /// @returns a Tint type object
    const core::type::Type* Type(uint32_t id) {
        return Type(spirv_context_->get_type_mgr()->GetType(id));
    }

    /// @param arr_ty a SPIR-V array object
    /// @returns a Tint array object
    const core::type::Type* EmitArray(const spvtools::opt::analysis::Array* arr_ty) {
        const auto& length = arr_ty->length_info();
        TINT_ASSERT_OR_RETURN_VALUE(!length.words.empty(), ty_.void_());
        if (length.words[0] != spvtools::opt::analysis::Array::LengthInfo::kConstant) {
            TINT_UNIMPLEMENTED() << "specialized array lengths";
            return ty_.void_();
        }

        // Get the value from the constant used for the element count.
        const auto* count_const =
            spirv_context_->get_constant_mgr()->FindDeclaredConstant(length.id);
        TINT_ASSERT_OR_RETURN_VALUE(count_const, ty_.void_());
        const uint64_t count_val = count_const->GetZeroExtendedValue();
        TINT_ASSERT_OR_RETURN_VALUE(count_val <= UINT32_MAX, ty_.void_());

        // TODO(crbug.com/1907): Handle decorations that affect the array layout.

        return ty_.array(Type(arr_ty->element_type()), static_cast<uint32_t>(count_val));
    }

    /// @param struct_ty a SPIR-V struct object
    /// @returns a Tint struct object
    const core::type::Type* EmitStruct(const spvtools::opt::analysis::Struct* struct_ty) {
        if (struct_ty->NumberOfComponents() == 0) {
            TINT_ICE() << "empty structures are not supported";
            return ty_.void_();
        }

        // Build a list of struct members.
        uint32_t current_size = 0u;
        Vector<core::type::StructMember*, 4> members;
        for (uint32_t i = 0; i < struct_ty->NumberOfComponents(); i++) {
            auto* member_ty = Type(struct_ty->element_types()[i]);
            uint32_t align = std::max<uint32_t>(member_ty->Align(), 1u);
            uint32_t offset = tint::RoundUp(align, current_size);
            core::type::StructMemberAttributes attributes;

            // Handle member decorations that affect layout or attributes.
            if (struct_ty->element_decorations().count(i)) {
                for (auto& deco : struct_ty->element_decorations().at(i)) {
                    switch (spv::Decoration(deco[0])) {
                        case spv::Decoration::Offset:
                            offset = deco[1];
                            break;
                        default:
                            TINT_UNIMPLEMENTED() << "unhandled member decoration: " << deco[0];
                            break;
                    }
                }
            }

            // TODO(crbug.com/tint/1907): Use OpMemberName to name it.
            members.Push(ty_.Get<core::type::StructMember>(ir_.symbols.New(), member_ty, i, offset,
                                                           align, member_ty->Size(),
                                                           std::move(attributes)));

            current_size = offset + member_ty->Size();
        }
        // TODO(crbug.com/tint/1907): Use OpName to name it.
        return ty_.Struct(ir_.symbols.New(), std::move(members));
    }

    /// @param id a SPIR-V result ID for a function declaration instruction
    /// @returns a Tint function object
    core::ir::Function* Function(uint32_t id) {
        return functions_.GetOrCreate(id, [&] {
            return b_.Function(ty_.void_(), core::ir::Function::PipelineStage::kUndefined,
                               std::nullopt);
        });
    }

    /// @param id a SPIR-V result ID
    /// @returns a Tint value object
    core::ir::Value* Value(uint32_t id) {
        return values_.GetOrCreate(id, [&]() -> core::ir::Value* {
            if (auto* c = spirv_context_->get_constant_mgr()->FindDeclaredConstant(id)) {
                return b_.Constant(Constant(c));
            }
            TINT_UNREACHABLE() << "missing value for result ID " << id;
            return nullptr;
        });
    }

    /// @param constant a SPIR-V constant object
    /// @returns a Tint constant value
    const core::constant::Value* Constant(const spvtools::opt::analysis::Constant* constant) {
        // Handle OpConstantNull for all types.
        if (constant->AsNullConstant()) {
            return ir_.constant_values.Zero(Type(constant->type()));
        }

        if (auto* bool_ = constant->AsBoolConstant()) {
            return b_.ConstantValue(bool_->value());
        }
        if (auto* i = constant->AsIntConstant()) {
            auto* int_ty = i->type()->AsInteger();
            TINT_ASSERT_OR_RETURN_VALUE(int_ty->width() == 32, nullptr);
            if (int_ty->IsSigned()) {
                return b_.ConstantValue(i32(i->GetS32BitValue()));
            } else {
                return b_.ConstantValue(u32(i->GetU32BitValue()));
            }
        }
        if (auto* f = constant->AsFloatConstant()) {
            auto* float_ty = f->type()->AsFloat();
            if (float_ty->width() == 16) {
                return b_.ConstantValue(f16::FromBits(static_cast<uint16_t>(f->words()[0])));
            } else if (float_ty->width() == 32) {
                return b_.ConstantValue(f32(f->GetFloat()));
            } else {
                TINT_UNREACHABLE() << "unsupported floating point type width";
                return nullptr;
            }
        }
        if (auto* v = constant->AsVectorConstant()) {
            Vector<const core::constant::Value*, 4> elements;
            for (auto& el : v->GetComponents()) {
                elements.Push(Constant(el));
            }
            return ir_.constant_values.Composite(Type(v->type()), std::move(elements));
        }
        if (auto* m = constant->AsMatrixConstant()) {
            Vector<const core::constant::Value*, 4> columns;
            for (auto& el : m->GetComponents()) {
                columns.Push(Constant(el));
            }
            return ir_.constant_values.Composite(Type(m->type()), std::move(columns));
        }
        if (auto* a = constant->AsArrayConstant()) {
            Vector<const core::constant::Value*, 16> elements;
            for (auto& el : a->GetComponents()) {
                elements.Push(Constant(el));
            }
            return ir_.constant_values.Composite(Type(a->type()), std::move(elements));
        }
        if (auto* s = constant->AsStructConstant()) {
            Vector<const core::constant::Value*, 16> elements;
            for (auto& el : s->GetComponents()) {
                elements.Push(Constant(el));
            }
            return ir_.constant_values.Composite(Type(s->type()), std::move(elements));
        }
        TINT_UNIMPLEMENTED() << "unhandled constant type";
        return nullptr;
    }

    /// Emit the module-scope variables.
    void EmitModuleScopeVariables() {
        for (auto& inst : spirv_context_->module()->types_values()) {
            if (inst.opcode() == spv::Op::OpVariable) {
                ir_.root_block->Append(EmitVar(inst));
            }
        }
    }

    /// Emit the functions.
    void EmitFunctions() {
        for (auto& func : *spirv_context_->module()) {
            Vector<core::ir::FunctionParam*, 4> params;
            func.ForEachParam([&](spvtools::opt::Instruction* spirv_param) {
                auto* param = b_.FunctionParam(Type(spirv_param->type_id()));
                values_.Add(spirv_param->result_id(), param);
                params.Push(param);
            });

            current_function_ = Function(func.result_id());
            current_function_->SetParams(std::move(params));
            current_function_->SetReturnType(Type(func.type_id()));

            functions_.Add(func.result_id(), current_function_);
            EmitBlock(current_function_->Block(), *func.entry());
        }
    }

    /// Emit entry point attributes.
    void EmitEntryPoints() {
        // Handle OpEntryPoint declarations.
        for (auto& entry_point : spirv_context_->module()->entry_points()) {
            auto model = entry_point.GetSingleWordInOperand(0);
            auto* func = Function(entry_point.GetSingleWordInOperand(1));

            // Set the pipeline stage.
            switch (spv::ExecutionModel(model)) {
                case spv::ExecutionModel::GLCompute:
                    func->SetStage(core::ir::Function::PipelineStage::kCompute);
                    break;
                case spv::ExecutionModel::Fragment:
                    func->SetStage(core::ir::Function::PipelineStage::kFragment);
                    break;
                case spv::ExecutionModel::Vertex:
                    func->SetStage(core::ir::Function::PipelineStage::kVertex);
                    break;
                default:
                    TINT_UNIMPLEMENTED() << "unhandled execution model: " << model;
            }

            // Set the entry point name.
            ir_.SetName(func, entry_point.GetOperand(2).AsString());
        }

        // Handle OpExecutionMode declarations.
        for (auto& execution_mode : spirv_context_->module()->execution_modes()) {
            auto* func = functions_.Get(execution_mode.GetSingleWordInOperand(0)).value_or(nullptr);
            auto mode = execution_mode.GetSingleWordInOperand(1);
            TINT_ASSERT_OR_RETURN(func);

            switch (spv::ExecutionMode(mode)) {
                case spv::ExecutionMode::LocalSize:
                    func->SetWorkgroupSize(execution_mode.GetSingleWordInOperand(2),
                                           execution_mode.GetSingleWordInOperand(3),
                                           execution_mode.GetSingleWordInOperand(4));
                    break;
                case spv::ExecutionMode::OriginUpperLeft:
                    break;
                default:
                    TINT_UNIMPLEMENTED() << "unhandled execution mode: " << mode;
            }
        }
    }

    /// Emit the contents of SPIR-V block @p src into Tint IR block @p dst.
    /// @param dst the Tint IR block to append to
    /// @param src the SPIR-V block to emit
    void EmitBlock(core::ir::Block* dst, const spvtools::opt::BasicBlock& src) {
        for (auto& inst : src) {
            switch (inst.opcode()) {
                case spv::Op::OpFunctionCall:
                    dst->Append(EmitFunctionCall(inst));
                    break;
                case spv::Op::OpReturn:
                    dst->Append(b_.Return(current_function_));
                    break;
                case spv::Op::OpReturnValue:
                    dst->Append(b_.Return(current_function_, Value(inst.GetSingleWordOperand(0))));
                    break;
                case spv::Op::OpVariable:
                    dst->Append(EmitVar(inst));
                    break;
                default:
                    TINT_UNIMPLEMENTED()
                        << "unhandled SPIR-V instruction: " << static_cast<uint32_t>(inst.opcode());
            }
        }
    }

    /// @param inst the SPIR-V instruction for OpFunctionCall
    /// @returns the Tint IR instruction
    core::ir::UserCall* EmitFunctionCall(const spvtools::opt::Instruction& inst) {
        // TODO(crbug.com/tint/1907): Capture result.
        Vector<core::ir::Value*, 4> args;
        for (uint32_t i = 3; i < inst.NumOperandWords(); i++) {
            args.Push(Value(inst.GetSingleWordOperand(i)));
        }
        return b_.Call(Function(inst.GetSingleWordInOperand(0)), std::move(args));
    }

    /// @param inst the SPIR-V instruction for OpVariable
    /// @returns the Tint IR instruction
    core::ir::Var* EmitVar(const spvtools::opt::Instruction& inst) {
        auto* var = b_.Var(Type(inst.type_id())->As<core::type::Pointer>());
        if (inst.NumOperands() > 3) {
            var->SetInitializer(Value(inst.GetSingleWordOperand(3)));
        }
        return var;
    }

  private:
    /// The generated IR module.
    core::ir::Module ir_;
    /// The Tint IR builder.
    core::ir::Builder b_{ir_};
    /// The Tint type manager.
    core::type::Manager& ty_{ir_.Types()};

    /// The Tint IR function that is currently being emitted.
    core::ir::Function* current_function_ = nullptr;
    /// A map from a SPIR-V type declaration result ID to the corresponding Tint type object.
    Hashmap<const spvtools::opt::analysis::Type*, const core::type::Type*, 16> types_;
    /// A map from a SPIR-V function definition result ID to the corresponding Tint function object.
    Hashmap<uint32_t, core::ir::Function*, 8> functions_;
    /// A map from a SPIR-V result ID to the corresponding Tint value object.
    Hashmap<uint32_t, core::ir::Value*, 8> values_;

    /// The SPIR-V context containing the SPIR-V tools intermediate representation.
    std::unique_ptr<spvtools::opt::IRContext> spirv_context_;
};

}  // namespace

Result<core::ir::Module> Parse(Slice<const uint32_t> spirv) {
    return Parser{}.Run(spirv);
}

}  // namespace tint::spirv::reader
