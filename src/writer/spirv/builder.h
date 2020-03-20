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

#ifndef SRC_WRITER_SPIRV_BUILDER_H_
#define SRC_WRITER_SPIRV_BUILDER_H_

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/ast/module.h"
#include "src/writer/spirv/instruction.h"

namespace tint {
namespace writer {
namespace spirv {

/// Builder class to create SPIR-V instructions from a module.
class Builder {
 public:
  /// Constructor
  Builder();
  ~Builder();

  /// Generates the SPIR-V instructions for the given module
  /// @param module the module to generate from
  /// @returns true if the SPIR-V was successfully built
  bool Build(const ast::Module& module);

  /// @returns the number of uint32_t's needed to make up the results
  uint32_t total_size() const;

  /// @returns the id bound for this module
  uint32_t id_bound() const { return next_id_; }

  /// @returns the next id to be used
  uint32_t next_id() {
    auto id = next_id_;
    next_id_ += 1;
    return id;
  }

  /// Iterates over all the instructions in the correct order and calls the
  /// given callback
  /// @param cb the callback to execute
  void iterate(std::function<void(const Instruction&)> cb) const;

  /// Adds an instruction to the preamble
  /// @param op the op to set
  /// @param operands the operands for the instruction
  void push_preamble(spv::Op op, const std::vector<Operand>& operands) {
    preamble_.push_back(Instruction{op, operands});
  }
  /// @returns the preamble
  const std::vector<Instruction>& preamble() const { return preamble_; }
  /// Adds an instruction to the debug
  /// @param op the op to set
  /// @param operands the operands for the instruction
  void push_debug(spv::Op op, const std::vector<Operand>& operands) {
    debug_.push_back(Instruction{op, operands});
  }
  /// @returns the debug instructions
  const std::vector<Instruction>& debug() const { return debug_; }
  /// Adds an instruction to the types
  /// @param op the op to set
  /// @param operands the operands for the instruction
  void push_type(spv::Op op, const std::vector<Operand>& operands) {
    types_.push_back(Instruction{op, operands});
  }
  /// @returns the type instructions
  const std::vector<Instruction>& type() const { return types_; }
  /// Adds an instruction to the instruction list
  /// @param op the op to set
  /// @param operands the operands for the instruction
  void push_inst(spv::Op op, const std::vector<Operand>& operands) {
    instructions_.push_back(Instruction{op, operands});
  }
  /// @returns the instruction list
  const std::vector<Instruction>& inst() const { return instructions_; }
  /// Adds an instruction to the annotations
  /// @param op the op to set
  /// @param operands the operands for the instruction
  void push_annot(spv::Op op, const std::vector<Operand>& operands) {
    annotations_.push_back(Instruction{op, operands});
  }
  /// @returns the annotations
  const std::vector<Instruction>& annot() const { return annotations_; }

  /// Generates an import instruction
  /// @param imp the import
  void GenerateImport(ast::Import* imp);

 private:
  Operand result_op();

  uint32_t next_id_ = 1;
  std::vector<Instruction> preamble_;
  std::vector<Instruction> debug_;
  std::vector<Instruction> types_;
  std::vector<Instruction> instructions_;
  std::vector<Instruction> annotations_;

  std::unordered_map<std::string, uint32_t> import_name_to_id_;
};

}  // namespace spirv
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_SPIRV_BUILDER_H_
