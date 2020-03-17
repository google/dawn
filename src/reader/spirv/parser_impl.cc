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

#include <cstring>

#include "source/opt/build_module.h"
#include "spirv-tools/libspirv.hpp"
#include "src/reader/spirv/parser_impl.h"

namespace tint {
namespace reader {
namespace spv {

namespace {

const spv_target_env kTargetEnv = SPV_ENV_WEBGPU_0;

}  // namespace

ParserImpl::ParserImpl(const std::vector<uint32_t>& spv_binary)
    : Reader(),
      spv_binary_(spv_binary),
      fail_stream_(&success_, &errors_),
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
  return ParseExtendedInstructionImports();
  // TODO(dneto): fill in the rest
}

bool ParserImpl::ParseExtendedInstructionImports() {
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

}  // namespace spv
}  // namespace reader
}  // namespace tint
