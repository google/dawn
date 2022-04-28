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

#include "src/tint/reader/spirv/parser_impl.h"

#include <algorithm>
#include <limits>
#include <locale>
#include <utility>

#include "source/opt/build_module.h"
#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/ast/disable_validation_attribute.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/interpolate_attribute.h"
#include "src/tint/ast/type_name.h"
#include "src/tint/ast/unary_op_expression.h"
#include "src/tint/reader/spirv/function.h"
#include "src/tint/sem/depth_texture_type.h"
#include "src/tint/sem/multisampled_texture_type.h"
#include "src/tint/sem/sampled_texture_type.h"
#include "src/tint/utils/unique_vector.h"

namespace tint::reader::spirv {

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

// Returns true if the GLSL extended instruction expects operands to be signed.
// @param extended_opcode GLSL.std.450 opcode
// @returns true if all operands must be signed integral type
bool AssumesSignedOperands(GLSLstd450 extended_opcode) {
  switch (extended_opcode) {
    case GLSLstd450SAbs:
    case GLSLstd450SSign:
    case GLSLstd450SMin:
    case GLSLstd450SMax:
    case GLSLstd450SClamp:
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

// Returns true if the GLSL extended instruction expects operands to be
// unsigned.
// @param extended_opcode GLSL.std.450 opcode
// @returns true if all operands must be unsigned integral type
bool AssumesUnsignedOperands(GLSLstd450 extended_opcode) {
  switch (extended_opcode) {
    case GLSLstd450UMin:
    case GLSLstd450UMax:
    case GLSLstd450UClamp:
      return true;
    default:
      break;
  }
  return false;
}

// Returns true if the corresponding WGSL operation requires
// the signedness of the second operand to match the signedness of the
// first operand, and it's not one of the OpU* or OpS* instructions.
// (Those are handled via MakeOperand.)
bool AssumesSecondOperandSignednessMatchesFirstOperand(SpvOp opcode) {
  switch (opcode) {
    // All the OpI* integer binary operations.
    case SpvOpIAdd:
    case SpvOpISub:
    case SpvOpIMul:
    case SpvOpIEqual:
    case SpvOpINotEqual:
    // All the bitwise integer binary operations.
    case SpvOpBitwiseAnd:
    case SpvOpBitwiseOr:
    case SpvOpBitwiseXor:
      return true;
    default:
      break;
  }
  return false;
}

// Returns true if the corresponding WGSL operation requires
// the signedness of the result to match the signedness of the first operand.
bool AssumesResultSignednessMatchesFirstOperand(SpvOp opcode) {
  switch (opcode) {
    case SpvOpNot:
    case SpvOpSNegate:
    case SpvOpBitCount:
    case SpvOpBitReverse:
    case SpvOpSDiv:
    case SpvOpSMod:
    case SpvOpSRem:
    case SpvOpIAdd:
    case SpvOpISub:
    case SpvOpIMul:
    case SpvOpBitwiseAnd:
    case SpvOpBitwiseOr:
    case SpvOpBitwiseXor:
    case SpvOpShiftLeftLogical:
    case SpvOpShiftRightLogical:
    case SpvOpShiftRightArithmetic:
      return true;
    default:
      break;
  }
  return false;
}

// Returns true if the extended instruction requires the signedness of the
// result to match the signedness of the first operand to the operation.
// @param extended_opcode GLSL.std.450 opcode
// @returns true if the result type must match the first operand type.
bool AssumesResultSignednessMatchesFirstOperand(GLSLstd450 extended_opcode) {
  switch (extended_opcode) {
    case GLSLstd450SAbs:
    case GLSLstd450SSign:
    case GLSLstd450SMin:
    case GLSLstd450SMax:
    case GLSLstd450SClamp:
    case GLSLstd450UMin:
    case GLSLstd450UMax:
    case GLSLstd450UClamp:
      // TODO(dneto): FindSMsb?
      // TODO(dneto): FindUMsb?
      return true;
    default:
      break;
  }
  return false;
}

// @param a SPIR-V decoration
// @return true when the given decoration is a pipeline decoration other than a
// bulitin variable.
bool IsPipelineDecoration(const Decoration& deco) {
  if (deco.size() < 1) {
    return false;
  }
  switch (deco[0]) {
    case SpvDecorationLocation:
    case SpvDecorationFlat:
    case SpvDecorationNoPerspective:
    case SpvDecorationCentroid:
    case SpvDecorationSample:
      return true;
    default:
      break;
  }
  return false;
}

}  // namespace

TypedExpression::TypedExpression() = default;

TypedExpression::TypedExpression(const TypedExpression&) = default;

TypedExpression& TypedExpression::operator=(const TypedExpression&) = default;

TypedExpression::TypedExpression(const Type* type_in,
                                 const ast::Expression* expr_in)
    : type(type_in), expr(expr_in) {}

ParserImpl::ParserImpl(const std::vector<uint32_t>& spv_binary)
    : Reader(),
      spv_binary_(spv_binary),
      fail_stream_(&success_, &errors_),
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
    success_ = false;
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

Program ParserImpl::program() {
  // TODO(dneto): Should we clear out spv_binary_ here, to reduce
  // memory usage?
  return tint::Program(std::move(builder_));
}

const Type* ParserImpl::ConvertType(uint32_t type_id, PtrAs ptr_as) {
  if (!success_) {
    return nullptr;
  }

  if (type_mgr_ == nullptr) {
    Fail() << "ConvertType called when the internal module has not been built";
    return nullptr;
  }

  auto* spirv_type = type_mgr_->GetType(type_id);
  if (spirv_type == nullptr) {
    Fail() << "ID is not a SPIR-V type: " << type_id;
    return nullptr;
  }

  switch (spirv_type->kind()) {
    case spvtools::opt::analysis::Type::kVoid:
      return ty_.Void();
    case spvtools::opt::analysis::Type::kBool:
      return ty_.Bool();
    case spvtools::opt::analysis::Type::kInteger:
      return ConvertType(spirv_type->AsInteger());
    case spvtools::opt::analysis::Type::kFloat:
      return ConvertType(spirv_type->AsFloat());
    case spvtools::opt::analysis::Type::kVector:
      return ConvertType(spirv_type->AsVector());
    case spvtools::opt::analysis::Type::kMatrix:
      return ConvertType(spirv_type->AsMatrix());
    case spvtools::opt::analysis::Type::kRuntimeArray:
      return ConvertType(type_id, spirv_type->AsRuntimeArray());
    case spvtools::opt::analysis::Type::kArray:
      return ConvertType(type_id, spirv_type->AsArray());
    case spvtools::opt::analysis::Type::kStruct:
      return ConvertType(type_id, spirv_type->AsStruct());
    case spvtools::opt::analysis::Type::kPointer:
      return ConvertType(type_id, ptr_as, spirv_type->AsPointer());
    case spvtools::opt::analysis::Type::kFunction:
      // Tint doesn't have a Function type.
      // We need to convert the result type and parameter types.
      // But the SPIR-V defines those before defining the function
      // type.  No further work is required here.
      return nullptr;
    case spvtools::opt::analysis::Type::kSampler:
    case spvtools::opt::analysis::Type::kSampledImage:
    case spvtools::opt::analysis::Type::kImage:
      // Fake it for sampler and texture types.  These are handled in an
      // entirely different way.
      return ty_.Void();
    default:
      break;
  }

  Fail() << "unknown SPIR-V type with ID " << type_id << ": "
         << def_use_mgr_->GetDef(type_id)->PrettyPrint();
  return nullptr;
}

DecorationList ParserImpl::GetDecorationsFor(uint32_t id) const {
  DecorationList result;
  const auto& decorations = deco_mgr_->GetDecorationsFor(id, true);
  std::unordered_set<uint32_t> visited;
  for (const auto* inst : decorations) {
    if (inst->opcode() != SpvOpDecorate) {
      continue;
    }
    // Example: OpDecorate %struct_id Block
    // Example: OpDecorate %array_ty ArrayStride 16
    auto decoration_kind = inst->GetSingleWordInOperand(1);
    switch (decoration_kind) {
      // Restrict and RestrictPointer have no effect in graphics APIs.
      case SpvDecorationRestrict:
      case SpvDecorationRestrictPointer:
        break;
      default:
        if (visited.emplace(decoration_kind).second) {
          std::vector<uint32_t> inst_as_words;
          inst->ToBinaryWithoutAttachedDebugInsts(&inst_as_words);
          Decoration d(inst_as_words.begin() + 2, inst_as_words.end());
          result.push_back(d);
        }
        break;
    }
  }
  return result;
}

DecorationList ParserImpl::GetDecorationsForMember(
    uint32_t id,
    uint32_t member_index) const {
  DecorationList result;
  const auto& decorations = deco_mgr_->GetDecorationsFor(id, true);
  std::unordered_set<uint32_t> visited;
  for (const auto* inst : decorations) {
    // Example: OpMemberDecorate %struct_id 1 Offset 16
    if ((inst->opcode() != SpvOpMemberDecorate) ||
        (inst->GetSingleWordInOperand(1) != member_index)) {
      continue;
    }
    auto decoration_kind = inst->GetSingleWordInOperand(2);
    switch (decoration_kind) {
      // Restrict and RestrictPointer have no effect in graphics APIs.
      case SpvDecorationRestrict:
      case SpvDecorationRestrictPointer:
        break;
      default:
        if (visited.emplace(decoration_kind).second) {
          std::vector<uint32_t> inst_as_words;
          inst->ToBinaryWithoutAttachedDebugInsts(&inst_as_words);
          Decoration d(inst_as_words.begin() + 3, inst_as_words.end());
          result.push_back(d);
        }
    }
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

ast::AttributeList ParserImpl::ConvertMemberDecoration(
    uint32_t struct_type_id,
    uint32_t member_index,
    const Type* member_ty,
    const Decoration& decoration) {
  if (decoration.empty()) {
    Fail() << "malformed SPIR-V decoration: it's empty";
    return {};
  }
  switch (decoration[0]) {
    case SpvDecorationOffset:
      if (decoration.size() != 2) {
        Fail()
            << "malformed Offset decoration: expected 1 literal operand, has "
            << decoration.size() - 1 << ": member " << member_index << " of "
            << ShowType(struct_type_id);
        return {};
      }
      return {
          create<ast::StructMemberOffsetAttribute>(Source{}, decoration[1]),
      };
    case SpvDecorationNonReadable:
      // WGSL doesn't have a member decoration for this.  Silently drop it.
      return {};
    case SpvDecorationNonWritable:
      // WGSL doesn't have a member decoration for this.
      return {};
    case SpvDecorationColMajor:
      // WGSL only supports column major matrices.
      return {};
    case SpvDecorationRelaxedPrecision:
      // WGSL doesn't support relaxed precision.
      return {};
    case SpvDecorationRowMajor:
      Fail() << "WGSL does not support row-major matrices: can't "
                "translate member "
             << member_index << " of " << ShowType(struct_type_id);
      return {};
    case SpvDecorationMatrixStride: {
      if (decoration.size() != 2) {
        Fail() << "malformed MatrixStride decoration: expected 1 literal "
                  "operand, has "
               << decoration.size() - 1 << ": member " << member_index << " of "
               << ShowType(struct_type_id);
        return {};
      }
      uint32_t stride = decoration[1];
      auto* ty = member_ty->UnwrapAlias();
      while (auto* arr = ty->As<Array>()) {
        ty = arr->type->UnwrapAlias();
      }
      auto* mat = ty->As<Matrix>();
      if (!mat) {
        Fail() << "MatrixStride cannot be applied to type " << ty->String();
        return {};
      }
      uint32_t natural_stride = (mat->rows == 2) ? 8 : 16;
      if (stride == natural_stride) {
        return {};  // Decoration matches the natural stride for the matrix
      }
      if (!member_ty->Is<Matrix>()) {
        Fail() << "custom matrix strides not currently supported on array of "
                  "matrices";
        return {};
      }
      return {
          create<ast::StrideAttribute>(Source{}, decoration[1]),
          builder_.ASTNodes().Create<ast::DisableValidationAttribute>(
              builder_.ID(), ast::DisabledValidation::kIgnoreStrideAttribute),
      };
    }
    default:
      // TODO(dneto): Support the remaining member decorations.
      break;
  }
  Fail() << "unhandled member decoration: " << decoration[0] << " on member "
         << member_index << " of " << ShowType(struct_type_id);
  return {};
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

  topologically_ordered_functions_ =
      FunctionTraverser(*module_).TopologicallyOrderedFunctions();

  return success_;
}

void ParserImpl::ResetInternalModule() {
  ir_context_.reset(nullptr);
  module_ = nullptr;
  def_use_mgr_ = nullptr;
  constant_mgr_ = nullptr;
  type_mgr_ = nullptr;
  deco_mgr_ = nullptr;

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
  Source::Location instruction_number{};

  // Has there been an OpLine since the last OpNoLine or start of the module?
  bool in_op_line_scope = false;
  // The source location provided by the most recent OpLine instruction.
  Source::Location op_line_source{};
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
  return Source{where->second };
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
  if (!RegisterWorkgroupSizeBuiltin()) {
    return false;
  }
  if (!RegisterEntryPoints()) {
    return false;
  }
  if (!RegisterHandleUsage()) {
    return false;
  }
  if (!RegisterTypes()) {
    return false;
  }
  if (!RejectInvalidPointerRoots()) {
    return false;
  }
  if (!EmitScalarSpecConstants()) {
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
      glsl_std_450_imports_.insert(import.result_id());
    } else if (name.find("NonSemantic.") == 0) {
      ignored_imports_.insert(import.result_id());
    } else {
      return Fail() << "Unrecognized extended instruction set: " << name;
    }
  }
  return true;
}

bool ParserImpl::IsGlslExtendedInstruction(
    const spvtools::opt::Instruction& inst) const {
  return (inst.opcode() == SpvOpExtInst) &&
         (glsl_std_450_imports_.count(inst.GetSingleWordInOperand(0)) > 0);
}

bool ParserImpl::IsIgnoredExtendedInstruction(
    const spvtools::opt::Instruction& inst) const {
  return (inst.opcode() == SpvOpExtInst) &&
         (ignored_imports_.count(inst.GetSingleWordInOperand(0)) > 0);
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

    // This translator requires the entry point to be a valid WGSL identifier.
    // Allowing otherwise leads to difficulties in that the programmer needs
    // to get a mapping from their original entry point name to the WGSL name,
    // and we don't have a good mechanism for that.
    if (!IsValidIdentifier(name)) {
      return Fail() << "entry point name is not a valid WGSL identifier: "
                    << name;
    }

    // SPIR-V allows a single function to be the implementation for more
    // than one entry point.  In the common case, it's one-to-one, and we should
    // try to name the function after the entry point.  Otherwise, give the
    // function a name automatically derived from the entry point name.
    namer_.SuggestSanitizedName(function_id, name);

    // There is another many-to-one relationship to take care of:  In SPIR-V
    // the same name can be used for multiple entry points, provided they are
    // for different shader stages. Take action now to ensure we can use the
    // entry point name later on, and not have it taken for another identifier
    // by an accidental collision with a derived name made for a different ID.
    if (!namer_.IsRegistered(name)) {
      // The entry point name is "unoccupied" becase an earlier entry point
      // grabbed the slot for the function that implements both entry points.
      // Register this new entry point's name, to avoid accidental collisions
      // with a future generated ID.
      if (!namer_.RegisterWithoutId(name)) {
        return false;
      }
    }
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

bool ParserImpl::IsValidIdentifier(const std::string& str) {
  if (str.empty()) {
    return false;
  }
  std::locale c_locale("C");
  if (str[0] == '_') {
    if (str.length() == 1u || str[1] == '_') {
      // https://www.w3.org/TR/WGSL/#identifiers
      // must not be '_' (a single underscore)
      // must not start with two underscores
      return false;
    }
  } else if (!std::isalpha(str[0], c_locale)) {
    return false;
  }
  for (const char& ch : str) {
    if ((ch != '_') && !std::isalnum(ch, c_locale)) {
      return false;
    }
  }
  return true;
}

bool ParserImpl::RegisterWorkgroupSizeBuiltin() {
  WorkgroupSizeInfo& info = workgroup_size_builtin_;
  for (const spvtools::opt::Instruction& inst : module_->annotations()) {
    if (inst.opcode() != SpvOpDecorate) {
      continue;
    }
    if (inst.GetSingleWordInOperand(1) != SpvDecorationBuiltIn) {
      continue;
    }
    if (inst.GetSingleWordInOperand(2) != SpvBuiltInWorkgroupSize) {
      continue;
    }
    info.id = inst.GetSingleWordInOperand(0);
  }
  if (info.id == 0) {
    return true;
  }
  // Gather the values.
  const spvtools::opt::Instruction* composite_def =
      def_use_mgr_->GetDef(info.id);
  if (!composite_def) {
    return Fail() << "Invalid WorkgroupSize builtin value";
  }
  // SPIR-V validation checks that the result is a 3-element vector of 32-bit
  // integer scalars (signed or unsigned).  Rely on validation to check the
  // type.  In theory the instruction could be OpConstantNull and still
  // pass validation, but that would be non-sensical.  Be a little more
  // stringent here and check for specific opcodes.  WGSL does not support
  // const-expr yet, so avoid supporting OpSpecConstantOp here.
  // TODO(dneto): See https://github.com/gpuweb/gpuweb/issues/1272 for WGSL
  // const_expr proposals.
  if ((composite_def->opcode() != SpvOpSpecConstantComposite &&
       composite_def->opcode() != SpvOpConstantComposite)) {
    return Fail() << "Invalid WorkgroupSize builtin.  Expected 3-element "
                     "OpSpecConstantComposite or OpConstantComposite:  "
                  << composite_def->PrettyPrint();
  }
  info.type_id = composite_def->type_id();
  // Extract the component type from the vector type.
  info.component_type_id =
      def_use_mgr_->GetDef(info.type_id)->GetSingleWordInOperand(0);

  /// Sets the ID and value of the index'th member of the composite constant.
  /// Returns false and emits a diagnostic on error.
  auto set_param = [this, composite_def](uint32_t* id_ptr, uint32_t* value_ptr,
                                         int index) -> bool {
    const auto id = composite_def->GetSingleWordInOperand(index);
    const auto* def = def_use_mgr_->GetDef(id);
    if (!def ||
        (def->opcode() != SpvOpSpecConstant &&
         def->opcode() != SpvOpConstant) ||
        (def->NumInOperands() != 1)) {
      return Fail() << "invalid component " << index << " of workgroupsize "
                    << (def ? def->PrettyPrint()
                            : std::string("no definition"));
    }
    *id_ptr = id;
    // Use the default value of a spec constant.
    *value_ptr = def->GetSingleWordInOperand(0);
    return true;
  };

  return set_param(&info.x_id, &info.x_value, 0) &&
         set_param(&info.y_id, &info.y_value, 1) &&
         set_param(&info.z_id, &info.z_value, 2);
}

bool ParserImpl::RegisterEntryPoints() {
  // Mapping from entry point ID to GridSize computed from LocalSize
  // decorations.
  std::unordered_map<uint32_t, GridSize> local_size;
  for (const spvtools::opt::Instruction& inst : module_->execution_modes()) {
    auto mode = static_cast<SpvExecutionMode>(inst.GetSingleWordInOperand(1));
    if (mode == SpvExecutionModeLocalSize) {
      if (inst.NumInOperands() != 5) {
        // This won't even get past SPIR-V binary parsing.
        return Fail() << "invalid LocalSize execution mode: "
                      << inst.PrettyPrint();
      }
      uint32_t function_id = inst.GetSingleWordInOperand(0);
      local_size[function_id] = GridSize{inst.GetSingleWordInOperand(2),
                                         inst.GetSingleWordInOperand(3),
                                         inst.GetSingleWordInOperand(4)};
    }
  }

  for (const spvtools::opt::Instruction& entry_point :
       module_->entry_points()) {
    const auto stage = SpvExecutionModel(entry_point.GetSingleWordInOperand(0));
    const uint32_t function_id = entry_point.GetSingleWordInOperand(1);

    const std::string ep_name = entry_point.GetOperand(2).AsString();
    if (!IsValidIdentifier(ep_name)) {
      return Fail() << "entry point name is not a valid WGSL identifier: "
                    << ep_name;
    }

    bool owns_inner_implementation = false;
    std::string inner_implementation_name;

    auto where = function_to_ep_info_.find(function_id);
    if (where == function_to_ep_info_.end()) {
      // If this is the first entry point to have function_id as its
      // implementation, then this entry point is responsible for generating
      // the inner implementation.
      owns_inner_implementation = true;
      inner_implementation_name = namer_.MakeDerivedName(ep_name);
    } else {
      // Reuse the inner implementation owned by the first entry point.
      inner_implementation_name = where->second[0].inner_name;
    }
    TINT_ASSERT(Reader, !inner_implementation_name.empty());
    TINT_ASSERT(Reader, ep_name != inner_implementation_name);

    utils::UniqueVector<uint32_t> inputs;
    utils::UniqueVector<uint32_t> outputs;
    for (unsigned iarg = 3; iarg < entry_point.NumInOperands(); iarg++) {
      const uint32_t var_id = entry_point.GetSingleWordInOperand(iarg);
      if (const auto* var_inst = def_use_mgr_->GetDef(var_id)) {
        switch (SpvStorageClass(var_inst->GetSingleWordInOperand(0))) {
          case SpvStorageClassInput:
            inputs.add(var_id);
            break;
          case SpvStorageClassOutput:
            outputs.add(var_id);
            break;
          default:
            break;
        }
      }
    }
    // Save the lists, in ID-sorted order.
    std::vector<uint32_t> sorted_inputs(inputs);
    std::sort(sorted_inputs.begin(), sorted_inputs.end());
    std::vector<uint32_t> sorted_outputs(outputs);
    std::sort(sorted_outputs.begin(), sorted_outputs.end());

    const auto ast_stage = enum_converter_.ToPipelineStage(stage);
    GridSize wgsize;
    if (ast_stage == ast::PipelineStage::kCompute) {
      if (workgroup_size_builtin_.id) {
        // Store the default values.
        // WGSL allows specializing these, but this code doesn't support that
        // yet. https://github.com/gpuweb/gpuweb/issues/1442
        wgsize = GridSize{workgroup_size_builtin_.x_value,
                          workgroup_size_builtin_.y_value,
                          workgroup_size_builtin_.z_value};
      } else {
        // Use the LocalSize execution mode.  This is the second choice.
        auto where_local_size = local_size.find(function_id);
        if (where_local_size != local_size.end()) {
          wgsize = where_local_size->second;
        }
      }
    }
    function_to_ep_info_[function_id].emplace_back(
        ep_name, ast_stage, owns_inner_implementation,
        inner_implementation_name, std::move(sorted_inputs),
        std::move(sorted_outputs), wgsize);
  }

  // The enum conversion could have failed, so return the existing status value.
  return success_;
}

const Type* ParserImpl::ConvertType(
    const spvtools::opt::analysis::Integer* int_ty) {
  if (int_ty->width() == 32) {
    return int_ty->IsSigned() ? static_cast<const Type*>(ty_.I32())
                              : static_cast<const Type*>(ty_.U32());
  }
  Fail() << "unhandled integer width: " << int_ty->width();
  return nullptr;
}

const Type* ParserImpl::ConvertType(
    const spvtools::opt::analysis::Float* float_ty) {
  if (float_ty->width() == 32) {
    return ty_.F32();
  }
  Fail() << "unhandled float width: " << float_ty->width();
  return nullptr;
}

const Type* ParserImpl::ConvertType(
    const spvtools::opt::analysis::Vector* vec_ty) {
  const auto num_elem = vec_ty->element_count();
  auto* ast_elem_ty = ConvertType(type_mgr_->GetId(vec_ty->element_type()));
  if (ast_elem_ty == nullptr) {
    return ast_elem_ty;
  }
  return ty_.Vector(ast_elem_ty, num_elem);
}

const Type* ParserImpl::ConvertType(
    const spvtools::opt::analysis::Matrix* mat_ty) {
  const auto* vec_ty = mat_ty->element_type()->AsVector();
  const auto* scalar_ty = vec_ty->element_type();
  const auto num_rows = vec_ty->element_count();
  const auto num_columns = mat_ty->element_count();
  auto* ast_scalar_ty = ConvertType(type_mgr_->GetId(scalar_ty));
  if (ast_scalar_ty == nullptr) {
    return nullptr;
  }
  return ty_.Matrix(ast_scalar_ty, num_columns, num_rows);
}

const Type* ParserImpl::ConvertType(
    uint32_t type_id,
    const spvtools::opt::analysis::RuntimeArray* rtarr_ty) {
  auto* ast_elem_ty = ConvertType(type_mgr_->GetId(rtarr_ty->element_type()));
  if (ast_elem_ty == nullptr) {
    return nullptr;
  }
  uint32_t array_stride = 0;
  if (!ParseArrayDecorations(rtarr_ty, &array_stride)) {
    return nullptr;
  }
  const Type* result = ty_.Array(ast_elem_ty, 0, array_stride);
  return MaybeGenerateAlias(type_id, rtarr_ty, result);
}

const Type* ParserImpl::ConvertType(
    uint32_t type_id,
    const spvtools::opt::analysis::Array* arr_ty) {
  // Get the element type. The SPIR-V optimizer's types representation
  // deduplicates array types that have the same parameterization.
  // We don't want that deduplication, so get the element type from
  // the SPIR-V type directly.
  const auto* inst = def_use_mgr_->GetDef(type_id);
  const auto elem_type_id = inst->GetSingleWordInOperand(0);
  auto* ast_elem_ty = ConvertType(elem_type_id);
  if (ast_elem_ty == nullptr) {
    return nullptr;
  }
  // Get the length.
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
  uint32_t array_stride = 0;
  if (!ParseArrayDecorations(arr_ty, &array_stride)) {
    return nullptr;
  }
  if (remap_buffer_block_type_.count(elem_type_id)) {
    remap_buffer_block_type_.insert(type_mgr_->GetId(arr_ty));
  }
  const Type* result =
      ty_.Array(ast_elem_ty, static_cast<uint32_t>(num_elem), array_stride);
  return MaybeGenerateAlias(type_id, arr_ty, result);
}

bool ParserImpl::ParseArrayDecorations(
    const spvtools::opt::analysis::Type* spv_type,
    uint32_t* array_stride) {
  *array_stride = 0;  // Implicit stride case.
  const auto type_id = type_mgr_->GetId(spv_type);
  for (auto& decoration : this->GetDecorationsFor(type_id)) {
    if (decoration.size() == 2 && decoration[0] == SpvDecorationArrayStride) {
      const auto stride = decoration[1];
      if (stride == 0) {
        return Fail() << "invalid array type ID " << type_id
                      << ": ArrayStride can't be 0";
      }
      *array_stride = stride;
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

const Type* ParserImpl::ConvertType(
    uint32_t type_id,
    const spvtools::opt::analysis::Struct* struct_ty) {
  // Compute the struct decoration.
  auto struct_decorations = this->GetDecorationsFor(type_id);
  if (struct_decorations.size() == 1) {
    const auto decoration = struct_decorations[0][0];
    if (decoration == SpvDecorationBufferBlock) {
      remap_buffer_block_type_.insert(type_id);
    } else if (decoration != SpvDecorationBlock) {
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
  if (members.empty()) {
    Fail() << "WGSL does not support empty structures. can't convert type: "
           << def_use_mgr_->GetDef(type_id)->PrettyPrint();
    return nullptr;
  }
  TypeList ast_member_types;
  unsigned num_non_writable_members = 0;
  for (uint32_t member_index = 0; member_index < members.size();
       ++member_index) {
    const auto member_type_id = type_mgr_->GetId(members[member_index]);
    auto* ast_member_ty = ConvertType(member_type_id);
    if (ast_member_ty == nullptr) {
      // Already emitted diagnostics.
      return nullptr;
    }

    ast_member_types.emplace_back(ast_member_ty);

    // Scan member for built-in decorations. Some vertex built-ins are handled
    // specially, and should not generate a structure member.
    bool create_ast_member = true;
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
            builtin_position_.position_member_index = member_index;
            builtin_position_.position_member_type_id = member_type_id;
            create_ast_member = false;  // Not part of the WGSL structure.
            break;
          case SpvBuiltInPointSize:  // not supported in WGSL, but ignore
            builtin_position_.pointsize_member_index = member_index;
            create_ast_member = false;  // Not part of the WGSL structure.
            break;
          case SpvBuiltInClipDistance:  // not supported in WGSL
          case SpvBuiltInCullDistance:  // not supported in WGSL
            create_ast_member = false;  // Not part of the WGSL structure.
            break;
          default:
            Fail() << "unrecognized builtin " << decoration[1];
            return nullptr;
        }
      }
    }
    if (!create_ast_member) {
      // This member is decorated as a built-in, and is handled specially.
      continue;
    }

    bool is_non_writable = false;
    ast::AttributeList ast_member_decorations;
    for (auto& decoration : GetDecorationsForMember(type_id, member_index)) {
      if (IsPipelineDecoration(decoration)) {
        // IO decorations are handled when emitting the entry point.
        continue;
      } else if (decoration[0] == SpvDecorationNonWritable) {
        // WGSL doesn't represent individual members as non-writable. Instead,
        // apply the ReadOnly access control to the containing struct if all
        // the members are non-writable.
        is_non_writable = true;
      } else {
        auto decos = ConvertMemberDecoration(type_id, member_index,
                                             ast_member_ty, decoration);
        for (auto* deco : decos) {
          ast_member_decorations.emplace_back(deco);
        }
        if (!success_) {
          return nullptr;
        }
      }
    }

    if (is_non_writable) {
      // Count a member as non-writable only once, no matter how many
      // NonWritable decorations are applied to it.
      ++num_non_writable_members;
    }
    const auto member_name = namer_.GetMemberName(type_id, member_index);
    auto* ast_struct_member = create<ast::StructMember>(
        Source{}, builder_.Symbols().Register(member_name),
        ast_member_ty->Build(builder_), std::move(ast_member_decorations));
    ast_members.push_back(ast_struct_member);
  }

  if (ast_members.empty()) {
    // All members were likely built-ins. Don't generate an empty AST structure.
    return nullptr;
  }

  namer_.SuggestSanitizedName(type_id, "S");

  auto name = namer_.GetName(type_id);

  // Now make the struct.
  auto sym = builder_.Symbols().Register(name);
  auto* ast_struct = create<ast::Struct>(Source{}, sym, std::move(ast_members),
                                         ast::AttributeList());
  if (num_non_writable_members == members.size()) {
    read_only_struct_types_.insert(ast_struct->name);
  }
  AddTypeDecl(sym, ast_struct);
  const auto* result = ty_.Struct(sym, std::move(ast_member_types));
  struct_id_for_symbol_[sym] = type_id;
  return result;
}

void ParserImpl::AddTypeDecl(Symbol name, const ast::TypeDecl* decl) {
  auto iter = declared_types_.insert(name);
  if (iter.second) {
    builder_.AST().AddTypeDecl(decl);
  }
}

const Type* ParserImpl::ConvertType(uint32_t type_id,
                                    PtrAs ptr_as,
                                    const spvtools::opt::analysis::Pointer*) {
  const auto* inst = def_use_mgr_->GetDef(type_id);
  const auto pointee_type_id = inst->GetSingleWordInOperand(1);
  const auto storage_class = SpvStorageClass(inst->GetSingleWordInOperand(0));

  if (pointee_type_id == builtin_position_.struct_type_id) {
    builtin_position_.pointer_type_id = type_id;
    // Pipeline IO builtins map to private variables.
    builtin_position_.storage_class = SpvStorageClassPrivate;
    return nullptr;
  }
  auto* ast_elem_ty = ConvertType(pointee_type_id, PtrAs::Ptr);
  if (ast_elem_ty == nullptr) {
    Fail() << "SPIR-V pointer type with ID " << type_id
           << " has invalid pointee type " << pointee_type_id;
    return nullptr;
  }

  auto ast_storage_class = enum_converter_.ToStorageClass(storage_class);
  if (ast_storage_class == ast::StorageClass::kInvalid) {
    Fail() << "SPIR-V pointer type with ID " << type_id
           << " has invalid storage class "
           << static_cast<uint32_t>(storage_class);
    return nullptr;
  }
  if (ast_storage_class == ast::StorageClass::kUniform &&
      remap_buffer_block_type_.count(pointee_type_id)) {
    ast_storage_class = ast::StorageClass::kStorage;
    remap_buffer_block_type_.insert(type_id);
  }

  // Pipeline input and output variables map to private variables.
  if (ast_storage_class == ast::StorageClass::kInput ||
      ast_storage_class == ast::StorageClass::kOutput) {
    ast_storage_class = ast::StorageClass::kPrivate;
  }
  switch (ptr_as) {
    case PtrAs::Ref:
      return ty_.Reference(ast_elem_ty, ast_storage_class);
    case PtrAs::Ptr:
      return ty_.Pointer(ast_elem_ty, ast_storage_class);
  }
  Fail() << "invalid value for ptr_as: " << static_cast<int>(ptr_as);
  return nullptr;
}

bool ParserImpl::RegisterTypes() {
  if (!success_) {
    return false;
  }

  // First record the structure types that should have a `block` decoration
  // in WGSL. In particular, exclude user-defined pipeline IO in a
  // block-decorated struct.
  for (const auto& type_or_value : module_->types_values()) {
    if (type_or_value.opcode() != SpvOpVariable) {
      continue;
    }
    const auto& var = type_or_value;
    const auto spirv_storage_class =
        SpvStorageClass(var.GetSingleWordInOperand(0));
    if ((spirv_storage_class != SpvStorageClassStorageBuffer) &&
        (spirv_storage_class != SpvStorageClassUniform)) {
      continue;
    }
    const auto* ptr_type = def_use_mgr_->GetDef(var.type_id());
    if (ptr_type->opcode() != SpvOpTypePointer) {
      return Fail() << "OpVariable type expected to be a pointer: "
                    << var.PrettyPrint();
    }
    const auto* store_type =
        def_use_mgr_->GetDef(ptr_type->GetSingleWordInOperand(1));
    if (store_type->opcode() == SpvOpTypeStruct) {
      struct_types_for_buffers_.insert(store_type->result_id());
    } else {
      Fail() << "WGSL does not support arrays of buffers: "
             << var.PrettyPrint();
    }
  }

  // Now convert each type.
  for (auto& type_or_const : module_->types_values()) {
    const auto* type = type_mgr_->GetType(type_or_const.result_id());
    if (type == nullptr) {
      continue;
    }
    ConvertType(type_or_const.result_id());
  }
  // Manufacture a type for the gl_Position variable if we have to.
  if ((builtin_position_.struct_type_id != 0) &&
      (builtin_position_.position_member_pointer_type_id == 0)) {
    builtin_position_.position_member_pointer_type_id =
        type_mgr_->FindPointerToType(builtin_position_.position_member_type_id,
                                     builtin_position_.storage_class);
    ConvertType(builtin_position_.position_member_pointer_type_id);
  }
  return success_;
}

bool ParserImpl::RejectInvalidPointerRoots() {
  if (!success_) {
    return false;
  }
  for (auto& inst : module_->types_values()) {
    if (const auto* result_type = type_mgr_->GetType(inst.type_id())) {
      if (result_type->AsPointer()) {
        switch (inst.opcode()) {
          case SpvOpVariable:
            // This is the only valid case.
            break;
          case SpvOpUndef:
            return Fail() << "undef pointer is not valid: "
                          << inst.PrettyPrint();
          case SpvOpConstantNull:
            return Fail() << "null pointer is not valid: "
                          << inst.PrettyPrint();
          default:
            return Fail() << "module-scope pointer is not valid: "
                          << inst.PrettyPrint();
        }
      }
    }
  }
  return success();
}

bool ParserImpl::EmitScalarSpecConstants() {
  if (!success_) {
    return false;
  }
  // Generate a module-scope const declaration for each instruction
  // that is OpSpecConstantTrue, OpSpecConstantFalse, or OpSpecConstant.
  for (auto& inst : module_->types_values()) {
    // These will be populated for a valid scalar spec constant.
    const Type* ast_type = nullptr;
    ast::LiteralExpression* ast_expr = nullptr;

    switch (inst.opcode()) {
      case SpvOpSpecConstantTrue:
      case SpvOpSpecConstantFalse: {
        ast_type = ConvertType(inst.type_id());
        ast_expr = create<ast::BoolLiteralExpression>(
            Source{}, inst.opcode() == SpvOpSpecConstantTrue);
        break;
      }
      case SpvOpSpecConstant: {
        ast_type = ConvertType(inst.type_id());
        const uint32_t literal_value = inst.GetSingleWordInOperand(0);
        if (ast_type->Is<I32>()) {
          ast_expr = create<ast::SintLiteralExpression>(
              Source{}, static_cast<int32_t>(literal_value));
        } else if (ast_type->Is<U32>()) {
          ast_expr = create<ast::UintLiteralExpression>(
              Source{}, static_cast<uint32_t>(literal_value));
        } else if (ast_type->Is<F32>()) {
          float float_value;
          // Copy the bits so we can read them as a float.
          std::memcpy(&float_value, &literal_value, sizeof(float_value));
          ast_expr = create<ast::FloatLiteralExpression>(Source{}, float_value);
        } else {
          return Fail() << " invalid result type for OpSpecConstant "
                        << inst.PrettyPrint();
        }
        break;
      }
      default:
        break;
    }
    if (ast_type && ast_expr) {
      ast::AttributeList spec_id_decos;
      for (const auto& deco : GetDecorationsFor(inst.result_id())) {
        if ((deco.size() == 2) && (deco[0] == SpvDecorationSpecId)) {
          const uint32_t id = deco[1];
          if (id > 65535) {
            return Fail() << "SpecId too large. WGSL override IDs must be "
                             "between 0 and 65535: ID %"
                          << inst.result_id() << " has SpecId " << id;
          }
          auto* cid = create<ast::IdAttribute>(Source{}, id);
          spec_id_decos.push_back(cid);
          break;
        }
      }
      auto* ast_var =
          MakeVariable(inst.result_id(), ast::StorageClass::kNone, ast_type,
                       true, true, ast_expr, std::move(spec_id_decos));
      if (ast_var) {
        builder_.AST().AddGlobalVariable(ast_var);
        scalar_spec_constants_.insert(inst.result_id());
      }
    }
  }
  return success_;
}

const Type* ParserImpl::MaybeGenerateAlias(
    uint32_t type_id,
    const spvtools::opt::analysis::Type* type,
    const Type* ast_type) {
  if (!success_) {
    return nullptr;
  }

  // We only care about arrays, and runtime arrays.
  switch (type->kind()) {
    case spvtools::opt::analysis::Type::kRuntimeArray:
      // Runtime arrays are always decorated with ArrayStride so always get a
      // type alias.
      namer_.SuggestSanitizedName(type_id, "RTArr");
      break;
    case spvtools::opt::analysis::Type::kArray:
      // Only make a type aliase for arrays with decorations.
      if (GetDecorationsFor(type_id).empty()) {
        return ast_type;
      }
      namer_.SuggestSanitizedName(type_id, "Arr");
      break;
    default:
      // Ignore constants, and any other types.
      return ast_type;
  }
  auto* ast_underlying_type = ast_type;
  if (ast_underlying_type == nullptr) {
    Fail() << "internal error: no type registered for SPIR-V ID: " << type_id;
    return nullptr;
  }
  const auto name = namer_.GetName(type_id);
  const auto sym = builder_.Symbols().Register(name);
  auto* ast_alias_type =
      builder_.ty.alias(sym, ast_underlying_type->Build(builder_));

  // Record this new alias as the AST type for this SPIR-V ID.
  AddTypeDecl(sym, ast_alias_type);

  return ty_.Alias(sym, ast_underlying_type);
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
      builtin_position_.per_vertex_var_init_id =
          var.NumInOperands() > 1 ? var.GetSingleWordInOperand(1) : 0u;
      continue;
    }
    switch (enum_converter_.ToStorageClass(spirv_storage_class)) {
      case ast::StorageClass::kNone:
      case ast::StorageClass::kInput:
      case ast::StorageClass::kOutput:
      case ast::StorageClass::kUniform:
      case ast::StorageClass::kHandle:
      case ast::StorageClass::kStorage:
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
    const Type* ast_type = nullptr;
    if (spirv_storage_class == SpvStorageClassUniformConstant) {
      // These are opaque handles: samplers or textures
      ast_type = GetTypeForHandleVar(var);
      if (!ast_type) {
        return false;
      }
    } else {
      ast_type = ConvertType(type_id);
      if (ast_type == nullptr) {
        return Fail() << "internal error: failed to register Tint AST type for "
                         "SPIR-V type with ID: "
                      << var.type_id();
      }
      if (!ast_type->Is<Pointer>()) {
        return Fail() << "variable with ID " << var.result_id()
                      << " has non-pointer type " << var.type_id();
      }
    }

    auto* ast_store_type = ast_type->As<Pointer>()->type;
    auto ast_storage_class = ast_type->As<Pointer>()->storage_class;
    const ast::Expression* ast_constructor = nullptr;
    if (var.NumInOperands() > 1) {
      // SPIR-V initializers are always constants.
      // (OpenCL also allows the ID of an OpVariable, but we don't handle that
      // here.)
      ast_constructor =
          MakeConstantExpression(var.GetSingleWordInOperand(1)).expr;
    }
    auto* ast_var =
        MakeVariable(var.result_id(), ast_storage_class, ast_store_type, false,
                     false, ast_constructor, ast::AttributeList{});
    // TODO(dneto): initializers (a.k.a. constructor expression)
    if (ast_var) {
      builder_.AST().AddGlobalVariable(ast_var);
    }
  }

  // Emit gl_Position instead of gl_PerVertex
  if (builtin_position_.per_vertex_var_id) {
    // Make sure the variable has a name.
    namer_.SuggestSanitizedName(builtin_position_.per_vertex_var_id,
                                "gl_Position");
    const ast::Expression* ast_constructor = nullptr;
    if (builtin_position_.per_vertex_var_init_id) {
      // The initializer is complex.
      const auto* init =
          def_use_mgr_->GetDef(builtin_position_.per_vertex_var_init_id);
      switch (init->opcode()) {
        case SpvOpConstantComposite:
        case SpvOpSpecConstantComposite:
          ast_constructor = MakeConstantExpression(
                                init->GetSingleWordInOperand(
                                    builtin_position_.position_member_index))
                                .expr;
          break;
        default:
          return Fail() << "gl_PerVertex initializer too complex. only "
                           "OpCompositeConstruct and OpSpecConstantComposite "
                           "are supported: "
                        << init->PrettyPrint();
      }
    }
    auto* ast_var = MakeVariable(
        builtin_position_.per_vertex_var_id,
        enum_converter_.ToStorageClass(builtin_position_.storage_class),
        ConvertType(builtin_position_.position_member_type_id), false, false,
        ast_constructor, {});

    builder_.AST().AddGlobalVariable(ast_var);
  }
  return success_;
}

// @param var_id SPIR-V id of an OpVariable, assumed to be pointer
// to an array
// @returns the IntConstant for the size of the array, or nullptr
const spvtools::opt::analysis::IntConstant* ParserImpl::GetArraySize(
    uint32_t var_id) {
  auto* var = def_use_mgr_->GetDef(var_id);
  if (!var || var->opcode() != SpvOpVariable) {
    return nullptr;
  }
  auto* ptr_type = def_use_mgr_->GetDef(var->type_id());
  if (!ptr_type || ptr_type->opcode() != SpvOpTypePointer) {
    return nullptr;
  }
  auto* array_type = def_use_mgr_->GetDef(ptr_type->GetSingleWordInOperand(1));
  if (!array_type || array_type->opcode() != SpvOpTypeArray) {
    return nullptr;
  }
  auto* size = constant_mgr_->FindDeclaredConstant(
      array_type->GetSingleWordInOperand(1));
  if (!size) {
    return nullptr;
  }
  return size->AsIntConstant();
}

ast::Variable* ParserImpl::MakeVariable(uint32_t id,
                                        ast::StorageClass sc,
                                        const Type* storage_type,
                                        bool is_const,
                                        bool is_overridable,
                                        const ast::Expression* constructor,
                                        ast::AttributeList decorations) {
  if (storage_type == nullptr) {
    Fail() << "internal error: can't make ast::Variable for null type";
    return nullptr;
  }

  ast::Access access = ast::Access::kUndefined;
  if (sc == ast::StorageClass::kStorage) {
    bool read_only = false;
    if (auto* tn = storage_type->As<Named>()) {
      read_only = read_only_struct_types_.count(tn->name) > 0;
    }

    // Apply the access(read) or access(read_write) modifier.
    access = read_only ? ast::Access::kRead : ast::Access::kReadWrite;
  }

  // Handle variables (textures and samplers) are always in the handle
  // storage class, so we don't mention the storage class.
  if (sc == ast::StorageClass::kHandle) {
    sc = ast::StorageClass::kNone;
  }

  if (!ConvertDecorationsForVariable(id, &storage_type, &decorations,
                                     sc != ast::StorageClass::kPrivate)) {
    return nullptr;
  }

  std::string name = namer_.Name(id);

  // Note: we're constructing the variable here with the *storage* type,
  // regardless of whether this is a `let`, `override`, or `var` declaration.
  // `var` declarations will have a resolved type of ref<storage>, but at the
  // AST level all three are declared with the same type.
  return create<ast::Variable>(Source{}, builder_.Symbols().Register(name), sc,
                               access, storage_type->Build(builder_), is_const,
                               is_overridable, constructor, decorations);
}

bool ParserImpl::ConvertDecorationsForVariable(uint32_t id,
                                               const Type** store_type,
                                               ast::AttributeList* decorations,
                                               bool transfer_pipeline_io) {
  DecorationList non_builtin_pipeline_decorations;
  for (auto& deco : GetDecorationsFor(id)) {
    if (deco.empty()) {
      return Fail() << "malformed decoration on ID " << id << ": it is empty";
    }
    if (deco[0] == SpvDecorationBuiltIn) {
      if (deco.size() == 1) {
        return Fail() << "malformed BuiltIn decoration on ID " << id
                      << ": has no operand";
      }
      const auto spv_builtin = static_cast<SpvBuiltIn>(deco[1]);
      switch (spv_builtin) {
        case SpvBuiltInPointSize:
          special_builtins_[id] = spv_builtin;
          return false;  // This is not an error
        case SpvBuiltInSampleId:
        case SpvBuiltInVertexIndex:
        case SpvBuiltInInstanceIndex:
        case SpvBuiltInLocalInvocationId:
        case SpvBuiltInLocalInvocationIndex:
        case SpvBuiltInGlobalInvocationId:
        case SpvBuiltInWorkgroupId:
        case SpvBuiltInNumWorkgroups:
          // The SPIR-V variable may signed (because GLSL requires signed for
          // some of these), but WGSL requires unsigned.  Handle specially
          // so we always perform the conversion at load and store.
          special_builtins_[id] = spv_builtin;
          if (auto* forced_type = UnsignedTypeFor(*store_type)) {
            // Requires conversion and special handling in code generation.
            if (transfer_pipeline_io) {
              *store_type = forced_type;
            }
          }
          break;
        case SpvBuiltInSampleMask: {
          // In SPIR-V this is used for both input and output variable.
          // The SPIR-V variable has store type of array of integer scalar,
          // either signed or unsigned.
          // WGSL requires the store type to be u32.
          auto* size = GetArraySize(id);
          if (!size || size->GetZeroExtendedValue() != 1) {
            Fail() << "WGSL supports a sample mask of at most 32 bits. "
                      "SampleMask must be an array of 1 element.";
          }
          special_builtins_[id] = spv_builtin;
          if (transfer_pipeline_io) {
            *store_type = ty_.U32();
          }
          break;
        }
        default:
          break;
      }
      auto ast_builtin = enum_converter_.ToBuiltin(spv_builtin);
      if (ast_builtin == ast::Builtin::kNone) {
        // A diagnostic has already been emitted.
        return false;
      }
      if (transfer_pipeline_io) {
        decorations->emplace_back(
            create<ast::BuiltinAttribute>(Source{}, ast_builtin));
      }
    }
    if (transfer_pipeline_io && IsPipelineDecoration(deco)) {
      non_builtin_pipeline_decorations.push_back(deco);
    }
    if (deco[0] == SpvDecorationDescriptorSet) {
      if (deco.size() == 1) {
        return Fail() << "malformed DescriptorSet decoration on ID " << id
                      << ": has no operand";
      }
      decorations->emplace_back(create<ast::GroupAttribute>(Source{}, deco[1]));
    }
    if (deco[0] == SpvDecorationBinding) {
      if (deco.size() == 1) {
        return Fail() << "malformed Binding decoration on ID " << id
                      << ": has no operand";
      }
      decorations->emplace_back(
          create<ast::BindingAttribute>(Source{}, deco[1]));
    }
  }

  if (transfer_pipeline_io) {
    if (!ConvertPipelineDecorations(
            *store_type, non_builtin_pipeline_decorations, decorations)) {
      return false;
    }
  }

  return success();
}

DecorationList ParserImpl::GetMemberPipelineDecorations(
    const Struct& struct_type,
    int member_index) {
  // Yes, I could have used std::copy_if or std::copy_if.
  DecorationList result;
  for (const auto& deco : GetDecorationsForMember(
           struct_id_for_symbol_[struct_type.name], member_index)) {
    if (IsPipelineDecoration(deco)) {
      result.emplace_back(deco);
    }
  }
  return result;
}

const ast::Attribute* ParserImpl::SetLocation(
    ast::AttributeList* attributes,
    const ast::Attribute* replacement) {
  if (!replacement) {
    return nullptr;
  }
  for (auto*& attribute : *attributes) {
    if (attribute->Is<ast::LocationAttribute>()) {
      // Replace this location attribute with the replacement.
      // The old one doesn't leak because it's kept in the builder's AST node
      // list.
      const ast::Attribute* result = nullptr;
      result = attribute;
      attribute = replacement;
      return result;  // Assume there is only one such decoration.
    }
  }
  // The list didn't have a location. Add it.
  attributes->push_back(replacement);
  return nullptr;
}

bool ParserImpl::ConvertPipelineDecorations(const Type* store_type,
                                            const DecorationList& decorations,
                                            ast::AttributeList* attributes) {
  // Vulkan defaults to perspective-correct interpolation.
  ast::InterpolationType type = ast::InterpolationType::kPerspective;
  ast::InterpolationSampling sampling = ast::InterpolationSampling::kNone;

  for (const auto& deco : decorations) {
    TINT_ASSERT(Reader, deco.size() > 0);
    switch (deco[0]) {
      case SpvDecorationLocation:
        if (deco.size() != 2) {
          return Fail() << "malformed Location decoration on ID requires one "
                           "literal operand";
        }
        SetLocation(attributes,
                    create<ast::LocationAttribute>(Source{}, deco[1]));
        if (store_type->IsIntegerScalarOrVector()) {
          // Default to flat interpolation for integral user-defined IO types.
          type = ast::InterpolationType::kFlat;
        }
        break;
      case SpvDecorationFlat:
        type = ast::InterpolationType::kFlat;
        break;
      case SpvDecorationNoPerspective:
        if (store_type->IsIntegerScalarOrVector()) {
          // This doesn't capture the array or struct case.
          return Fail() << "NoPerspective is invalid on integral IO";
        }
        type = ast::InterpolationType::kLinear;
        break;
      case SpvDecorationCentroid:
        if (store_type->IsIntegerScalarOrVector()) {
          // This doesn't capture the array or struct case.
          return Fail()
                 << "Centroid interpolation sampling is invalid on integral IO";
        }
        sampling = ast::InterpolationSampling::kCentroid;
        break;
      case SpvDecorationSample:
        if (store_type->IsIntegerScalarOrVector()) {
          // This doesn't capture the array or struct case.
          return Fail()
                 << "Sample interpolation sampling is invalid on integral IO";
        }
        sampling = ast::InterpolationSampling::kSample;
        break;
      default:
        break;
    }
  }

  // Apply interpolation.
  if (type == ast::InterpolationType::kPerspective &&
      sampling == ast::InterpolationSampling::kNone) {
    // This is the default. Don't add a decoration.
  } else {
    attributes->emplace_back(create<ast::InterpolateAttribute>(type, sampling));
  }

  return success();
}

bool ParserImpl::CanMakeConstantExpression(uint32_t id) {
  if ((id == workgroup_size_builtin_.id) ||
      (id == workgroup_size_builtin_.x_id) ||
      (id == workgroup_size_builtin_.y_id) ||
      (id == workgroup_size_builtin_.z_id)) {
    return true;
  }
  const auto* inst = def_use_mgr_->GetDef(id);
  if (!inst) {
    return false;
  }
  if (inst->opcode() == SpvOpUndef) {
    return true;
  }
  return nullptr != constant_mgr_->FindDeclaredConstant(id);
}

TypedExpression ParserImpl::MakeConstantExpression(uint32_t id) {
  if (!success_) {
    return {};
  }

  // Handle the special cases for workgroup sizing.
  if (id == workgroup_size_builtin_.id) {
    auto x = MakeConstantExpression(workgroup_size_builtin_.x_id);
    auto y = MakeConstantExpression(workgroup_size_builtin_.y_id);
    auto z = MakeConstantExpression(workgroup_size_builtin_.z_id);
    auto* ast_type = ty_.Vector(x.type, 3);
    return {ast_type,
            builder_.Construct(Source{}, ast_type->Build(builder_),
                               ast::ExpressionList{x.expr, y.expr, z.expr})};
  } else if (id == workgroup_size_builtin_.x_id) {
    return MakeConstantExpressionForScalarSpirvConstant(
        Source{}, ConvertType(workgroup_size_builtin_.component_type_id),
        constant_mgr_->GetConstant(
            type_mgr_->GetType(workgroup_size_builtin_.component_type_id),
            {workgroup_size_builtin_.x_value}));
  } else if (id == workgroup_size_builtin_.y_id) {
    return MakeConstantExpressionForScalarSpirvConstant(
        Source{}, ConvertType(workgroup_size_builtin_.component_type_id),
        constant_mgr_->GetConstant(
            type_mgr_->GetType(workgroup_size_builtin_.component_type_id),
            {workgroup_size_builtin_.y_value}));
  } else if (id == workgroup_size_builtin_.z_id) {
    return MakeConstantExpressionForScalarSpirvConstant(
        Source{}, ConvertType(workgroup_size_builtin_.component_type_id),
        constant_mgr_->GetConstant(
            type_mgr_->GetType(workgroup_size_builtin_.component_type_id),
            {workgroup_size_builtin_.z_value}));
  }

  // Handle the general case where a constant is already registered
  // with the SPIR-V optimizer's analysis framework.
  const auto* inst = def_use_mgr_->GetDef(id);
  if (inst == nullptr) {
    Fail() << "ID " << id << " is not a registered instruction";
    return {};
  }
  auto source = GetSourceForInst(inst);

  // TODO(dneto): Handle spec constants too?

  auto* original_ast_type = ConvertType(inst->type_id());
  if (original_ast_type == nullptr) {
    return {};
  }

  switch (inst->opcode()) {
    case SpvOpUndef:  // Remap undef to null.
    case SpvOpConstantNull:
      return {original_ast_type, MakeNullValue(original_ast_type)};
    case SpvOpConstantTrue:
    case SpvOpConstantFalse:
    case SpvOpConstant: {
      const auto* spirv_const = constant_mgr_->FindDeclaredConstant(id);
      if (spirv_const == nullptr) {
        Fail() << "ID " << id << " is not a constant";
        return {};
      }
      return MakeConstantExpressionForScalarSpirvConstant(
          source, original_ast_type, spirv_const);
    }
    case SpvOpConstantComposite: {
      // Handle vector, matrix, array, and struct

      // Generate a composite from explicit components.
      ast::ExpressionList ast_components;
      if (!inst->WhileEachInId([&](const uint32_t* id_ref) -> bool {
            auto component = MakeConstantExpression(*id_ref);
            if (!component) {
              this->Fail() << "invalid constant with ID " << *id_ref;
              return false;
            }
            ast_components.emplace_back(component.expr);
            return true;
          })) {
        // We've already emitted a diagnostic.
        return {};
      }
      return {original_ast_type,
              builder_.Construct(source, original_ast_type->Build(builder_),
                                 std::move(ast_components))};
    }
    default:
      break;
  }
  Fail() << "unhandled constant instruction " << inst->PrettyPrint();
  return {};
}

TypedExpression ParserImpl::MakeConstantExpressionForScalarSpirvConstant(
    Source source,
    const Type* original_ast_type,
    const spvtools::opt::analysis::Constant* spirv_const) {
  auto* ast_type = original_ast_type->UnwrapAlias();

  // TODO(dneto): Note: NullConstant for int, uint, float map to a regular 0.
  // So canonicalization should map that way too.
  // Currently "null<type>" is missing from the WGSL parser.
  // See https://bugs.chromium.org/p/tint/issues/detail?id=34
  if (ast_type->Is<U32>()) {
    return {ty_.U32(),
            create<ast::UintLiteralExpression>(source, spirv_const->GetU32())};
  }
  if (ast_type->Is<I32>()) {
    return {ty_.I32(),
            create<ast::SintLiteralExpression>(source, spirv_const->GetS32())};
  }
  if (ast_type->Is<F32>()) {
    return {ty_.F32(), create<ast::FloatLiteralExpression>(
                           source, spirv_const->GetFloat())};
  }
  if (ast_type->Is<Bool>()) {
    const bool value = spirv_const->AsNullConstant()
                           ? false
                           : spirv_const->AsBoolConstant()->value();
    return {ty_.Bool(), create<ast::BoolLiteralExpression>(source, value)};
  }
  Fail() << "expected scalar constant";
  return {};
}

const ast::Expression* ParserImpl::MakeNullValue(const Type* type) {
  // TODO(dneto): Use the no-operands constructor syntax when it becomes
  // available in Tint.
  // https://github.com/gpuweb/gpuweb/issues/685
  // https://bugs.chromium.org/p/tint/issues/detail?id=34

  if (!type) {
    Fail() << "trying to create null value for a null type";
    return nullptr;
  }

  auto* original_type = type;
  type = type->UnwrapAlias();

  if (type->Is<Bool>()) {
    return create<ast::BoolLiteralExpression>(Source{}, false);
  }
  if (type->Is<U32>()) {
    return create<ast::UintLiteralExpression>(Source{}, 0u);
  }
  if (type->Is<I32>()) {
    return create<ast::SintLiteralExpression>(Source{}, 0);
  }
  if (type->Is<F32>()) {
    return create<ast::FloatLiteralExpression>(Source{}, 0.0f);
  }
  if (type->IsAnyOf<Vector, Matrix, Array>()) {
    return builder_.Construct(Source{}, type->Build(builder_));
  }
  if (auto* struct_ty = type->As<Struct>()) {
    ast::ExpressionList ast_components;
    for (auto* member : struct_ty->members) {
      ast_components.emplace_back(MakeNullValue(member));
    }
    return builder_.Construct(Source{}, original_type->Build(builder_),
                              std::move(ast_components));
  }
  Fail() << "can't make null value for type: " << type->TypeInfo().name;
  return nullptr;
}

TypedExpression ParserImpl::MakeNullExpression(const Type* type) {
  return {type, MakeNullValue(type)};
}

const Type* ParserImpl::UnsignedTypeFor(const Type* type) {
  if (type->Is<I32>()) {
    return ty_.U32();
  }
  if (auto* v = type->As<Vector>()) {
    if (v->type->Is<I32>()) {
      return ty_.Vector(ty_.U32(), v->size);
    }
  }
  return {};
}

const Type* ParserImpl::SignedTypeFor(const Type* type) {
  if (type->Is<U32>()) {
    return ty_.I32();
  }
  if (auto* v = type->As<Vector>()) {
    if (v->type->Is<U32>()) {
      return ty_.Vector(ty_.I32(), v->size);
    }
  }
  return {};
}

TypedExpression ParserImpl::RectifyOperandSignedness(
    const spvtools::opt::Instruction& inst,
    TypedExpression&& expr) {
  bool requires_signed = false;
  bool requires_unsigned = false;
  if (IsGlslExtendedInstruction(inst)) {
    const auto extended_opcode =
        static_cast<GLSLstd450>(inst.GetSingleWordInOperand(1));
    requires_signed = AssumesSignedOperands(extended_opcode);
    requires_unsigned = AssumesUnsignedOperands(extended_opcode);
  } else {
    const auto opcode = inst.opcode();
    requires_signed = AssumesSignedOperands(opcode);
    requires_unsigned = AssumesUnsignedOperands(opcode);
  }
  if (!requires_signed && !requires_unsigned) {
    // No conversion is required, assuming our tables are complete.
    return std::move(expr);
  }
  if (!expr) {
    Fail() << "internal error: RectifyOperandSignedness given a null expr\n";
    return {};
  }
  auto* type = expr.type;
  if (!type) {
    Fail() << "internal error: unmapped type for: "
           << expr.expr->TypeInfo().name << "\n";
    return {};
  }
  if (requires_unsigned) {
    if (auto* unsigned_ty = UnsignedTypeFor(type)) {
      // Conversion is required.
      return {unsigned_ty,
              create<ast::BitcastExpression>(
                  Source{}, unsigned_ty->Build(builder_), expr.expr)};
    }
  } else if (requires_signed) {
    if (auto* signed_ty = SignedTypeFor(type)) {
      // Conversion is required.
      return {signed_ty, create<ast::BitcastExpression>(
                             Source{}, signed_ty->Build(builder_), expr.expr)};
    }
  }
  // We should not reach here.
  return std::move(expr);
}

TypedExpression ParserImpl::RectifySecondOperandSignedness(
    const spvtools::opt::Instruction& inst,
    const Type* first_operand_type,
    TypedExpression&& second_operand_expr) {
  if ((first_operand_type != second_operand_expr.type) &&
      AssumesSecondOperandSignednessMatchesFirstOperand(inst.opcode())) {
    // Conversion is required.
    return {first_operand_type,
            create<ast::BitcastExpression>(Source{},
                                           first_operand_type->Build(builder_),
                                           second_operand_expr.expr)};
  }
  // No conversion necessary.
  return std::move(second_operand_expr);
}

const Type* ParserImpl::ForcedResultType(const spvtools::opt::Instruction& inst,
                                         const Type* first_operand_type) {
  const auto opcode = inst.opcode();
  if (AssumesResultSignednessMatchesFirstOperand(opcode)) {
    return first_operand_type;
  }
  if (IsGlslExtendedInstruction(inst)) {
    const auto extended_opcode =
        static_cast<GLSLstd450>(inst.GetSingleWordInOperand(1));
    if (AssumesResultSignednessMatchesFirstOperand(extended_opcode)) {
      return first_operand_type;
    }
  }
  return nullptr;
}

const Type* ParserImpl::GetSignedIntMatchingShape(const Type* other) {
  if (other == nullptr) {
    Fail() << "no type provided";
  }
  if (other->Is<F32>() || other->Is<U32>() || other->Is<I32>()) {
    return ty_.I32();
  }
  if (auto* vec_ty = other->As<Vector>()) {
    return ty_.Vector(ty_.I32(), vec_ty->size);
  }
  Fail() << "required numeric scalar or vector, but got "
         << other->TypeInfo().name;
  return nullptr;
}

const Type* ParserImpl::GetUnsignedIntMatchingShape(const Type* other) {
  if (other == nullptr) {
    Fail() << "no type provided";
    return nullptr;
  }
  if (other->Is<F32>() || other->Is<U32>() || other->Is<I32>()) {
    return ty_.U32();
  }
  if (auto* vec_ty = other->As<Vector>()) {
    return ty_.Vector(ty_.U32(), vec_ty->size);
  }
  Fail() << "required numeric scalar or vector, but got "
         << other->TypeInfo().name;
  return nullptr;
}

TypedExpression ParserImpl::RectifyForcedResultType(
    TypedExpression expr,
    const spvtools::opt::Instruction& inst,
    const Type* first_operand_type) {
  auto* forced_result_ty = ForcedResultType(inst, first_operand_type);
  if ((!forced_result_ty) || (forced_result_ty == expr.type)) {
    return expr;
  }
  return {expr.type, create<ast::BitcastExpression>(
                         Source{}, expr.type->Build(builder_), expr.expr)};
}

TypedExpression ParserImpl::AsUnsigned(TypedExpression expr) {
  if (expr.type && expr.type->IsSignedScalarOrVector()) {
    auto* new_type = GetUnsignedIntMatchingShape(expr.type);
    return {new_type, create<ast::BitcastExpression>(
                          Source{}, new_type->Build(builder_), expr.expr)};
  }
  return expr;
}

TypedExpression ParserImpl::AsSigned(TypedExpression expr) {
  if (expr.type && expr.type->IsUnsignedScalarOrVector()) {
    auto* new_type = GetSignedIntMatchingShape(expr.type);
    return {new_type, create<ast::BitcastExpression>(
                          Source{}, new_type->Build(builder_), expr.expr)};
  }
  return expr;
}

bool ParserImpl::EmitFunctions() {
  if (!success_) {
    return false;
  }
  for (const auto* f : topologically_ordered_functions_) {
    if (!success_) {
      return false;
    }

    auto id = f->result_id();
    auto it = function_to_ep_info_.find(id);
    if (it == function_to_ep_info_.end()) {
      FunctionEmitter emitter(this, *f, nullptr);
      success_ = emitter.Emit();
    } else {
      for (const auto& ep : it->second) {
        FunctionEmitter emitter(this, *f, &ep);
        success_ = emitter.Emit();
        if (!success_) {
          return false;
        }
      }
    }
  }
  return success_;
}

const spvtools::opt::Instruction*
ParserImpl::GetMemoryObjectDeclarationForHandle(uint32_t id,
                                                bool follow_image) {
  auto saved_id = id;
  auto local_fail = [this, saved_id, id,
                     follow_image]() -> const spvtools::opt::Instruction* {
    const auto* inst = def_use_mgr_->GetDef(id);
    Fail() << "Could not find memory object declaration for the "
           << (follow_image ? "image" : "sampler") << " underlying id " << id
           << " (from original id " << saved_id << ") "
           << (inst ? inst->PrettyPrint() : std::string());
    return nullptr;
  };

  auto& memo_table =
      (follow_image ? mem_obj_decl_image_ : mem_obj_decl_sampler_);

  // Use a visited set to defend against bad input which might have long
  // chains or even loops.
  std::unordered_set<uint32_t> visited;

  // Trace backward in the SSA data flow until we hit a memory object
  // declaration.
  while (true) {
    auto where = memo_table.find(id);
    if (where != memo_table.end()) {
      return where->second;
    }
    // Protect against loops.
    auto visited_iter = visited.find(id);
    if (visited_iter != visited.end()) {
      // We've hit a loop. Mark all the visited nodes
      // as dead ends.
      for (auto iter : visited) {
        memo_table[iter] = nullptr;
      }
      return nullptr;
    }
    visited.insert(id);

    const auto* inst = def_use_mgr_->GetDef(id);
    if (inst == nullptr) {
      return local_fail();
    }
    switch (inst->opcode()) {
      case SpvOpFunctionParameter:
      case SpvOpVariable:
        // We found the memory object declaration.
        // Remember it as the answer for the whole path.
        for (auto iter : visited) {
          memo_table[iter] = inst;
        }
        return inst;
      case SpvOpLoad:
        // Follow the pointer being loaded
        id = inst->GetSingleWordInOperand(0);
        break;
      case SpvOpCopyObject:
        // Follow the object being copied.
        id = inst->GetSingleWordInOperand(0);
        break;
      case SpvOpAccessChain:
      case SpvOpInBoundsAccessChain:
      case SpvOpPtrAccessChain:
      case SpvOpInBoundsPtrAccessChain:
        // Follow the base pointer.
        id = inst->GetSingleWordInOperand(0);
        break;
      case SpvOpSampledImage:
        // Follow the image or the sampler, depending on the follow_image
        // parameter.
        id = inst->GetSingleWordInOperand(follow_image ? 0 : 1);
        break;
      case SpvOpImage:
        // Follow the sampled image
        id = inst->GetSingleWordInOperand(0);
        break;
      default:
        // Can't trace further.
        // Remember it as the answer for the whole path.
        for (auto iter : visited) {
          memo_table[iter] = nullptr;
        }
        return nullptr;
    }
  }
}

const spvtools::opt::Instruction*
ParserImpl::GetSpirvTypeForHandleMemoryObjectDeclaration(
    const spvtools::opt::Instruction& var) {
  if (!success()) {
    return nullptr;
  }
  // The WGSL handle type is determined by looking at information from
  // several sources:
  //    - the usage of the handle by image access instructions
  //    - the SPIR-V type declaration
  // Each source does not have enough information to completely determine
  // the result.

  // Messages are phrased in terms of images and samplers because those
  // are the only SPIR-V handles supported by WGSL.

  // Get the SPIR-V handle type.
  const auto* ptr_type = def_use_mgr_->GetDef(var.type_id());
  if (!ptr_type || (ptr_type->opcode() != SpvOpTypePointer)) {
    Fail() << "Invalid type for variable or function parameter "
           << var.PrettyPrint();
    return nullptr;
  }
  const auto* raw_handle_type =
      def_use_mgr_->GetDef(ptr_type->GetSingleWordInOperand(1));
  if (!raw_handle_type) {
    Fail() << "Invalid pointer type for variable or function parameter "
           << var.PrettyPrint();
    return nullptr;
  }
  switch (raw_handle_type->opcode()) {
    case SpvOpTypeSampler:
    case SpvOpTypeImage:
      // The expected cases.
      break;
    case SpvOpTypeArray:
    case SpvOpTypeRuntimeArray:
      Fail()
          << "arrays of textures or samplers are not supported in WGSL; can't "
             "translate variable or function parameter: "
          << var.PrettyPrint();
      return nullptr;
    case SpvOpTypeSampledImage:
      Fail() << "WGSL does not support combined image-samplers: "
             << var.PrettyPrint();
      return nullptr;
    default:
      Fail() << "invalid type for image or sampler variable or function "
                "parameter: "
             << var.PrettyPrint();
      return nullptr;
  }
  return raw_handle_type;
}

const Pointer* ParserImpl::GetTypeForHandleVar(
    const spvtools::opt::Instruction& var) {
  auto where = handle_type_.find(&var);
  if (where != handle_type_.end()) {
    return where->second;
  }

  const spvtools::opt::Instruction* raw_handle_type =
      GetSpirvTypeForHandleMemoryObjectDeclaration(var);
  if (!raw_handle_type) {
    return nullptr;
  }

  // The variable could be a sampler or image.
  // Where possible, determine which one it is from the usage inferred
  // for the variable.
  Usage usage = handle_usage_[&var];
  if (!usage.IsValid()) {
    Fail() << "Invalid sampler or texture usage for variable "
           << var.PrettyPrint() << "\n"
           << usage;
    return nullptr;
  }
  // Infer a handle type, if usage didn't already tell us.
  if (!usage.IsComplete()) {
    // In SPIR-V you could statically reference a texture or sampler without
    // using it in a way that gives us a clue on how to declare it.  Look inside
    // the store type to infer a usage.
    if (raw_handle_type->opcode() == SpvOpTypeSampler) {
      usage.AddSampler();
    } else {
      // It's a texture.
      if (raw_handle_type->NumInOperands() != 7) {
        Fail() << "invalid SPIR-V image type: expected 7 operands: "
               << raw_handle_type->PrettyPrint();
        return nullptr;
      }
      const auto sampled_param = raw_handle_type->GetSingleWordInOperand(5);
      const auto format_param = raw_handle_type->GetSingleWordInOperand(6);
      // Only storage images have a format.
      if ((format_param != SpvImageFormatUnknown) ||
          sampled_param == 2 /* without sampler */) {
        // Get NonWritable and NonReadable attributes of the variable.
        bool is_nonwritable = false;
        bool is_nonreadable = false;
        for (const auto& deco : GetDecorationsFor(var.result_id())) {
          if (deco.size() != 1) {
            continue;
          }
          if (deco[0] == SpvDecorationNonWritable) {
            is_nonwritable = true;
          }
          if (deco[0] == SpvDecorationNonReadable) {
            is_nonreadable = true;
          }
        }
        if (is_nonwritable && is_nonreadable) {
          Fail() << "storage image variable is both NonWritable and NonReadable"
                 << var.PrettyPrint();
        }
        if (!is_nonwritable && !is_nonreadable) {
          Fail()
              << "storage image variable is neither NonWritable nor NonReadable"
              << var.PrettyPrint();
        }
        // Let's make it one of the storage textures.
        if (is_nonwritable) {
          usage.AddStorageReadTexture();
        } else {
          usage.AddStorageWriteTexture();
        }
      } else {
        usage.AddSampledTexture();
      }
    }
    if (!usage.IsComplete()) {
      Fail()
          << "internal error: should have inferred a complete handle type. got "
          << usage.to_str();
      return nullptr;
    }
  }

  // Construct the Tint handle type.
  const Type* ast_store_type = nullptr;
  if (usage.IsSampler()) {
    ast_store_type = ty_.Sampler(usage.IsComparisonSampler()
                                     ? ast::SamplerKind::kComparisonSampler
                                     : ast::SamplerKind::kSampler);
  } else if (usage.IsTexture()) {
    const spvtools::opt::analysis::Image* image_type =
        type_mgr_->GetType(raw_handle_type->result_id())->AsImage();
    if (!image_type) {
      Fail() << "internal error: Couldn't look up image type"
             << raw_handle_type->PrettyPrint();
      return nullptr;
    }

    if (image_type->is_arrayed()) {
      // Give a nicer error message here, where we have the offending variable
      // in hand, rather than inside the enum converter.
      switch (image_type->dim()) {
        case SpvDim2D:
        case SpvDimCube:
          break;
        default:
          Fail() << "WGSL arrayed textures must be 2d_array or cube_array: "
                    "invalid multisampled texture variable "
                 << namer_.Name(var.result_id()) << ": " << var.PrettyPrint();
          return nullptr;
      }
    }

    const ast::TextureDimension dim =
        enum_converter_.ToDim(image_type->dim(), image_type->is_arrayed());
    if (dim == ast::TextureDimension::kNone) {
      return nullptr;
    }

    // WGSL textures are always formatted.  Unformatted textures are always
    // sampled.
    if (usage.IsSampledTexture() || usage.IsStorageReadTexture() ||
        (image_type->format() == SpvImageFormatUnknown)) {
      // Make a sampled texture type.
      auto* ast_sampled_component_type =
          ConvertType(raw_handle_type->GetSingleWordInOperand(0));

      // Vulkan ignores the depth parameter on OpImage, so pay attention to the
      // usage as well.  That is, it's valid for a Vulkan shader to use an
      // OpImage variable with an OpImage*Dref* instruction.  In WGSL we must
      // treat that as a depth texture.
      if (image_type->depth() || usage.IsDepthTexture()) {
        if (image_type->is_multisampled()) {
          ast_store_type = ty_.DepthMultisampledTexture(dim);
        } else {
          ast_store_type = ty_.DepthTexture(dim);
        }
      } else if (image_type->is_multisampled()) {
        if (dim != ast::TextureDimension::k2d) {
          Fail() << "WGSL multisampled textures must be 2d and non-arrayed: "
                    "invalid multisampled texture variable "
                 << namer_.Name(var.result_id()) << ": " << var.PrettyPrint();
        }
        // Multisampled textures are never depth textures.
        ast_store_type =
            ty_.MultisampledTexture(dim, ast_sampled_component_type);
      } else {
        ast_store_type = ty_.SampledTexture(dim, ast_sampled_component_type);
      }
    } else {
      const auto access = ast::Access::kWrite;
      const auto format = enum_converter_.ToTexelFormat(image_type->format());
      if (format == ast::TexelFormat::kNone) {
        return nullptr;
      }
      ast_store_type = ty_.StorageTexture(dim, format, access);
    }
  } else {
    Fail() << "unsupported: UniformConstant variable is not a recognized "
              "sampler or texture"
           << var.PrettyPrint();
    return nullptr;
  }

  // Form the pointer type.
  auto* result = ty_.Pointer(ast_store_type, ast::StorageClass::kHandle);
  // Remember it for later.
  handle_type_[&var] = result;
  return result;
}

const Type* ParserImpl::GetComponentTypeForFormat(ast::TexelFormat format) {
  switch (format) {
    case ast::TexelFormat::kR32Uint:
    case ast::TexelFormat::kRgba8Uint:
    case ast::TexelFormat::kRg32Uint:
    case ast::TexelFormat::kRgba16Uint:
    case ast::TexelFormat::kRgba32Uint:
      return ty_.U32();

    case ast::TexelFormat::kR32Sint:
    case ast::TexelFormat::kRgba8Sint:
    case ast::TexelFormat::kRg32Sint:
    case ast::TexelFormat::kRgba16Sint:
    case ast::TexelFormat::kRgba32Sint:
      return ty_.I32();

    case ast::TexelFormat::kRgba8Unorm:
    case ast::TexelFormat::kRgba8Snorm:
    case ast::TexelFormat::kR32Float:
    case ast::TexelFormat::kRg32Float:
    case ast::TexelFormat::kRgba16Float:
    case ast::TexelFormat::kRgba32Float:
      return ty_.F32();
    default:
      break;
  }
  Fail() << "unknown format " << int(format);
  return nullptr;
}

unsigned ParserImpl::GetChannelCountForFormat(ast::TexelFormat format) {
  switch (format) {
    case ast::TexelFormat::kR32Float:
    case ast::TexelFormat::kR32Sint:
    case ast::TexelFormat::kR32Uint:
      // One channel
      return 1;

    case ast::TexelFormat::kRg32Float:
    case ast::TexelFormat::kRg32Sint:
    case ast::TexelFormat::kRg32Uint:
      // Two channels
      return 2;

    case ast::TexelFormat::kRgba16Float:
    case ast::TexelFormat::kRgba16Sint:
    case ast::TexelFormat::kRgba16Uint:
    case ast::TexelFormat::kRgba32Float:
    case ast::TexelFormat::kRgba32Sint:
    case ast::TexelFormat::kRgba32Uint:
    case ast::TexelFormat::kRgba8Sint:
    case ast::TexelFormat::kRgba8Snorm:
    case ast::TexelFormat::kRgba8Uint:
    case ast::TexelFormat::kRgba8Unorm:
      // Four channels
      return 4;

    default:
      break;
  }
  Fail() << "unknown format " << int(format);
  return 0;
}

const Type* ParserImpl::GetTexelTypeForFormat(ast::TexelFormat format) {
  const auto* component_type = GetComponentTypeForFormat(format);
  if (!component_type) {
    return nullptr;
  }
  return ty_.Vector(component_type, 4);
}

bool ParserImpl::RegisterHandleUsage() {
  if (!success_) {
    return false;
  }

  // Map a function ID to the list of its function parameter instructions, in
  // order.
  std::unordered_map<uint32_t, std::vector<const spvtools::opt::Instruction*>>
      function_params;
  for (const auto* f : topologically_ordered_functions_) {
    // Record the instructions defining this function's parameters.
    auto& params = function_params[f->result_id()];
    f->ForEachParam([&params](const spvtools::opt::Instruction* param) {
      params.push_back(param);
    });
  }

  // Returns the memory object declaration for an image underlying the first
  // operand of the given image instruction.
  auto get_image = [this](const spvtools::opt::Instruction& image_inst) {
    return this->GetMemoryObjectDeclarationForHandle(
        image_inst.GetSingleWordInOperand(0), true);
  };
  // Returns the memory object declaration for a sampler underlying the first
  // operand of the given image instruction.
  auto get_sampler = [this](const spvtools::opt::Instruction& image_inst) {
    return this->GetMemoryObjectDeclarationForHandle(
        image_inst.GetSingleWordInOperand(0), false);
  };

  // Scan the bodies of functions for image operations, recording their implied
  // usage properties on the memory object declarations (i.e. variables or
  // function parameters).  We scan the functions in an order so that callees
  // precede callers. That way the usage on a function parameter is already
  // computed before we see the call to that function.  So when we reach
  // a function call, we can add the usage from the callee formal parameters.
  for (const auto* f : topologically_ordered_functions_) {
    for (const auto& bb : *f) {
      for (const auto& inst : bb) {
        switch (inst.opcode()) {
            // Single texel reads and writes

          case SpvOpImageRead:
            handle_usage_[get_image(inst)].AddStorageReadTexture();
            break;
          case SpvOpImageWrite:
            handle_usage_[get_image(inst)].AddStorageWriteTexture();
            break;
          case SpvOpImageFetch:
            handle_usage_[get_image(inst)].AddSampledTexture();
            break;

            // Sampling and gathering from a sampled image.

          case SpvOpImageSampleImplicitLod:
          case SpvOpImageSampleExplicitLod:
          case SpvOpImageSampleProjImplicitLod:
          case SpvOpImageSampleProjExplicitLod:
          case SpvOpImageGather:
            handle_usage_[get_image(inst)].AddSampledTexture();
            handle_usage_[get_sampler(inst)].AddSampler();
            break;
          case SpvOpImageSampleDrefImplicitLod:
          case SpvOpImageSampleDrefExplicitLod:
          case SpvOpImageSampleProjDrefImplicitLod:
          case SpvOpImageSampleProjDrefExplicitLod:
          case SpvOpImageDrefGather:
            // Depth reference access implies usage as a depth texture, which
            // in turn is a sampled texture.
            handle_usage_[get_image(inst)].AddDepthTexture();
            handle_usage_[get_sampler(inst)].AddComparisonSampler();
            break;

            // Image queries

          case SpvOpImageQuerySizeLod:
            // Vulkan requires Sampled=1 for this. SPIR-V already requires MS=0.
            handle_usage_[get_image(inst)].AddSampledTexture();
            break;
          case SpvOpImageQuerySize:
            // Applies to either MS=1 or Sampled=0 or 2.
            // So we can't force it to be multisampled, or storage image.
            break;
          case SpvOpImageQueryLod:
            handle_usage_[get_image(inst)].AddSampledTexture();
            handle_usage_[get_sampler(inst)].AddSampler();
            break;
          case SpvOpImageQueryLevels:
            // We can't tell anything more than that it's an image.
            handle_usage_[get_image(inst)].AddTexture();
            break;
          case SpvOpImageQuerySamples:
            handle_usage_[get_image(inst)].AddMultisampledTexture();
            break;

            // Function calls

          case SpvOpFunctionCall: {
            // Propagate handle usages from callee function formal parameters to
            // the matching caller parameters.  This is where we rely on the
            // fact that callees have been processed earlier in the flow.
            const auto num_in_operands = inst.NumInOperands();
            // The first operand of the call is the function ID.
            // The remaining operands are the operands to the function.
            if (num_in_operands < 1) {
              return Fail() << "Call instruction must have at least one operand"
                            << inst.PrettyPrint();
            }
            const auto function_id = inst.GetSingleWordInOperand(0);
            const auto& formal_params = function_params[function_id];
            if (formal_params.size() != (num_in_operands - 1)) {
              return Fail() << "Called function has " << formal_params.size()
                            << " parameters, but function call has "
                            << (num_in_operands - 1) << " parameters"
                            << inst.PrettyPrint();
            }
            for (uint32_t i = 1; i < num_in_operands; ++i) {
              auto where = handle_usage_.find(formal_params[i - 1]);
              if (where == handle_usage_.end()) {
                // We haven't recorded any handle usage on the formal parameter.
                continue;
              }
              const Usage& formal_param_usage = where->second;
              const auto operand_id = inst.GetSingleWordInOperand(i);
              const auto* operand_as_sampler =
                  GetMemoryObjectDeclarationForHandle(operand_id, false);
              const auto* operand_as_image =
                  GetMemoryObjectDeclarationForHandle(operand_id, true);
              if (operand_as_sampler) {
                handle_usage_[operand_as_sampler].Add(formal_param_usage);
              }
              if (operand_as_image &&
                  (operand_as_image != operand_as_sampler)) {
                handle_usage_[operand_as_image].Add(formal_param_usage);
              }
            }
            break;
          }

          default:
            break;
        }
      }
    }
  }
  return success_;
}

Usage ParserImpl::GetHandleUsage(uint32_t id) const {
  const auto where = handle_usage_.find(def_use_mgr_->GetDef(id));
  if (where != handle_usage_.end()) {
    return where->second;
  }
  return Usage();
}

const spvtools::opt::Instruction* ParserImpl::GetInstructionForTest(
    uint32_t id) const {
  return def_use_mgr_ ? def_use_mgr_->GetDef(id) : nullptr;
}

std::string ParserImpl::GetMemberName(const Struct& struct_type,
                                      int member_index) {
  auto where = struct_id_for_symbol_.find(struct_type.name);
  if (where == struct_id_for_symbol_.end()) {
    Fail() << "no structure type registered for symbol";
    return "";
  }
  return namer_.GetMemberName(where->second, member_index);
}

WorkgroupSizeInfo::WorkgroupSizeInfo() = default;

WorkgroupSizeInfo::~WorkgroupSizeInfo() = default;

}  // namespace tint::reader::spirv
