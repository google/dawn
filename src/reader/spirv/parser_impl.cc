// Copyright 2020 The Tint Authors.
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

#include "src/reader/spirv/parser_impl.h"

#include <cassert>
#include <cstring>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "source/opt/basic_block.h"
#include "source/opt/build_module.h"
#include "source/opt/constants.h"
#include "source/opt/decoration_manager.h"
#include "source/opt/function.h"
#include "source/opt/instruction.h"
#include "source/opt/module.h"
#include "source/opt/type_manager.h"
#include "source/opt/types.h"
#include "spirv-tools/libspirv.hpp"
#include "src/ast/as_expression.h"
#include "src/ast/binary_expression.h"
#include "src/ast/binding_decoration.h"
#include "src/ast/bool_literal.h"
#include "src/ast/builtin.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/float_literal.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/set_decoration.h"
#include "src/ast/sint_literal.h"
#include "src/ast/struct.h"
#include "src/ast/struct_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/variable_decoration.h"
#include "src/reader/spirv/enum_converter.h"
#include "src/reader/spirv/function.h"
#include "src/type_manager.h"

namespace tint {
namespace reader {
namespace spirv {

namespace {

// Input SPIR-V needs only to conform to Vulkan 1.1 requirements.
// The combination of the SPIR-V reader and the semantics of WGSL
// tighten up the code so that the output of the SPIR-V *writer*
// will satisfy SPV_ENV_WEBGPU_0 validation.
const spv_target_env kInputEnv = SPV_ENV_VULKAN_1_1;

// A FunctionTraverser is used to compute an ordering of functions in the
// module such that callees precede callers.
class FunctionTraverser {
 public:
  explicit FunctionTraverser(const spvtools::opt::Module& module)
      : module_(module) {}

  // @returns the functions in the modules such that callees precede callers.
  std::vector<const spvtools::opt::Function*> TopologicallyOrderedFunctions() {
    visited_.clear();
    ordered_.clear();
    id_to_func_.clear();
    for (const auto& f : module_) {
      id_to_func_[f.result_id()] = &f;
    }
    for (const auto& f : module_) {
      Visit(f);
    }
    return ordered_;
  }

 private:
  void Visit(const spvtools::opt::Function& f) {
    if (visited_.count(&f)) {
      return;
    }
    visited_.insert(&f);
    for (const auto& bb : f) {
      for (const auto& inst : bb) {
        if (inst.opcode() != SpvOpFunctionCall) {
          continue;
        }
        const auto* callee = id_to_func_[inst.GetSingleWordInOperand(0)];
        if (callee) {
          Visit(*callee);
        }
      }
    }
    ordered_.push_back(&f);
  }

