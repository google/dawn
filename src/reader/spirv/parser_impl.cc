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

#include <cstring>
#include <memory>
#include <string>
#include <utility>

#include "source/opt/build_module.h"
#include "source/opt/instruction.h"
#include "source/opt/module.h"
#include "source/opt/type_manager.h"
#include "spirv-tools/libspirv.hpp"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/type_manager.h"

namespace tint {
namespace reader {
namespace spirv {

namespace {

const spv_target_env kTargetEnv = SPV_ENV_WEBGPU_0;

}  // namespace

ParserImpl::ParserImpl(const Context& ctx,
                       const std::vector<uint32_t>& spv_binary)
    : Reader(ctx),
      spv_binary_(spv_binary),
      fail_stream_(&success_, &errors_),
      namer_(fail_stream_),
      enum_converter_(fail_stream_),
      tools_context_(kTargetEnv),
      tools_(kTargetEnv) {
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
        this->Fail() << "error: line " << position.index << ": " << message;
    }
  };
}

ParserImpl::~ParserImpl() = default;

bool ParserImpl::Parse() {
  if (ctx_.type_mgr == nullptr) {
    Fail() << "Missing type manager";
    return false;
  }

  if (!success_) {
    return false;
  }

  // Set up use of SPIRV-Tools utilities.
  spvtools::SpirvTools spv_tools(kTargetEnv);

  // Error messages from SPIRV-Tools are forwarded as failures.
  spv_tools.SetMessageConsumer(message_consumer_);

  // Only consider valid modules.
  if (success_) {
    success_ = spv_tools.Validate(spv_binary_);
  }

  if (success_) {
    success_ = BuildInternalModule();
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

  ast::type::Type* result = nullptr;

  switch (spirv_type->kind()) {
    case spvtools::opt::analysis::Type::kVoid:
      result = ctx_.type_mgr->Get(std::make_unique<ast::type::VoidType>());
      break;
    case spvtools::opt::analysis::Type::kBool:
      result = ctx_.type_mgr->Get(std::make_unique<ast::type::BoolType>());
      break;
    case spvtools::opt::analysis::Type::kInteger: {
      const auto* int_ty = spirv_type->AsInteger();
      if (int_ty->width() == 32) {
        if (int_ty->IsSigned()) {
          result = ctx_.type_mgr->Get(std::make_unique<ast::type::I32Type>());
        } else {
          result = ctx_.type_mgr->Get(std::make_unique<ast::type::U32Type>());
        }
      } else {
        Fail() << "unhandled integer width: " << int_ty->width();
      }
      break;
    }
    case spvtools::opt::analysis::Type::kFloat: {
      const auto* float_ty = spirv_type->AsFloat();
      if (float_ty->width() == 32) {
        result = ctx_.type_mgr->Get(std::make_unique<ast::type::F32Type>());
      } else {
        Fail() << "unhandled float width: " << float_ty->width();
      }
      break;
    }
    case spvtools::opt::analysis::Type::kVector: {
      const auto* vec_ty = spirv_type->AsVector();
      const auto num_elem = vec_ty->element_count();
      auto* ast_elem_ty = ConvertType(type_mgr_->GetId(vec_ty->element_type()));
      if (ast_elem_ty != nullptr) {
        result = ctx_.type_mgr->Get(
            std::make_unique<ast::type::VectorType>(ast_elem_ty, num_elem));
      }
      // In the error case, we'll already have emitted a diagnostic.
      break;
    }
    case spvtools::opt::analysis::Type::kMatrix: {
      const auto* mat_ty = spirv_type->AsMatrix();
      const auto* vec_ty = mat_ty->element_type()->AsVector();
      const auto* scalar_ty = vec_ty->element_type();
      const auto num_rows = vec_ty->element_count();
      const auto num_columns = mat_ty->element_count();
      auto* ast_scalar_ty = ConvertType(type_mgr_->GetId(scalar_ty));
      if (ast_scalar_ty != nullptr) {
        result = ctx_.type_mgr->Get(std::make_unique<ast::type::MatrixType>(
            ast_scalar_ty, num_rows, num_columns));
      }
      // In the error case, we'll already have emitted a diagnostic.
      break;
    }
    default:
      // The error diagnostic will be generated below because result is still
      // nullptr.
      break;
  }

  if (result == nullptr) {
    if (success_) {
      // Only emit a new diagnostic if we haven't already emitted a more
      // specific one.
      Fail() << "unknown SPIR-V type: " << type_id;
    }
  } else {
    id_to_type_[type_id] = result;
  }
  return result;
}

bool ParserImpl::BuildInternalModule() {
  tools_.SetMessageConsumer(message_consumer_);

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

  return true;
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
  };
  if (!RegisterExtendedInstructionImports()) {
    return false;
  }
  if (!RegisterUserNames()) {
    return false;
  }
  if (!EmitEntryPoints()) {
    return false;
  }
  // TODO(dneto): fill in the rest
  return true;
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
            std::make_unique<tint::ast::Import>(name, "std::glsl");
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

bool ParserImpl::RegisterUserNames() {
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
      case SpvOpName:
        namer_.SuggestSanitizedName(inst.GetSingleWordInOperand(0),
                                    inst.GetInOperand(1).AsString());
        break;
      case SpvOpMemberName:
        namer_.SuggestSanitizedMemberName(inst.GetSingleWordInOperand(0),
                                          inst.GetSingleWordInOperand(1),
                                          inst.GetInOperand(2).AsString());
        break;
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

}  // namespace spirv
}  // namespace reader
}  // namespace tint
