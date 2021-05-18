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

#include <algorithm>
#include <limits>
#include <locale>
#include <utility>

#include "source/opt/build_module.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/override_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/type_name.h"
#include "src/ast/unary_op_expression.h"
#include "src/reader/spirv/function.h"
#include "src/sem/depth_texture_type.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/utils/unique_vector.h"

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

}  // namespace

TypedExpression::TypedExpression() = default;

TypedExpression::TypedExpression(const TypedExpression&) = default;

TypedExpression& TypedExpression::operator=(const TypedExpression&) = default;

TypedExpression::TypedExpression(const Type* type_in, ast::Expression* expr_in)
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

  auto maybe_generate_alias = [this, type_id,
                               spirv_type](const Type* type) -> const Type* {
    if (type != nullptr) {
      return MaybeGenerateAlias(type_id, spirv_type, type);
    }
    return type;
  };

  switch (spirv_type->kind()) {
    case spvtools::opt::analysis::Type::kVoid:
      return maybe_generate_alias(ty_.Void());
    case spvtools::opt::analysis::Type::kBool:
      return maybe_generate_alias(ty_.Bool());
    case spvtools::opt::analysis::Type::kInteger:
      return maybe_generate_alias(ConvertType(spirv_type->AsInteger()));
    case spvtools::opt::analysis::Type::kFloat:
      return maybe_generate_alias(ConvertType(spirv_type->AsFloat()));
    case spvtools::opt::analysis::Type::kVector:
      return maybe_generate_alias(ConvertType(spirv_type->AsVector()));
    case spvtools::opt::analysis::Type::kMatrix:
      return maybe_generate_alias(ConvertType(spirv_type->AsMatrix()));
    case spvtools::opt::analysis::Type::kRuntimeArray:
      return maybe_generate_alias(ConvertType(spirv_type->AsRuntimeArray()));
    case spvtools::opt::analysis::Type::kArray:
      return maybe_generate_alias(ConvertType(spirv_type->AsArray()));
    case spvtools::opt::analysis::Type::kStruct:
      return maybe_generate_alias(ConvertType(type_id, spirv_type->AsStruct()));
    case spvtools::opt::analysis::Type::kPointer:
      return maybe_generate_alias(
          ConvertType(type_id, ptr_as, spirv_type->AsPointer()));
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
      return maybe_generate_alias(ty_.Void());
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

ast::Decoration* ParserImpl::ConvertMemberDecoration(
    uint32_t struct_type_id,
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
      return create<ast::StructMemberOffsetDecoration>(Source{}, decoration[1]);
    case SpvDecorationNonReadable:
      // WGSL doesn't have a member decoration for this.  Silently drop it.
      return nullptr;
    case SpvDecorationNonWritable:
      // WGSL doesn't have a member decoration for this.
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
  return Source{where->second};
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
  if (!RegisterEntryPoints()) {
    return false;
  }
  if (!RegisterHandleUsage()) {
    return false;
  }
  if (!RegisterTypes()) {
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
  if (!std::isalpha(str[0], c_locale)) {
    return false;
  }
  for (const char& ch : str) {
    if ((ch != '_') && !std::isalnum(ch, c_locale)) {
      return false;
    }
  }
  return true;
}

bool ParserImpl::RegisterEntryPoints() {
  for (const spvtools::opt::Instruction& entry_point :
       module_->entry_points()) {
    const auto stage = SpvExecutionModel(entry_point.GetSingleWordInOperand(0));
    const uint32_t function_id = entry_point.GetSingleWordInOperand(1);

    const std::string ep_name = entry_point.GetOperand(2).AsString();

    bool owns_inner_implementation = false;
    std::string inner_implementation_name;

    if (hlsl_style_pipeline_io_) {
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
    }
    TINT_ASSERT(ep_name != inner_implementation_name);

    tint::UniqueVector<uint32_t> inputs;
    tint::UniqueVector<uint32_t> outputs;
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
    std::vector<uint32_t> sorted_inputs(inputs.begin(), inputs.end());
    std::sort(sorted_inputs.begin(), sorted_inputs.end());
    std::vector<uint32_t> sorted_outputs(outputs.begin(), outputs.end());
    std::sort(sorted_inputs.begin(), sorted_inputs.end());

    function_to_ep_info_[function_id].emplace_back(
        ep_name, enum_converter_.ToPipelineStage(stage),
        owns_inner_implementation, inner_implementation_name,
        std::move(sorted_inputs), std::move(sorted_outputs));
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
    const spvtools::opt::analysis::RuntimeArray* rtarr_ty) {
  auto* ast_elem_ty = ConvertType(type_mgr_->GetId(rtarr_ty->element_type()));
  if (ast_elem_ty == nullptr) {
    return nullptr;
  }
  uint32_t array_stride = 0;
  if (!ParseArrayDecorations(rtarr_ty, &array_stride)) {
    return nullptr;
  }
  return ty_.Array(ast_elem_ty, 0, array_stride);
}

const Type* ParserImpl::ConvertType(
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
  uint32_t array_stride = 0;
  if (!ParseArrayDecorations(arr_ty, &array_stride)) {
    return nullptr;
  }
  if (remap_buffer_block_type_.count(elem_type_id)) {
    remap_buffer_block_type_.insert(type_mgr_->GetId(arr_ty));
  }
  return ty_.Array(ast_elem_ty, static_cast<uint32_t>(num_elem), array_stride);
}

bool ParserImpl::ParseArrayDecorations(
    const spvtools::opt::analysis::Type* spv_type,
    uint32_t* array_stride) {
  bool has_array_stride = false;
  const auto type_id = type_mgr_->GetId(spv_type);
  for (auto& decoration : this->GetDecorationsFor(type_id)) {
    if (decoration.size() == 2 && decoration[0] == SpvDecorationArrayStride) {
      const auto stride = decoration[1];
      if (stride == 0) {
        return Fail() << "invalid array type ID " << type_id
                      << ": ArrayStride can't be 0";
      }
      if (has_array_stride) {
        return Fail() << "invalid array type ID " << type_id
                      << ": multiple ArrayStride decorations";
      }
      has_array_stride = true;
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
  bool is_block_decorated = false;
  if (struct_decorations.size() == 1) {
    const auto decoration = struct_decorations[0][0];
    if (decoration == SpvDecorationBlock) {
      is_block_decorated = true;
    } else if (decoration == SpvDecorationBufferBlock) {
      is_block_decorated = true;
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
    ast::DecorationList ast_member_decorations;
    for (auto& decoration : GetDecorationsForMember(type_id, member_index)) {
      if (decoration[0] == SpvDecorationNonWritable) {
        // WGSL doesn't represent individual members as non-writable. Instead,
        // apply the ReadOnly access control to the containing struct if all
        // the members are non-writable.
        is_non_writable = true;
      } else {
        auto* ast_member_decoration =
            ConvertMemberDecoration(type_id, member_index, decoration);
        if (!success_) {
          return nullptr;
        }
        if (ast_member_decoration) {
          ast_member_decorations.push_back(ast_member_decoration);
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
  ast::DecorationList ast_struct_decorations;
  if (is_block_decorated) {
    ast_struct_decorations.emplace_back(
        create<ast::StructBlockDecoration>(Source{}));
  }
  auto* ast_struct = create<ast::Struct>(Source{}, sym, std::move(ast_members),
                                         std::move(ast_struct_decorations));
  if (num_non_writable_members == members.size()) {
    read_only_struct_types_.insert(ast_struct->name());
  }
  AddConstructedType(sym, ast_struct);
  return ty_.Struct(sym, std::move(ast_member_types));
}

void ParserImpl::AddConstructedType(Symbol name, ast::NamedType* type) {
  auto iter = constructed_types_.insert(name);
  if (iter.second) {
    builder_.AST().AddConstructedType(type);
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
    builtin_position_.storage_class = storage_class;
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

  if (hlsl_style_pipeline_io_) {
    // When using HLSL-style pipeline IO, intput and output variables
    // are mapped to private variables.
    if (ast_storage_class == ast::StorageClass::kInput ||
        ast_storage_class == ast::StorageClass::kOutput) {
      ast_storage_class = ast::StorageClass::kPrivate;
    }
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

bool ParserImpl::EmitScalarSpecConstants() {
  if (!success_) {
    return false;
  }
  // Generate a module-scope const declaration for each instruction
  // that is OpSpecConstantTrue, OpSpecConstantFalse, or OpSpecConstant.
  for (auto& inst : module_->types_values()) {
    // These will be populated for a valid scalar spec constant.
    const Type* ast_type = nullptr;
    ast::ScalarConstructorExpression* ast_expr = nullptr;

    switch (inst.opcode()) {
      case SpvOpSpecConstantTrue:
      case SpvOpSpecConstantFalse: {
        ast_type = ConvertType(inst.type_id());
        ast_expr = create<ast::ScalarConstructorExpression>(
            Source{}, create<ast::BoolLiteral>(
                          Source{}, inst.opcode() == SpvOpSpecConstantTrue));
        break;
      }
      case SpvOpSpecConstant: {
        ast_type = ConvertType(inst.type_id());
        const uint32_t literal_value = inst.GetSingleWordInOperand(0);
        if (ast_type->Is<I32>()) {
          ast_expr = create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::SintLiteral>(
                            Source{}, static_cast<int32_t>(literal_value)));
        } else if (ast_type->Is<U32>()) {
          ast_expr = create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::UintLiteral>(
                            Source{}, static_cast<uint32_t>(literal_value)));
        } else if (ast_type->Is<F32>()) {
          float float_value;
          // Copy the bits so we can read them as a float.
          std::memcpy(&float_value, &literal_value, sizeof(float_value));
          ast_expr = create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::FloatLiteral>(Source{}, float_value));
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
      ast::DecorationList spec_id_decos;
      for (const auto& deco : GetDecorationsFor(inst.result_id())) {
        if ((deco.size() == 2) && (deco[0] == SpvDecorationSpecId)) {
          auto* cid = create<ast::OverrideDecoration>(Source{}, deco[1]);
          spec_id_decos.push_back(cid);
          break;
        }
      }
      auto* ast_var =
          MakeVariable(inst.result_id(), ast::StorageClass::kNone, ast_type,
                       true, ast_expr, std::move(spec_id_decos));
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
  AddConstructedType(sym, ast_alias_type);

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
      continue;
    }
    switch (enum_converter_.ToStorageClass(spirv_storage_class)) {
      case ast::StorageClass::kNone:
      case ast::StorageClass::kInput:
      case ast::StorageClass::kOutput:
      case ast::StorageClass::kUniform:
      case ast::StorageClass::kUniformConstant:
      case ast::StorageClass::kStorage:
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
    ast::Expression* ast_constructor = nullptr;
    if (var.NumInOperands() > 1) {
      // SPIR-V initializers are always constants.
      // (OpenCL also allows the ID of an OpVariable, but we don't handle that
      // here.)
      ast_constructor =
          MakeConstantExpression(var.GetSingleWordInOperand(1)).expr;
    }
    auto* ast_var =
        MakeVariable(var.result_id(), ast_storage_class, ast_store_type, false,
                     ast_constructor, ast::DecorationList{});
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
    auto* var = MakeVariable(
        builtin_position_.per_vertex_var_id,
        enum_converter_.ToStorageClass(builtin_position_.storage_class),
        ConvertType(builtin_position_.position_member_type_id), false, nullptr,
        ast::DecorationList{
            create<ast::BuiltinDecoration>(Source{}, ast::Builtin::kPosition),
        });

    builder_.AST().AddGlobalVariable(var);
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
                                        ast::Expression* constructor,
                                        ast::DecorationList decorations) {
  if (storage_type == nullptr) {
    Fail() << "internal error: can't make ast::Variable for null type";
    return nullptr;
  }

  if (sc == ast::StorageClass::kStorage) {
    bool read_only = false;
    if (auto* tn = storage_type->As<Named>()) {
      read_only = read_only_struct_types_.count(tn->name) > 0;
    }

    // Apply the access(read) or access(read_write) modifier.
    auto access = read_only ? ast::AccessControl::kReadOnly
                            : ast::AccessControl::kReadWrite;
    storage_type = ty_.AccessControl(storage_type, access);
  }

  // Handle variables (textures and samplers) are always in the handle
  // storage class, so we don't mention the storage class.
  if (sc == ast::StorageClass::kUniformConstant) {
    sc = ast::StorageClass::kNone;
  }

  // In almost all cases, copy the decorations from SPIR-V to the variable.
  // But avoid doing so when converting pipeline IO to private variables.
  if (sc != ast::StorageClass::kPrivate) {
    if (!ConvertDecorationsForVariable(id, &storage_type, &decorations)) {
      return nullptr;
    }
  }

  std::string name = namer_.Name(id);

  // Note: we're constructing the variable here with the *storage* type,
  // regardless of whether this is a `let` or `var` declaration.
  // `var` declarations will have a resolved type of ref<storage>, but at the
  // AST level both `var` and `let` are declared with the same type.
  return create<ast::Variable>(Source{}, builder_.Symbols().Register(name), sc,
                               storage_type->Build(builder_), is_const,
                               constructor, decorations);
}

bool ParserImpl::ConvertDecorationsForVariable(
    uint32_t id,
    const Type** type,
    ast::DecorationList* decorations) {
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
          // The SPIR-V variable is likely to be signed (because GLSL
          // requires signed), but WGSL requires unsigned.  Handle specially
          // so we always perform the conversion at load and store.
          if (auto* forced_type = UnsignedTypeFor(*type)) {
            // Requires conversion and special handling in code generation.
            special_builtins_[id] = spv_builtin;
            *type = forced_type;
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
          *type = ty_.U32();
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
      decorations->emplace_back(
          create<ast::BuiltinDecoration>(Source{}, ast_builtin));
    }
    if (deco[0] == SpvDecorationLocation) {
      if (deco.size() != 2) {
        return Fail() << "malformed Location decoration on ID " << id
                      << ": requires one literal operand";
      }
      decorations->emplace_back(
          create<ast::LocationDecoration>(Source{}, deco[1]));
    }
    if (deco[0] == SpvDecorationDescriptorSet) {
      if (deco.size() == 1) {
        return Fail() << "malformed DescriptorSet decoration on ID " << id
                      << ": has no operand";
      }
      decorations->emplace_back(
          create<ast::GroupDecoration>(Source{}, deco[1]));
    }
    if (deco[0] == SpvDecorationBinding) {
      if (deco.size() == 1) {
        return Fail() << "malformed Binding decoration on ID " << id
                      << ": has no operand";
      }
      decorations->emplace_back(
          create<ast::BindingDecoration>(Source{}, deco[1]));
    }
  }
  return success();
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

  auto source = GetSourceForInst(inst);
  auto* ast_type = original_ast_type->UnwrapAliasAndAccess();

  // TODO(dneto): Note: NullConstant for int, uint, float map to a regular 0.
  // So canonicalization should map that way too.
  // Currently "null<type>" is missing from the WGSL parser.
  // See https://bugs.chromium.org/p/tint/issues/detail?id=34
  if (ast_type->Is<U32>()) {
    return {ty_.U32(), create<ast::ScalarConstructorExpression>(
                           Source{}, create<ast::UintLiteral>(
                                         source, spirv_const->GetU32()))};
  }
  if (ast_type->Is<I32>()) {
    return {ty_.I32(), create<ast::ScalarConstructorExpression>(
                           Source{}, create<ast::SintLiteral>(
                                         source, spirv_const->GetS32()))};
  }
  if (ast_type->Is<F32>()) {
    return {ty_.F32(), create<ast::ScalarConstructorExpression>(
                           Source{}, create<ast::FloatLiteral>(
                                         source, spirv_const->GetFloat()))};
  }
  if (ast_type->Is<Bool>()) {
    const bool value = spirv_const->AsNullConstant()
                           ? false
                           : spirv_const->AsBoolConstant()->value();
    return {ty_.Bool(), create<ast::ScalarConstructorExpression>(
                            Source{}, create<ast::BoolLiteral>(source, value))};
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
      ast_components.emplace_back(ast_component.expr);
    }
    return {original_ast_type, create<ast::TypeConstructorExpression>(
                                   Source{}, original_ast_type->Build(builder_),
                                   std::move(ast_components))};
  }
  auto* spirv_null_const = spirv_const->AsNullConstant();
  if (spirv_null_const != nullptr) {
    return {original_ast_type, MakeNullValue(original_ast_type)};
  }
  Fail() << "Unhandled constant type " << inst->type_id() << " for value ID "
         << id;
  return {};
}

ast::Expression* ParserImpl::MakeNullValue(const Type* type) {
  // TODO(dneto): Use the no-operands constructor syntax when it becomes
  // available in Tint.
  // https://github.com/gpuweb/gpuweb/issues/685
  // https://bugs.chromium.org/p/tint/issues/detail?id=34

  if (!type) {
    Fail() << "trying to create null value for a null type";
    return nullptr;
  }

  auto* original_type = type;
  type = type->UnwrapAliasAndAccess();

  if (type->Is<Bool>()) {
    return create<ast::ScalarConstructorExpression>(
        Source{}, create<ast::BoolLiteral>(Source{}, false));
  }
  if (type->Is<U32>()) {
    return create<ast::ScalarConstructorExpression>(
        Source{}, create<ast::UintLiteral>(Source{}, 0u));
  }
  if (type->Is<I32>()) {
    return create<ast::ScalarConstructorExpression>(
        Source{}, create<ast::SintLiteral>(Source{}, 0));
  }
  if (type->Is<F32>()) {
    return create<ast::ScalarConstructorExpression>(
        Source{}, create<ast::FloatLiteral>(Source{}, 0.0f));
  }
  if (type->Is<Alias>()) {
    // TODO(amaiorano): No type constructor for TypeName (yet?)
    ast::ExpressionList ast_components;
    return create<ast::TypeConstructorExpression>(
        Source{}, original_type->Build(builder_), std::move(ast_components));
  }
  if (auto* vec_ty = type->As<Vector>()) {
    ast::ExpressionList ast_components;
    for (size_t i = 0; i < vec_ty->size; ++i) {
      ast_components.emplace_back(MakeNullValue(vec_ty->type));
    }
    return create<ast::TypeConstructorExpression>(
        Source{}, type->Build(builder_), std::move(ast_components));
  }
  if (auto* mat_ty = type->As<Matrix>()) {
    // Matrix components are columns
    auto* column_ty = ty_.Vector(mat_ty->type, mat_ty->rows);
    ast::ExpressionList ast_components;
    for (size_t i = 0; i < mat_ty->columns; ++i) {
      ast_components.emplace_back(MakeNullValue(column_ty));
    }
    return create<ast::TypeConstructorExpression>(
        Source{}, type->Build(builder_), std::move(ast_components));
  }
  if (auto* arr_ty = type->As<Array>()) {
    ast::ExpressionList ast_components;
    for (size_t i = 0; i < arr_ty->size; ++i) {
      ast_components.emplace_back(MakeNullValue(arr_ty->type));
    }
    return create<ast::TypeConstructorExpression>(
        Source{}, original_type->Build(builder_), std::move(ast_components));
  }
  if (auto* struct_ty = type->As<Struct>()) {
    ast::ExpressionList ast_components;
    for (auto* member : struct_ty->members) {
      ast_components.emplace_back(MakeNullValue(member));
    }
    return create<ast::TypeConstructorExpression>(
        Source{}, original_type->Build(builder_), std::move(ast_components));
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
    Fail() << "internal error: unmapped type for: " << builder_.str(expr.expr)
           << "\n";
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
    if (usage.IsSampledTexture() ||
        (image_type->format() == SpvImageFormatUnknown)) {
      // Make a sampled texture type.
      auto* ast_sampled_component_type =
          ConvertType(raw_handle_type->GetSingleWordInOperand(0));

      // Vulkan ignores the depth parameter on OpImage, so pay attention to the
      // usage as well.  That is, it's valid for a Vulkan shader to use an
      // OpImage variable with an OpImage*Dref* instruction.  In WGSL we must
      // treat that as a depth texture.
      if (image_type->depth() || usage.IsDepthTexture()) {
        ast_store_type = ty_.DepthTexture(dim);
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
      const auto access = usage.IsStorageReadTexture()
                              ? ast::AccessControl::kReadOnly
                              : ast::AccessControl::kWriteOnly;
      const auto format = enum_converter_.ToImageFormat(image_type->format());
      if (format == ast::ImageFormat::kNone) {
        return nullptr;
      }
      ast_store_type =
          ty_.AccessControl(ty_.StorageTexture(dim, format), access);
    }
  } else {
    Fail() << "unsupported: UniformConstant variable is not a recognized "
              "sampler or texture"
           << var.PrettyPrint();
    return nullptr;
  }

  // Form the pointer type.
  auto* result =
      ty_.Pointer(ast_store_type, ast::StorageClass::kUniformConstant);
  // Remember it for later.
  handle_type_[&var] = result;
  return result;
}

const Type* ParserImpl::GetComponentTypeForFormat(ast::ImageFormat format) {
  switch (format) {
    case ast::ImageFormat::kR8Uint:
    case ast::ImageFormat::kR16Uint:
    case ast::ImageFormat::kRg8Uint:
    case ast::ImageFormat::kR32Uint:
    case ast::ImageFormat::kRg16Uint:
    case ast::ImageFormat::kRgba8Uint:
    case ast::ImageFormat::kRg32Uint:
    case ast::ImageFormat::kRgba16Uint:
    case ast::ImageFormat::kRgba32Uint:
      return ty_.U32();

    case ast::ImageFormat::kR8Sint:
    case ast::ImageFormat::kR16Sint:
    case ast::ImageFormat::kRg8Sint:
    case ast::ImageFormat::kR32Sint:
    case ast::ImageFormat::kRg16Sint:
    case ast::ImageFormat::kRgba8Sint:
    case ast::ImageFormat::kRg32Sint:
    case ast::ImageFormat::kRgba16Sint:
    case ast::ImageFormat::kRgba32Sint:
      return ty_.I32();

    case ast::ImageFormat::kR8Unorm:
    case ast::ImageFormat::kRg8Unorm:
    case ast::ImageFormat::kRgba8Unorm:
    case ast::ImageFormat::kRgba8UnormSrgb:
    case ast::ImageFormat::kBgra8Unorm:
    case ast::ImageFormat::kBgra8UnormSrgb:
    case ast::ImageFormat::kRgb10A2Unorm:
    case ast::ImageFormat::kR8Snorm:
    case ast::ImageFormat::kRg8Snorm:
    case ast::ImageFormat::kRgba8Snorm:
    case ast::ImageFormat::kR16Float:
    case ast::ImageFormat::kR32Float:
    case ast::ImageFormat::kRg16Float:
    case ast::ImageFormat::kRg11B10Float:
    case ast::ImageFormat::kRg32Float:
    case ast::ImageFormat::kRgba16Float:
    case ast::ImageFormat::kRgba32Float:
      return ty_.F32();
    default:
      break;
  }
  Fail() << "unknown format " << int(format);
  return nullptr;
}

unsigned ParserImpl::GetChannelCountForFormat(ast::ImageFormat format) {
  switch (format) {
    case ast::ImageFormat::kR16Float:
    case ast::ImageFormat::kR16Sint:
    case ast::ImageFormat::kR16Uint:
    case ast::ImageFormat::kR32Float:
    case ast::ImageFormat::kR32Sint:
    case ast::ImageFormat::kR32Uint:
    case ast::ImageFormat::kR8Sint:
    case ast::ImageFormat::kR8Snorm:
    case ast::ImageFormat::kR8Uint:
    case ast::ImageFormat::kR8Unorm:
      // One channel
      return 1;

    case ast::ImageFormat::kRg11B10Float:
    case ast::ImageFormat::kRg16Float:
    case ast::ImageFormat::kRg16Sint:
    case ast::ImageFormat::kRg16Uint:
    case ast::ImageFormat::kRg32Float:
    case ast::ImageFormat::kRg32Sint:
    case ast::ImageFormat::kRg32Uint:
    case ast::ImageFormat::kRg8Sint:
    case ast::ImageFormat::kRg8Snorm:
    case ast::ImageFormat::kRg8Uint:
    case ast::ImageFormat::kRg8Unorm:
      // Two channels
      return 2;

    case ast::ImageFormat::kBgra8Unorm:
    case ast::ImageFormat::kBgra8UnormSrgb:
    case ast::ImageFormat::kRgb10A2Unorm:
    case ast::ImageFormat::kRgba16Float:
    case ast::ImageFormat::kRgba16Sint:
    case ast::ImageFormat::kRgba16Uint:
    case ast::ImageFormat::kRgba32Float:
    case ast::ImageFormat::kRgba32Sint:
    case ast::ImageFormat::kRgba32Uint:
    case ast::ImageFormat::kRgba8Sint:
    case ast::ImageFormat::kRgba8Snorm:
    case ast::ImageFormat::kRgba8Uint:
    case ast::ImageFormat::kRgba8Unorm:
    case ast::ImageFormat::kRgba8UnormSrgb:
      // Four channels
      return 4;

    default:
      break;
  }
  Fail() << "unknown format " << int(format);
  return 0;
}

const Type* ParserImpl::GetTexelTypeForFormat(ast::ImageFormat format) {
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

}  // namespace spirv
}  // namespace reader
}  // namespace tint
