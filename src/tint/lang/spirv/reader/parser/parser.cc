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
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

TINT_BEGIN_DISABLE_WARNING(NEWLINE_EOF);
TINT_BEGIN_DISABLE_WARNING(OLD_STYLE_CAST);
TINT_BEGIN_DISABLE_WARNING(SIGN_CONVERSION);
TINT_BEGIN_DISABLE_WARNING(WEAK_VTABLES);
TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);
#include "source/opt/build_module.h"
TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);
TINT_END_DISABLE_WARNING(WEAK_VTABLES);
TINT_END_DISABLE_WARNING(SIGN_CONVERSION);
TINT_END_DISABLE_WARNING(OLD_STYLE_CAST);
TINT_END_DISABLE_WARNING(NEWLINE_EOF);

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/spirv/builtin_fn.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"
#include "src/tint/lang/spirv/validate/validate.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

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

        // Check for unsupported extensions.
        for (const auto& ext : spirv_context_->extensions()) {
            auto name = ext.GetOperand(0).AsString();
            if (name != "SPV_KHR_storage_buffer_storage_class" &&
                name != "SPV_KHR_non_semantic_info") {
                return Failure("SPIR-V extension '" + name + "' is not supported");
            }
        }

        // Register imported instruction sets
        for (const auto& import : spirv_context_->ext_inst_imports()) {
            auto name = import.GetInOperand(0).AsString();

            // TODO(dneto): Handle other extended instruction sets when needed.
            if (name == "GLSL.std.450") {
                glsl_std_450_imports_.insert(import.result_id());
            } else if (name.find("NonSemantic.") == 0) {
                ignored_imports_.insert(import.result_id());
            } else {
                return Failure("Unrecognized extended instruction set: " + name);
            }
        }

        id_stack_.emplace_back();
        {
            TINT_SCOPED_ASSIGNMENT(current_block_, ir_.root_block);
            EmitModuleScopeVariables();
        }

        RegisterNames();

        EmitFunctions();
        EmitEntryPointAttributes();

        // TODO(crbug.com/tint/1907): Handle annotation instructions.

        return std::move(ir_);
    }

    void RegisterNames() {
        // Register names from OpName
        for (const auto& inst : spirv_context_->debugs2()) {
            switch (inst.opcode()) {
                case spv::Op::OpName: {
                    const auto name = inst.GetInOperand(1).AsString();
                    if (!name.empty()) {
                        id_to_name_[inst.GetSingleWordInOperand(0)] = name;
                    }
                    break;
                }
                case spv::Op::OpMemberName: {
                    const auto name = inst.GetInOperand(2).AsString();
                    if (!name.empty()) {
                        uint32_t struct_id = inst.GetSingleWordInOperand(0);
                        uint32_t member_idx = inst.GetSingleWordInOperand(1);
                        auto iter = struct_to_member_names_.insert({struct_id, {}});
                        auto& members = (*(iter.first)).second;

                        if (members.size() < (member_idx + 1)) {
                            members.resize(member_idx + 1);
                        }
                        members[member_idx] = name;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    /// @param sc a SPIR-V storage class
    /// @returns the Tint address space for a SPIR-V storage class
    core::AddressSpace AddressSpace(spv::StorageClass sc) {
        switch (sc) {
            case spv::StorageClass::Input:
                return core::AddressSpace::kIn;
            case spv::StorageClass::Output:
                return core::AddressSpace::kOut;
            case spv::StorageClass::Function:
                return core::AddressSpace::kFunction;
            case spv::StorageClass::Private:
                return core::AddressSpace::kPrivate;
            case spv::StorageClass::StorageBuffer:
                return core::AddressSpace::kStorage;
            case spv::StorageClass::Uniform:
                return core::AddressSpace::kUniform;
            case spv::StorageClass::UniformConstant:
                return core::AddressSpace::kHandle;
            default:
                TINT_UNIMPLEMENTED()
                    << "unhandled SPIR-V storage class: " << static_cast<uint32_t>(sc);
        }
    }

    /// @param b a SPIR-V BuiltIn
    /// @returns the Tint builtin value for a SPIR-V BuiltIn decoration
    core::BuiltinValue Builtin(spv::BuiltIn b) {
        switch (b) {
            case spv::BuiltIn::FragCoord:
                return core::BuiltinValue::kPosition;
            case spv::BuiltIn::FragDepth:
                return core::BuiltinValue::kFragDepth;
            case spv::BuiltIn::FrontFacing:
                return core::BuiltinValue::kFrontFacing;
            case spv::BuiltIn::GlobalInvocationId:
                return core::BuiltinValue::kGlobalInvocationId;
            case spv::BuiltIn::InstanceIndex:
                return core::BuiltinValue::kInstanceIndex;
            case spv::BuiltIn::LocalInvocationId:
                return core::BuiltinValue::kLocalInvocationId;
            case spv::BuiltIn::LocalInvocationIndex:
                return core::BuiltinValue::kLocalInvocationIndex;
            case spv::BuiltIn::NumWorkgroups:
                return core::BuiltinValue::kNumWorkgroups;
            case spv::BuiltIn::PointSize:
                return core::BuiltinValue::kPointSize;
            case spv::BuiltIn::Position:
                return core::BuiltinValue::kPosition;
            case spv::BuiltIn::SampleId:
                return core::BuiltinValue::kSampleIndex;
            case spv::BuiltIn::SampleMask:
                return core::BuiltinValue::kSampleMask;
            case spv::BuiltIn::VertexIndex:
                return core::BuiltinValue::kVertexIndex;
            case spv::BuiltIn::WorkgroupId:
                return core::BuiltinValue::kWorkgroupId;
            case spv::BuiltIn::ClipDistance:
                return core::BuiltinValue::kClipDistances;
            case spv::BuiltIn::CullDistance:
                return core::BuiltinValue::kCullDistance;
            default:
                TINT_UNIMPLEMENTED() << "unhandled SPIR-V BuiltIn: " << static_cast<uint32_t>(b);
        }
    }

    /// @param type a SPIR-V type object
    /// @param access_mode an optional access mode (for pointers)
    /// @returns a Tint type object
    const core::type::Type* Type(const spvtools::opt::analysis::Type* type,
                                 core::Access access_mode = core::Access::kUndefined) {
        return types_.GetOrAdd(TypeKey{type, access_mode}, [&]() -> const core::type::Type* {
            switch (type->kind()) {
                case spvtools::opt::analysis::Type::kVoid:
                    return ty_.void_();
                case spvtools::opt::analysis::Type::kBool:
                    return ty_.bool_();
                case spvtools::opt::analysis::Type::kInteger: {
                    auto* int_ty = type->AsInteger();
                    TINT_ASSERT(int_ty->width() == 32);
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
                    }
                }
                case spvtools::opt::analysis::Type::kVector: {
                    auto* vec_ty = type->AsVector();
                    TINT_ASSERT(vec_ty->element_count() <= 4);
                    return ty_.vec(Type(vec_ty->element_type()), vec_ty->element_count());
                }
                case spvtools::opt::analysis::Type::kMatrix: {
                    auto* mat_ty = type->AsMatrix();
                    TINT_ASSERT(mat_ty->element_count() <= 4);
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
                                   Type(ptr_ty->pointee_type()), access_mode);
                }
                case spvtools::opt::analysis::Type::kSampler: {
                    // TODO(dsinclair): How to determine comparison samplers ...
                    return ty_.sampler();
                }
                default:
                    TINT_UNIMPLEMENTED() << "unhandled SPIR-V type: " << type->str();
            }
        });
    }

    /// @param id a SPIR-V result ID for a type declaration instruction
    /// @param access_mode an optional access mode (for pointers)
    /// @returns a Tint type object
    const core::type::Type* Type(uint32_t id, core::Access access_mode = core::Access::kUndefined) {
        return Type(spirv_context_->get_type_mgr()->GetType(id), access_mode);
    }

    /// @param arr_ty a SPIR-V array object
    /// @returns a Tint array object
    const core::type::Type* EmitArray(const spvtools::opt::analysis::Array* arr_ty) {
        const auto& length = arr_ty->length_info();
        TINT_ASSERT(!length.words.empty());
        if (length.words[0] != spvtools::opt::analysis::Array::LengthInfo::kConstant) {
            TINT_UNIMPLEMENTED() << "specialized array lengths";
        }

        // Get the value from the constant used for the element count.
        const auto* count_const =
            spirv_context_->get_constant_mgr()->FindDeclaredConstant(length.id);
        TINT_ASSERT(count_const);
        const uint64_t count_val = count_const->GetZeroExtendedValue();
        TINT_ASSERT(count_val <= UINT32_MAX);

        // TODO(crbug.com/1907): Handle decorations that affect the array layout.

        return ty_.array(Type(arr_ty->element_type()), static_cast<uint32_t>(count_val));
    }

    /// @param struct_ty a SPIR-V struct object
    /// @returns a Tint struct object
    const core::type::Type* EmitStruct(const spvtools::opt::analysis::Struct* struct_ty) {
        if (struct_ty->NumberOfComponents() == 0) {
            TINT_ICE() << "empty structures are not supported";
        }

        auto* type_mgr = spirv_context_->get_type_mgr();
        auto struct_id = type_mgr->GetId(struct_ty);

        std::vector<std::string>* member_names = nullptr;
        auto struct_to_member_iter = struct_to_member_names_.find(struct_id);
        if (struct_to_member_iter != struct_to_member_names_.end()) {
            member_names = &((*struct_to_member_iter).second);
        }

        // Build a list of struct members.
        uint32_t current_size = 0u;
        Vector<core::type::StructMember*, 4> members;
        for (uint32_t i = 0; i < struct_ty->NumberOfComponents(); i++) {
            auto* member_ty = Type(struct_ty->element_types()[i]);
            uint32_t align = std::max<uint32_t>(member_ty->Align(), 1u);
            uint32_t offset = tint::RoundUp(align, current_size);
            core::IOAttributes attributes;
            auto interpolation = [&]() -> core::Interpolation& {
                // Create the interpolation field with the default values on first call.
                if (!attributes.interpolation.has_value()) {
                    attributes.interpolation =
                        core::Interpolation{core::InterpolationType::kPerspective,
                                            core::InterpolationSampling::kCenter};
                }
                return attributes.interpolation.value();
            };

            // Handle member decorations that affect layout or attributes.
            if (struct_ty->element_decorations().count(i)) {
                for (auto& deco : struct_ty->element_decorations().at(i)) {
                    switch (spv::Decoration(deco[0])) {
                        case spv::Decoration::Offset:
                            offset = deco[1];
                            break;
                        case spv::Decoration::BuiltIn:
                            attributes.builtin = Builtin(spv::BuiltIn(deco[1]));
                            break;
                        case spv::Decoration::Invariant:
                            attributes.invariant = true;
                            break;
                        case spv::Decoration::Location:
                            attributes.location = deco[1];
                            break;
                        case spv::Decoration::NoPerspective:
                            interpolation().type = core::InterpolationType::kLinear;
                            break;
                        case spv::Decoration::Flat:
                            interpolation().type = core::InterpolationType::kFlat;
                            break;
                        case spv::Decoration::Centroid:
                            interpolation().sampling = core::InterpolationSampling::kCentroid;
                            break;
                        case spv::Decoration::Sample:
                            interpolation().sampling = core::InterpolationSampling::kSample;
                            break;

                        default:
                            TINT_UNIMPLEMENTED() << "unhandled member decoration: " << deco[0];
                    }
                }
            }

            Symbol name;
            if (member_names && member_names->size() > i) {
                auto n = (*member_names)[i];
                if (!n.empty()) {
                    name = ir_.symbols.Register(n);
                }
            }
            if (!name.IsValid()) {
                name = ir_.symbols.New();
            }

            members.Push(ty_.Get<core::type::StructMember>(
                name, member_ty, i, offset, align, member_ty->Size(), std::move(attributes)));

            current_size = offset + member_ty->Size();
        }

        Symbol name = GetUniqueSymbolFor(struct_id);
        if (!name.IsValid()) {
            name = ir_.symbols.New();
        }
        return ty_.Struct(name, std::move(members));
    }

    Symbol GetUniqueSymbolFor(uint32_t id) {
        auto iter = id_to_name_.find(id);
        if (iter != id_to_name_.end()) {
            return ir_.symbols.New(iter->second);
        }
        return Symbol{};
    }

    Symbol GetSymbolFor(uint32_t id) {
        auto iter = id_to_name_.find(id);
        if (iter != id_to_name_.end()) {
            return ir_.symbols.Register(iter->second);
        }
        return Symbol{};
    }

    /// @param id a SPIR-V result ID for a function declaration instruction
    /// @returns a Tint function object
    core::ir::Function* Function(uint32_t id) {
        return functions_.GetOrAdd(id, [&] { return b_.Function(ty_.void_()); });
    }

    core::ir::Value* Propagate(uint32_t id, core::ir::Value* src) {
        auto* src_res = src->As<core::ir::InstructionResult>();
        TINT_ASSERT(src_res);

        auto* blk = src_res->Instruction()->Block();
        while (blk) {
            if (current_blocks_.count(blk) > 0) {
                break;
            }

            auto* ctrl = blk->Parent();

            TINT_ASSERT(blk->Terminator());

            // Add ourselves as part of the terminator return value
            blk->Terminator()->PushOperand(src);
            // Add a new result to the control instruction
            ctrl->AddResult(b_.InstructionResult(src->Type()));
            // The source instruction is now the control result we just inserted
            src = ctrl->Results().Back();
            // The SPIR-V ID now refers to the propagated value.
            values_.Replace(id, src);

            blk = ctrl->Block();
        }
        return src;
    }

    bool IdIsInScope(uint32_t id) {
        for (auto iter = id_stack_.rbegin(); iter != id_stack_.rend(); ++iter) {
            if (iter->count(id) > 0) {
                return true;
            }
        }
        return false;
    }

    /// @param id a SPIR-V result ID
    /// @returns a Tint value object
    core::ir::Value* Value(uint32_t id) {
        auto v = values_.Get(id);
        if (v) {
            if (!(*v)->Is<core::ir::InstructionResult>()) {
                return *v;
            }
            if (IdIsInScope(id)) {
                return *v;
            }

            // The Value is not in scope, so we need to find the originating Value, and then
            // propagate it up through the control instructions. That will then change the
            // `Value` which is returned so, set it into the values map as the new "Value" and
            // return it.

            auto* new_v = Propagate(id, *v);
            values_.Replace(id, new_v);
            return new_v;
        }

        if (auto* c = spirv_context_->get_constant_mgr()->FindDeclaredConstant(id)) {
            auto* val = b_.Constant(Constant(c));
            values_.Add(id, val);
            return val;
        }
        TINT_UNREACHABLE() << "missing value for result ID " << id;
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
            TINT_ASSERT(int_ty->width() == 32);
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
    }

    /// Register an IR value for a SPIR-V result ID.
    /// @param result_id the SPIR-V result ID
    /// @param value the IR value
    void AddValue(uint32_t result_id, core::ir::Value* value) {
        id_stack_.back().insert(result_id);
        values_.Replace(result_id, value);
    }

    /// Emit an instruction to the current block and associates the result to
    /// the spirv result id.
    /// @param inst the instruction to emit
    /// @param result_id the SPIR-V result ID to register the instruction result for
    void Emit(core::ir::Instruction* inst, uint32_t result_id) {
        current_block_->Append(inst);
        TINT_ASSERT(inst->Results().Length() == 1u);
        AddValue(result_id, inst->Result(0));

        Symbol name = GetSymbolFor(result_id);
        if (name.IsValid()) {
            ir_.SetName(inst, name);
        }
    }

    /// Emit an instruction to the current block.
    /// @param inst the instruction to emit
    void EmitWithoutSpvResult(core::ir::Instruction* inst) {
        current_block_->Append(inst);
        TINT_ASSERT(inst->Results().Length() == 1u);
    }

    /// Emit an instruction to the current block.
    /// @param inst the instruction to emit
    void EmitWithoutResult(core::ir::Instruction* inst) {
        TINT_ASSERT(inst->Results().IsEmpty());
        current_block_->Append(inst);
    }

    /// Emit the module-scope variables.
    void EmitModuleScopeVariables() {
        for (auto& inst : spirv_context_->module()->types_values()) {
            switch (inst.opcode()) {
                case spv::Op::OpVariable:
                    EmitVar(inst);
                    break;
                case spv::Op::OpUndef:
                    AddValue(inst.result_id(), b_.Zero(Type(inst.type_id())));
                    break;
                default:
                    break;
            }
        }
    }

    /// Emit the functions.
    void EmitFunctions() {
        for (auto& func : *spirv_context_->module()) {
            current_spirv_function_ = &func;

            Vector<core::ir::FunctionParam*, 4> params;
            func.ForEachParam([&](spvtools::opt::Instruction* spirv_param) {
                auto* param = b_.FunctionParam(Type(spirv_param->type_id()));
                values_.Add(spirv_param->result_id(), param);

                Symbol name = GetSymbolFor(spirv_param->result_id());
                if (name.IsValid()) {
                    ir_.SetName(param, name);
                }

                params.Push(param);
            });

            current_function_ = Function(func.result_id());
            current_function_->SetParams(std::move(params));
            current_function_->SetReturnType(Type(func.type_id()));

            Symbol name = GetSymbolFor(func.result_id());
            if (name.IsValid()) {
                ir_.SetName(current_function_, name);
            }

            functions_.Add(func.result_id(), current_function_);
            EmitBlockParent(current_function_->Block(), *func.entry());
        }
        current_spirv_function_ = nullptr;
    }

    /// Emit entry point attributes.
    void EmitEntryPointAttributes() {
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
            auto* func = functions_.GetOr(execution_mode.GetSingleWordInOperand(0), nullptr);
            auto mode = execution_mode.GetSingleWordInOperand(1);
            TINT_ASSERT(func);

            switch (spv::ExecutionMode(mode)) {
                case spv::ExecutionMode::LocalSize:
                    func->SetWorkgroupSize(
                        b_.Constant(u32(execution_mode.GetSingleWordInOperand(2))),
                        b_.Constant(u32(execution_mode.GetSingleWordInOperand(3))),
                        b_.Constant(u32(execution_mode.GetSingleWordInOperand(4))));
                    break;
                case spv::ExecutionMode::DepthReplacing:
                case spv::ExecutionMode::OriginUpperLeft:
                    // These are ignored as they are implicitly supported by Tint IR.
                    break;
                default:
                    TINT_UNIMPLEMENTED() << "unhandled execution mode: " << mode;
            }
        }
    }

    // A block parent is a container for a scope, like a `{}`d section in code. It controls the
    // block addition to the current blocks and the ID stack entry for the block.
    void EmitBlockParent(core::ir::Block* dst, const spvtools::opt::BasicBlock& src) {
        TINT_ASSERT(current_blocks_.count(dst) == 0);

        id_stack_.emplace_back();
        current_blocks_.insert(dst);

        EmitBlock(dst, src);

        current_blocks_.erase(dst);
        id_stack_.pop_back();
    }

    /// Emit the contents of SPIR-V block @p src into Tint IR block @p dst.
    /// @param dst the Tint IR block to append to
    /// @param src the SPIR-V block to emit
    void EmitBlock(core::ir::Block* dst, const spvtools::opt::BasicBlock& src) {
        TINT_SCOPED_ASSIGNMENT(current_block_, dst);
        for (auto& inst : src) {
            switch (inst.opcode()) {
                case spv::Op::OpNop:
                    break;
                case spv::Op::OpUndef:
                    AddValue(inst.result_id(), b_.Zero(Type(inst.type_id())));
                    break;
                case spv::Op::OpBranch:
                    EmitBranch(inst);
                    break;
                case spv::Op::OpBranchConditional:
                    EmitBranchConditional(inst);
                    break;
                case spv::Op::OpSelectionMerge:
                    HandleSelectionMerge(inst, src);
                    break;
                case spv::Op::OpExtInst:
                    EmitExtInst(inst);
                    break;
                case spv::Op::OpCopyObject:
                    EmitCopyObject(inst);
                    break;
                case spv::Op::OpConvertFToS:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kConvertFToS);
                    break;
                case spv::Op::OpConvertFToU:
                    Emit(b_.Convert(Type(inst.type_id()), Value(inst.GetSingleWordOperand(2))),
                         inst.result_id());
                    break;
                case spv::Op::OpConvertSToF:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kConvertSToF);
                    break;
                case spv::Op::OpConvertUToF:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kConvertUToF);
                    break;
                case spv::Op::OpBitwiseAnd:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kBitwiseAnd);
                    break;
                case spv::Op::OpBitwiseOr:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kBitwiseOr);
                    break;
                case spv::Op::OpBitwiseXor:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kBitwiseXor);
                    break;
                case spv::Op::OpAccessChain:
                case spv::Op::OpInBoundsAccessChain:
                    EmitAccess(inst);
                    break;
                case spv::Op::OpCompositeConstruct:
                    EmitConstruct(inst);
                    break;
                case spv::Op::OpCompositeExtract:
                    EmitCompositeExtract(inst);
                    break;
                case spv::Op::OpFAdd:
                    EmitBinary(inst, core::BinaryOp::kAdd);
                    break;
                case spv::Op::OpIAdd:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kAdd);
                    break;
                case spv::Op::OpSDiv:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kSDiv);
                    break;
                case spv::Op::OpFDiv:
                case spv::Op::OpUDiv:
                    EmitBinary(inst, core::BinaryOp::kDivide);
                    break;
                case spv::Op::OpIMul:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kMul);
                    break;
                case spv::Op::OpFMul:
                case spv::Op::OpVectorTimesScalar:
                case spv::Op::OpMatrixTimesScalar:
                case spv::Op::OpVectorTimesMatrix:
                case spv::Op::OpMatrixTimesVector:
                case spv::Op::OpMatrixTimesMatrix:
                    EmitBinary(inst, core::BinaryOp::kMultiply);
                    break;
                case spv::Op::OpFRem:
                case spv::Op::OpUMod:
                    EmitBinary(inst, core::BinaryOp::kModulo);
                    break;
                case spv::Op::OpSMod:
                case spv::Op::OpSRem:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kSMod);
                    break;
                case spv::Op::OpFSub:
                    EmitBinary(inst, core::BinaryOp::kSubtract);
                    break;
                case spv::Op::OpFOrdEqual:
                    EmitBinary(inst, core::BinaryOp::kEqual);
                    break;
                case spv::Op::OpFOrdNotEqual:
                    EmitBinary(inst, core::BinaryOp::kNotEqual);
                    break;
                case spv::Op::OpFOrdGreaterThan:
                    EmitBinary(inst, core::BinaryOp::kGreaterThan);
                    break;
                case spv::Op::OpFOrdGreaterThanEqual:
                    EmitBinary(inst, core::BinaryOp::kGreaterThanEqual);
                    break;
                case spv::Op::OpFOrdLessThan:
                    EmitBinary(inst, core::BinaryOp::kLessThan);
                    break;
                case spv::Op::OpFOrdLessThanEqual:
                    EmitBinary(inst, core::BinaryOp::kLessThanEqual);
                    break;
                case spv::Op::OpFUnordEqual:
                    EmitInvertedBinary(inst, core::BinaryOp::kNotEqual);
                    break;
                case spv::Op::OpFUnordNotEqual:
                    EmitInvertedBinary(inst, core::BinaryOp::kEqual);
                    break;
                case spv::Op::OpFUnordGreaterThan:
                    EmitInvertedBinary(inst, core::BinaryOp::kLessThanEqual);
                    break;
                case spv::Op::OpFUnordGreaterThanEqual:
                    EmitInvertedBinary(inst, core::BinaryOp::kLessThan);
                    break;
                case spv::Op::OpFUnordLessThan:
                    EmitInvertedBinary(inst, core::BinaryOp::kGreaterThanEqual);
                    break;
                case spv::Op::OpFUnordLessThanEqual:
                    EmitInvertedBinary(inst, core::BinaryOp::kGreaterThan);
                    break;
                case spv::Op::OpIEqual:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kEqual);
                    break;
                case spv::Op::OpINotEqual:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kNotEqual);
                    break;
                case spv::Op::OpSGreaterThan:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kSGreaterThan);
                    break;
                case spv::Op::OpSGreaterThanEqual:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kSGreaterThanEqual);
                    break;
                case spv::Op::OpSLessThan:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kSLessThan);
                    break;
                case spv::Op::OpSLessThanEqual:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kSLessThanEqual);
                    break;
                case spv::Op::OpUGreaterThan:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kUGreaterThan);
                    break;
                case spv::Op::OpUGreaterThanEqual:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kUGreaterThanEqual);
                    break;
                case spv::Op::OpULessThan:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kULessThan);
                    break;
                case spv::Op::OpULessThanEqual:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kULessThanEqual);
                    break;
                case spv::Op::OpISub:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kSub);
                    break;
                case spv::Op::OpFunctionCall:
                    EmitFunctionCall(inst);
                    break;
                case spv::Op::OpLoad:
                    Emit(b_.Load(Value(inst.GetSingleWordOperand(2))), inst.result_id());
                    break;
                case spv::Op::OpReturn:
                    EmitWithoutResult(b_.Return(current_function_));
                    break;
                case spv::Op::OpReturnValue:
                    EmitWithoutResult(
                        b_.Return(current_function_, Value(inst.GetSingleWordOperand(0))));
                    break;
                case spv::Op::OpStore:
                    EmitWithoutResult(b_.Store(Value(inst.GetSingleWordOperand(0)),
                                               Value(inst.GetSingleWordOperand(1))));
                    break;
                case spv::Op::OpVariable:
                    EmitVar(inst);
                    break;
                case spv::Op::OpUnreachable:
                    EmitWithoutResult(b_.Unreachable());
                    break;
                case spv::Op::OpKill:
                    EmitKill(inst);
                    break;
                case spv::Op::OpDot:
                    EmitBuiltinCall(inst, core::BuiltinFn::kDot);
                    break;
                case spv::Op::OpBitCount:
                    EmitBitCount(inst);
                    break;
                case spv::Op::OpBitFieldInsert:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kBitFieldInsert);
                    break;
                case spv::Op::OpBitFieldSExtract:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kBitFieldSExtract);
                    break;
                case spv::Op::OpBitFieldUExtract:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kBitFieldUExtract);
                    break;
                case spv::Op::OpBitReverse:
                    EmitBuiltinCall(inst, core::BuiltinFn::kReverseBits);
                    break;
                case spv::Op::OpAll:
                    EmitBuiltinCall(inst, core::BuiltinFn::kAll);
                    break;
                case spv::Op::OpAny:
                    EmitBuiltinCall(inst, core::BuiltinFn::kAny);
                    break;
                case spv::Op::OpDPdx:
                    EmitBuiltinCall(inst, core::BuiltinFn::kDpdx);
                    break;
                case spv::Op::OpDPdy:
                    EmitBuiltinCall(inst, core::BuiltinFn::kDpdy);
                    break;
                case spv::Op::OpFwidth:
                    EmitBuiltinCall(inst, core::BuiltinFn::kFwidth);
                    break;
                case spv::Op::OpDPdxFine:
                    EmitBuiltinCall(inst, core::BuiltinFn::kDpdxFine);
                    break;
                case spv::Op::OpDPdyFine:
                    EmitBuiltinCall(inst, core::BuiltinFn::kDpdyFine);
                    break;
                case spv::Op::OpFwidthFine:
                    EmitBuiltinCall(inst, core::BuiltinFn::kFwidthFine);
                    break;
                case spv::Op::OpDPdxCoarse:
                    EmitBuiltinCall(inst, core::BuiltinFn::kDpdxCoarse);
                    break;
                case spv::Op::OpDPdyCoarse:
                    EmitBuiltinCall(inst, core::BuiltinFn::kDpdyCoarse);
                    break;
                case spv::Op::OpFwidthCoarse:
                    EmitBuiltinCall(inst, core::BuiltinFn::kFwidthCoarse);
                    break;
                case spv::Op::OpLogicalAnd:
                    EmitBinary(inst, core::BinaryOp::kAnd);
                    break;
                case spv::Op::OpLogicalOr:
                    EmitBinary(inst, core::BinaryOp::kOr);
                    break;
                case spv::Op::OpLogicalEqual:
                    EmitBinary(inst, core::BinaryOp::kEqual);
                    break;
                case spv::Op::OpLogicalNotEqual:
                    EmitBinary(inst, core::BinaryOp::kNotEqual);
                    break;
                case spv::Op::OpLogicalNot:
                    EmitUnary(inst, core::UnaryOp::kNot);
                    break;
                case spv::Op::OpFNegate:
                    EmitUnary(inst, core::UnaryOp::kNegation);
                    break;
                case spv::Op::OpNot:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kNot);
                    break;
                case spv::Op::OpShiftLeftLogical:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kShiftLeftLogical);
                    break;
                case spv::Op::OpShiftRightLogical:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kShiftRightLogical);
                    break;
                case spv::Op::OpShiftRightArithmetic:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kShiftRightArithmetic);
                    break;
                case spv::Op::OpBitcast:
                    EmitBitcast(inst);
                    break;
                case spv::Op::OpQuantizeToF16:
                    EmitBuiltinCall(inst, core::BuiltinFn::kQuantizeToF16);
                    break;
                case spv::Op::OpTranspose:
                    EmitBuiltinCall(inst, core::BuiltinFn::kTranspose);
                    break;
                case spv::Op::OpSNegate:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kSNegate);
                    break;
                default:
                    TINT_UNIMPLEMENTED()
                        << "unhandled SPIR-V instruction: " << static_cast<uint32_t>(inst.opcode());
            }
        }
    }

    void EmitBitcast(const spvtools::opt::Instruction& inst) {
        auto val = Value(inst.GetSingleWordInOperand(0));
        auto ty = Type(inst.type_id());
        Emit(b_.Bitcast(ty, val), inst.result_id());
    }

    void HandleSelectionMerge(const spvtools::opt::Instruction& inst,
                              const spvtools::opt::BasicBlock& src) {
        merge_stack_.push_back(MergeInfo{inst.GetSingleWordOperand(0), src.terminator()});
    }

    void EmitBranch(const spvtools::opt::Instruction& inst) {
        auto dest_id = inst.GetSingleWordInOperand(0);

        // If this is branching to the current merge block then nothing to do.
        if (!merge_stack_.empty() && dest_id == merge_stack_.back().id) {
            return;
        }

        TINT_ASSERT(current_spirv_function_);
        const auto& bb = current_spirv_function_->FindBlock(dest_id);

        EmitBlock(current_block_, *bb);
    }

    // Given a true and false branch find if there is a common convergence point before the merge
    // block.
    std::optional<uint32_t> FindPremergeId(uint32_t true_id,
                                           uint32_t false_id,
                                           std::optional<uint32_t> merge_id) {
        auto* cfg = spirv_context_->cfg();

        // We need a merge block, the true and false to be unique and the true and false to not be
        // the merge.
        if (!merge_id || true_id == false_id || true_id == merge_id || false_id == merge_id) {
            return std::nullopt;
        }

        // Get the list of blocks from the true branch to the merge
        std::list<spvtools::opt::BasicBlock*> true_blocks;
        cfg->ComputeStructuredOrder(
            current_spirv_function_, &*(current_spirv_function_->FindBlock(true_id)),
            &*(current_spirv_function_->FindBlock(merge_id.value())), &true_blocks);

        // Get the list of blocks from the false branch to the merge
        std::list<spvtools::opt::BasicBlock*> false_blocks;
        cfg->ComputeStructuredOrder(
            current_spirv_function_, &*(current_spirv_function_->FindBlock(false_id)),
            &*(current_spirv_function_->FindBlock(merge_id.value())), &false_blocks);

        // It's possible one of the blocks returns, so bail early if they don't both end at the
        // merge.
        if (true_blocks.back()->id() != merge_id || false_blocks.back()->id() != merge_id) {
            return std::nullopt;
        }

        std::optional<uint32_t> id = std::nullopt;
        while (!true_blocks.empty() && !false_blocks.empty()) {
            auto* tb = true_blocks.back();
            if (tb != false_blocks.back()) {
                break;
            }

            id = tb->id();

            true_blocks.pop_back();
            false_blocks.pop_back();
        }
        return id;
    }

    void EmitBranchConditional(const spvtools::opt::Instruction& inst) {
        auto cond = Value(inst.GetSingleWordInOperand(0));
        auto true_id = inst.GetSingleWordInOperand(1);
        auto false_id = inst.GetSingleWordInOperand(2);

        std::optional<uint32_t> merge_id = std::nullopt;
        if (!merge_stack_.empty()) {
            merge_id = merge_stack_.back().id;
        }

        TINT_ASSERT(current_spirv_function_);

        // If the true and false block are the same, then we change the condition into
        // `cond || true` so that we always take the true block, the false block will be marked
        // unreachable.
        if (true_id == false_id) {
            auto* binary = b_.Binary(core::BinaryOp::kOr, cond->Type(), cond, b_.Constant(true));
            EmitWithoutSpvResult(binary);

            cond = binary->Result(0);
        }

        // Determine if there is a premerge block to handle
        std::optional<uint32_t> premerge_start_id = FindPremergeId(true_id, false_id, merge_id);

        // If we found the start of a premerge, push it onto the merge stack so this ends up being a
        // temporary merge block for the if branches.
        if (premerge_start_id.has_value()) {
            merge_stack_.push_back({premerge_start_id.value(), nullptr});
        }

        auto* if_ = b_.If(cond);
        EmitWithoutResult(if_);

        if (true_id != merge_id) {
            const auto& bb_true = current_spirv_function_->FindBlock(true_id);
            EmitBlockParent(if_->True(), *bb_true);
        }
        if (!if_->True()->Terminator()) {
            if_->True()->Append(b_.ExitIf(if_));
        }

        // Pre-SPIRV 1.6 the true and false blocks could be the same. If that's the case then we
        // will have changed the condition and the false block is now unreachable.
        if (false_id == true_id) {
            if_->False()->Append(b_.Unreachable());
        } else if (false_id != merge_id) {
            const auto& bb_false = current_spirv_function_->FindBlock(false_id);
            EmitBlockParent(if_->False(), *bb_false);
            if (!if_->False()->Terminator()) {
                if_->False()->Append(b_.ExitIf(if_));
            }
        }

        // There was a premerge, remove it from the merge stack and then emit the premerge into an
        // `if true` block in order to maintain re-convergence guarantees. The premerge will contain
        // all the blocks up to the merge block.
        if (premerge_start_id.has_value()) {
            merge_stack_.pop_back();

            auto* premerge_if_ = b_.If(b_.Constant(true));
            EmitWithoutResult(premerge_if_);

            const auto& bb_premerge = current_spirv_function_->FindBlock(premerge_start_id.value());
            EmitBlockParent(premerge_if_->True(), *bb_premerge);
            if (!premerge_if_->True()->Terminator()) {
                premerge_if_->True()->Append(b_.ExitIf(premerge_if_));
            }

            premerge_if_->False()->Append(b_.Unreachable());
        }

        // Emit the merge block if it exists.
        if (merge_id.has_value()) {
            if (&inst == merge_stack_.back().merge_inst) {
                merge_stack_.pop_back();
                const auto& bb_merge = current_spirv_function_->FindBlock(merge_id.value());
                EmitBlock(current_block_, *bb_merge);
            }
        } else {
            EmitWithoutResult(b_.Unreachable());
        }
    }

    Vector<core::ir::Value*, 4> Args(const spvtools::opt::Instruction& inst, uint32_t start) {
        Vector<core::ir::Value*, 4> args;
        for (uint32_t i = start; i < inst.NumOperandWords(); i++) {
            args.Push(Value(inst.GetSingleWordOperand(i)));
        }
        return args;
    }

    void EmitBuiltinCall(const spvtools::opt::Instruction& inst, core::BuiltinFn fn) {
        Emit(b_.Call(Type(inst.type_id()), fn, Args(inst, 2)), inst.result_id());
    }

    void EmitSpirvExplicitBuiltinCall(const spvtools::opt::Instruction& inst, spirv::BuiltinFn fn) {
        Emit(b_.CallExplicit<spirv::ir::BuiltinCall>(Type(inst.type_id()), fn,
                                                     Vector{Type(inst.type_id())->DeepestElement()},
                                                     Args(inst, 2)),
             inst.result_id());
    }

    void EmitSpirvBuiltinCall(const spvtools::opt::Instruction& inst, spirv::BuiltinFn fn) {
        Emit(b_.Call<spirv::ir::BuiltinCall>(Type(inst.type_id()), fn, Args(inst, 2)),
             inst.result_id());
    }

    void EmitBitCount(const spvtools::opt::Instruction& inst) {
        auto* res_ty = Type(inst.type_id());
        Emit(b_.CallExplicit<spirv::ir::BuiltinCall>(res_ty, spirv::BuiltinFn::kBitCount,
                                                     Vector{res_ty->DeepestElement()},
                                                     Args(inst, 2)),
             inst.result_id());
    }

    /// @param inst the SPIR-V instruction
    /// Note: This isn't technically correct, but there is no `kill` equivalent in WGSL. The closets
    /// we have is `discard` which maps to `OpDemoteToHelperInvocation` in SPIR-V.
    void EmitKill([[maybe_unused]] const spvtools::opt::Instruction& inst) {
        EmitWithoutResult(b_.Discard());

        // An `OpKill` is a terminator in SPIR-V. `discard` is not a terminator in WGSL. After the
        // `discard` we inject a `return` for the current function. This is similar in spirit to
        // what `OpKill` does although not totally correct (i.e. we don't early return from calling
        // functions, just the function where `OpKill` was emitted. There are also limited places in
        // which `OpKill` can be used. So, we don't have to worry about it in a `continuing` block
        // because the continuing must end with a branching terminator which `OpKill` does not
        // branch.
        if (current_function_->ReturnType()->Is<core::type::Void>()) {
            EmitWithoutResult(b_.Return(current_function_));
        } else {
            EmitWithoutResult(
                b_.Return(current_function_, b_.Zero(current_function_->ReturnType())));
        }
    }

    /// @param inst the SPIR-V instruction for OpCopyObject
    void EmitCopyObject(const spvtools::opt::Instruction& inst) {
        // Make the result Id a pointer to the original copied value.
        auto* l = b_.Let(Value(inst.GetSingleWordOperand(2)));
        Emit(l, inst.result_id());
    }

    /// @param inst the SPIR-V instruction for OpExtInst
    void EmitExtInst(const spvtools::opt::Instruction& inst) {
        auto inst_set = inst.GetSingleWordInOperand(0);
        if (ignored_imports_.count(inst_set) > 0) {
            // Ignore it but don't error out.
            return;
        }
        if (glsl_std_450_imports_.count(inst_set) > 0) {
            EmitGlslStd450ExtInst(inst);
            return;
        }

        TINT_UNIMPLEMENTED() << "unhandled extended instruction import with ID "
                             << inst.GetSingleWordInOperand(0);
    }

    // Returns the WGSL standard library function for the given GLSL.std.450 extended instruction
    // operation code. This handles GLSL functions which directly translate to the WGSL equivalent.
    // Any non-direct translation is returned as `kNone`.
    core::BuiltinFn GetGlslStd450WgslEquivalentFuncName(uint32_t ext_opcode) {
        switch (ext_opcode) {
            case GLSLstd450Acos:
                return core::BuiltinFn::kAcos;
            case GLSLstd450Acosh:
                return core::BuiltinFn::kAcosh;
            case GLSLstd450Asin:
                return core::BuiltinFn::kAsin;
            case GLSLstd450Asinh:
                return core::BuiltinFn::kAsinh;
            case GLSLstd450Atan:
                return core::BuiltinFn::kAtan;
            case GLSLstd450Atanh:
                return core::BuiltinFn::kAtanh;
            case GLSLstd450Atan2:
                return core::BuiltinFn::kAtan2;
            case GLSLstd450Ceil:
                return core::BuiltinFn::kCeil;
            case GLSLstd450Cos:
                return core::BuiltinFn::kCos;
            case GLSLstd450Cosh:
                return core::BuiltinFn::kCosh;
            case GLSLstd450Cross:
                return core::BuiltinFn::kCross;
            case GLSLstd450Degrees:
                return core::BuiltinFn::kDegrees;
            case GLSLstd450Determinant:
                return core::BuiltinFn::kDeterminant;
            case GLSLstd450Distance:
                return core::BuiltinFn::kDistance;
            case GLSLstd450Exp:
                return core::BuiltinFn::kExp;
            case GLSLstd450Exp2:
                return core::BuiltinFn::kExp2;
            case GLSLstd450FAbs:
                return core::BuiltinFn::kAbs;
            case GLSLstd450FSign:
                return core::BuiltinFn::kSign;
            case GLSLstd450Floor:
                return core::BuiltinFn::kFloor;
            case GLSLstd450Fract:
                return core::BuiltinFn::kFract;
            case GLSLstd450Fma:
                return core::BuiltinFn::kFma;
            case GLSLstd450InverseSqrt:
                return core::BuiltinFn::kInverseSqrt;
            case GLSLstd450Length:
                return core::BuiltinFn::kLength;
            case GLSLstd450Log:
                return core::BuiltinFn::kLog;
            case GLSLstd450Log2:
                return core::BuiltinFn::kLog2;
            case GLSLstd450NClamp:
            case GLSLstd450FClamp:  // FClamp is less prescriptive about NaN operands
                return core::BuiltinFn::kClamp;
            case GLSLstd450ModfStruct:
                return core::BuiltinFn::kModf;
            case GLSLstd450FrexpStruct:
                return core::BuiltinFn::kFrexp;
            case GLSLstd450NMin:
            case GLSLstd450FMin:  // FMin is less prescriptive about NaN operands
                return core::BuiltinFn::kMin;
            case GLSLstd450NMax:
            case GLSLstd450FMax:  // FMax is less prescriptive about NaN operands
                return core::BuiltinFn::kMax;
            case GLSLstd450FMix:
                return core::BuiltinFn::kMix;
            case GLSLstd450PackSnorm4x8:
                return core::BuiltinFn::kPack4X8Snorm;
            case GLSLstd450PackUnorm4x8:
                return core::BuiltinFn::kPack4X8Unorm;
            case GLSLstd450PackSnorm2x16:
                return core::BuiltinFn::kPack2X16Snorm;
            case GLSLstd450PackUnorm2x16:
                return core::BuiltinFn::kPack2X16Unorm;
            case GLSLstd450PackHalf2x16:
                return core::BuiltinFn::kPack2X16Float;
            case GLSLstd450Pow:
                return core::BuiltinFn::kPow;
            case GLSLstd450Radians:
                return core::BuiltinFn::kRadians;
            case GLSLstd450Round:
            case GLSLstd450RoundEven:
                return core::BuiltinFn::kRound;
            case GLSLstd450Sin:
                return core::BuiltinFn::kSin;
            case GLSLstd450Sinh:
                return core::BuiltinFn::kSinh;
            case GLSLstd450SmoothStep:
                return core::BuiltinFn::kSmoothstep;
            case GLSLstd450Sqrt:
                return core::BuiltinFn::kSqrt;
            case GLSLstd450Step:
                return core::BuiltinFn::kStep;
            case GLSLstd450Tan:
                return core::BuiltinFn::kTan;
            case GLSLstd450Tanh:
                return core::BuiltinFn::kTanh;
            case GLSLstd450Trunc:
                return core::BuiltinFn::kTrunc;
            case GLSLstd450UnpackSnorm4x8:
                return core::BuiltinFn::kUnpack4X8Snorm;
            case GLSLstd450UnpackUnorm4x8:
                return core::BuiltinFn::kUnpack4X8Unorm;
            case GLSLstd450UnpackSnorm2x16:
                return core::BuiltinFn::kUnpack2X16Snorm;
            case GLSLstd450UnpackUnorm2x16:
                return core::BuiltinFn::kUnpack2X16Unorm;
            case GLSLstd450UnpackHalf2x16:
                return core::BuiltinFn::kUnpack2X16Float;

            default:
                break;
        }
        return core::BuiltinFn::kNone;
    }

    spirv::BuiltinFn GetGlslStd450SpirvEquivalentFuncName(uint32_t ext_opcode) {
        switch (ext_opcode) {
            case GLSLstd450SAbs:
                return spirv::BuiltinFn::kAbs;
            case GLSLstd450SSign:
                return spirv::BuiltinFn::kSign;
            case GLSLstd450Normalize:
                return spirv::BuiltinFn::kNormalize;
            case GLSLstd450MatrixInverse:
                return spirv::BuiltinFn::kInverse;
            case GLSLstd450SMax:
                return spirv::BuiltinFn::kSMax;
            case GLSLstd450SMin:
                return spirv::BuiltinFn::kSMin;
            case GLSLstd450SClamp:
                return spirv::BuiltinFn::kSClamp;
            case GLSLstd450UMax:
                return spirv::BuiltinFn::kUMax;
            case GLSLstd450UMin:
                return spirv::BuiltinFn::kUMin;
            case GLSLstd450UClamp:
                return spirv::BuiltinFn::kUClamp;
            case GLSLstd450FindILsb:
                return spirv::BuiltinFn::kFindILsb;
            case GLSLstd450FindSMsb:
                return spirv::BuiltinFn::kFindSMsb;
            case GLSLstd450FindUMsb:
                return spirv::BuiltinFn::kFindUMsb;
            case GLSLstd450Refract:
                return spirv::BuiltinFn::kRefract;
            case GLSLstd450Reflect:
                return spirv::BuiltinFn::kReflect;
            case GLSLstd450FaceForward:
                return spirv::BuiltinFn::kFaceForward;
            case GLSLstd450Ldexp:
                return spirv::BuiltinFn::kLdexp;
            case GLSLstd450Modf:
                return spirv::BuiltinFn::kModf;
            case GLSLstd450Frexp:
                return spirv::BuiltinFn::kFrexp;
            default:
                break;
        }
        return spirv::BuiltinFn::kNone;
    }

    Vector<const core::type::Type*, 1> GlslStd450ExplicitParams(uint32_t ext_opcode,
                                                                const core::type::Type* result_ty) {
        if (ext_opcode == GLSLstd450SSign || ext_opcode == GLSLstd450SAbs ||
            ext_opcode == GLSLstd450SMax || ext_opcode == GLSLstd450SMin ||
            ext_opcode == GLSLstd450SClamp || ext_opcode == GLSLstd450UMax ||
            ext_opcode == GLSLstd450UMin || ext_opcode == GLSLstd450UClamp ||
            ext_opcode == GLSLstd450FindILsb || ext_opcode == GLSLstd450FindSMsb ||
            ext_opcode == GLSLstd450FindUMsb) {
            return {result_ty->DeepestElement()};
        }
        return {};
    }

    /// @param inst the SPIR-V instruction for OpAccessChain
    void EmitGlslStd450ExtInst(const spvtools::opt::Instruction& inst) {
        const auto ext_opcode = inst.GetSingleWordInOperand(1);
        auto* spv_ty = Type(inst.type_id());

        Vector<core::ir::Value*, 4> operands;
        // All parameters to GLSL.std.450 extended instructions are IDs.
        for (uint32_t idx = 2; idx < inst.NumInOperands(); ++idx) {
            operands.Push(Value(inst.GetSingleWordInOperand(idx)));
        }

        const auto wgsl_fn = GetGlslStd450WgslEquivalentFuncName(ext_opcode);
        if (wgsl_fn == core::BuiltinFn::kModf) {
            // For `ModfStruct`, which is, essentially, a WGSL `modf` instruction
            // we need some special handling. The result type that we produce
            // must be the SPIR-V type as we don't know how the result is used
            // later. So, we need to make the WGSL query and re-construct an
            // object of the right SPIR-V type. We can't, easily, do this later
            // as we lose the SPIR-V type as soon as we replace the result of the
            // `modf`. So, inline the work here to generate the correct results.

            auto* mem_ty = operands[0]->Type();
            auto* result_ty = core::type::CreateModfResult(ty_, ir_.symbols, mem_ty);

            auto* call = b_.Call(result_ty, wgsl_fn, operands);
            auto* fract = b_.Access(mem_ty, call, 0_u);
            auto* whole = b_.Access(mem_ty, call, 1_u);

            EmitWithoutSpvResult(call);
            EmitWithoutSpvResult(fract);
            EmitWithoutSpvResult(whole);
            Emit(b_.Construct(spv_ty, fract, whole), inst.result_id());
            return;
        }
        if (wgsl_fn == core::BuiltinFn::kFrexp) {
            // For `FrexpStruct`, which is, essentially, a WGSL `frexp`
            // instruction we need some special handling. The result type that we
            // produce must be the SPIR-V type as we don't know how the result is
            // used later. So, we need to make the WGSL query and re-construct an
            // object of the right SPIR-V type. We can't, easily, do this later
            // as we lose the SPIR-V type as soon as we replace the result of the
            // `frexp`. So, inline the work here to generate the correct results.

            auto* mem_ty = operands[0]->Type();
            auto* result_ty = core::type::CreateFrexpResult(ty_, ir_.symbols, mem_ty);

            auto* call = b_.Call(result_ty, wgsl_fn, operands);
            auto* fract = b_.Access(mem_ty, call, 0_u);
            auto* exp = b_.Access(ty_.MatchWidth(ty_.i32(), mem_ty), call, 1_u);
            auto* exp_res = exp->Result(0);

            EmitWithoutSpvResult(call);
            EmitWithoutSpvResult(fract);
            EmitWithoutSpvResult(exp);

            if (auto* str = spv_ty->As<core::type::Struct>()) {
                auto* exp_ty = str->Members()[1]->Type();
                if (exp_ty->DeepestElement()->IsUnsignedIntegerScalar()) {
                    auto* uexp = b_.Bitcast(exp_ty, exp);
                    exp_res = uexp->Result(0);
                    EmitWithoutSpvResult(uexp);
                }
            }

            Emit(b_.Construct(spv_ty, fract, exp_res), inst.result_id());
            return;
        }
        if (wgsl_fn != core::BuiltinFn::kNone) {
            Emit(b_.Call(spv_ty, wgsl_fn, operands), inst.result_id());
            return;
        }

        const auto spv_fn = GetGlslStd450SpirvEquivalentFuncName(ext_opcode);
        if (spv_fn != spirv::BuiltinFn::kNone) {
            auto explicit_params = GlslStd450ExplicitParams(ext_opcode, spv_ty);
            Emit(b_.CallExplicit<spirv::ir::BuiltinCall>(spv_ty, spv_fn, explicit_params, operands),
                 inst.result_id());
            return;
        }

        TINT_UNIMPLEMENTED() << "unhandled GLSL.std.450 instruction " << ext_opcode;
    }

    /// @param inst the SPIR-V instruction for OpAccessChain
    void EmitAccess(const spvtools::opt::Instruction& inst) {
        Vector indices = Args(inst, 3);
        auto* base = Value(inst.GetSingleWordOperand(2));

        if (indices.IsEmpty()) {
            // There are no indices, so just forward the base object.
            AddValue(inst.result_id(), base);
            return;
        }

        // Propagate the access mode of the base object.
        auto access_mode = core::Access::kUndefined;
        if (auto* ptr = base->Type()->As<core::type::Pointer>()) {
            access_mode = ptr->Access();
        }

        auto* access = b_.Access(Type(inst.type_id(), access_mode), base, std::move(indices));
        Emit(access, inst.result_id());
    }

    /// @param inst the SPIR-V instruction
    /// @param op the unary operator to use
    void EmitUnary(const spvtools::opt::Instruction& inst, core::UnaryOp op) {
        auto* val = Value(inst.GetSingleWordOperand(2));
        auto* unary = b_.Unary(op, Type(inst.type_id()), val);
        Emit(unary, inst.result_id());
    }

    /// @param inst the SPIR-V instruction
    /// @param op the binary operator to use
    void EmitBinary(const spvtools::opt::Instruction& inst, core::BinaryOp op) {
        auto* lhs = Value(inst.GetSingleWordOperand(2));
        auto* rhs = Value(inst.GetSingleWordOperand(3));
        auto* binary = b_.Binary(op, Type(inst.type_id()), lhs, rhs);
        Emit(binary, inst.result_id());
    }

    /// @param inst the SPIR-V instruction
    /// @param op the binary operator to use
    void EmitInvertedBinary(const spvtools::opt::Instruction& inst, core::BinaryOp op) {
        auto* lhs = Value(inst.GetSingleWordOperand(2));
        auto* rhs = Value(inst.GetSingleWordOperand(3));
        auto* binary = b_.Binary(op, Type(inst.type_id()), lhs, rhs);
        EmitWithoutSpvResult(binary);

        auto* res = b_.Not(Type(inst.type_id()), binary);
        Emit(res, inst.result_id());
    }

    /// @param inst the SPIR-V instruction for OpCompositeExtract
    void EmitCompositeExtract(const spvtools::opt::Instruction& inst) {
        Vector<core::ir::Value*, 4> indices;
        for (uint32_t i = 3; i < inst.NumOperandWords(); i++) {
            indices.Push(b_.Constant(u32(inst.GetSingleWordOperand(i))));
        }
        auto* object = Value(inst.GetSingleWordOperand(2));
        auto* access = b_.Access(Type(inst.type_id()), object, std::move(indices));
        Emit(access, inst.result_id());
    }

    /// @param inst the SPIR-V instruction for OpCompositeConstruct
    void EmitConstruct(const spvtools::opt::Instruction& inst) {
        auto* construct = b_.Construct(Type(inst.type_id()), Args(inst, 2));
        Emit(construct, inst.result_id());
    }

    /// @param inst the SPIR-V instruction for OpFunctionCall
    void EmitFunctionCall(const spvtools::opt::Instruction& inst) {
        Emit(b_.Call(Function(inst.GetSingleWordInOperand(0)), Args(inst, 3)), inst.result_id());
    }

    /// @param inst the SPIR-V instruction for OpVariable
    void EmitVar(const spvtools::opt::Instruction& inst) {
        // Handle decorations.
        std::optional<uint32_t> group;
        std::optional<uint32_t> binding;
        core::Access access_mode = core::Access::kUndefined;
        core::IOAttributes io_attributes;
        auto interpolation = [&]() -> core::Interpolation& {
            // Create the interpolation field with the default values on first call.
            if (!io_attributes.interpolation.has_value()) {
                io_attributes.interpolation = core::Interpolation{
                    core::InterpolationType::kPerspective, core::InterpolationSampling::kCenter};
            }
            return io_attributes.interpolation.value();
        };
        for (auto* deco :
             spirv_context_->get_decoration_mgr()->GetDecorationsFor(inst.result_id(), false)) {
            auto d = deco->GetSingleWordOperand(1);
            switch (spv::Decoration(d)) {
                case spv::Decoration::NonWritable:
                    access_mode = core::Access::kRead;
                    break;
                case spv::Decoration::DescriptorSet:
                    group = deco->GetSingleWordOperand(2);
                    break;
                case spv::Decoration::Binding:
                    binding = deco->GetSingleWordOperand(2);
                    break;
                case spv::Decoration::BuiltIn:
                    io_attributes.builtin = Builtin(spv::BuiltIn(deco->GetSingleWordOperand(2)));
                    break;
                case spv::Decoration::Invariant:
                    io_attributes.invariant = true;
                    break;
                case spv::Decoration::Location:
                    io_attributes.location = deco->GetSingleWordOperand(2);
                    break;
                case spv::Decoration::NoPerspective:
                    interpolation().type = core::InterpolationType::kLinear;
                    break;
                case spv::Decoration::Flat:
                    interpolation().type = core::InterpolationType::kFlat;
                    break;
                case spv::Decoration::Centroid:
                    interpolation().sampling = core::InterpolationSampling::kCentroid;
                    break;
                case spv::Decoration::Sample:
                    interpolation().sampling = core::InterpolationSampling::kSample;
                    break;
                case spv::Decoration::Index:
                    io_attributes.blend_src = deco->GetSingleWordOperand(2);
                    break;
                default:
                    TINT_UNIMPLEMENTED() << "unhandled decoration " << d;
            }
        }

        auto* var = b_.Var(Type(inst.type_id(), access_mode)->As<core::type::Pointer>());
        if (inst.NumOperands() > 3) {
            var->SetInitializer(Value(inst.GetSingleWordOperand(3)));
        }

        if (group || binding) {
            TINT_ASSERT(group && binding);
            var->SetBindingPoint(group.value(), binding.value());
        }
        var->SetAttributes(std::move(io_attributes));

        Emit(var, inst.result_id());
    }

  private:
    /// TypeKey describes a SPIR-V type with an access mode.
    struct TypeKey {
        /// The SPIR-V type object.
        const spvtools::opt::analysis::Type* type;
        /// The access mode.
        core::Access access_mode;

        // Equality operator for TypeKey.
        bool operator==(const TypeKey& other) const {
            return type == other.type && access_mode == other.access_mode;
        }

        /// @returns the hash code of the TypeKey
        tint::HashCode HashCode() const { return Hash(type, access_mode); }
    };

    /// The generated IR module.
    core::ir::Module ir_;
    /// The Tint IR builder.
    core::ir::Builder b_{ir_};
    /// The Tint type manager.
    core::type::Manager& ty_{ir_.Types()};

    /// The Tint IR function that is currently being emitted.
    core::ir::Function* current_function_ = nullptr;
    /// The Tint IR block that is currently being emitted.
    core::ir::Block* current_block_ = nullptr;
    /// A map from a SPIR-V type declaration to the corresponding Tint type object.
    Hashmap<TypeKey, const core::type::Type*, 16> types_;
    /// A map from a SPIR-V function definition result ID to the corresponding Tint function object.
    Hashmap<uint32_t, core::ir::Function*, 8> functions_;
    /// A map from a SPIR-V result ID to the corresponding Tint value object.
    Hashmap<uint32_t, core::ir::Value*, 8> values_;

    /// The SPIR-V context containing the SPIR-V tools intermediate representation.
    std::unique_ptr<spvtools::opt::IRContext> spirv_context_;
    /// The current SPIR-V function being emitted
    spvtools::opt::Function* current_spirv_function_ = nullptr;

    // The set of IDs that are imports of the GLSL.std.450 extended instruction sets.
    std::unordered_set<uint32_t> glsl_std_450_imports_;
    // The set of IDs of imports that are ignored. For example, any "NonSemanticInfo." import is
    // ignored.
    std::unordered_set<uint32_t> ignored_imports_;

    // Map of SPIR-V IDs to string names
    std::unordered_map<uint32_t, std::string> id_to_name_;
    // Map of SPIR-V Struct IDs to a list of member string names
    std::unordered_map<uint32_t, std::vector<std::string>> struct_to_member_names_;

    struct MergeInfo {
        uint32_t id;
        const spvtools::opt::Instruction* merge_inst;
    };

    // Stack of merge blocks
    std::vector<MergeInfo> merge_stack_;

    std::unordered_set<core::ir::Block*> current_blocks_;
    std::vector<std::unordered_set<uint32_t>> id_stack_;
};

}  // namespace

Result<core::ir::Module> Parse(Slice<const uint32_t> spirv) {
    return Parser{}.Run(spirv);
}

}  // namespace tint::spirv::reader
