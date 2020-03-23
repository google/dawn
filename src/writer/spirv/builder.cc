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

#include "src/writer/spirv/builder.h"

#include "spirv/unified1/spirv.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

uint32_t size_of(const std::vector<Instruction>& instructions) {
  uint32_t size = 0;
  for (const auto& inst : instructions)
    size += inst.word_length();

  return size;
}

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

}  // namespace

Builder::Builder() = default;

Builder::~Builder() = default;

bool Builder::Build(const ast::Module& m) {
  push_preamble(spv::Op::OpCapability, {Operand::Int(SpvCapabilityShader)});
  push_preamble(spv::Op::OpCapability,
                {Operand::Int(SpvCapabilityVulkanMemoryModel)});

  for (const auto& imp : m.imports()) {
    GenerateImport(imp.get());
  }

  push_preamble(spv::Op::OpMemoryModel,
                {Operand::Int(SpvAddressingModelLogical),
                 Operand::Int(SpvMemoryModelVulkanKHR)});

  for (const auto& ep : m.entry_points()) {
    if (!GenerateEntryPoint(ep.get())) {
      return false;
    }
  }

  return true;
}

Operand Builder::result_op() {
  return Operand::Int(next_id());
}

uint32_t Builder::total_size() const {
  // The 5 covers the magic, version, generator, id bound and reserved.
  uint32_t size = 5;

  size += size_of(preamble_);
  size += size_of(debug_);
  size += size_of(annotations_);
  size += size_of(types_);
  size += size_of(instructions_);

  return size;
}

void Builder::iterate(std::function<void(const Instruction&)> cb) const {
  for (const auto& inst : preamble_) {
    cb(inst);
  }
  for (const auto& inst : debug_) {
    cb(inst);
  }
  for (const auto& inst : annotations_) {
    cb(inst);
  }
  for (const auto& inst : types_) {
    cb(inst);
  }
  for (const auto& inst : instructions_) {
    cb(inst);
  }
}

bool Builder::GenerateEntryPoint(ast::EntryPoint* ep) {
  auto name = ep->name();
  if (name.empty()) {
    name = ep->function_name();
  }

  auto id = id_for_func_name(ep->function_name());
  if (id == 0) {
    error_ = "unable to find ID for function: " + ep->function_name();
    return false;
  }

  auto stage = pipeline_stage_to_execution_model(ep->stage());
  if (stage == SpvExecutionModelMax) {
    error_ = "Unknown pipeline stage provided";
    return false;
  }

  push_preamble(spv::Op::OpEntryPoint,
                {Operand::Int(stage), Operand::Int(id), Operand::String(name)});

  return true;
}

void Builder::GenerateImport(ast::Import* imp) {
  auto op = result_op();
  auto id = op.to_i();

  push_preamble(spv::Op::OpExtInstImport, {op, Operand::String(imp->path())});

  import_name_to_id_[imp->name()] = id;
}

}  // namespace spirv
}  // namespace writer
}  // namespace tint