  const spvtools::opt::Module& module_;
  std::unordered_set<const spvtools::opt::Function*> visited_;
  std::unordered_map<uint32_t, const spvtools::opt::Function*> id_to_func_;
  std::vector<const spvtools::opt::Function*> ordered_;
};

// Returns true if the opcode operates as if its operands are signed integral.
bool AssumesSignedOperands(SpvOp opcode) {
  switch (opcode) {
    case SpvOpSNegate:
    case SpvOpSDiv:
    case SpvOpSRem:
    case SpvOpSMod:
    case SpvOpSLessThan:
    case SpvOpSLessThanEqual:
    case SpvOpSGreaterThan:
    case SpvOpSGreaterThanEqual:
    case SpvOpConvertSToF:
      return true;
    default:
      break;
  }
  return false;
}

// Returns true if the opcode operates as if its operands are unsigned integral.
bool AssumesUnsignedOperands(SpvOp opcode) {
  switch (opcode) {
    case SpvOpUDiv:
    case SpvOpUMod:
    case SpvOpULessThan:
    case SpvOpULessThanEqual:
    case SpvOpUGreaterThan:
    case SpvOpUGreaterThanEqual:
    case SpvOpConvertUToF:
      return true;
    default:
      break;
  }
  return false;
}

// Returns true if the operation is binary, and the WGSL operation requires
// the signedness of the result to match the signedness of the first operand.
bool AssumesResultSignednessMatchesBinaryFirstOperand(SpvOp opcode) {
  switch (opcode) {
    case SpvOpSDiv:
    case SpvOpSMod:
    case SpvOpSRem:
      return true;
    default:
      break;
  }
  return false;
}

}  // namespace

TypedExpression::TypedExpression() : type(nullptr), expr(nullptr) {}

TypedExpression::TypedExpression(ast::type::Type* t,
                                 std::unique_ptr<ast::Expression> e)
    : type(t), expr(std::move(e)) {}

TypedExpression::TypedExpression(TypedExpression&& other)
    : type(other.type), expr(std::move(other.expr)) {}

TypedExpression::~TypedExpression() {}

void TypedExpression::reset(TypedExpression&& other) {
  type = other.type;
  expr = std::move(other.expr);
}

ParserImpl::ParserImpl(Context* ctx, const std::vector<uint32_t>& spv_binary)
    : Reader(ctx),
      spv_binary_(spv_binary),
      fail_stream_(&success_, &errors_),
      bool_type_(ctx->type_mgr().Get(std::make_unique<ast::type::BoolType>())),
      namer_(fail_stream_),
      enum_converter_(fail_stream_),
      tools_context_(kInputEnv) {
  // Create a message consumer to propagate error messages from SPIRV-Tools
  // out as our own failures.
  message_consumer_ = [this](spv_message_level_t level, const char* /*source*/,
                             const spv_position_t& position,
                             const char* message) {
    switch (level) {
      // Ignore info and warning message.
      case SPV_MSG_WARNING:
      case SPV_MSG_INFO:
        break;
      // Otherwise, propagate the error.
      default:
        // For binary validation errors, we only have the instruction
        // number.  It's not text, so there is no column number.
        this->Fail() << "line:" << position.index << ": " << message;
    }
  };
}

ParserImpl::~ParserImpl() = default;

bool ParserImpl::Parse() {
  // Set up use of SPIRV-Tools utilities.
  spvtools::SpirvTools spv_tools(kInputEnv);

  // Error messages from SPIRV-Tools are forwarded as failures, including
  // setting |success_| to false.
  spv_tools.SetMessageConsumer(message_consumer_);

  if (!success_) {
    return false;
  }

  // Only consider modules valid for Vulkan 1.0.  On failure, the message
  // consumer will set the error status.
  if (!spv_tools.Validate(spv_binary_)) {
    return false;
  }
  if (!BuildInternalModule()) {
    return false;
  }
  if (!ParseInternalModule()) {
    return false;
  }

  return success_;
}

ast::Module ParserImpl::module() {
  // TODO(dneto): Should we clear out spv_binary_ here, to reduce
  // memory usage?
  return std::move(ast_module_);
}

ast::type::Type* ParserImpl::ConvertType(uint32_t type_id) {
  if (!success_) {
    return nullptr;
  }

  if (type_mgr_ == nullptr) {
    Fail() << "ConvertType called when the internal module has not been built";
    return nullptr;
  }

  auto where = id_to_type_.find(type_id);
  if (where != id_to_type_.end()) {
    return where->second;
  }

  auto* spirv_type = type_mgr_->GetType(type_id);
  if (spirv_type == nullptr) {
    Fail() << "ID is not a SPIR-V type: " << type_id;
    return nullptr;
  }

  auto save = [this, type_id, spirv_type](ast::type::Type* type) {
    if (type != nullptr) {
      id_to_type_[type_id] = type;
      MaybeGenerateAlias(type_id, spirv_type);
    }
    return type;
  };

  switch (spirv_type->kind()) {
    case spvtools::opt::analysis::Type::kVoid:
      return save(ctx_.type_mgr().Get(std::make_unique<ast::type::VoidType>()));
    case spvtools::opt::analysis::Type::kBool:
      return save(bool_type_);
    case spvtools::opt::analysis::Type::kInteger:
      return save(ConvertType(spirv_type->AsInteger()));
    case spvtools::opt::analysis::Type::kFloat:
      return save(ConvertType(spirv_type->AsFloat()));
    case spvtools::opt::analysis::Type::kVector:
      return save(ConvertType(spirv_type->AsVector()));
    case spvtools::opt::analysis::Type::kMatrix:
      return save(ConvertType(spirv_type->AsMatrix()));
    case spvtools::opt::analysis::Type::kRuntimeArray:
      return save(ConvertType(spirv_type->AsRuntimeArray()));
    case spvtools::opt::analysis::Type::kArray:
      return save(ConvertType(spirv_type->AsArray()));
    case spvtools::opt::analysis::Type::kStruct:
      return save(ConvertType(type_id, spirv_type->AsStruct()));
    case spvtools::opt::analysis::Type::kPointer:
      return save(ConvertType(type_id, spirv_type->AsPointer()));
    case spvtools::opt::analysis::Type::kFunction:
      // Tint doesn't have a Function type.
      // We need to convert the result type and parameter types.
      // But the SPIR-V defines those before defining the function
      // type.  No further work is required here.
      return nullptr;
    default:
      break;
  }

  Fail() << "unknown SPIR-V type: " << type_id;
  return nullptr;
}

DecorationList ParserImpl::GetDecorationsFor(uint32_t id) const {
  DecorationList result;
  const auto& decorations = deco_mgr_->GetDecorationsFor(id, true);
  for (const auto* inst : decorations) {
    if (inst->opcode() != SpvOpDecorate) {
      continue;
    }
    // Example: OpDecorate %struct_id Block
    // Example: OpDecorate %array_ty ArrayStride 16
    std::vector<uint32_t> inst_as_words;
    inst->ToBinaryWithoutAttachedDebugInsts(&inst_as_words);
    Decoration d(inst_as_words.begin() + 2, inst_as_words.end());
    result.push_back(d);
  }
  return result;
}

DecorationList ParserImpl::GetDecorationsForMember(
    uint32_t id,
    uint32_t member_index) const {
  DecorationList result;
  const auto& decorations = deco_mgr_->GetDecorationsFor(id, true);
  for (const auto* inst : decorations) {
    if ((inst->opcode() != SpvOpMemberDecorate) ||
        (inst->GetSingleWordInOperand(1) != member_index)) {
      continue;
    }
    // Example: OpMemberDecorate %struct_id 2 Offset 24
    std::vector<uint32_t> inst_as_words;
    inst->ToBinaryWithoutAttachedDebugInsts(&inst_as_words);
    Decoration d(inst_as_words.begin() + 3, inst_as_words.end());
    result.push_back(d);
  }
  return result;
}

std::string ParserImpl::ShowType(uint32_t type_id) {
  if (def_use_mgr_) {
    const auto* type_inst = def_use_mgr_->GetDef(type_id);
    if (type_inst) {
      return type_inst->PrettyPrint();
    }
  }
  return "SPIR-V type " + std::to_string(type_id);
}

std::unique_ptr<ast::StructMemberDecoration>
ParserImpl::ConvertMemberDecoration(uint32_t struct_type_id,
                                    uint32_t member_index,
                                    const Decoration& decoration) {
  if (decoration.empty()) {
    Fail() << "malformed SPIR-V decoration: it's empty";
    return nullptr;
  }
  switch (decoration[0]) {
    case SpvDecorationOffset:
      if (decoration.size() != 2) {
        Fail()
            << "malformed Offset decoration: expected 1 literal operand, has "
            << decoration.size() - 1 << ": member " << member_index << " of "
            << ShowType(struct_type_id);
        return nullptr;
      }
      return std::make_unique<ast::StructMemberOffsetDecoration>(decoration[1]);
    case SpvDecorationNonReadable:
    case SpvDecorationNonWritable:
      // TODO(dneto): Drop these for now.
      // https://github.com/gpuweb/gpuweb/issues/935
      return nullptr;
    case SpvDecorationColMajor:
      // WGSL only supports column major matrices.
      return nullptr;
    case SpvDecorationRowMajor:
      Fail() << "WGSL does not support row-major matrices: can't "
                "translate member "
             << member_index << " of " << ShowType(struct_type_id);
      return nullptr;
    case SpvDecorationMatrixStride: {
      if (decoration.size() != 2) {
        Fail() << "malformed MatrixStride decoration: expected 1 literal "
                  "operand, has "
               << decoration.size() - 1 << ": member " << member_index << " of "
               << ShowType(struct_type_id);
        return nullptr;
      }
      // TODO(dneto): Fail if the matrix stride is not allocation size of the
      // column vector of the underlying matrix.  This would need to unpack
      // any levels of array-ness.
      return nullptr;
    }
    default:
      // TODO(dneto): Support the remaining member decorations.
      break;
  }
  Fail() << "unhandled member decoration: " << decoration[0] << " on member "
         << member_index << " of " << ShowType(struct_type_id);
  return nullptr;
}

bool ParserImpl::BuildInternalModule() {
  if (!success_) {
    return false;
  }

  const spv_context& context = tools_context_.CContext();
  ir_context_ = spvtools::BuildModule(context->target_env, context->consumer,
                                      spv_binary_.data(), spv_binary_.size());
  if (!ir_context_) {
    return Fail() << "internal error: couldn't build the internal "
                     "representation of the module";
  }
  module_ = ir_context_->module();
  def_use_mgr_ = ir_context_->get_def_use_mgr();
  constant_mgr_ = ir_context_->get_constant_mgr();
  type_mgr_ = ir_context_->get_type_mgr();
  deco_mgr_ = ir_context_->get_decoration_mgr();

  return success_;
}

void ParserImpl::ResetInternalModule() {
  ir_context_.reset(nullptr);
  module_ = nullptr;
  def_use_mgr_ = nullptr;
  constant_mgr_ = nullptr;
  type_mgr_ = nullptr;
  deco_mgr_ = nullptr;

  import_map_.clear();
  glsl_std_450_imports_.clear();
}

bool ParserImpl::ParseInternalModule() {
  if (!success_) {
    return false;
  }
  RegisterLineNumbers();
  if (!ParseInternalModuleExceptFunctions()) {
    return false;
  }
  if (!EmitFunctions()) {
    return false;
  }
  return success_;
}

void ParserImpl::RegisterLineNumbers() {
  Source instruction_number{0, 0};

  // Has there been an OpLine since the last OpNoLine or start of the module?
  bool in_op_line_scope = false;
  // The source location provided by the most recent OpLine instruction.
  Source op_line_source{0, 0};
  const bool run_on_debug_insts = true;
  module_->ForEachInst(
      [this, &in_op_line_scope, &op_line_source,
       &instruction_number](const spvtools::opt::Instruction* inst) {
        ++instruction_number.line;
        switch (inst->opcode()) {
          case SpvOpLine:
            in_op_line_scope = true;
            // TODO(dneto): This ignores the File ID (operand 0), since the Tint
            // Source concept doesn't represent that.
            op_line_source.line = inst->GetSingleWordInOperand(1);
            op_line_source.column = inst->GetSingleWordInOperand(2);
            break;
          case SpvOpNoLine:
            in_op_line_scope = false;
            break;
          default:
            break;
        }
        this->inst_source_[inst] =
            in_op_line_scope ? op_line_source : instruction_number;
      },
      run_on_debug_insts);
}

Source ParserImpl::GetSourceForResultIdForTest(uint32_t id) const {
  return GetSourceForInst(def_use_mgr_->GetDef(id));
}

Source ParserImpl::GetSourceForInst(
    const spvtools::opt::Instruction* inst) const {
  auto where = inst_source_.find(inst);
  if (where == inst_source_.end()) {
    return {};
  }
  return where->second;
}

bool ParserImpl::ParseInternalModuleExceptFunctions() {
  if (!success_) {
    return false;
  }
  if (!RegisterExtendedInstructionImports()) {
    return false;
  }
  if (!RegisterUserAndStructMemberNames()) {
    return false;
  }
  if (!EmitEntryPoints()) {
    return false;
  }
  if (!RegisterTypes()) {
    return false;
  }
  if (!EmitModuleScopeVariables()) {
    return false;
  }
  return success_;
}

bool ParserImpl::RegisterExtendedInstructionImports() {
  for (const spvtools::opt::Instruction& import : module_->ext_inst_imports()) {
    std::string name(
        reinterpret_cast<const char*>(import.GetInOperand(0).words.data()));
    // TODO(dneto): Handle other extended instruction sets when needed.
    if (name == "GLSL.std.450") {
      // Only create the AST import once, so we can use import name 'std::glsl'.
      // This is a canonicalization.
      if (glsl_std_450_imports_.empty()) {
        auto ast_import =
            std::make_unique<tint::ast::Import>(name, GlslStd450Prefix());
        import_map_[import.result_id()] = ast_import.get();
        ast_module_.AddImport(std::move(ast_import));
      }
      glsl_std_450_imports_.insert(import.result_id());
    } else {
      return Fail() << "Unrecognized extended instruction set: " << name;
    }
  }
  return true;
}

bool ParserImpl::RegisterUserAndStructMemberNames() {
  if (!success_) {
    return false;
  }
  // Register entry point names. An entry point name is the point of contact
  // between the API and the shader. It has the highest priority for
  // preservation, so register it first.
  for (const spvtools::opt::Instruction& entry_point :
       module_->entry_points()) {
    const uint32_t function_id = entry_point.GetSingleWordInOperand(1);
    const std::string name = entry_point.GetInOperand(2).AsString();
    namer_.SuggestSanitizedName(function_id, name);
  }

  // Register names from OpName and OpMemberName
  for (const auto& inst : module_->debugs2()) {
    switch (inst.opcode()) {
      case SpvOpName: {
        const auto name = inst.GetInOperand(1).AsString();
        if (!name.empty()) {
          namer_.SuggestSanitizedName(inst.GetSingleWordInOperand(0), name);
        }
        break;
      }
      case SpvOpMemberName: {
        const auto name = inst.GetInOperand(2).AsString();
        if (!name.empty()) {
          namer_.SuggestSanitizedMemberName(inst.GetSingleWordInOperand(0),
                                            inst.GetSingleWordInOperand(1),
                                            name);
        }
        break;
      }
      default:
        break;
    }
  }

  // Fill in struct member names, and disambiguate them.
  for (const auto* type_inst : module_->GetTypes()) {
    if (type_inst->opcode() == SpvOpTypeStruct) {
      namer_.ResolveMemberNamesForStruct(type_inst->result_id(),
                                         type_inst->NumInOperands());
    }
  }

  return true;
}

bool ParserImpl::EmitEntryPoints() {
  for (const spvtools::opt::Instruction& entry_point :
       module_->entry_points()) {
    const auto stage = SpvExecutionModel(entry_point.GetSingleWordInOperand(0));
    const uint32_t function_id = entry_point.GetSingleWordInOperand(1);
    const std::string name = namer_.GetName(function_id);

    ast_module_.AddEntryPoint(std::make_unique<ast::EntryPoint>(
        enum_converter_.ToPipelineStage(stage), "", name));
  }
  // The enum conversion could have failed, so return the existing status value.
  return success_;
}

ast::type::Type* ParserImpl::ConvertType(
    const spvtools::opt::analysis::Integer* int_ty) {
  if (int_ty->width() == 32) {
    auto* signed_ty =
        ctx_.type_mgr().Get(std::make_unique<ast::type::I32Type>());
    auto* unsigned_ty =
        ctx_.type_mgr().Get(std::make_unique<ast::type::U32Type>());
    signed_type_for_[unsigned_ty] = signed_ty;
    unsigned_type_for_[signed_ty] = unsigned_ty;
    return int_ty->IsSigned() ? signed_ty : unsigned_ty;
  }
  Fail() << "unhandled integer width: " << int_ty->width();
  return nullptr;
}

ast::type::Type* ParserImpl::ConvertType(
    const spvtools::opt::analysis::Float* float_ty) {
  if (float_ty->width() == 32) {
    return ctx_.type_mgr().Get(std::make_unique<ast::type::F32Type>());
  }
  Fail() << "unhandled float width: " << float_ty->width();
  return nullptr;
}

ast::type::Type* ParserImpl::ConvertType(
    const spvtools::opt::analysis::Vector* vec_ty) {
  const auto num_elem = vec_ty->element_count();
  auto* ast_elem_ty = ConvertType(type_mgr_->GetId(vec_ty->element_type()));
  if (ast_elem_ty == nullptr) {
    return nullptr;
  }
  auto* this_ty = ctx_.type_mgr().Get(
      std::make_unique<ast::type::VectorType>(ast_elem_ty, num_elem));
  // Generate the opposite-signedness vector type, if this type is integral.
  if (unsigned_type_for_.count(ast_elem_ty)) {
    auto* other_ty =
        ctx_.type_mgr().Get(std::make_unique<ast::type::VectorType>(
            unsigned_type_for_[ast_elem_ty], num_elem));
    signed_type_for_[other_ty] = this_ty;
    unsigned_type_for_[this_ty] = other_ty;
  } else if (signed_type_for_.count(ast_elem_ty)) {
    auto* other_ty =
        ctx_.type_mgr().Get(std::make_unique<ast::type::VectorType>(
            signed_type_for_[ast_elem_ty], num_elem));
    unsigned_type_for_[other_ty] = this_ty;
    signed_type_for_[this_ty] = other_ty;
  }
  return this_ty;
}

ast::type::Type* ParserImpl::ConvertType(
    const spvtools::opt::analysis::Matrix* mat_ty) {
  const auto* vec_ty = mat_ty->element_type()->AsVector();
  const auto* scalar_ty = vec_ty->element_type();
  const auto num_rows = vec_ty->element_count();
  const auto num_columns = mat_ty->element_count();
  auto* ast_scalar_ty = ConvertType(type_mgr_->GetId(scalar_ty));
  if (ast_scalar_ty == nullptr) {
    return nullptr;
  }
  return ctx_.type_mgr().Get(std::make_unique<ast::type::MatrixType>(
      ast_scalar_ty, num_rows, num_columns));
}

ast::type::Type* ParserImpl::ConvertType(
    const spvtools::opt::analysis::RuntimeArray* rtarr_ty) {
  auto* ast_elem_ty = ConvertType(type_mgr_->GetId(rtarr_ty->element_type()));
  if (ast_elem_ty == nullptr) {
    return nullptr;
  }
  auto ast_type = std::make_unique<ast::type::ArrayType>(ast_elem_ty);
  if (!ApplyArrayDecorations(rtarr_ty, ast_type.get())) {
    return nullptr;
  }
  return ctx_.type_mgr().Get(std::move(ast_type));
}

ast::type::Type* ParserImpl::ConvertType(
    const spvtools::opt::analysis::Array* arr_ty) {
  const auto elem_type_id = type_mgr_->GetId(arr_ty->element_type());
  auto* ast_elem_ty = ConvertType(elem_type_id);
  if (ast_elem_ty == nullptr) {
    return nullptr;
  }
  const auto& length_info = arr_ty->length_info();
  if (length_info.words.empty()) {
    // The internal representation is invalid. The discriminant vector
    // is mal-formed.
    Fail() << "internal error: Array length info is invalid";
    return nullptr;
  }
  if (length_info.words[0] !=
      spvtools::opt::analysis::Array::LengthInfo::kConstant) {
    Fail() << "Array type " << type_mgr_->GetId(arr_ty)
           << " length is a specialization constant";
    return nullptr;
  }
  const auto* constant = constant_mgr_->FindDeclaredConstant(length_info.id);
  if (constant == nullptr) {
    Fail() << "Array type " << type_mgr_->GetId(arr_ty) << " length ID "
           << length_info.id << " does not name an OpConstant";
    return nullptr;
  }
  const uint64_t num_elem = constant->GetZeroExtendedValue();
  // For now, limit to only 32bits.
  if (num_elem > std::numeric_limits<uint32_t>::max()) {
    Fail() << "Array type " << type_mgr_->GetId(arr_ty)
           << " has too many elements (more than can fit in 32 bits): "
           << num_elem;
    return nullptr;
  }
  auto ast_type = std::make_unique<ast::type::ArrayType>(
      ast_elem_ty, static_cast<uint32_t>(num_elem));
  if (!ApplyArrayDecorations(arr_ty, ast_type.get())) {
    return nullptr;
  }
  if (remap_buffer_block_type_.count(elem_type_id)) {
    remap_buffer_block_type_.insert(type_mgr_->GetId(arr_ty));
  }
  return ctx_.type_mgr().Get(std::move(ast_type));
}

bool ParserImpl::ApplyArrayDecorations(
    const spvtools::opt::analysis::Type* spv_type,
    ast::type::ArrayType* ast_type) {
  const auto type_id = type_mgr_->GetId(spv_type);
  for (auto& decoration : this->GetDecorationsFor(type_id)) {
    if (decoration.size() == 2 && decoration[0] == SpvDecorationArrayStride) {
      const auto stride = decoration[1];
      if (stride == 0) {
        return Fail() << "invalid array type ID " << type_id
                      << ": ArrayStride can't be 0";
      }
      if (ast_type->has_array_stride()) {
        return Fail() << "invalid array type ID " << type_id
                      << ": multiple ArrayStride decorations";
      }
      ast_type->set_array_stride(stride);
    } else {
      return Fail() << "invalid array type ID " << type_id
                    << ": unknown decoration "
                    << (decoration.empty() ? "(empty)"
                                           : std::to_string(decoration[0]))
                    << " with " << decoration.size() << " total words";
    }
  }
  return true;
}

ast::type::Type* ParserImpl::ConvertType(
    uint32_t type_id,
    const spvtools::opt::analysis::Struct* struct_ty) {
  // Compute the struct decoration.
  auto struct_decorations = this->GetDecorationsFor(type_id);
  auto ast_struct_decoration = ast::StructDecoration::kNone;
  if (struct_decorations.size() == 1) {
    const auto decoration = struct_decorations[0][0];
    if (decoration == SpvDecorationBlock) {
      ast_struct_decoration = ast::StructDecoration::kBlock;
    } else if (decoration == SpvDecorationBufferBlock) {
      ast_struct_decoration = ast::StructDecoration::kBlock;
      remap_buffer_block_type_.insert(type_id);
    } else {
      Fail() << "struct with ID " << type_id
             << " has unrecognized decoration: " << int(decoration);
    }
  } else if (struct_decorations.size() > 1) {
    Fail() << "can't handle a struct with more than one decoration: struct "
           << type_id << " has " << struct_decorations.size();
    return nullptr;
  }

  // Compute members
  ast::StructMemberList ast_members;
  const auto members = struct_ty->element_types();
  for (uint32_t member_index = 0; member_index < members.size();
       ++member_index) {
    const auto member_type_id = type_mgr_->GetId(members[member_index]);
    auto* ast_member_ty = ConvertType(member_type_id);
    if (ast_member_ty == nullptr) {
      // Already emitted diagnostics.
      return nullptr;
    }
    ast::StructMemberDecorationList ast_member_decorations;
    for (auto& decoration : GetDecorationsForMember(type_id, member_index)) {
      if (decoration.empty()) {
        Fail() << "malformed SPIR-V decoration: it's empty";
        return nullptr;
      }
      if ((decoration[0] == SpvDecorationBuiltIn) && (decoration.size() > 1)) {
        switch (decoration[1]) {
          case SpvBuiltInPosition:
            // Record this built-in variable specially.
            builtin_position_.struct_type_id = type_id;
            builtin_position_.member_index = member_index;
            builtin_position_.member_type_id = member_type_id;
            // Don't map the struct type.  But this is not an error either.
            return nullptr;
          case SpvBuiltInPointSize:     // not supported in WGSL
          case SpvBuiltInCullDistance:  // not supported in WGSL
          case SpvBuiltInClipDistance:  // not supported in WGSL
          default:
            break;
        }
        Fail() << "unrecognized builtin " << decoration[1];
        return nullptr;
      } else {
        auto ast_member_decoration =
            ConvertMemberDecoration(type_id, member_index, decoration);
        if (!success_) {
          return nullptr;
        }
        if (ast_member_decoration) {
          ast_member_decorations.push_back(std::move(ast_member_decoration));
        }
      }
    }
    const auto member_name = namer_.GetMemberName(type_id, member_index);
    auto ast_struct_member = std::make_unique<ast::StructMember>(
        member_name, ast_member_ty, std::move(ast_member_decorations));
    ast_members.push_back(std::move(ast_struct_member));
  }

  // Now make the struct.
  auto ast_struct = std::make_unique<ast::Struct>(ast_struct_decoration,
                                                  std::move(ast_members));
  // The struct type will be assigned a name during EmitAliasTypes.
  auto ast_struct_type =
      std::make_unique<ast::type::StructType>(std::move(ast_struct));
  // Set the struct name before registering it.
  namer_.SuggestSanitizedName(type_id, "S");
  ast_struct_type->set_name(namer_.GetName(type_id));
  auto* result = ctx_.type_mgr().Get(std::move(ast_struct_type));
  return result;
}

ast::type::Type* ParserImpl::ConvertType(
    uint32_t type_id,
    const spvtools::opt::analysis::Pointer*) {
  const auto* inst = def_use_mgr_->GetDef(type_id);
  const auto pointee_type_id = inst->GetSingleWordInOperand(1);
  const auto storage_class = SpvStorageClass(inst->GetSingleWordInOperand(0));
  if (pointee_type_id == builtin_position_.struct_type_id) {
    builtin_position_.pointer_type_id = type_id;
    builtin_position_.storage_class = storage_class;
    return nullptr;
  }
  auto* ast_elem_ty = ConvertType(pointee_type_id);
  if (ast_elem_ty == nullptr) {
    Fail() << "SPIR-V pointer type with ID " << type_id
           << " has invalid pointee type " << pointee_type_id;
    return nullptr;
  }

  auto ast_storage_class = enum_converter_.ToStorageClass(storage_class);
  if (ast_storage_class == ast::StorageClass::kNone) {
    Fail() << "SPIR-V pointer type with ID " << type_id
           << " has invalid storage class "
           << static_cast<uint32_t>(storage_class);
    return nullptr;
  }
  if (ast_storage_class == ast::StorageClass::kUniform &&
      remap_buffer_block_type_.count(pointee_type_id)) {
    ast_storage_class = ast::StorageClass::kStorageBuffer;
    remap_buffer_block_type_.insert(type_id);
  }
  return ctx_.type_mgr().Get(
      std::make_unique<ast::type::PointerType>(ast_elem_ty, ast_storage_class));
}

bool ParserImpl::RegisterTypes() {
  if (!success_) {
    return false;
  }
  for (auto& type_or_const : module_->types_values()) {
    const auto* type = type_mgr_->GetType(type_or_const.result_id());
    if (type == nullptr) {
      continue;
    }
    ConvertType(type_or_const.result_id());
  }
  // Manufacture a type for the gl_Position varible if we have to.
  if ((builtin_position_.struct_type_id != 0) &&
      (builtin_position_.member_pointer_type_id == 0)) {
    builtin_position_.member_pointer_type_id = type_mgr_->FindPointerToType(
        builtin_position_.member_type_id, builtin_position_.storage_class);
    ConvertType(builtin_position_.member_pointer_type_id);
  }
  return success_;
}

void ParserImpl::MaybeGenerateAlias(uint32_t type_id,
                                    const spvtools::opt::analysis::Type* type) {
  if (!success_) {
    return;
  }

  // We only care about struct, arrays, and runtime arrays.
  switch (type->kind()) {
    case spvtools::opt::analysis::Type::kStruct:
      // The struct already got a name when the type was first registered.
      break;
    case spvtools::opt::analysis::Type::kRuntimeArray:
      // Runtime arrays are always decorated with ArrayStride so always get a
      // type alias.
      namer_.SuggestSanitizedName(type_id, "RTArr");
      break;
    case spvtools::opt::analysis::Type::kArray:
      // Only make a type aliase for arrays with decorations.
      if (GetDecorationsFor(type_id).empty()) {
        return;
      }
      namer_.SuggestSanitizedName(type_id, "Arr");
      break;
    default:
      // Ignore constants, and any other types.
      return;
  }
  auto* ast_underlying_type = id_to_type_[type_id];
  if (ast_underlying_type == nullptr) {
    Fail() << "internal error: no type registered for SPIR-V ID: " << type_id;
    return;
  }
  const auto name = namer_.GetName(type_id);
  auto* ast_alias_type = ctx_.type_mgr()
                             .Get(std::make_unique<ast::type::AliasType>(
                                 name, ast_underlying_type))
                             ->AsAlias();
  // Record this new alias as the AST type for this SPIR-V ID.
  id_to_type_[type_id] = ast_alias_type;
  ast_module_.AddAliasType(ast_alias_type);
}

bool ParserImpl::EmitModuleScopeVariables() {
  if (!success_) {
    return false;
  }
  for (const auto& type_or_value : module_->types_values()) {
    if (type_or_value.opcode() != SpvOpVariable) {
      continue;
    }
    const auto& var = type_or_value;
    const auto spirv_storage_class =
        SpvStorageClass(var.GetSingleWordInOperand(0));

    uint32_t type_id = var.type_id();
    if ((type_id == builtin_position_.pointer_type_id) &&
        ((spirv_storage_class == SpvStorageClassInput) ||
         (spirv_storage_class == SpvStorageClassOutput))) {
      // Skip emitting gl_PerVertex.
      builtin_position_.per_vertex_var_id = var.result_id();
      continue;
    }
    switch (enum_converter_.ToStorageClass(spirv_storage_class)) {
      case ast::StorageClass::kInput:
      case ast::StorageClass::kOutput:
      case ast::StorageClass::kUniform:
      case ast::StorageClass::kUniformConstant:
      case ast::StorageClass::kStorageBuffer:
      case ast::StorageClass::kImage:
      case ast::StorageClass::kWorkgroup:
      case ast::StorageClass::kPrivate:
        break;
      default:
        return Fail() << "invalid SPIR-V storage class "
                      << int(spirv_storage_class)
                      << " for module scope variable: " << var.PrettyPrint();
    }
    if (!success_) {
      return false;
    }
    auto* ast_type = id_to_type_[type_id];
    if (ast_type == nullptr) {
      return Fail() << "internal error: failed to register Tint AST type for "
                       "SPIR-V type with ID: "
                    << var.type_id();
    }
    if (!ast_type->IsPointer()) {
      return Fail() << "variable with ID " << var.result_id()
                    << " has non-pointer type " << var.type_id();
    }
    auto* ast_store_type = ast_type->AsPointer()->type();
    auto ast_storage_class = ast_type->AsPointer()->storage_class();
    auto ast_var =
        MakeVariable(var.result_id(), ast_storage_class, ast_store_type);
    if (var.NumInOperands() > 1) {
      // SPIR-V initializers are always constants.
      // (OpenCL also allows the ID of an OpVariable, but we don't handle that
      // here.)
      ast_var->set_constructor(
          MakeConstantExpression(var.GetSingleWordInOperand(1)).expr);
    }
    // TODO(dneto): initializers (a.k.a. constructor expression)
    ast_module_.AddGlobalVariable(std::move(ast_var));
  }

  // Emit gl_Position instead of gl_PerVertex
  if (builtin_position_.per_vertex_var_id) {
    // Make sure the variable has a name.
    namer_.SuggestSanitizedName(builtin_position_.per_vertex_var_id,
                                "gl_Position");
    auto var = std::make_unique<ast::DecoratedVariable>(MakeVariable(
        builtin_position_.per_vertex_var_id,
        enum_converter_.ToStorageClass(builtin_position_.storage_class),
        ConvertType(builtin_position_.member_type_id)));
    ast::VariableDecorationList decos;
    decos.push_back(
        std::make_unique<ast::BuiltinDecoration>(ast::Builtin::kPosition));
    var->set_decorations(std::move(decos));

    ast_module_.AddGlobalVariable(std::move(var));
  }
  return success_;
}

std::unique_ptr<ast::Variable> ParserImpl::MakeVariable(uint32_t id,
                                                        ast::StorageClass sc,
                                                        ast::type::Type* type) {
  if (type == nullptr) {
    Fail() << "internal error: can't make ast::Variable for null type";
    return nullptr;
  }
  auto ast_var = std::make_unique<ast::Variable>(namer_.Name(id), sc, type);

  ast::VariableDecorationList ast_decorations;
  for (auto& deco : GetDecorationsFor(id)) {
    if (deco.empty()) {
      Fail() << "malformed decoration on ID " << id << ": it is empty";
      return nullptr;
    }
    if (deco[0] == SpvDecorationBuiltIn) {
      if (deco.size() == 1) {
        Fail() << "malformed BuiltIn decoration on ID " << id
               << ": has no operand";
        return nullptr;
      }
      auto ast_builtin =
          enum_converter_.ToBuiltin(static_cast<SpvBuiltIn>(deco[1]));
      if (ast_builtin == ast::Builtin::kNone) {
        return nullptr;
      }
      ast_decorations.emplace_back(
          std::make_unique<ast::BuiltinDecoration>(ast_builtin));
    }
    if (deco[0] == SpvDecorationLocation) {
      if (deco.size() != 2) {
        Fail() << "malformed Location decoration on ID " << id
               << ": requires one literal operand";
        return nullptr;
      }
      ast_decorations.emplace_back(
          std::make_unique<ast::LocationDecoration>(deco[1]));
    }
    if (deco[0] == SpvDecorationDescriptorSet) {
      if (deco.size() == 1) {
        Fail() << "malformed DescriptorSet decoration on ID " << id
               << ": has no operand";
        return nullptr;
      }
      ast_decorations.emplace_back(
          std::make_unique<ast::SetDecoration>(deco[1]));
    }
    if (deco[0] == SpvDecorationBinding) {
      if (deco.size() == 1) {
        Fail() << "malformed Binding decoration on ID " << id
               << ": has no operand";
        return nullptr;
      }
      ast_decorations.emplace_back(
          std::make_unique<ast::BindingDecoration>(deco[1]));
    }
  }
  if (!ast_decorations.empty()) {
    auto decorated_var =
        std::make_unique<ast::DecoratedVariable>(std::move(ast_var));
    decorated_var->set_decorations(std::move(ast_decorations));
    ast_var = std::move(decorated_var);
  }
  return ast_var;
}

TypedExpression ParserImpl::MakeConstantExpression(uint32_t id) {
  if (!success_) {
    return {};
  }
  const auto* inst = def_use_mgr_->GetDef(id);
  if (inst == nullptr) {
    Fail() << "ID " << id << " is not a registered instruction";
    return {};
  }
  auto* original_ast_type = ConvertType(inst->type_id());
  if (original_ast_type == nullptr) {
    return {};
  }

  if (inst->opcode() == SpvOpUndef) {
    // Remap undef to null.
    return {original_ast_type, MakeNullValue(original_ast_type)};
  }

  // TODO(dneto): Handle spec constants too?
  const auto* spirv_const = constant_mgr_->FindDeclaredConstant(id);
  if (spirv_const == nullptr) {
    Fail() << "ID " << id << " is not a constant";
    return {};
  }

  auto* ast_type = original_ast_type->UnwrapAliasesIfNeeded();

  // TODO(dneto): Note: NullConstant for int, uint, float map to a regular 0.
  // So canonicalization should map that way too.
  // Currently "null<type>" is missing from the WGSL parser.
  // See https://bugs.chromium.org/p/tint/issues/detail?id=34
  if (ast_type->IsU32()) {
    return {ast_type, std::make_unique<ast::ScalarConstructorExpression>(
                          std::make_unique<ast::UintLiteral>(
                              ast_type, spirv_const->GetU32()))};
  }
  if (ast_type->IsI32()) {
    return {ast_type, std::make_unique<ast::ScalarConstructorExpression>(
                          std::make_unique<ast::SintLiteral>(
                              ast_type, spirv_const->GetS32()))};
  }
  if (ast_type->IsF32()) {
    return {ast_type, std::make_unique<ast::ScalarConstructorExpression>(
                          std::make_unique<ast::FloatLiteral>(
                              ast_type, spirv_const->GetFloat()))};
  }
  if (ast_type->IsBool()) {
    const bool value = spirv_const->AsNullConstant()
                           ? false
                           : spirv_const->AsBoolConstant()->value();
    return {ast_type, std::make_unique<ast::ScalarConstructorExpression>(
                          std::make_unique<ast::BoolLiteral>(ast_type, value))};
  }
  auto* spirv_composite_const = spirv_const->AsCompositeConstant();
  if (spirv_composite_const != nullptr) {
    // Handle vector, matrix, array, and struct

    // TODO(dneto): Handle the spirv_composite_const->IsZero() case specially.
    // See https://github.com/gpuweb/gpuweb/issues/685

    // Generate a composite from explicit components.
    ast::ExpressionList ast_components;
    for (const auto* component : spirv_composite_const->GetComponents()) {
      auto* def = constant_mgr_->GetDefiningInstruction(component);
      if (def == nullptr) {
        Fail() << "internal error: SPIR-V constant doesn't have defining "
                  "instruction";
        return {};
      }
      auto ast_component = MakeConstantExpression(def->result_id());
      if (!success_) {
        // We've already emitted a diagnostic.
        return {};
      }
      ast_components.emplace_back(std::move(ast_component.expr));
    }
    return {original_ast_type,
            std::make_unique<ast::TypeConstructorExpression>(
                original_ast_type, std::move(ast_components))};
  }
  auto* spirv_null_const = spirv_const->AsNullConstant();
  if (spirv_null_const != nullptr) {
    return {original_ast_type, MakeNullValue(original_ast_type)};
  }
  Fail() << "Unhandled constant type " << inst->type_id() << " for value ID "
         << id;
  return {};
}

std::unique_ptr<ast::Expression> ParserImpl::MakeNullValue(
    ast::type::Type* type) {
  // TODO(dneto): Use the no-operands constructor syntax when it becomes
  // available in Tint.
  // https://github.com/gpuweb/gpuweb/issues/685
  // https://bugs.chromium.org/p/tint/issues/detail?id=34

  if (!type) {
    Fail() << "trying to create null value for a null type";
    return nullptr;
  }

  auto* original_type = type;
  type = type->UnwrapAliasesIfNeeded();

  if (type->IsBool()) {
    return std::make_unique<ast::ScalarConstructorExpression>(
        std::make_unique<ast::BoolLiteral>(type, false));
  }
  if (type->IsU32()) {
    return std::make_unique<ast::ScalarConstructorExpression>(
        std::make_unique<ast::UintLiteral>(type, 0u));
  }
  if (type->IsI32()) {
    return std::make_unique<ast::ScalarConstructorExpression>(
        std::make_unique<ast::SintLiteral>(type, 0));
  }
  if (type->IsF32()) {
    return std::make_unique<ast::ScalarConstructorExpression>(
        std::make_unique<ast::FloatLiteral>(type, 0.0f));
  }
  if (type->IsVector()) {
    const auto* vec_ty = type->AsVector();
    ast::ExpressionList ast_components;
    for (size_t i = 0; i < vec_ty->size(); ++i) {
      ast_components.emplace_back(MakeNullValue(vec_ty->type()));
    }
    return std::make_unique<ast::TypeConstructorExpression>(
        type, std::move(ast_components));
  }
  if (type->IsMatrix()) {
    const auto* mat_ty = type->AsMatrix();
    // Matrix components are columns
    auto* column_ty =
        ctx_.type_mgr().Get(std::make_unique<ast::type::VectorType>(
            mat_ty->type(), mat_ty->rows()));
    ast::ExpressionList ast_components;
    for (size_t i = 0; i < mat_ty->columns(); ++i) {
      ast_components.emplace_back(MakeNullValue(column_ty));
    }
    return std::make_unique<ast::TypeConstructorExpression>(
        type, std::move(ast_components));
  }
  if (type->IsArray()) {
    auto* arr_ty = type->AsArray();
    ast::ExpressionList ast_components;
    for (size_t i = 0; i < arr_ty->size(); ++i) {
      ast_components.emplace_back(MakeNullValue(arr_ty->type()));
    }
    return std::make_unique<ast::TypeConstructorExpression>(
        original_type, std::move(ast_components));
  }
  if (type->IsStruct()) {
    auto* struct_ty = type->AsStruct();
    ast::ExpressionList ast_components;
    for (auto& member : struct_ty->impl()->members()) {
      ast_components.emplace_back(MakeNullValue(member->type()));
    }
    return std::make_unique<ast::TypeConstructorExpression>(
        original_type, std::move(ast_components));
  }
  Fail() << "can't make null value for type: " << type->type_name();
  return nullptr;
}

TypedExpression ParserImpl::RectifyOperandSignedness(SpvOp op,
                                                     TypedExpression&& expr) {
  const bool requires_signed = AssumesSignedOperands(op);
  const bool requires_unsigned = AssumesUnsignedOperands(op);
  if (!requires_signed && !requires_unsigned) {
    // No conversion is required, assuming our tables are complete.
    return std::move(expr);
  }
  if (!expr.expr) {
    Fail() << "internal error: RectifyOperandSignedness given a null expr\n";
    return {};
  }
  auto* type = expr.type;
  if (!type) {
    Fail() << "internal error: unmapped type for: " << expr.expr->str() << "\n";
    return {};
  }
  if (requires_unsigned) {
    auto* unsigned_ty = unsigned_type_for_[type];
    if (unsigned_ty != nullptr) {
      // Conversion is required.
      return {unsigned_ty, std::make_unique<ast::AsExpression>(
                               unsigned_ty, std::move(expr.expr))};
    }
  } else if (requires_signed) {
    auto* signed_ty = signed_type_for_[type];
    if (signed_ty != nullptr) {
      // Conversion is required.
      return {signed_ty, std::make_unique<ast::AsExpression>(
                             signed_ty, std::move(expr.expr))};
    }
  }
  // We should not reach here.
  return std::move(expr);
}

ast::type::Type* ParserImpl::ForcedResultType(
    SpvOp op,
    ast::type::Type* first_operand_type) {
  const bool binary_match_first_operand =
      AssumesResultSignednessMatchesBinaryFirstOperand(op);
  const bool unary_match_operand = (op == SpvOpSNegate) || (op == SpvOpNot);
  if (binary_match_first_operand || unary_match_operand) {
    return first_operand_type;
  }
  return nullptr;
}

ast::type::Type* ParserImpl::GetSignedIntMatchingShape(ast::type::Type* other) {
  if (other == nullptr) {
    Fail() << "no type provided";
  }
  auto* i32 = ctx_.type_mgr().Get(std::make_unique<ast::type::I32Type>());
  if (other->IsF32() || other->IsU32() || other->IsI32()) {
    return i32;
  }
  auto* vec_ty = other->AsVector();
  if (vec_ty) {
    return ctx_.type_mgr().Get(
        std::make_unique<ast::type::VectorType>(i32, vec_ty->size()));
  }
  Fail() << "required numeric scalar or vector, but got " << other->type_name();
  return nullptr;
}

ast::type::Type* ParserImpl::GetUnsignedIntMatchingShape(
    ast::type::Type* other) {
  if (other == nullptr) {
    Fail() << "no type provided";
    return nullptr;
  }
  auto* u32 = ctx_.type_mgr().Get(std::make_unique<ast::type::U32Type>());
  if (other->IsF32() || other->IsU32() || other->IsI32()) {
    return u32;
  }
  auto* vec_ty = other->AsVector();
  if (vec_ty) {
    return ctx_.type_mgr().Get(
        std::make_unique<ast::type::VectorType>(u32, vec_ty->size()));
  }
  Fail() << "required numeric scalar or vector, but got " << other->type_name();
  return nullptr;
}

TypedExpression ParserImpl::RectifyForcedResultType(
    TypedExpression expr,
    SpvOp op,
    ast::type::Type* first_operand_type) {
  auto* forced_result_ty = ForcedResultType(op, first_operand_type);
  if ((forced_result_ty == nullptr) || (forced_result_ty == expr.type)) {
    return expr;
  }
  return {expr.type,
          std::make_unique<ast::AsExpression>(expr.type, std::move(expr.expr))};
}

bool ParserImpl::EmitFunctions() {
  if (!success_) {
    return false;
  }
  for (const auto* f :
       FunctionTraverser(*module_).TopologicallyOrderedFunctions()) {
    if (!success_) {
      return false;
    }
    FunctionEmitter emitter(this, *f);
    success_ = emitter.Emit();
  }
  return success_;
}

}  // namespace spirv
}  // namespace reader
}  // namespace tint
